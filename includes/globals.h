#ifndef GLOBALS_H
#define GLOBALS_H

#include "kernel_functions.h"

extern volatile uint Ticks;
extern int KernelMode;
extern TCB *PreviousTask, *NextTask;
extern list *ReadyList, *WaitingList, *TimerList;

#endif