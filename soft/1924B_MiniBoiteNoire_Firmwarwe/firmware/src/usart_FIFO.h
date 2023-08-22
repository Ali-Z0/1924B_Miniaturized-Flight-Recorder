/*******************************************************************************
  MPLAB Harmony Application Source File
  
  Company:
    Microchip Technology Inc.
  
  File Name:
    uasrt_FIFO.c

  Summary:
    This file contains the header code for the fifo.

 *******************************************************************************/

#ifndef USART_FIFO_H
#define	USART_FIFO_H

#ifdef	__cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------// Includes
#include "app.h"

    
#define FIFO_RX_SIZE 82
#define FIFO_TX_SIZE 82

    
//----------------------------------------------------------------------------// Structure
typedef struct {
    
    int32_t fifoSize;   // Size of the FIFO
    char *pHead;       // Head of the FIFO
    char *pTail;       // Tail of the FIFO
    char *pWrite;     // Writing pointer
    char *pRead;      // Reading pointer
    
} USART_FIFO;


//----------------------------------------------------------------------------// Functions
void initFifo(USART_FIFO *pFifo, int32_t fifoSize, char *pHead, 
        char initVal);

int32_t getWriteSpace(USART_FIFO *pFifo);
int32_t getReadSize(USART_FIFO *pFifo);

uint8_t putCharInFifo(USART_FIFO *pFifo, char charToPut);
uint8_t getCharFromFifo(USART_FIFO *pFifo, char *charRead);

void putStringInFifo(USART_FIFO *pFifo, size_t arraySize, char* pArrayToSave);
int32_t getFullFifo(USART_FIFO *pFifo, char* pArrayToModify);
int32_t getStringFromFifo(USART_FIFO *pFifo, char* pArrayToModify);

//----------------------------------------------------------------------------// Variables and structures delarations
extern USART_FIFO usartFifoRx;
extern USART_FIFO usartFifoTx;
    
extern char a_fifoRx[FIFO_RX_SIZE];
extern char a_fifoTx[FIFO_TX_SIZE];


#ifdef	__cplusplus
}
#endif

#endif	/* USART_FIFO_H */

