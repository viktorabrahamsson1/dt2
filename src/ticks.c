#include "../includes/ticks.h"
#include "../includes/kernel_functions.h"
#include "../includes/globals.h"
#include <stdint.h>

exception wait(uint nTicks)
{
  uint start = Ticks;
  while ((Ticks - start) < nTicks)
  {
    ;
  }
}

void set_ticks(uint nTicks)
{
  Ticks = nTicks;
}

uint ticks(void)
{
  uint32_t t = Ticks;
  return t;
}

uint deadline(void)
{
  return ReadyList->pHead->pTask->Deadline;
}

void set_deadline(uint deadline)
{
  isr_off();
}

void TimerInt(void)
{
}