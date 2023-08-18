/*
 * Copyright 2019-2023 u-blox
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* Only #includes of u_* and the C standard library are allowed here,
 * no platform stuff and no OS stuff.  Anything required from
 * the platform/OS must be brought in through u_port* to maintain
 * portability.
 */

/** @file
 * @brief Implementation of the GNSS APIs to read position.
 */

#ifdef U_CFG_OVERRIDE
# include "u_cfg_override.h" // For a customer's configuration override
#endif

#include "app.h"
#include "limits.h"    // INT_MIN
#include "stddef.h"    // NULL, size_t etc.
#include "stdint.h"    // int32_t etc.
#include "stdbool.h"
#include "string.h"    // memcpy()
#include "u_error_common.h"
#include "u_time.h"
#include "u_ubx_protocol.h"
#include "u_gnss_pos.h"

/* ----------------------------------------------------------------
 * COMPILE-TIME MACROS
 * -------------------------------------------------------------- */

#ifndef U_GNSS_POS_CALLBACK_TASK_STACK_SIZE_BYTES
/** The stack size for the position establishment task.  The limiting
 * factor is ESP-IDF, and in particular on Arduino, which seems to
 * require the most stack, and if power saving may be on then
 * additional stack will be used by the AT client.
 */
# define U_GNSS_POS_CALLBACK_TASK_STACK_SIZE_BYTES (1024 * 5)
#endif

#ifndef U_GNSS_POS_CALLBACK_TASK_PRIORITY
/** The task priority for the position establishment task.
 */
# define U_GNSS_POS_CALLBACK_TASK_PRIORITY (U_CFG_OS_PRIORITY_MIN + 2)
#endif

#ifndef U_GNSS_POS_CALLBACK_TASK_STACK_DELAY_SECONDS
/** The delay between position attempts in the asynchronous task.
 */
# define U_GNSS_POS_CALLBACK_TASK_STACK_DELAY_SECONDS 5
#endif

#ifndef U_GNSS_POS_RRLP_HEADER_SIZE_BYTES
/** The number of bytes of UBX protocol header that
 * will be added to the front of the raw RRLP binary data.
 */
#define U_GNSS_POS_RRLP_HEADER_SIZE_BYTES (U_UBX_PROTOCOL_OVERHEAD_LENGTH_BYTES - 2)
#endif

/* ----------------------------------------------------------------
 * TYPES
 * -------------------------------------------------------------- */

/** Parameters to pass to the asynchronous position establishment task.
 */
typedef struct {
    void (*pCallback) (int32_t errorCode,
                       int32_t latitudeX1e7,
                       int32_t longitudeX1e7,
                       int32_t altitudeMillimetres,
                       int32_t radiusMillimetres,
                       int32_t speedMillimetresPerSecond,
                       int32_t svs,
                       int64_t timeUtc);
} uGnssPosGetTaskParameters_t;

/* ----------------------------------------------------------------
 * STATIC VARIABLES
 * -------------------------------------------------------------- */

/** Table to convert U_GNSS_RRLP_MODE_MEASxxx into a message class
 * of UBX-RXM-MEASxxx.
 */
int32_t gRrlpModeToUbxRxmMessageClass[] = {
    0x14, // UBX-RXM-MEASX
    0x86, // UBX_RXM_MEAS50
    0x84, // UBX_RXM_MEAS20
    0x82, // UBX_RXM_MEASC12
    0x80  // UBX_RXM_MEASD12
};

/* ----------------------------------------------------------------
 * STATIC FUNCTIONS
 * -------------------------------------------------------------- */

