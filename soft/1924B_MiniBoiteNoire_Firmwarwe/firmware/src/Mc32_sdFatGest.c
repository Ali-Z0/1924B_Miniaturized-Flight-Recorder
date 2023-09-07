/* ************************************************************************** */
/** Descriptive File Name

  @Company
    ETML-ES

  @File Name
    sd_fat_gest.c

  @Summary
    SD card fat system management

  @Description
    SD card fat system management
 */
/* ************************************************************************** */

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

/* This section lists the other files that are included in this file.
 */

#include "Mc32_sdFatGest.h"
#include <stdio.h>
#include "app.h"
#include "bno055_support.h"
#include "GNSS/u_gnss_pos.h"
#include <stdio.h>
#include "usart_FIFO.h"
#include "MC32_serComm.h"

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */

APP_FAT_DATA COHERENT_ALIGNED appFatData;
/* ************************************************************************** */
/** Descriptive Data Item Name

  @Summary
    Brief one-line summary of the data item.
    
  @Description
    Full description, explaining the purpose and usage of data item.
    <p>
    Additional description in consecutive paragraphs separated by HTML 
    paragraph breaks, as necessary.
    <p>
    Type "JavaDoc" in the "How Do I?" IDE toolbar for more information on tags.
    
  @Remarks
    Any additional remarks
 */


/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */
// Function prototype
static uint8_t parseConfig(unsigned long *tGnss, unsigned long *tImu, bool *ledState, unsigned long *tInactive);
/* ************************************************************************** */

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */

void sd_fat_config_task ( bool init )
{
    /* The application task cfg_state machine */
    switch(appFatData.cfg_state)
    {
        case APP_CFG_MOUNT_DISK:
            if(SYS_FS_Mount("/dev/mmcblka1", "/mnt/myDrive", FAT, 0, NULL) != 0)
            {
                /* The disk could not be mounted. Try
                 * mounting again untill success. */
                LED_ROn();
                appFatData.cfg_state = APP_CFG_MOUNT_DISK;
            }
            else
            {
                /* Mount was successful. Unmount the disk, for testing. */
                LED_ROff();
                appFatData.cfg_state = APP_CFG_SET_CURRENT_DRIVE;
            }
            break;

        case APP_CFG_SET_CURRENT_DRIVE:
            if(SYS_FS_CurrentDriveSet("/mnt/myDrive") == SYS_FS_RES_FAILURE)
            {
                /* Error while setting current drive */
                appFatData.cfg_state = APP_CFG_ERROR;
            }
            else
            {
                if(init == true)
                    /* Open config file for reading. */
                    appFatData.cfg_state = APP_CFG_OPEN_READ_CONFIG_FILE;
                else
                    /* Wait for further commands. */
                    appFatData.cfg_state = APP_CFG_IDLE;
            }
            break;
            
        case APP_CFG_OPEN_READ_CONFIG_FILE:
            appFatData.fileCfgHandle = SYS_FS_FileOpen("CONFIG.txt",
                    (SYS_FS_FILE_OPEN_READ));
            if(appFatData.fileCfgHandle == SYS_FS_HANDLE_INVALID)
            {
                /* No config file, write default config file */
                sd_CFG_Write(T_INTERVAL_GNSS_DEFAULT, T_INTERVAL_IMU_DEFAULT, LED_STATE_DEFAULT, T_INACTIVE_PERIOD_DEFAULT, true);
                
                /* Re-try to open file as read */
                //appFatData.cfg_state = APP_CFG_OPEN_READ_CONFIG_FILE;         
                
            }
            else
            {
                /* Create a directory. */
                appFatData.cfg_state = APP_CFG_READ_CONFIG_FILE;
            }
            break;
        
        case APP_CFG_READ_CONFIG_FILE:                             
            /* If read was success, try writing to the new file */
            if(SYS_FS_FileRead(appFatData.fileCfgHandle, appFatData.cfg_data, 
                    SYS_FS_FileSize(appFatData.fileCfgHandle)) == -1)
            {
                /* Write was not successful. Close the file
                 * and error out.*/
                SYS_FS_FileClose(appFatData.fileCfgHandle);
                appFatData.cfg_state = APP_CFG_ERROR;
            }
            else
            {
                appFatData.cfg_state = APP_CFG_CLOSE_FILE;
            }
            break;
        case APP_CFG_OPEN_WRITE_CONFIG_FILE:
			appFatData.fileCfgHandle = SYS_FS_FileOpen("CONFIG.txt",
					(SYS_FS_FILE_OPEN_WRITE));
			if(appFatData.fileCfgHandle == SYS_FS_HANDLE_INVALID)
			{
				/* Could not open the file. Error out*/
				appFatData.cfg_state = APP_CFG_ERROR;
			}
			else
			{
				/* Create a directory. */
				appFatData.cfg_state = APP_CFG_WRITE_CONFIG_FILE;
			}
			break;

		case APP_CFG_WRITE_CONFIG_FILE:                             
			/* If read was success, try writing to the new file */
			if(SYS_FS_FileStringPut(appFatData.fileCfgHandle, appFatData.cfg_data)== -1)
			{
				/* Write was not successful. Close the file
				 * and error out.*/
				SYS_FS_FileClose(appFatData.fileCfgHandle);
				appFatData.cfg_state = APP_CFG_ERROR;
			}
			else
			{
				appFatData.cfg_state = APP_CFG_CLOSE_FILE;
			}
			break;
        case APP_CFG_CLOSE_FILE:
            /* Close the file */
            SYS_FS_FileClose(appFatData.fileCfgHandle);
             /* The test was successful. Lets idle. */
            if(init == true)
                appFatData.cfg_state = APP_CFG_UNMOUNT_DISK;
            else
                appFatData.cfg_state = APP_CFG_IDLE;
            break;

        case APP_CFG_IDLE:
            /* The appliction comes here when the demo
             * has completed successfully. Switch on
             * green LED. */
            //BSP_LEDOn(APP_SUCCESS_LED);
            LED_ROff();
            break;
        case APP_CFG_ERROR:
            /* The appliction comes here when the demo
             * has failed. Switch on the red LED.*/
            //BSP_LEDOn(APP_FAILURE_LED);
            LED_ROn();
            break;
        default:
            break;
            
        case APP_CFG_UNMOUNT_DISK:
            if(SYS_FS_Unmount("/mnt/myDrive") != 0)
            {
                /* The disk could not be un mounted. Try
                 * un mounting again untill success. */

                appFatData.cfg_state = APP_CFG_UNMOUNT_DISK;
            }
            else
            {
                /* UnMount was successful. Mount the disk again */
                appFatData.cfg_state = APP_CFG_IDLE;
            }
            break;
    }
}

