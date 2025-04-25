#ifndef CTRL_PROCESS_H
#define CTRL_PROCESS_H

#include "SignalList.hpp"


class ctrlProcess_c : public process_c
{
  
 
  public :

  ctrlProcess_c(uint16_t stackSize, uint8_t priority, uint8_t queueSize, HANDLERS_et procId);

  void main(void);

  

};

#endif