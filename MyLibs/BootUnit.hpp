

#ifndef BOOT_CLASS_C
#define BOOT_CLASS_C

#include "SignalList.hpp"
#include "CommonDef.hpp"
#include "stm32f4xx.h"
#include "commandHandler.hpp"

class BootUnit_c;

class Com_boot : public Command_c
{
  BootUnit_c* bootUnit;
  public:
  char* GetComString(void) { return (char*)"boot"; }
  void PrintHelp(CommandHandler_c* commandHandler ){}
  comResp_et Handle(CommandData_st* comData_);
  Com_boot(BootUnit_c* bootUnit_) { bootUnit = bootUnit_; }

};

class BootUnit_c
{
  RTC_HandleTypeDef* hrtc;
  Com_boot comBoot;

  public:
  void Init(RTC_HandleTypeDef* hrtc_);

  BootUnit_c(void);

  void PrintStatus(char* buffer);
  bool WriteToPassiveFlash(char* fileName);
  void SwapPassiveSide(void);
  void ConfirmActSide(void);
  void Reboot(void);
  

};



#endif