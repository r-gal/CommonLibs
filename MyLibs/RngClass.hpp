

#ifndef RNG_CLASS_C
#define RNG_CLASS_C

#include "SignalList.hpp"
#include "CommonDef.hpp"


class RngUnit_c
{
  static RNG_HandleTypeDef hrng;

  public:
  void Init(void);
  static uint32_t GetRandomVal(void);

};



#endif