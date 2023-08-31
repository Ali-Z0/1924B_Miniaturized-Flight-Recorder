/*******************************************************************************
  MPLAB Harmony Application Source File
  
  Company:
    Microchip Technology Inc.
  
  File Name:
    uasrt_FIFO.c

  Summary:
    This file contains the source code for the fifo.

 *******************************************************************************/


#include "usart_FIFO.h"


USART_FIFO usartFifoRx;
USART_FIFO usartFifoTx;

char a_fifoRx[FIFO_RX_SIZE];
char a_fifoTx[FIFO_TX_SIZE];

//----------------------------------------------------------------------------// initFifo
// Initialization with fill value

void initFifo(USART_FIFO *pFifo, int32_t fifoSize, char *pHead, 
        char initVal){
    
   int32_t i;
   char *pFif;
   
   pFifo->fifoSize  = fifoSize;
   pFifo->pHead     = pHead; 
   pFifo->pTail     = pHead + (fifoSize - 1);
   pFifo->pWrite    = pHead;
   pFifo->pRead     = pHead;
   
   pFif = pHead;
   
    for (i=0; i < fifoSize; i++){
       
        *pFif = initVal;
        pFif++;
    }
}



//----------------------------------------------------------------------------// GetWriteSpace
// Return the available writing space

int32_t getWriteSpace(USART_FIFO *pFifo){
   
    int32_t writeSize;

    //  Determines the number of characters that can be deposited
    writeSize = pFifo->pRead - pFifo->pWrite - 1;
    
    if (writeSize < 0) {
        
        writeSize = writeSize + pFifo->fifoSize;
    }
    return writeSize;
}



//----------------------------------------------------------------------------// GetReadSize
// Returns the number of characters to read

int32_t getReadSize(USART_FIFO *pFifo){
    
   int32_t readSize;

    readSize = pFifo->pWrite - pFifo->pRead;
    if (readSize < 0){
        
        readSize = readSize +  pFifo->fifoSize;
    }

   return readSize;
}



//----------------------------------------------------------------------------// PutCharInFifo
// Put a character in the FIFO and return 0 if if all is good or 1 if the FIFO
// is full

uint8_t putCharInFifo(USART_FIFO *pFifo, char charToPut){
    
    uint8_t writeStatus;

    // test si fifo est FULL
    if (getWriteSpace(pFifo) == 0) {
        writeStatus = 1; // fifo FULL
    }
    else {
        // Writes the char in FIFO
        *(pFifo->pWrite) = charToPut;

        // Increments the writing pointer
        pFifo->pWrite++;
        
        // Rebound management 
        if (pFifo->pWrite > pFifo->pTail){

            pFifo->pWrite = pFifo->pHead;
        }

        writeStatus = 0; // GOOD
    }
    return writeStatus;
}



//----------------------------------------------------------------------------// getCharFromFifo
// Character read is returned as reference and it returns 0 if GOOD or 1 if
// the FIFO is empty

uint8_t getCharFromFifo(USART_FIFO *pFifo, char *charRead){
    
    int32_t readSize;
    char readStatus;

    // Determines the number of characters that can be read
    readSize = getReadSize(pFifo);

    // Checks if the FIFO is empty
    if(readSize == 0){
       
        readStatus = 1; // fifo EMPTY
        *charRead = 0;  // charRead = NULL
    }
    else {
        // Reads the char in FIFO
        *charRead = *(pFifo->pRead);

        // Increments the reading pointer
        pFifo->pRead++;
        
        // Rebound management 
        if (pFifo->pRead > pFifo->pTail){
          
            pFifo->pRead = pFifo->pHead;
        }
        readStatus = 0; // GOOD
    }
    return readStatus;
} 



//----------------------------------------------------------------------------// putStringInFifo
// Puts a string in the FIFO buffer

void putStringInFifo(USART_FIFO *pFifo, size_t arraySize, char* pArrayToSave){
    
    int i;
    
    for (i = 0; i < arraySize; i++) {
        
        putCharInFifo(pFifo, pArrayToSave[i]);
    }
}



//----------------------------------------------------------------------------//
// Reads a complete string from FIFO

int32_t getStringFromFifo(USART_FIFO *pFifo, char* pArrayToModify){
    
    int32_t arraySize;
    int i = 1;
    
    arraySize = getReadSize(pFifo);
    
    getCharFromFifo(pFifo, &pArrayToModify[i]);
    
    while((i < arraySize)&&(pArrayToModify[i-1] != '\n'))
    {
        getCharFromFifo(pFifo, &pArrayToModify[i]);
        i++;
    }
    return arraySize;
}

int32_t getFifoToLastReturn(USART_FIFO *pFifo, char* pArrayToModify){
    
    int32_t arraySize;
    int i = 0;
    
    char *ptTail;
    //char *ptHead;

    ptTail = strrchr(a_fifoRx, '\n');
    //ptHead = strchr(a_fifoRx, '$');
    
    arraySize = (ptTail - &a_fifoRx[0]);
    
    getCharFromFifo(pFifo, &pArrayToModify[i]);
    
    for(i = 1; i < (arraySize); i++){
        
        getCharFromFifo(pFifo, &pArrayToModify[i]);
    }
    
    return arraySize;
}

int32_t getFullFifo(USART_FIFO *pFifo, char* pArrayToModify){
    
    int32_t arraySize;
    int i = 0;
    
    arraySize = getReadSize(pFifo);
    
    if(arraySize > sizeof(pArrayToModify))
        arraySize = sizeof(pArrayToModify);
    
    getCharFromFifo(pFifo, &pArrayToModify[i]);
    
    for(i = 1; i < (arraySize); i++){
        
        getCharFromFifo(pFifo, &pArrayToModify[i]);
    }
    
    return arraySize;
}