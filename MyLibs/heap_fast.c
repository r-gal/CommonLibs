/*
    FreeRTOS V9.0.0 - Copyright (C) 2016 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>>> AND MODIFIED BY <<<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

/*
 * A sample implementation of pvPortMalloc() that allows the heap to be defined
 * across multiple non-contigous blocks and combines (coalescences) adjacent
 * memory blocks as they are freed.
 *
 * See heap_1.c, heap_2.c, heap_3.c and heap_4.c for alternative
 * implementations, and the memory management pages of http://www.FreeRTOS.org
 * for more information.
 *
 * Usage notes:
 *
 * vPortDefineHeapRegions() ***must*** be called before pvPortMalloc().
 * pvPortMalloc() will be called if any task objects (tasks, queues, event
 * groups, etc.) are created, therefore vPortDefineHeapRegions() ***must*** be
 * called before any other objects are defined.
 *
 * vPortDefineHeapRegions() takes a single parameter.  The parameter is an array
 * of HeapRegion_t structures.  HeapRegion_t is defined in portable.h as
 *
 * typedef struct HeapRegion
 * {
 *	uint8_t *pucStartAddress; << Start address of a block of memory that will be part of the heap.
 *	size_t xSizeInBytes;	  << Size of the block of memory.
 * } HeapRegion_t;
 *
 * The array is terminated using a NULL zero sized region definition, and the
 * memory regions defined in the array ***must*** appear in address order from
 * low address to high address.  So the following is a valid example of how
 * to use the function.
 *
 * HeapRegion_t xHeapRegions[] =
 * {
 * 	{ ( uint8_t * ) 0x80000000UL, 0x10000 }, << Defines a block of 0x10000 bytes starting at address 0x80000000
 * 	{ ( uint8_t * ) 0x90000000UL, 0xa0000 }, << Defines a block of 0xa0000 bytes starting at address of 0x90000000
 * 	{ NULL, 0 }                << Terminates the array.
 * };
 *
 * vPortDefineHeapRegions( xHeapRegions ); << Pass the array into vPortDefineHeapRegions().
 *
 * Note 0x80000000 is the lower address so appears in the array first.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Defining MPU_WRAPPERS_INCLUDED_FROM_API_FILE prevents task.h from redefining
all the API functions to use the MPU wrappers.  That should only be done when
task.h is included from an application file. */
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#include "FreeRTOS.h"
#include "task.h"

#undef MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#ifndef HEAP_NO_OF_LISTS
#define NO_OF_LISTS 8
const uint16_t allowedSizes[] = {16,32,64,128,256,1024,2048,4096};
#else
#define NO_OF_LISTS HEAP_NO_OF_LISTS
const uint16_t allowedSizes[] = HEAP_LIST_SIZES;
#endif

#if STORE_EXTRA_MEMORY_STATS == 1
struct ExtraMemStats_st
{
  uint16_t size;
  uint16_t requests;
  uint16_t allocated;
} extraMemStats[EXTRA_MEMORY_SIZE_DEPTH];
#endif

struct list_st
{
 struct elementInfo_st* first;
 uint16_t noOfFree;
 uint16_t noOfAllocated;
};

struct elementInfo_st
{
  struct elementInfo_st* next;
  uint16_t size;
  uint8_t bitMap;
   /*
   bit 0 - used
   bit 1 - from ISR
   */
  uint8_t blockNo;
  
};

static   uint8_t memoryInitialized = 0;

struct memData_st
{
  struct list_st memoryList[NO_OF_LISTS];
  HeapRegion_t * heapRegions;
  uint32_t freeSpace ;
  uint32_t minFreeSpace ;
  uint32_t unallocatedSpace ;
  uint32_t primaryUnallocatedSpace ;
  uint8_t errorCode;
};

  
static struct memData_st memData; /*__attribute__ ((section (".memory_section")));*/

