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
    /* Increment utility timers */
    timeData.ledCnt ++;
    timeData.measCnt[BNO055_idx] ++;
    timeData.measCnt[GNSS_idx] ++;
    timeData.tmrTickFlag = true;
    /* If button is pressed, count pressed time */
    if(timeData.flagCntBtnPressed){
        timeData.cntBtnPressed++;
    }
     /* Do debounce every 10 ms */
     DoDebounce(&switchDescr, ButtonMFStateGet());
    /* Start a measure set each 100ms */        
    if ( ( timeData.measCnt[BNO055_idx] % (timeData.measPeriod[BNO055_idx]/10) ) == 0)
        timeData.measTodo[BNO055_idx] = true;
     
    /* Start a measure set each 100ms */        
    if ( ( timeData.measCnt[GNSS_idx] % (timeData.measPeriod[GNSS_idx]/10) ) == 0)
        timeData.measTodo[GNSS_idx] = true;
     
     if((timeData.ledCnt >= 50) && (appData.ledState > 0))
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

    /* Reset IMU */
    RST_IMUOff();
    BNO055_delay_msek(100);
    RST_IMUOn();
    BNO055_delay_msek(100);
        
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
    char * ptTrame = 0;
            
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
            /* go to service task */
            appData.state = APP_STATE_CONFIG;
            /* Init ltime_BNO055 counter */
            timeData.ltime[BNO055_idx] = 0;
            break;
        }
        case APP_STATE_CONFIG:
        {
            /* Init sd card parameters and read/create config File */
            sd_fat_cfg_init(&timeData.measPeriod[GNSS_idx], &timeData.measPeriod[BNO055_idx], &appData.ledState);
            
            LED_GOff();
            break;
        }
        case APP_STATE_LOGGING:
        {    
            // BNO055 Measure routine
            if((timeData.measTodo[BNO055_idx] == true )&&(sd_logGetState() == APP_IDLE))
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
                /* Pressure measure */
                bno055_local_data.pressure = 0;
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
                LED_BOff();
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
                serTransmitString("MODE CONFIGURATION \r\n");
                // Set config state to idle
                sd_cfgSetState(APP_CFG_IDLE);
                // Turn off state 
                appData.state = APP_STATE_CONFIGURATE_BBX;
                LED_GOn();
            }
            
            /* --- GET GNSS LOGS --- */
            /*if(pollSerialCmds(USART_ID_1, "glog", "GLOG", "-gl", "-GL")){       
                // Stop SD card logging 
                stopLogging();
                // Turn off state 
                appData.state = APP_STATE_SHUTDOWN;
            }*/
            
            /* --- GEST IMU LOGS --- */
            /*if(pollSerialCmds(USART_ID_1, "glog", "GLOG", "-gl", "-GL")){       
                // Stop SD card logging 
                stopLogging();
                // Turn off state 
                appData.state = APP_STATE_SHUTDOWN;
            }*/
           break;
        }
        case APP_STATE_COMM_LIVE_GNSS:  
            
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
                /* Pressure measure */
                bno055_local_data.pressure = 0;
                
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
            
            // Get command's characters
            while((PLIB_USART_ReceiverDataIsAvailable(USART_ID_1))&&(readCnt < 30)){
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
            scanf("",);
//            if(strstr(charRead, "INTG:") != NULL)
//            {
//                ptTrame = strstr(charRead, "INTG:");
//                // Copy the data between the head and the tail in a sub-pointer
//                strncpy(ptTrame, (ptTrame+5), 5);
//                if(atoi(ptTrame) > 0){
//                    timeData.measPeriod[GNSS_idx] = atoi(ptTrame);
//                    sd_CFG_Write (timeData.measPeriod[GNSS_idx], timeData.measPeriod[BNO055_idx], appData.ledState, true);
//                }
//                /* Reset read counter */
//                readCnt = 0;
//                /* Clear read buffer */
//                memset(charRead,0,strlen(charRead));
//             }
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
            //sd_fat_config_task ( false );
            
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
            if(timeData.cntBtnPressed >= TIME_POWER_OFF){
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
        sd_fat_cfg_init(&timeData.measPeriod[GNSS_idx], &timeData.measPeriod[BNO055_idx], &appData.ledState);
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
