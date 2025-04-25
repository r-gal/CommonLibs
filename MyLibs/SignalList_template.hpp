#ifndef SIGNALLIST_C
#define SIGNALLIST_C

class signalList_c
{

  public: enum HANDLERS_et
  {
    HANDLE_CTRL,
    HANDLE_NO_OF

  };


  public: enum SIGNO_et
  {
    SIGNO_TEST = 0
  };
};

#include "commonSignal.hpp"


class testSig_c : public sig_c
{
  public:
  testSig_c(void) : sig_c(SIGNO_TEST,HANDLE_CTRL) {}

};


#endif