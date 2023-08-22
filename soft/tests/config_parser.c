/******************************************************************************

                            Online C Compiler.
                Code, Compile, Run and Debug C program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdint.h>

int main()
{
    char cfgBuffer[86];
    sprintf(cfgBuffer, "$LOG INTERVAL GNSS [ms] : 5000\r\n$LOG INTERVAL IMU [ms] : 500\r\n$LED STATE [1/0] : 1");
    
    char *ptBufferHead;
    char *ptBufferTail;
    char ptTrame[10];
    
    uint32_t tGnss = 0;
    uint32_t tImu = 0;
    uint8_t ledState = 0;
    uint8_t error = 0;
        
    ptBufferHead = strstr(cfgBuffer, ": ");
    ptBufferTail = strstr(cfgBuffer, "\r\n");
    if((ptBufferHead != NULL)&&(ptBufferTail != NULL)&&(ptBufferHead < ptBufferTail)){
        if(ptBufferHead < ptBufferTail)
            strncpy(ptTrame, (ptBufferHead+2), (ptBufferTail-ptBufferHead));
        tGnss = (uint32_t) atoi(ptTrame);
    }
    else 
        error++;
    
    ptBufferHead = strstr(ptBufferTail, ": ");
    ptBufferTail = strstr(ptBufferHead, "\r\n");
    if((ptBufferHead != NULL)&&(ptBufferTail != NULL)&&(ptBufferHead < ptBufferTail)){
        strncpy(ptTrame, (ptBufferHead+2), (ptBufferTail-ptBufferHead));
        tImu = (uint32_t) atoi(ptTrame);
    }
    else
        error++;
    
    
    ptBufferHead = strstr(ptBufferTail, ": ");
    ptBufferTail = (ptBufferHead + 3);
    if((ptBufferHead != NULL)&&(ptBufferTail != NULL)&&(ptBufferHead < ptBufferTail)){
        strncpy(ptTrame, (ptBufferHead+2), (ptBufferTail-ptBufferHead));
        ledState = (uint32_t) atoi(ptTrame);
    }
    else
        error++;
    
    printf("%d\n", tGnss);
    printf("%d\n", tImu);
    printf("%d\n", ledState);
    
    return error;
}