// Loggin task
void sd_fat_logging_task ( void )
{   
    /* The application task log_state machine */
    switch(appFatData.log_state)
    {
        case APP_MOUNT_DISK:
            if(SYS_FS_Mount("/dev/mmcblka1", "/mnt/myDrive", FAT, 0, NULL) != 0)
            {
                /* The disk could not be mounted. Try
                 * mounting again untill success. */

                appFatData.log_state = APP_MOUNT_DISK;
            }
            else
            {
                /* Mount was successful. Unmount the disk, for testing. */

                appFatData.log_state = APP_SET_CURRENT_DRIVE;
            }
            break;

        case APP_SET_CURRENT_DRIVE:
            if(SYS_FS_CurrentDriveSet("/mnt/myDrive") == SYS_FS_RES_FAILURE)
            {
                /* Error while setting current drive */
                appFatData.log_state = APP_ERROR;
            }
            else
            {
                /* Open a file for reading. */
                appFatData.log_state = APP_IDLE;
            }
            break;
            
        case APP_WRITE_MEASURE_FILE:
            appFatData.fileMeasureHandle = SYS_FS_FileOpen(appFatData.fileName,
                    (SYS_FS_FILE_OPEN_APPEND_PLUS));
            if(appFatData.fileMeasureHandle == SYS_FS_HANDLE_INVALID)
            {
                /* Could not open the file. Error out*/
                appFatData.log_state = APP_ERROR;
            }
            else
            {
                /* Create a directory. */
                appFatData.log_state = APP_WRITE_TO_MEASURE_FILE;
            }
            break;

        case APP_WRITE_TO_MEASURE_FILE:                             
            /* If read was success, try writing to the new file */
            if(SYS_FS_FileStringPut(appFatData.fileMeasureHandle, appFatData.data) == -1)
            {
                /* Write was not successful. Close the file
                 * and error out.*/
                SYS_FS_FileClose(appFatData.fileMeasureHandle);
                appFatData.log_state = APP_ERROR;
            }
            else
            {
                appFatData.log_state = APP_CLOSE_FILE;
            }
            break;

        case APP_CLOSE_FILE:
            /* Close both files */
            SYS_FS_FileClose(appFatData.fileMeasureHandle);
             /* The test was successful. Lets idle. */
            appFatData.log_state = APP_IDLE;
            break;

        case APP_IDLE:
            /* The appliction comes here when the demo
             * has completed successfully. Switch on
             * green LED. */
            //BSP_LEDOn(APP_SUCCESS_LED);
            LED_ROff();
            break;
        case APP_ERROR:
            /* The appliction comes here when the demo
             * has failed. Switch on the red LED.*/
            //BSP_LEDOn(APP_FAILURE_LED);
            LED_ROn();
            break;
        default:
            break;
            
        case APP_UNMOUNT_DISK:
            if(SYS_FS_Unmount("/mnt/myDrive") != 0)
            {
                /* The disk could not be un mounted. Try
                 * un mounting again untill success. */

                appFatData.log_state = APP_UNMOUNT_DISK;
            }
            else
            {
                /* UnMount was successful. Mount the disk again */
                appFatData.log_state = APP_IDLE;
            }
            break;

    }

//    SYS_FS_Tasks();
} //End of APP_Tasks

