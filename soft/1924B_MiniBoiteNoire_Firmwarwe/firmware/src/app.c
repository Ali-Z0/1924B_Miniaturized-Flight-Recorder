/*******************************************************************************
  MPLAB Harmony Application Source File
  
  Company:
    Microchip Technology Inc.
  
  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It 
    implements the logic of the application's state machine and it may call 
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013-2014 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END


// *****************************************************************************
// *****************************************************************************
// Section: Included Files 
// *****************************************************************************
// *****************************************************************************

#include "app.h"
#include "bno055.h"
#include "bno055_support.h"
#include "GNSS/u_gnss_pos.h"
#include "Mc32_I2cUtilCCS.h"
#include "Mc32_serComm.h"
#include "Mc32_sdFatGest.h"
#include "Mc32Debounce.h"
#include "usart_FIFO.h"
#include "GNSS/u_ubx_protocol.h"
#include <stdio.h>

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************
/* Switch descriptor */
S_SwitchDescriptor switchDescr;
// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.
    
    Application strings and buffers are be defined outside this structure.
*/

APP_DATA appData;
TIMER_DATA timeData;


// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

void delayTimer_callback(){
    /* Increment delay timer */
    timeData.delayCnt ++;
}

void stateTimer_callback()
{
    /* Increment all counters */
    timeData.ledCnt ++;
    timeData.measCnt[BNO055_idx] ++;
    timeData.measCnt[GNSS_idx] ++;
    timeData.inactiveCnt ++;
    timeData.tmrTickFlag = true;
    /* When the button is pressed, the hold time is counted. */
    if(timeData.flagCntBtnPressed){
        timeData.cntBtnPressed++;
    }
     /* Do debounce on button every 10 ms */
     DoDebounce(&switchDescr, ButtonMFStateGet());
    /* Start a measure set each IMU period */        
    if ( ( timeData.measCnt[BNO055_idx] % (timeData.measPeriod[BNO055_idx]/10) ) == 0)
        timeData.measTodo[BNO055_idx] = true;
     
    /* Start a measure set each GNSS period */        
    if ( ( timeData.measCnt[GNSS_idx] % (timeData.measPeriod[GNSS_idx]/10) ) == 0)
        timeData.measTodo[GNSS_idx] = true;
    /* Manage LED if enabled */
    if((timeData.ledCnt % LED_PERIOD == 0) && (appData.ledState == true))
       LED_BOff();
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************
static void stopLogging (void);
static void btnTaskGest( void );
static void sys_shutdown( void );
static void startLogging (void);
// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Initialize ( void )
{
    /* Keep the device ON */
    PWR_HOLDOn();
    LED_GOn();
    
    // Start GNSS
    //char gnssMessage[4+U_UBX_PROTOCOL_OVERHEAD_LENGTH_BYTES];
    //char msgBody[4] = {0xFF, 0xFF, 0X09, 0x00};
    
    // GNSS initialsiation data
    /*char gnssMessage2[4+U_UBX_PROTOCOL_OVERHEAD_LENGTH_BYTES];
    char msgBody2[13] = {0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x01};
    char msgBody3[4] = {0xFF, 0xFF, 0X09, 0x00};*/
    
        
    // Initialization of the USART FIFOs
    initFifo(&usartFifoRx, FIFO_RX_SIZE, a_fifoRx, 0);
    initFifo(&usartFifoTx, FIFO_TX_SIZE, a_fifoTx, 0);
       
    /* Start timers*/
    DRV_TMR0_Start();
    /* Init i2c bus */
    i2c_init(1);
    
    /* Reset GNSS*/
    RESET_NOff();
    BNO055_delay_msek(100);
    /* Unreset GNSS */
    RESET_NOn();
    BNO055_delay_msek(300);
            
    // Start GNSS
    //uUbxProtocolEncode(0x06, 0x04, msgBody, 4, gnssMessage);
    //serTransmitbuffer(USART_ID_2, gnssMessage, sizeof(gnssMessage));

    /* Reset IMU */
    RST_IMUOff();
    BNO055_delay_msek(100);
    RST_IMUOn();
    BNO055_delay_msek(100);
    
    // Reset interrupt pin
    bno055_set_intr_rst(1);
        
    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_INIT;
    
}


/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks ( void )
{
    /* Local bno055 data */
    s_bno055_data bno055_local_data; 
    //s_gnssData gnss_ubx_local_data;
    minmea_messages gnss_nmea_local_data;
    //enum minmea_sentence_id gnss_nmea_msgId = MINMEA_UNKNOWN;
    /* CONFIGURATION */
    static char charRead[30] = {0};
    static uint32_t readCnt = 0;
    static unsigned long oldIntG = 0;
    static unsigned long oldIntI = 0;
    static uint32_t oldInaPer = 0;
    static bool oldLed = 0;
    static int ledStateTemp = 0;
    
    // Character to send trough USART
    static char charToSend = 0;

    /* Check the application's current state. */
    switch ( appData.state )
    {
        /* Application's initial state. */
        case APP_STATE_INIT:
        {
            // Init delay
            BNO055_delay_msek(500);
            // Init and Measure set
            bno055_init_readout();
            
            /* BNO055 motion interrupt mode */
            bno055_set_accel_any_motion_no_motion_axis_enable(BNO055_ACCEL_ANY_MOTION_NO_MOTION_X_AXIS, 1);
            bno055_set_accel_any_motion_no_motion_axis_enable(BNO055_ACCEL_ANY_MOTION_NO_MOTION_Y_AXIS, 1);
            bno055_set_accel_any_motion_no_motion_axis_enable(BNO055_ACCEL_ANY_MOTION_NO_MOTION_Z_AXIS, 1);
    
            bno055_set_accel_any_motion_thres(10);
            bno055_set_accel_any_motion_durn(20);
            bno055_set_intr_accel_any_motion(1);
            bno055_set_intr_mask_accel_any_motion(1);
                    
            /* go to service task */
            appData.state = APP_STATE_CONFIG;
            /* Init ltime_BNO055 counter */
            timeData.ltime[BNO055_idx] = 0;
            break;
        }
        case APP_STATE_CONFIG:
        {
            // Reset interrupt pin
            bno055_set_intr_rst(1);
            /* Init sd card parameters and read/create config File */
            sd_fat_cfg_init(&timeData.measPeriod[GNSS_idx], &timeData.measPeriod[BNO055_idx], &appData.ledState, &timeData.inactivePeriod);
            
            LED_GOff();
            
            /* ---  Unmount timeout --- */
            if (ButtonMFStateGet())
                appData.state = APP_STATE_SHUTDOWN;
            break;
        }
        case APP_STATE_LOGGING:
        {    
            // BNO055 Measure routine
            if((timeData.measTodo[BNO055_idx] == true )&&(sd_logGetState() == APP_IDLE))
            {
                // If LED enabled
                if(appData.ledState == true){
                    timeData.ledCnt = 0;
                    LED_BOn();
                }
                /* BNO055 Read all important info routine */
                bno055_local_data.comres = bno055_read_routine(&bno055_local_data);
                /* Delta time */
                bno055_local_data.d_time = timeData.measCnt[BNO055_idx] - timeData.ltime[BNO055_idx];
                /* Flag measure if acceleration detected */
                if((bno055_local_data.linear_accel.x >= 2*G) || (bno055_local_data.linear_accel.y >= 2*G) || (bno055_local_data.linear_accel.z >= 2*G))
                    bno055_local_data.flagImportantMeas = 1;
                else
                    bno055_local_data.flagImportantMeas = 0;
                
                /* Detect activity */
                if((bno055_local_data.linear_accel.x >= ACCEL_ACTIV_DETECT_msq) 
                   || (bno055_local_data.linear_accel.y >= ACCEL_ACTIV_DETECT_msq) 
                   || (bno055_local_data.linear_accel.z >= ACCEL_ACTIV_DETECT_msq))
                    timeData.inactiveCnt = 0;
                
                /* Write value to sdCard */
                sd_IMU_scheduleWrite(&bno055_local_data);
                /* Reset measure flag */
                timeData.measTodo[BNO055_idx] = false;
                /* Update last time counter */
                timeData.ltime[BNO055_idx] = timeData.measCnt[BNO055_idx];
            }
            // GNSS Measure routine
            else if((timeData.measTodo[GNSS_idx] == true )&&(sd_logGetState() == APP_IDLE))
            {
                /* Read GNSS position measure */
                //gnss_posGet_nmea(&gnss_nmea_local_data, &gnss_nmea_msgId);
                /* Write value to sdCard */
                sd_GNSS_scheduleWrite(&gnss_nmea_local_data);
                /* Reset measure flag */
                timeData.measTodo[GNSS_idx] = false;
                            }
            else
            {
                /* No comm, so no error */
                bno055_local_data.comres = 0;
                //LED_BOff();
            }
            
            /* If error detected : error LED */
            if((bno055_local_data.comres != 0)||(sd_logGetState() == APP_MOUNT_DISK))
                LED_ROn();
            else
                LED_ROff();
            
            /* --- SD FAT routine --- */
            sd_fat_logging_task();
            /* --- Button routine --- */
            btnTaskGest();
            /* --- Inactivity shutdown --- */
            if (timeData.inactiveCnt >= (timeData.inactivePeriod*100))
                appData.state = APP_STATE_SHUTDOWN;
                
            
            /* --- LIVE GNSS COMMAND --- */
            if(pollSerialCmds(USART_ID_1, "glive", "GLIVE", "-lvg", "-LVG")){
                /* Stop SD card logging */
                stopLogging();       
                /* USB communication states */
                appData.state = APP_STATE_COMM_LIVE_GNSS;
                LED_BOn();
            }
            /* --- LIVE IMU COMMAND --- */
            if(pollSerialCmds(USART_ID_1, "ilive", "ILIVE", "-lvi", "-LVI")){
                /* Stop SD card logging */
                stopLogging();       
                /* USB communication states */
                appData.state = APP_STATE_COMM_LIVE_IMU;
                LED_GOn();
                /* Deactivate USART2 (not used) */
                PLIB_USART_Disable(USART_ID_2);
                /* Reset measure flags and stop timer */
                DRV_TMR1_Start();
            }
            
            /* --- SHUTDOWN SYSTEM COMMAND  --- */
            if(pollSerialCmds(USART_ID_1, "shutdown", "SHUTDOWN", "-off", "-OFF")){       
                /* Turn off state */
                appData.state = APP_STATE_SHUTDOWN;
            }
            
            /* --- CONFIG BLACKBOX --- */
            if(pollSerialCmds(USART_ID_1, "config", "CONFIG", "-cfg", "-CFG")){       
                // Stop SD card logging 
                stopLogging(); 
                /* Deactivate USART2 (not used) */
                PLIB_USART_Disable(USART_ID_2);
                serTransmitString(USART_ID_1, "CONFIGURATION MODE \r\n");
                // Set config state to idle
                sd_cfgSetState(APP_CFG_IDLE);
                // Update configuration variables
                oldIntG = timeData.measPeriod[GNSS_idx];
                oldIntI = timeData.measPeriod[BNO055_idx];
                oldLed = appData.ledState;
                ledStateTemp = appData.ledState;
                // Turn off state 
                appData.state = APP_STATE_CONFIGURATE_BBX;
                LED_GOn();
            }
            
            /* --- GET GNSS LOGS --- */
            if(pollSerialCmds(USART_ID_1, "glog", "GLOG", "-gl", "-GL")){       
                // Display GNSS logs
                sd_fat_readDisplayFile("LOG_GNSS.txt");
            }
            
            /* --- GEST IMU LOGS --- */
            if(pollSerialCmds(USART_ID_1, "ilog", "ILOG", "-il", "-IL")){       
                // Display IMU logs
                sd_fat_readDisplayFile("LOG_IMU.csv");
            }
            
            /* --- DELETE COMMAND --- */
            if(pollSerialCmds(USART_ID_1, "gclr", "GCLR", "-gc", "-GC")){       
                // Delete file
                SYS_FS_FileDirectoryRemove("LOG_GNSS.txt");
                serTransmitString(USART_ID_1, "GNSS LOG DELETED \r\n");
            }
            
            /* --- DELETE COMMAND --- */
            if(pollSerialCmds(USART_ID_1, "iclr", "ICLR", "-ic", "-IC")){       
                // Delete file
                SYS_FS_FileDirectoryRemove("LOG_IMU.csv");
                serTransmitString(USART_ID_1, "IMU LOG DELETED \r\n");
            } 
            
           break;
        }
        case APP_STATE_COMM_LIVE_GNSS:
            /* No inactivity during this mode */
            timeData.inactiveCnt = 0;
            // Display GNSS live data trough USART 1
            if(getReadSize(&usartFifoRx) > 0){
                getCharFromFifo(&usartFifoRx, &charToSend);
                PLIB_USART_TransmitterByteSend(USART_ID_1, charToSend);
            }
            // If exit command detected, return to logging
            if(pollSerialCmds(USART_ID_1, "exit", "EXIT", "x" ,"X"))
                startLogging();
            break;
        case APP_STATE_COMM_LIVE_IMU:
            /* No inactivity during this mode */
            timeData.inactiveCnt = 0;
            // BNO055 Measure routine
            if(timeData.measTodo[BNO055_idx] == true )
            {
                // If LED enabled
                if(appData.ledState > 0){
                    timeData.ledCnt = 0;
                    LED_BOn();
                }
                /* BNO055 Read all important info routine */
                bno055_local_data.comres = bno055_read_routine(&bno055_local_data);
                /* Delta time */
                bno055_local_data.d_time = timeData.measCnt[BNO055_idx] - timeData.ltime[BNO055_idx];
                
                /* Display readed values */
                serDisplayValues(&bno055_local_data);
                
                /* Reset measure flag */
                timeData.measTodo[BNO055_idx] = false;
                /* Update last time counter */
                timeData.ltime[BNO055_idx] = timeData.measCnt[BNO055_idx];
            }
            // If exit command detected, return to logging
            if(pollSerialCmds(USART_ID_1, "exit", "EXIT", "x" ,"X")){
                startLogging();
                /* Reactivate USART2 (used) */
                PLIB_USART_Enable(USART_ID_2);
            }
                
            break;
            
        case APP_STATE_CONFIGURATE_BBX:
            /* No inactivity during this mode */
            timeData.inactiveCnt = 0;
            // Get command's characters
            while(!(DRV_USART0_ReceiverBufferIsEmpty())&&(readCnt < 30)){
                charRead[readCnt] = PLIB_USART_ReceiverByteReceive(USART_ID_1);
                readCnt++;
            }
            // Command 
            if(readCnt >= 30)
            {
                /* Reset read counter */
                readCnt = 0;
                /* Clear read buffer */
                memset(charRead,0,strlen(charRead));
            }
            
            // Detect ENTER (End of command)
            if(strstr(charRead, "\r\0") != NULL){
                // Scan command data
                sscanf(charRead, "\rINTG:%lu", &timeData.measPeriod[GNSS_idx]);
                sscanf(charRead, "\rINTI:%lu", &timeData.measPeriod[BNO055_idx]);
                sscanf(charRead, "\rLEDV:%d", &ledStateTemp);
                sscanf(charRead, "\rTOFF:%d", &timeData.inactivePeriod);
                // Cast int into boolean
                if (ledStateTemp > 0)
                    appData.ledState = true;
                else
                    appData.ledState = false;
                
                /* Reset read counter */
                readCnt = 0;
                /* Clear read buffer */
                memset(charRead,0,strlen(charRead));
            }
            // If config value changed
            if((timeData.measPeriod[GNSS_idx] != oldIntG) || (timeData.measPeriod[BNO055_idx] != oldIntI) || (appData.ledState != oldLed)
                || (timeData.inactivePeriod != oldInaPer) ){
                                
                serTransmitString(USART_ID_1, "COMMAND : VALUE CHANGED \r\n");
                // If data is not valid, keep the previous one
                if(timeData.measPeriod[GNSS_idx] <= 0){
                    timeData.measPeriod[GNSS_idx] = oldIntG;
                    serTransmitString(USART_ID_1, "ERROR GNSS VALUE <= 0 \r\n");
                }
                // If data is not valid, keep the previous one
                if(timeData.measPeriod[BNO055_idx] <= 0){
                    timeData.measPeriod[BNO055_idx] = oldIntI;
                    serTransmitString(USART_ID_1, "ERROR IMU VALUE <= 0 \r\n");
                }
                // If data is not valid, keep the previous one
                if(timeData.inactivePeriod <= 10){
                    timeData.inactivePeriod = oldInaPer;
                    serTransmitString(USART_ID_1, "ERROR INACTIVE PERIOD VALUE <= 10 \r\n");
                }
                /* Clear read buffer */
                memset(charRead,0,strlen(charRead));
                // Write new config file
                sd_CFG_Write (timeData.measPeriod[GNSS_idx], timeData.measPeriod[BNO055_idx], appData.ledState, timeData.inactivePeriod, true);
            }
            // Update polling config parameter
            oldIntG = timeData.measPeriod[GNSS_idx];
            oldIntI = timeData.measPeriod[BNO055_idx];
            oldLed = appData.ledState;
            oldInaPer = timeData.inactivePeriod;
                
            // Check occurence with commands
            if((strstr(charRead, "exit") != NULL)||(strstr(charRead, "EXIT") != NULL)
                || (strstr(charRead, "x") != NULL) || (strstr(charRead, "X") != NULL)) { 
                /* Command detected */
                startLogging();
                /* Reactivate USART2 (used) */
                PLIB_USART_Enable(USART_ID_2);
                /* Reset read counter */
                readCnt = 0;
                /* Clear read buffer */
                memset(charRead,0,strlen(charRead));
            }
            // Manipulate config file
            sd_fat_config_task ( false );
            
            break;
            
        case APP_STATE_SHUTDOWN:
        {
            /* Save and shutdown system */
            sys_shutdown();
            break;
        }

        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}

void appStateSet( APP_STATES newState ){
     appData.state = newState;
}

static void btnTaskGest( void ){
    static bool Hold = false;
    /* Button management : if rising edge detected */
    if(((ButtonMFStateGet()))||(Hold == true))
    {
        /* Hold until falling edge */
        Hold = true;
        /* Start counting pressed time */
        timeData.flagCntBtnPressed = true;
        /* If falling edge detected */
        if (ButtonMFStateGet() == 0)
        {
            /* Reset flag and switchdescr */
            timeData.flagCntBtnPressed = false;
            DebounceClearReleased(&switchDescr);
            /* If pressed more time than power off */
            if(timeData.cntBtnPressed >= BTN_HOLD_SHUTDOWN_x10ms){
                /* Power off the system */
                appData.state = APP_STATE_SHUTDOWN;
            }
            timeData.cntBtnPressed = 0;
            Hold = false;
        }
    }
}

static void sys_shutdown( void ) {
    /* Display shutting off mode */
    LED_BOff();
    LED_GOff();
    LED_ROn();

    /* If and SD card is mounted */
    if(sd_logGetState() != APP_MOUNT_DISK){
        /* Wait until SD availaible */
        while(sd_logGetState() != APP_IDLE){
            /* SD FAT routine */
            sd_fat_logging_task();
        }
        /* Unmount disk */
        sd_logSetState(APP_UNMOUNT_DISK);
        /* Wait until unmounted*/
        while(sd_logGetState() != APP_IDLE){
            sd_fat_logging_task();
        }
    }
    /* Set acceleration only operation to save power */
    bno055_set_operation_mode(BNO055_OPERATION_MODE_ACCONLY);
    /* set the power mode as LOW POWER*/
    bno055_set_power_mode(BNO055_POWER_MODE_LOWPOWER);
    // Reset interrupt pin
    bno055_set_intr_rst(1);
    /* turn off the device */
    PWR_HOLDOff();
}

static void stopLogging (void)
{
    /* Reset measure flags and stop timer */
    DRV_TMR1_Stop();
    timeData.measTodo[GNSS_idx] = false;
    timeData.measTodo[BNO055_idx] = false;

    /* Finish config */
    while(sd_cfgGetState() != APP_CFG_IDLE){
        sd_fat_cfg_init(&timeData.measPeriod[GNSS_idx], &timeData.measPeriod[BNO055_idx], &appData.ledState, &timeData.inactivePeriod);
    }

    /* Finish logging */
    while(sd_logGetState() != APP_IDLE){
        sd_fat_logging_task();
    }

    /* Reset Leds states */
    LED_ROff();
    LED_ROff();
    LED_GOff();
}

static void startLogging (void)
{
    // Logging state
    appData.state = APP_STATE_LOGGING;
    // Restart timer 1
    DRV_TMR1_Start();
    /* Reset Leds states */
    LED_ROff();
    LED_ROff();
    LED_GOff();
}

/*******************************************************************************
 End of File
 */
