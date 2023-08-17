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

/* ************************************************************************** */



/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */

void sd_fat_task ( void )
{   
    /* The application task state machine */
    switch(appFatData.state)
    {
        case APP_MOUNT_DISK:
            if(SYS_FS_Mount("/dev/mmcblka1", "/mnt/myDrive", FAT, 0, NULL) != 0)
            {
                /* The disk could not be mounted. Try
                 * mounting again untill success. */

                appFatData.state = APP_MOUNT_DISK;
            }
            else
            {
                /* Mount was successful. Unmount the disk, for testing. */

                appFatData.state = APP_SET_CURRENT_DRIVE;
            }
            break;

        case APP_SET_CURRENT_DRIVE:
            if(SYS_FS_CurrentDriveSet("/mnt/myDrive") == SYS_FS_RES_FAILURE)
            {
                /* Error while setting current drive */
                appFatData.state = APP_ERROR;
            }
            else
            {
                /* Open a file for reading. */
                appFatData.state = APP_IDLE;
            }
            break;
            
        case APP_WRITE_MEASURE_FILE:
            appFatData.fileHandle = SYS_FS_FileOpen("MESURES.csv",
                    (SYS_FS_FILE_OPEN_APPEND_PLUS));
            if(appFatData.fileHandle == SYS_FS_HANDLE_INVALID)
            {
                /* Could not open the file. Error out*/
                appFatData.state = APP_ERROR;
            }
            else
            {
                /* Create a directory. */
                appFatData.state = APP_WRITE_TO_MEASURE_FILE;
            }
            break;

        case APP_WRITE_TO_MEASURE_FILE:                             
            /* If read was success, try writing to the new file */
            if(SYS_FS_FileStringPut(appFatData.fileHandle, appFatData.data) == -1)
            {
                /* Write was not successful. Close the file
                 * and error out.*/
                SYS_FS_FileClose(appFatData.fileHandle);
                appFatData.state = APP_ERROR;
            }
            else
            {
                appFatData.state = APP_CLOSE_FILE;
            }
            break;

        case APP_CLOSE_FILE:
            /* Close both files */
            SYS_FS_FileClose(appFatData.fileHandle);
             /* The test was successful. Lets idle. */
            appFatData.state = APP_IDLE;
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

                appFatData.state = APP_UNMOUNT_DISK;
            }
            else
            {
                /* UnMount was successful. Mount the disk again */
                appFatData.state = APP_IDLE;
            }
            break;

    }

//    SYS_FS_Tasks();
} //End of APP_Tasks

void sd_BNO_scheduleWrite_BNO055 (s_bno055_data * data)
{
    /* If sd Card available */
    if(appFatData.state == APP_IDLE)
    {
        /* Next state : write to file */
        appFatData.state = APP_WRITE_MEASURE_FILE;
        /* Write the buffer */
        sprintf(appFatData.data, "%d;%d0;%.4f;%.4f;%.4f;%.4f;%.4f;%.4f;%.4f;%.4f;%.4f;%.4f;%.4f;%.4f;%.4f;%.4f;%.4f;%d;%d;%d;%d;\r\n"
                                  ,data->flagImportantMeas, (data->d_time), data->gravity.x, data->gravity.y, data->gravity.z, data->gyro.x, data->gyro.y, data->gyro.z
                                  ,data->mag.x, data->mag.y, data->mag.z, data->linear_accel.x, data->linear_accel.y, data->linear_accel.z
                                  ,data->euler.h, data->euler.p, data->euler.r, data->quaternion.w, data->quaternion.x, data->quaternion.y, data->quaternion.z);
        /* Compute the number of bytes to send */
        appFatData.nBytesToWrite = strlen(appFatData.data);
    }
}

APP_FAT_STATES sd_getState( void )
{
    return appFatData.state;
}

void sd_setState( APP_FAT_STATES newState )
{
    appFatData.state = newState;
}

/* *****************************************************************************
 End of File
 */