void sd_IMU_scheduleWrite (s_bno055_data * data)
{
    /* If sd Card available */
    if(appFatData.log_state == APP_IDLE)
    {
        // Prepare file name
        sprintf(appFatData.fileName, "LOG_IMU.csv");
        /* Next log_state : write to file */
        appFatData.log_state = APP_WRITE_MEASURE_FILE;
        /* Write the buffer */
        sprintf(appFatData.data, "%d;%d0;%.4f;%.4f;%.4f;%.4f;%.4f;%.4f;%.4f;%.4f;%.4f;%.4f;%.4f;%.4f;%.4f;%.4f;%.4f;%d;%d;%d;%d;\r\n"
                                  ,data->flagImportantMeas, (data->d_time), data->gravity.x, data->gravity.y, data->gravity.z, data->gyro.x, data->gyro.y, data->gyro.z
                                  ,data->mag.x, data->mag.y, data->mag.z, data->linear_accel.x, data->linear_accel.y, data->linear_accel.z
                                  ,data->euler.h, data->euler.p, data->euler.r, data->quaternion.w, data->quaternion.x, data->quaternion.y, data->quaternion.z);
        /* Compute the number of bytes to send */
        appFatData.nBytesToWrite = strlen(appFatData.data);
    }
}

void sd_GNSS_scheduleWrite (minmea_messages * pGnssData)
{
    char fifoBuffer[FIFO_RX_SIZE];
    /* If sd Card available */
    if(appFatData.log_state == APP_IDLE)
    {
        // Prepare file name
        sprintf(appFatData.fileName, "LOG_GNSS.txt");
        /* Next log_state : write to file */
        appFatData.log_state = APP_WRITE_MEASURE_FILE;
        /* Write the buffer */
        getFifoToLastReturn(&usartFifoRx, fifoBuffer);
        
        sprintf(appFatData.data, "%s", fifoBuffer);
        
        //sprintf(appFatData.data, "%s",  );
        /* Compute the number of bytes to send */
        appFatData.nBytesToWrite = strlen(appFatData.data);
    }
}

