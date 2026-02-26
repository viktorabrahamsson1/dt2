#ifndef CONCURRENCY_H
#define CONCURRENCY_H

#include "./kernel_functions.h"

uint task_1_deadline;
uint task_2_deadline;
uint task_3_deadline;
uint task_4_deadline;

// Auxiliary funktioner
void setup(void);
void turn_on_led(int index);
void turn_off_led(int index);
void flash_led(int index);
void compute_primes(void);

// Task funktioner
void task_1(void);
void task_2(void);
void task_3(void);
void task_4(void);

#endif