void *pvPortMalloc2( size_t xWantedSize )
{
 
  uint8_t *pvReturn = NULL;
  uint8_t listIterator=0;
  uint8_t regionIterator=0;
  struct elementInfo_st* elementInfo;
  UBaseType_t uxSavedInterruptStatus;
  uint8_t bitISRmasc = 0;
  uint8_t fromISR;
  #if STORE_EXTRA_MEMORY_STATS == 1
  uint8_t newAllocation = 0;
  uint16_t wantedSize = xWantedSize;
  #endif

  /* The heap must be initialised before the first call to
  prvPortMalloc(). */
  configASSERT( memoryInitialized );

  if(xWantedSize <= allowedSizes[NO_OF_LISTS-1])
  {
    if((portNVIC_INT_CTRL_REG & 0x1F)==0)
    { 
      vPortEnterCritical();
      fromISR = 0;
    }
    else
    {
      uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
      bitISRmasc = 0x02;
      fromISR = 1;
      memData.errorCode = 3;
      while(1);
    }
    {

      /*round up to allowed size*/
      while(allowedSizes[listIterator]<xWantedSize)
      {
        listIterator++;
      }

   
      if(memData.memoryList[listIterator].noOfFree>0)
      {
        /*get from this list */
        pvReturn = (uint8_t*)memData.memoryList[listIterator].first;
        pvReturn += sizeof(struct elementInfo_st); /*header shift*/
        elementInfo = memData.memoryList[listIterator].first;
        memData.memoryList[listIterator].first = elementInfo ->next;
        memData.memoryList[listIterator].noOfFree--;
        elementInfo->bitMap |= (0x01|bitISRmasc);
        elementInfo->next = NULL;
        #if LOST_ALLOC_TRACE == 1
        elementInfo->key = 0xDEADBEEF;
        #endif
        memData.freeSpace -= elementInfo->size;
        if(memData.freeSpace<memData.minFreeSpace)
        {
          memData.minFreeSpace = memData.freeSpace;
        }

      }
      else
      {
        /*get from free heap */

        /*add header*/
        xWantedSize = allowedSizes[listIterator]+sizeof(struct elementInfo_st);

        while((memData.heapRegions[regionIterator].xSizeInBytes < xWantedSize) && (memData.heapRegions[regionIterator].pucStartAddress != NULL))
        {
          regionIterator++;
        }

        if(memData.heapRegions[regionIterator].pucStartAddress != NULL)
        {
          pvReturn = memData.heapRegions[regionIterator].pucStartAddress;
          pvReturn +=  sizeof(struct elementInfo_st);
          elementInfo = (struct elementInfo_st*) (memData.heapRegions[regionIterator].pucStartAddress);
          memData.heapRegions[regionIterator].pucStartAddress += xWantedSize;
          memData.heapRegions[regionIterator].xSizeInBytes -= xWantedSize;
          elementInfo->bitMap |= (0x01|bitISRmasc);
          elementInfo->size = xWantedSize ;//- sizeof(struct elementInfo_st);
          elementInfo->next = NULL;
          elementInfo->blockNo = regionIterator;
          memData.memoryList[listIterator].noOfAllocated++;
          memData.freeSpace -= elementInfo->size;
          memData.unallocatedSpace -= elementInfo->size;
          if(memData.freeSpace<memData.minFreeSpace)
          {
            memData.minFreeSpace= memData.freeSpace;
          }
          if(regionIterator == 0)
          {            
            memData.primaryUnallocatedSpace -= elementInfo->size;
          }
        }
        else
        {
          pvReturn = NULL;
        }

        if((pvReturn != NULL) &&
          (elementInfo->size<sizeof(struct elementInfo_st)))
        {
          elementInfo->size = 8;
        }
        #if STORE_EXTRA_MEMORY_STATS == 1
        newAllocation = 1;
        #endif
        /*  if(listIterator == 1)
          {
          printf("alloc %X s%d\r\n",elementInfo,xWantedSize);
          }*/
      }
      traceMALLOC( pvReturn, xWantedSize );
   //   printf("alloc %X s%d\r\n",elementInfo,xWantedSize);
    }
    if(fromISR == 0)
    {
      vPortExitCritical();
    }
    else
    {
      portCLEAR_INTERRUPT_MASK_FROM_ISR( uxSavedInterruptStatus );
    }

  }
  else
  {
    printf("Alloc oversize, requested %d\r\n",xWantedSize);
  }


  #if STORE_EXTRA_MEMORY_STATS == 1
  int statPos = 0;
  uint8_t foundSize = 0;
  for(int i = EXTRA_MEMORY_SIZE_DEPTH-1; i>=0; i--)
  {
    if(extraMemStats[i].size == wantedSize)
    {
      statPos = i;
      foundSize = 1;
      break;
    }
    if(extraMemStats[i].size > wantedSize)
    {
      statPos = i+1;
      break;
    }
  }

  if(statPos < EXTRA_MEMORY_SIZE_DEPTH)
  {
    if(foundSize == 0)
    {
      for(int i = EXTRA_MEMORY_SIZE_DEPTH-2; i>=statPos; i--)
      {
        extraMemStats[i+1] = extraMemStats[i];
      }
      extraMemStats[statPos].allocated = 0;
      extraMemStats[statPos].requests = 0;
      extraMemStats[statPos].size = wantedSize;
    }
    extraMemStats[statPos].allocated += newAllocation;
    extraMemStats[statPos].requests++;
  }

  #endif

#if( configUSE_MALLOC_FAILED_HOOK == 1 )
  {
    if( pvReturn == NULL )
    {
            extern void vApplicationMallocFailedHook( void );
            vApplicationMallocFailedHook();
    }

  }
#endif
  /*printf("alloc %X %d\n",pvReturn,xWantedSize);*/
  return (void*)pvReturn;
}
/*-----------------------------------------------------------*/