void sd_CFG_Write (uint32_t tLogGNSS_ms, uint32_t tLogIMU_ms, uint8_t ledState, uint32_t tInactiveP, bool skipMount)
{
    /* If sd Card available */
    if((appFatData.cfg_state == APP_CFG_IDLE)||(appFatData.cfg_state == APP_CFG_OPEN_READ_CONFIG_FILE))
    {
        /* Close the file */
        SYS_FS_FileClose(appFatData.fileCfgHandle);
            
        if(skipMount == false)
            /* Next config : mount disk */
            appFatData.cfg_state = APP_CFG_MOUNT_DISK;
        else if(skipMount == true)
            /* Next config : write to file */
            appFatData.cfg_state = APP_CFG_OPEN_WRITE_CONFIG_FILE;
        
        /* Write the buffer */
        sprintf(appFatData.cfg_data, "$LOG INTERVAL GNSS [ms] : %u\r\n$LOG INTERVAL IMU [ms] : %u\r\n$LED ENABLE [1/0] : %u\r\n$INACTIVE PERIOD [s] : %u\r\n", 
                tLogGNSS_ms, tLogIMU_ms, ledState, tInactiveP);
        /* Compute the number of bytes to send */
        appFatData.nBytesToWrite = strlen(appFatData.cfg_data);
        
    }
}


APP_FAT_LOG_STATES sd_logGetState( void )
{
    return appFatData.log_state;
}

void sd_logSetState( APP_FAT_LOG_STATES newState )
{
    appFatData.log_state = newState;
}

// CONFIG FUNCTIONS

APP_FAT_CONFIG_STATES sd_cfgGetState( void )
{
    return appFatData.cfg_state;
}
void sd_cfgSetState( APP_FAT_CONFIG_STATES newState )
{
     appFatData.cfg_state = newState;
}

char* sd_cfgGetCfgBuffer( void )
{
    return appFatData.cfg_data;
}

void sd_fat_cfg_init(unsigned long *tGnss, unsigned long *tImu, bool *ledState, uint32_t *tInactivePeriod)
{
    // Config parser error
    uint8_t parseError = 0;
    unsigned long tGnssLocal = 0;
    unsigned long tImuLocal = 0;
    unsigned long tInactive = 0;
    bool ledStateLocal = 0;
    
    //appFatData.nBytesRead = 0;
    //appFatData.nBytesToWrite = 0;

    //appFatData.log_state = APP_MOUNT_DISK;
    //appFatData.cfg_state = APP_CFG_MOUNT_DISK;
    
    // Read config routine, until error or success
    sd_fat_config_task(true);
    
    // If read config routine was a success
    if(sd_cfgGetState() == APP_CFG_IDLE)
        // Parse config buffer to get parameters
        parseError = parseConfig(&tGnssLocal, &tImuLocal, &ledStateLocal, &tInactive);
    // If the parsing failed or the read config routine failed
    if((parseError > 0)||(sd_cfgGetState() == APP_CFG_ERROR))
    {
        // Set default system parameters
        *tGnss = T_INTERVAL_GNSS_DEFAULT;
        *tImu = T_INTERVAL_IMU_DEFAULT;
        *ledState = LED_STATE_DEFAULT;
        *tInactivePeriod = T_INACTIVE_PERIOD_DEFAULT;
        appStateSet(APP_STATE_LOGGING);
        // Start measure timer
        DRV_TMR1_Start();
    }
    else if ((sd_cfgGetState() == APP_CFG_IDLE))
    {
        *tGnss = tGnssLocal;
        *tImu = tImuLocal;
        *ledState = ledStateLocal;
        *tInactivePeriod = tInactive;
        appStateSet(APP_STATE_LOGGING);
        // Start measure timer
        DRV_TMR1_Start();
    }
}

