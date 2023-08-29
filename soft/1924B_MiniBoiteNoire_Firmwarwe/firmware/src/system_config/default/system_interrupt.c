/*******************************************************************************
 System Interrupts File

  File Name:
    system_interrupt.c

  Summary:
    Raw ISR definitions.

  Description:
    This file contains a definitions of the raw ISRs required to support the
    interrupt sub-system.

  Summary:
    This file contains source code for the interrupt vector functions in the
    system.

  Description:
    This file contains source code for the interrupt vector functions in the
    system.  It implements the system and part specific vector "stub" functions
    from which the individual "Tasks" functions are called for any modules
    executing interrupt-driven in the MPLAB Harmony system.

  Remarks:
    This file requires access to the systemObjects global data structure that
    contains the object handles to all MPLAB Harmony module objects executing
    interrupt-driven in the system.  These handles are passed into the individual
    module "Tasks" functions to identify the instance of the module to maintain.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2011-2014 released Microchip Technology Inc.  All rights reserved.

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

#include "system/common/sys_common.h"
#include "app.h"
#include "system_definitions.h"
#include "usart_FIFO.h"

// *****************************************************************************
// *****************************************************************************
// Section: System Interrupt Vector Functions
// *****************************************************************************
// *****************************************************************************
void __ISR(_UART_1_VECTOR, ipl0AUTO) _IntHandlerDrvUsartInstance0(void)
{
    DRV_USART_TasksTransmit(sysObj.drvUsart0);
    DRV_USART_TasksError(sysObj.drvUsart0);
    DRV_USART_TasksReceive(sysObj.drvUsart0);
}

 


void __ISR(_UART_2_VECTOR, ipl1AUTO) _IntHandlerDrvUsartInstance1(void)
{
    USART_ERROR usartStatus;
    bool        isTxBuffFull;
    char      charReceived;
    char      charToSend;
    char      TXsize;
    
     //------------------------------------------------------------------------// RX interrupt
    if(PLIB_INT_SourceFlagGet(INT_ID_0, INT_SOURCE_USART_2_RECEIVE) &&
                PLIB_INT_SourceIsEnabled(INT_ID_0, INT_SOURCE_USART_2_RECEIVE)){

        // Parity error or overrun
        usartStatus = PLIB_USART_ErrorsGet(USART_ID_2);

        if ((usartStatus & (USART_ERROR_PARITY | USART_ERROR_FRAMING | 
                USART_ERROR_RECEIVER_OVERRUN)) == 0){

            // All char received are transferred to the FIFO
            // 1 if ONE_CHAR, 4 if HALF_FULL and 6 3B4FULL
            while(PLIB_USART_ReceiverDataIsAvailable(USART_ID_2)){
                
                charReceived = PLIB_USART_ReceiverByteReceive(USART_ID_2);
                putCharInFifo(&usartFifoRx, charReceived);
            }
            // Buffer is empty, clear interrupt flag
            PLIB_INT_SourceFlagClear(INT_ID_0, INT_SOURCE_USART_2_RECEIVE);
       
        }else{
            // Deleting errors
            // Reading errors clears them except for overrun
            if((usartStatus & USART_ERROR_RECEIVER_OVERRUN) == 
                    USART_ERROR_RECEIVER_OVERRUN){
                
                PLIB_USART_ReceiverOverrunErrorClear(USART_ID_2);
            }
        }
    }
    
    
    //------------------------------------------------------------------------// TX interrupt
    if (PLIB_INT_SourceFlagGet(INT_ID_0, INT_SOURCE_USART_2_TRANSMIT) &&
                 PLIB_INT_SourceIsEnabled(INT_ID_0, INT_SOURCE_USART_2_TRANSMIT)){
        
        TXsize = getReadSize(&usartFifoTx);
        // i_cts = input(RS232_CTS);
       
        isTxBuffFull = PLIB_USART_TransmitterBufferIsFull(USART_ID_2);
       
        if (/*(i_cts == 0) && */(TXsize > 0) && (isTxBuffFull == false)){
            do{
                getCharFromFifo(&usartFifoTx, &charToSend);
                if(charToSend != '\0') PLIB_USART_TransmitterByteSend(USART_ID_2, charToSend);
                /*i_cts = RS232_CTS;*/
                TXsize = getReadSize (&usartFifoTx);
                isTxBuffFull = PLIB_USART_TransmitterBufferIsFull(USART_ID_2);
            }while(/*(i_cts == 0) && */( TXsize > 0 ) && isTxBuffFull == false);
        }
		
        // Disables TX interrupt (to avoid unnecessary interruptions if there's 
        // nothing left to transmit)
        if(TXsize == 0){
            
            PLIB_INT_SourceDisable(INT_ID_0, INT_SOURCE_USART_2_TRANSMIT);
        }
        // Clears the TX interrupt Flag
        PLIB_INT_SourceFlagClear(INT_ID_0, INT_SOURCE_USART_2_TRANSMIT);
    }
}
 
void __ISR(_TIMER_1_VECTOR, ipl6AUTO) IntHandlerDrvTmrInstance0(void)
{
    PLIB_INT_SourceFlagClear(INT_ID_0,INT_SOURCE_TIMER_1);
    delayTimer_callback();
}
void __ISR(_TIMER_2_VECTOR, ipl5AUTO) IntHandlerDrvTmrInstance1(void)
{
    PLIB_INT_SourceFlagClear(INT_ID_0,INT_SOURCE_TIMER_2);
    stateTimer_callback();
}
 
void __ISR(_SPI_1_VECTOR, ipl1AUTO) _IntHandlerSPIInstance0(void)
{
    DRV_SPI_Tasks(sysObj.spiObjectIdx0);
}
/*******************************************************************************
 End of File
*/
