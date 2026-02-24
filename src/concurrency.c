
#include "../includes/concurrency.h"
#include "../includes/kernel_functions.h"

uint task_1_deadline = 1000;
uint task_2_deadline = 1000;
uint task_3_deadline = 1000;
uint task_4_deadline = 1000;

void setup(void)
{
}
void turn_on_led(int index)
{
}
void turn_off_led(int index)
{
}
void flash_led(int index)
{
  for (int i = 0; i < 8000; i++)
  {
    for (int j = 0; j < 100; j++)
    {
      ;
    }
  }
  turn_off_led(index);
}
void compute_primes(void)
{
  volatile long long x, y, n = 4000;
  bool isprime;
  for (x = 2; x < n; x++)
  {
    isprime = 1;
    for (y = 2; y <= x; y++)
    {
      if ((x % y) == 0)
      {
        isprime = 0;
        break;
      }
    }
  }
}