static uint8_t parseConfig(unsigned long *tGnss, unsigned long *tImu, bool *ledState, unsigned long *tInactive)
{
    char *ptBufferHead;
    char *ptBufferTail;
    char ptTrame[10];
    uint8_t error = 0;
        
    // Locate the head and tail of the first data
    ptBufferHead = strstr(appFatData.cfg_data, " :");
    ptBufferTail = strstr(appFatData.cfg_data, "\r\n");
    // Check if the pointers are corrects
    if((ptBufferHead != NULL)&&(ptBufferTail != NULL)&&(ptBufferHead < ptBufferTail)){
        // Copy the data between the head and the tail in a sub-pointer
        strncpy(ptTrame, (ptBufferHead+2), (ptBufferTail-ptBufferHead));
        // Convert the character to value
        *tGnss = (uint32_t) atoi(ptTrame);
    }
    else 
        error++;
    
    // Locate the head and tail of the first data
    ptBufferHead = strstr(ptBufferTail, " :");
    ptBufferTail = strstr(ptBufferHead, "\r\n");
    // Check if the pointers are corrects
    if((ptBufferHead != NULL)&&(ptBufferTail != NULL)&&(ptBufferHead < ptBufferTail)){
        // Copy the data between the head and the tail in a sub-pointer
        strncpy(ptTrame, (ptBufferHead+2), (ptBufferTail-ptBufferHead));
        // Convert the character to value
        *tImu = (uint32_t) atoi(ptTrame);
    }
    else
        error++;
    
    // Locate the head and tail of the first data
    ptBufferHead = strstr(ptBufferTail, " :");
    ptBufferTail = strstr(ptBufferHead, "\r\n");
    // Check if the pointers are corrects
    if((ptBufferHead != NULL)&&(ptBufferTail != NULL)&&(ptBufferHead < ptBufferTail)){
        // Copy the data between the head and the tail in a sub-pointer
        strncpy(ptTrame, (ptBufferHead+2), (ptBufferTail-ptBufferHead));
        // Convert the character to value
        *ledState = (bool) atoi(ptTrame);
    }
    else
        error++;
    
    // Locate the head and tail of the first data
    ptBufferHead = strstr(ptBufferTail, " :");
    ptBufferTail = strstr(ptBufferHead, "\r\n");
    // Check if the pointers are corrects
    if((ptBufferHead != NULL)&&(ptBufferTail != NULL)&&(ptBufferHead < ptBufferTail)){
        // Copy the data between the head and the tail in a sub-pointer
        strncpy(ptTrame, (ptBufferHead+2), (ptBufferTail-ptBufferHead));
        // Convert the character to value
        *tInactive = (uint32_t) atoi(ptTrame);
    }
    else
        error++;
    
    return error;
    
}

void sd_fat_readDisplayFile(const char * fileName)
{
    const uint16_t READSIZE = 256;
    uint32_t i = 0;
    char stringRead[READSIZE];
    unsigned long cntTimeaout = 0;

    /* Close both files */
    SYS_FS_FileClose(appFatData.fileMeasureHandle);
    // Read config file
    appFatData.fileCfgHandle = SYS_FS_FileOpen(fileName,(SYS_FS_FILE_OPEN_READ));
    
    do{
        
        SYS_FS_FileStringGet(appFatData.fileCfgHandle, stringRead, READSIZE);
        
        do{
            if(!PLIB_USART_TransmitterBufferIsFull(USART_ID_1))
            {
                PLIB_USART_TransmitterByteSend(USART_ID_1, stringRead[i]);
                i++;
            }
            cntTimeaout++;
        }while((i < strlen(stringRead))&&(cntTimeaout<TIME_OUT));
        
        i = 0;
        cntTimeaout = 0; 
        
        if(pollSerialCmds(USART_ID_1, "exit", "EXIT", "x" ,"X"))
            break;
        
    }while(!SYS_FS_FileEOF(appFatData.fileCfgHandle));
    
    /* Close both files */
    SYS_FS_FileClose(appFatData.fileMeasureHandle);
}

//bool sd_fat_readFile(const char * fileName, char readBuffer[])
//{
//    uint32_t fileSize = 0;
//    static bool fullyRead = false;
//    /* Close both files */
//    SYS_FS_FileClose(appFatData.fileMeasureHandle);
//    // Read config file
//    appFatData.fileCfgHandle = SYS_FS_FileOpen(fileName,(SYS_FS_FILE_OPEN_READ));
//    
//    fileSize = SYS_FS_FileSize(appFatData.fileCfgHandle);
//    
//    if (fileSize <= sizeof(readBuffer))
//        SYS_FS_FileRead(appFatData.fileCfgHandle, readBuffer, fileSize);
//    else{
//        
//    }
//    /* Close both files */
//    SYS_FS_FileClose(appFatData.fileMeasureHandle);
//}

/* *****************************************************************************
 End of File
 */