// Decode the contents of a UBX-NAV-PVT message.  pMessage must
// be a pointer to the 92 byte body of a UBX-NAV-PVT message.
static int32_t posDecode(char *pMessage, s_gnssData *pGnssData, bool printIt)
{
    int32_t errorCode = (int32_t) U_ERROR_COMMON_TIMEOUT;
    int32_t months;
    int32_t year;
    int32_t y;
    int64_t t = -1;

    if ((*(pMessage + 11) & 0x03) == 0x03) {
        // Time and date are valid; we don't indicate
        // success based on this but we report it anyway
        // if it is valid
        t = 0;
        // Year is 1999-2099, so need to adjust to get year since 1970
        year = ((int32_t) uUbxProtocolUint16Decode(pMessage + 4) - 1999) + 29;
        // Month (1 to 12), so take away 1 to make it zero-based
        months = *(pMessage + 6) - 1;
        months += year * 12;
        // Work out the number of seconds due to the year/month count
        t += uTimeMonthsToSecondsUtc(months);
        // Day (1 to 31)
        t += ((int32_t) * (pMessage + 7) - 1) * 3600 * 24;
        // Hour (0 to 23)
        t += ((int32_t) * (pMessage + 8)) * 3600;
        // Minute (0 to 59)
        t += ((int32_t) * (pMessage + 9)) * 60;
        // Second (0 to 60)
        t += *(pMessage + 10);
        if (printIt) {
            //uPortLog("U_GNSS_POS: UTC time = %d.\n", (int32_t) t);
        }
    }
    if (pGnssData != NULL) {
        pGnssData->TimeUtc = t;
    }
    // From here onwards Lint complains about accesses
    // into message[] and it doesn't seem to be possible
    // to suppress those warnings with -esym(690, message)
    // or even -e(690), hence do it the blunt way
    //lint -save -e690
    if ((t >= 0) && (*(pMessage + 21) & 0x01)) {
        if (printIt) {
            //uPortLog("U_GNSS_POS: %dD fix achieved.\n", *(pMessage + 20));
        }
        y = (int32_t) * (pMessage + 23);
        if (printIt) {
            //uPortLog("U_GNSS_POS: satellite(s) = %d.\n", y);
        }
        if (pGnssData->Svs != NULL) {
            pGnssData->Svs = y;
        }
        y = (int32_t) uUbxProtocolUint32Decode(pMessage + 24);
        if (printIt) {
            //uPortLog("U_GNSS_POS: longitude = %d (degrees * 10^7).\n", y);
        }
        if (pGnssData->LongitudeX1e7 != NULL) {
            pGnssData->LongitudeX1e7 = y;
        }
        y = (int32_t) uUbxProtocolUint32Decode(pMessage + 28);
        if (printIt) {
            //uPortLog("U_GNSS_POS: latitude = %d (degrees * 10^7).\n", y);
        }
        if (pGnssData->LatitudeX1e7 != NULL) {
            pGnssData->LatitudeX1e7 = y;
        }
        y = INT_MIN;
        if (*(pMessage + 20) == 0x03) {
            y = (int32_t) uUbxProtocolUint32Decode(pMessage + 36);
            if (printIt) {
                //uPortLog("U_GNSS_POS: altitude = %d (mm).\n", y);
            }
        }
        if (pGnssData->AltitudeMillimetres != NULL) {
            pGnssData->AltitudeMillimetres = y;
        }
        y = (int32_t) uUbxProtocolUint32Decode(pMessage + 40);
        if (printIt) {
            //uPortLog("U_GNSS_POS: radius = %d (mm).\n", y);
        }
        if (pGnssData->RadiusMillimetres != NULL) {
            pGnssData->RadiusMillimetres = y;
        }
        y = (int32_t) uUbxProtocolUint32Decode(pMessage + 60);
        if (printIt) {
            //uPortLog("U_GNSS_POS: speed = %d (mm/s).\n", y);
        }
        if (pGnssData->SpeedMillimetresPerSecond != NULL) {
            pGnssData->SpeedMillimetresPerSecond = y;
        }
        errorCode = (int32_t) U_ERROR_COMMON_SUCCESS;
        //lint -restore
    }

    return errorCode;
}

// Establish position.
int32_t gnss_posGet(s_gnssData *pGnssData, bool printIt)
{
    int32_t errorCode = (int32_t) U_ERROR_COMMON_TIMEOUT;
    // Enough room for the body of the UBX-NAV-PVT message
    char message[92] = {0};
    uint32_t i = 0;
    
    // Read to empty the receive buffer
    while(!(DRV_USART_ReceiverBufferIsEmpty(USART_ID_1)))    {
        // 
        message[i] = DRV_USART_ReadByte(USART_ID_1);
        i++;
    }
    // UBX-CFG-PRT Then wait for UBX-ACK-ACK
    //errorCode = uUbxProtocolEncode(0x07,0x01 , message, sizeof(message), );
    /*errorCode = uGnssPrivateSendReceiveUbxMessage(pInstance,
                                                  0x01, 0x07, NULL, 0,
                                                  message, sizeof(message));*/
    if (errorCode == sizeof(message)) {
        // Got the correct message body length, process it
        errorCode = posDecode(message, pGnssData, printIt);
    } else {
        if (errorCode >= 0) {
            errorCode = (int32_t) U_ERROR_COMMON_DEVICE_ERROR;
        }
    }

    return errorCode;
}

/* ----------------------------------------------------------------
 * FUNCTIONS
 * -------------------------------------------------------------- */

void gnss_receiveTask (void)
{
    
        
    
    
}

// End of file
