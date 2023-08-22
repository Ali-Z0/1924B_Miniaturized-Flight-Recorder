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
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************
void btnTaskGest( void );
void sys_shutdown( void );
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
       
    /* Init i2c bus */
    i2c_init(1);
    
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
    enum minmea_sentence_id gnss_nmea_msgId = MINMEA_UNKNOWN;

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
            appData.state = APP_STATE_LOGGING;
            /* Init ltime_BNO055 counter */
            timeData.ltime[BNO055_idx] = 0;
            // Initialization of the USART FIFOs
            initFifo(&usartFifoRx, FIFO_RX_SIZE, a_fifoRx, 0);
            initFifo(&usartFifoTx, FIFO_TX_SIZE, a_fifoTx, 0);
            
            /* Init sd card parameters and read/create config File */
            sd_fat_cfg_init(&timeData.measPeriod[BNO055_idx], &timeData.measPeriod[BNO055_idx], &appData.ledState);
            
            /* Start timers */
            DRV_TMR0_Start();
            DRV_TMR1_Start();
            
            
            break;
        }
        case APP_STATE_LOGGING:
        {    
            // BNO055 Measure routine
            if((timeData.measTodo[BNO055_idx] == true )&&(sd_logGetState() == APP_IDLE))
            {
                /* BNO055 Read all important info routine */
                bno055_local_data.comres = bno055_read_routine(&bno055_local_data);
                /* Delta time */
                bno055_local_data.d_time = timeData.measCnt[BNO055_idx] - timeData.ltime[BNO055_idx];
                /* Pressure measure */
                bno055_local_data.pressure = 0;
                /* Write value to sdCard */
                sd_BNO_scheduleWrite_BNO055(&bno055_local_data);
                /* Reset measure flag */
                timeData.measTodo[BNO055_idx] = false;
                /* Update last time counter */
                timeData.ltime[BNO055_idx] = timeData.measCnt[BNO055_idx];
            }
            // GNSS Measure routine
            if((timeData.measTodo[GNSS_idx] == true )&&(sd_logGetState() == APP_IDLE))
            {
                /* Read GNSS position measure */
                gnss_posGet_nmea(&gnss_nmea_local_data, &gnss_nmea_msgId);
                /* Write value to sdCard */
                //sd_BNO_scheduleWrite_GNSS(&bno055_local_data);
                /* Reset measure flag */
                timeData.measTodo[GNSS_idx] = false;
            }
            else
            {
                /* No comm, so no error */
                bno055_local_data.comres = 0;
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
            
           break;
        }
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

void btnTaskGest( void ){
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

void sys_shutdown( void ) {
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

/*******************************************************************************
 End of File
 */
