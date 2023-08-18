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

#ifndef _U_GNSS_POS_H_
#define _U_GNSS_POS_H_

/* Only header files representing a direct and unavoidable
 * dependency between the API of this module and the API
 * of another module should be included here; otherwise
 * please keep #includes to your .c files. */

/** \addtogroup _GNSS
 *  @{
 */

/** @file
 * @brief This header file defines the GNSS APIs to read position.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "system_config.h"
#include "system_definitions.h"
/* ----------------------------------------------------------------
 * COMPILE-TIME MACROS
 * -------------------------------------------------------------- */

#ifndef U_GNSS_POS_TIMEOUT_SECONDS
/** The timeout for position establishment in seconds.
 */
# define U_GNSS_POS_TIMEOUT_SECONDS 240
#endif

/** The default streamed position period in milliseconds.
 */
#define U_GNSS_POS_STREAMED_PERIOD_DEFAULT_MS 1000

/** The recommended minimum number of satellites required to
 * be visible and meet the criteria when calling uGnssPosGetRrlp()
 * for the Cloud Locate service.
 */
#define U_GNSS_RRLP_SVS_THRESHOLD_RECOMMENDED 5

/** The recommended threshold to use for carrier to noise
 * ratio when calling uGnssPosGetRrlp() for the Cloud Locate service.
 */
#define U_GNSS_RRLP_C_NO_THRESHOLD_RECOMMENDED 30

/** The recommended limit to use for multipath index when calling
 * uGnssPosGetRrlp() for the Cloud Locate service.
 */
#define U_GNSS_RRLP_MULTIPATH_INDEX_LIMIT_RECOMMENDED 1

/** The recommended limit to use for the pseudorange RMS error
 * index when calling uGnssPosGetRrlp() for the Cloud Locate service.
 */
#define U_GNSS_RRLP_PSEUDORANGE_RMS_ERROR_INDEX_LIMIT_RECOMMENDED 3

/* ----------------------------------------------------------------
 * TYPES
 * -------------------------------------------------------------- */
    typedef struct {
        int32_t LatitudeX1e7;
        int32_t LongitudeX1e7;
        int32_t AltitudeMillimetres;
        int32_t RadiusMillimetres;
        int32_t SpeedMillimetresPerSecond;
        int32_t Svs; 
        int64_t TimeUtc;
    }s_gnssData;
/* ----------------------------------------------------------------
 * FUNCTIONS:  WORKAROUND FOR LINKER ISSUE
 * -------------------------------------------------------------- */

int32_t gnss_posGet(s_gnssData *pGnssData, bool printIt);
/* ----------------------------------------------------------------
 * FUNCTIONS
 * -------------------------------------------------------------- */



#ifdef __cplusplus
}
#endif

/** @}*/

#endif // _U_GNSS_POS_H_

// End of file
