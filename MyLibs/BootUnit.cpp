 #include <stdio.h>
#include <stdlib.h>
#include <cstring>

#include "BootUnit.hpp"
#include "Ethernet.hpp"

#include "EthernetTcp.hpp"

#include "FileSystem.hpp"

#ifdef __cplusplus
 extern "C" {
#endif

extern void JumpToApplication(unsigned int);

#ifdef __cplusplus
}
#endif

//#pragma DATA_SECTION(VersionName, ".dataInfo")
//const char __attribute((section(".ARM.__at_0x080A0000"))) VersionName[] = "VER 0.2 06-11-2023 13:21"; 
const char __attribute((section(".dataInfo"))) VersionName[] = "VER 0.4 06-11-2023 13:21"; 


BootUnit_c::BootUnit_c(void): comBoot(this)
{
  


}

void BootUnit_c::Init(RTC_HandleTypeDef* hrtc_)
{
  if(hrtc_ != nullptr)
  {
    hrtc = hrtc_;
  }
  else
  {
    hrtc = new RTC_HandleTypeDef;
    __HAL_RCC_PWR_CLK_ENABLE();
    memset(hrtc,0,sizeof(RTC_HandleTypeDef));
    hrtc->Instance = RTC;
  }

}


comResp_et Com_boot::Handle(CommandData_st* comData_)
{
  
  int actionIdx = FetchParameterString(comData_,"ACTION");

  if(actionIdx == -1)
  {
    return COMRESP_NOPARAM;
  }
  if(strcmp(comData_->buffer+comData_->argValPos[actionIdx],"INFO") == 0)
  {
    char* buffer = new char[512];
    bootUnit->PrintStatus(buffer);
    Print(comData_->commandHandler,buffer);
    delete[] buffer;
    return COMRESP_OK;
  }
  else if(strcmp(comData_->buffer+comData_->argValPos[actionIdx],"REBOOT") == 0)
  {
    bootUnit->Reboot();
    return COMRESP_OK;
  }
  else if(strcmp(comData_->buffer+comData_->argValPos[actionIdx],"LOAD") == 0)
  {
    int fileIdx =  FetchParameterString(comData_,"FILE");
    if(fileIdx == -1)
    {
      return COMRESP_NOPARAM;
    }

    if(bootUnit->WriteToPassiveFlash(comData_->buffer+comData_->argValPos[fileIdx]) == true)
    {
      return COMRESP_OK;
    }
    else
    {
      return COMRESP_VALUE;
    }
  }
  else if(strcmp(comData_->buffer+comData_->argValPos[actionIdx],"SWAP") == 0)
  {
    bootUnit->SwapPassiveSide();
    return COMRESP_OK;
  }
  else if(strcmp(comData_->buffer+comData_->argValPos[actionIdx],"CONFIRM") == 0)
  {
    bootUnit->ConfirmActSide();
    return COMRESP_OK;
  }
  else
  {
    return COMRESP_VALUE;
  }
}

void BootUnit_c::PrintStatus(char* buffer)
{

 strcpy(buffer,"VERSIONS: \n");


 if(VersionName == (char*)APPLICATION1_FLASH )
 {
   strcat(buffer," CURR: APPL1\n");
 }
 else if(VersionName == (char*)APPLICATION2_FLASH )
 {
   strcat(buffer," CURR: APPL2\n");
 }

 strcat(buffer," APPL1: ");
 strncat(buffer,(char*) APPLICATION1_FLASH ,32);
 strcat(buffer,"\n");

 strcat(buffer," APPL2: ");
 strncat(buffer,(char*) APPLICATION2_FLASH ,32);
 strcat(buffer,"\n");


}
#define FILE_BUFFER_SIZE 512

