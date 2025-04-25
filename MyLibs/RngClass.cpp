 #include <stdio.h>
#include <stdlib.h>
#include <cstring>

#include "RngClass.hpp"

RNG_HandleTypeDef RngUnit_c::hrng;
void RngUnit_c::Init(void)
{
  __HAL_RCC_RNG_CLK_ENABLE();
  memset(&hrng,0,sizeof(hrng));
  hrng.Instance = RNG;
  HAL_RNG_Init(&hrng);
}
uint32_t RngUnit_c::GetRandomVal(void)
{
  uint32_t value;
  HAL_StatusTypeDef res = HAL_RNG_GenerateRandomNumber(&hrng, &value);
  if(res != HAL_OK)
  {
    printf(" Rng error\n");
  }
  return value;
}