void vPortFree2( void *pv )
{
  /*printf("free %X \n",pv);*/
  uint8_t *puc = ( uint8_t * ) pv;
  uint8_t listIterator=0;
  struct elementInfo_st* elementInfo;
  UBaseType_t uxSavedInterruptStatus;
  uint8_t formISR;

  if( puc != NULL )
  {
    puc -= sizeof(struct elementInfo_st); /*restore header*/
    elementInfo = (struct elementInfo_st*) (puc);

    while(allowedSizes[listIterator]<(elementInfo->size-sizeof(struct elementInfo_st)))
    {
      listIterator++;
      if(listIterator>= NO_OF_LISTS)
      {
        break;
      }
    }
    
    if((portNVIC_INT_CTRL_REG & 0x1F)==0)
    {
      vPortEnterCritical();
      formISR = 0;
    }
    else
    {
      uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
      formISR = 1;
       memData.errorCode = 3;
      while(1);
    }

    if((elementInfo->bitMap &0x01) == 0)
    {
      if( elementInfo->bitMap &0x02)
      {
        memData.errorCode = 1;
      }
      else
      {
        memData.errorCode = 2;
      }
      while(1);
    }

    if(listIterator < NO_OF_LISTS)
    {
      elementInfo->next = memData.memoryList[listIterator].first;
      elementInfo->bitMap =0;
      memData.memoryList[listIterator].first = elementInfo;      
      memData.memoryList[listIterator].noOfFree++;
      memData.freeSpace += elementInfo->size;
    }
    if(formISR == 0)
    {
      vPortExitCritical();
    }
    else
    {
      portCLEAR_INTERRUPT_MASK_FROM_ISR( uxSavedInterruptStatus );
    }
//printf("free %X, s %d\r\n",elementInfo,elementInfo->size);
  
  }
}
/*-----------------------------------------------------------*/

size_t xPortGetFreeHeapSize( void )
{
	return memData.freeSpace;
}
/*-----------------------------------------------------------*/

size_t xPortGetMinimumEverFreeHeapSize( void )
{
	return memData.minFreeSpace;
}

size_t xPortGetUnallocatedHeapSize( void )
{
	return memData.unallocatedSpace;
}
size_t xPortGetPrimaryUnallocatedHeapSize( void )
{
	return memData.primaryUnallocatedSpace;
}
/*-----------------------------------------------------------*/
void xPortGetHeapListStats(uint16_t* noOfAllocated_p, uint16_t* noOfFree_p,uint16_t* size,uint8_t listIndex)
{
  if(listIndex<NO_OF_LISTS)
  {
    *noOfAllocated_p = memData.memoryList[listIndex].noOfAllocated;
    *noOfFree_p = memData.memoryList[listIndex].noOfFree;
    *size = allowedSizes[listIndex];
  }
  else
  {
    *noOfAllocated_p = 0;
    *noOfFree_p = 0;
    *size = 0;
  }
}

#if STORE_EXTRA_MEMORY_STATS == 1

void xPortGetHeapLargestRequests(uint16_t* noOfRequests, uint16_t* noOfAllocated, uint16_t* size, uint8_t idx)
{ 
  if(idx < EXTRA_MEMORY_SIZE_DEPTH)
  {
    *noOfRequests = extraMemStats[idx].requests;
    *noOfAllocated = extraMemStats[idx].allocated;
    *size = extraMemStats[idx].size;
  }
  else
  {
    *noOfRequests = 0;
    *noOfAllocated = 0;
    *size = 0;
  }
  

}


#endif

/*-----------------------------------------------------------*/

void vPortDefineHeapRegions( const HeapRegion_t * const pxHeapRegions  )
{  
  HeapRegion_t *pxHeapRegion;
  uint8_t i;

  //printf("size= %u\r\n",sizeof(struct memData_st));
  /* Can only call once! */
  configASSERT( memoryInitialized == 0 );

  memData.freeSpace = 0;
  memData.minFreeSpace = 0;
  memData.unallocatedSpace = 0;
  memData.primaryUnallocatedSpace = 0;
  memData.errorCode = 0;

  memData.heapRegions = pxHeapRegions;
  for(i=0;i<NO_OF_LISTS;i++)
  {
    memData.memoryList[i].first = NULL;
    memData.memoryList[i].noOfFree = 0;
    memData.memoryList[i].noOfAllocated = 0;
  }

  i=0;

  memData.primaryUnallocatedSpace = memData.heapRegions[0].xSizeInBytes;;

  while((memData.heapRegions[i].pucStartAddress != NULL))
  {
    memData.freeSpace += memData.heapRegions[i].xSizeInBytes;
    i++;
  }
  memData.minFreeSpace = memData.freeSpace;
  memData.unallocatedSpace = memData.freeSpace;

  #if STORE_EXTRA_MEMORY_STATS == 1
  memset(extraMemStats,0,sizeof(struct ExtraMemStats_st) * EXTRA_MEMORY_SIZE_DEPTH);
  #endif
  memoryInitialized = 1;
}