bool BootUnit_c::WriteToPassiveFlash(char* fileName)
{
  uint32_t flashAdr;
  uint32_t firstSector;
  if(VersionName == (char*)APPLICATION1_FLASH)
  {
    flashAdr = APPLICATION2_FLASH;
    firstSector = APPLICATION2_SECTOR;
    
  }
  else if(VersionName == (char*)APPLICATION2_FLASH)
  {
    flashAdr = APPLICATION1_FLASH;
    firstSector = APPLICATION1_SECTOR;
  }
  else
  {
    return false;
  }

  char* fileNamePatch = new char[ strlen(fileName)+8];
  strcpy(fileNamePatch,"/BOOT/");
  strcat(fileNamePatch,fileName);

  File_c * file = FileSystem_c::OpenFile( fileNamePatch,"r");
  delete [] fileNamePatch;

  if(file != nullptr)
  {
    uint32_t fileSize = file->GetSize();
    bool result = false;
    HAL_FLASH_Unlock();
    /* erase sectors */
    uint8_t sectorsCount = 1 + fileSize/(128*1024);

    FLASH_EraseInitTypeDef pEraseInit;
    pEraseInit.Banks = 1;
    pEraseInit.NbSectors = sectorsCount;
    pEraseInit.Sector = firstSector;
    pEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
    pEraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    
    uint32_t sectorError;
    HAL_FLASHEx_Erase(&pEraseInit, &sectorError);

    if(sectorError == 0xFFFFFFFF)
    {
      /* program flash */
      uint32_t* fileBuffer = new uint32_t[FILE_BUFFER_SIZE/4];
      uint32_t bytesWritten = 0;
      uint32_t bufferReadIdx = FILE_BUFFER_SIZE;

      result = true;
      while(bytesWritten < fileSize)
      { 
        if(bufferReadIdx >= FILE_BUFFER_SIZE)
        {
           file->Read((uint8_t*)fileBuffer,FILE_BUFFER_SIZE);
           bufferReadIdx = 0;
        }
        HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,flashAdr+bytesWritten,fileBuffer[bufferReadIdx/4]);

        if(status != HAL_OK)
        {
          result = false;
          break;
        }
        bufferReadIdx +=4;
        bytesWritten +=4;
      }

      delete[] fileBuffer;      
    }

    HAL_FLASH_Lock();
    file->Close();
    return result;
  }
  else
  {
    return false;
  }
}
void BootUnit_c::SwapPassiveSide(void)
{
  uint8_t bootSector;
  if(VersionName == (char*)APPLICATION1_FLASH)
  {
    bootSector = 1;
    
  }
  else if(VersionName == (char*)APPLICATION2_FLASH)
  {
    bootSector = 0;
  }
  else
  {
    return;
  }

  uint32_t newOrder = 0xBEAF0000 | bootSector | 0x0100;
  HAL_PWR_EnableBkUpAccess();
  HAL_RTCEx_BKUPWrite(hrtc,0,newOrder);
  HAL_PWR_DisableBkUpAccess();
}
void BootUnit_c::ConfirmActSide(void)
{
  uint8_t bootSector;
  if(VersionName == (char*)APPLICATION1_FLASH)
  {
    bootSector = 0;
    
  }
  else if(VersionName == (char*)APPLICATION2_FLASH)
  {
    bootSector = 1;
  }
  else
  {
    return;
  }

  uint32_t newOrder = 0xBEAF0000 | bootSector;

  HAL_PWR_EnableBkUpAccess();
  HAL_RTCEx_BKUPWrite(hrtc,0,newOrder);
  HAL_PWR_DisableBkUpAccess();
}

void vDelayedRestart( TimerHandle_t xTimer )
{
  unsigned int VectorTableAddress;
  unsigned int* p;
  p = (unsigned int*)(0x08000000);

  VectorTableAddress = 0x08000000;

  taskDISABLE_INTERRUPTS();
  vTaskSuspendAll();


  JumpToApplication(VectorTableAddress);
}

void BootUnit_c::Reboot(void)
{
  /* disconnect all sockets */

  Socket_c* socket = Socket_c::GetSocket(23,IP_PROTOCOL_TCP);
  if(socket != nullptr)
  {
    ((SocketTcp_c*) socket) ->DisconnectAllChild();
  }

  /*delayed restart, switch to another task to enable disconnect pending connection */
  TimerHandle_t timer = xTimerCreate("",pdMS_TO_TICKS(500),pdTRUE,( void * ) 0,vDelayedRestart);
  xTimerStart(timer,0);

}