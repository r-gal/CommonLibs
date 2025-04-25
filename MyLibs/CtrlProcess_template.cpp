 #include <stdio.h>
#include <stdlib.h>
#include <cstring>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"


#include "CtrlProcess.hpp"

/*
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rtc.h"
#include "stm32f4xx_dma.h"
#include "misc.h"

*/


void vFunction100msTimerCallback( TimerHandle_t xTimer )
{
  //dispActTimSig.SendISR();

}

ctrlProcess_c::ctrlProcess_c(uint16_t stackSize, uint8_t priority, uint8_t queueSize, HANDLERS_et procId) : process_c(stackSize,priority,queueSize,procId,"CONFIG")
{
/*
  TimerHandle_t timer = xTimerCreate("",500,pdTRUE,( void * ) 0,vFunction100msTimerCallback);
  xTimerStart(timer,0);
*/

}

void ctrlProcess_c::main(void)
{

   printf("Ctrl proc started \n");

  while(1)
  {
    releaseSig = true;
    RecSig();
    uint8_t sigNo = recSig_p->GetSigNo();
    switch(sigNo)
    {
       
      default:
      break;

    }
    if(releaseSig) { delete  recSig_p; }
 
  }


}
