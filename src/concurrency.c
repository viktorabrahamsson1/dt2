
#include "../drivers/at91sam3x8.h"
#include "../drivers/system_sam3x.h"
#include "../includes/concurrency.h"
#include "../includes/kernel_functions.h"

uint task_1_deadline = 10000;
uint task_2_deadline = 10000;
uint task_3_deadline = 10000;
uint task_4_deadline = 10000;

mailbox *input_events;
bool pressed;

void setup(void)
{
  input_events = calloc(1, sizeof(mailbox));
  if (!input_events)
    return;
  *AT91C_PMC_PCER |= (1 << 14) | (1 << 11) | (1 << 13);

  // FÖR LEDS
  *AT91C_PIOC_PER = (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4);
  *AT91C_PIOC_PPUDR = (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4);
  *AT91C_PIOC_OER = (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4);

  // FÖR KNAPPAR
  // Knapp 1:
  *AT91C_PIOA_PER = (1 << 14);
  *AT91C_PIOA_PPUER = (1 << 14);
  *AT91C_PIOA_ODR = (1 << 14);

  // Knapp 2:
  *AT91C_PIOD_PER = (1 << 0);
  *AT91C_PIOD_PPUER = (1 << 0);
  *AT91C_PIOD_ODR = (1 << 0);

  // Sätt på interrupts för port A
  *AT91C_PIOA_IER = (1 << 14);
  NVIC_ClearPendingIRQ(PIOA_IRQn);
  NVIC_EnableIRQ(PIOA_IRQn);
}
void turn_on_led(int index)
{
  if (index < 1 || index > 4)
    return;

  *AT91C_PIOC_SODR = (1 << index);
}
void turn_off_led(int index)
{
  if (index < 1 || index > 4)
    return;

  *AT91C_PIOC_CODR = (1 << index);
}
void flash_led(int index)
{
  turn_on_led(index);
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

// Task funktioner

void task_1(void)
{
  while (1)
  {
    turn_on_led(1);
    compute_primes();
    int i;
    for (i = 0; i < 3; i++)
    {
      flash_led(1);
    }
    exception r = wait(8000);
    set_deadline(task_1_deadline + ticks());
  }
}

void task_2(void)
{
  while (1)
  {
    turn_on_led(2);

    if (receive_wait(input_events, &pressed) == OK)
    {
      for (int i = 0; i < 3; i++)
      {
        flash_led(2);
      }
      pressed = 1;
      exception i_ex = send_wait(input_events, &pressed);
    }
    else
    {
      turn_off_led(2);
    }

    exception r = wait(8000);
    set_deadline(task_2_deadline + ticks());
  }
}

void task_3(void)
{

  while (1)
  {
    if (receive_wait(input_events, &pressed) == OK)
    {
      turn_on_led(3);
      compute_primes();
    }
  }
  for (int i = 0; i < 3; i++)
  {
    flash_led(3);
  }

  exception r = wait(8000);
  set_deadline(task_3_deadline + ticks());
}

void task_4(void)
{
  while (1)
  {
    // kollar om button 1 trycks
    if ((*AT91C_PIOD_PDSR & (1 << 0)) == 0)
    {
      while ((*AT91C_PIOD_PDSR & (1 << 0)) == 0)
      {
        flash_led(4);
      }
    }
    turn_off_led(4);

    exception r = wait(10);
    (void)r;

    set_deadline(task_4_deadline + ticks());
  }
}

void PIOA_Handler(void)
{
  uint32_t status = *AT91C_PIOA_ISR;
  if ((*AT91C_PIOA_ISR & (1 << 14)) == (1 << 14))
  {
    pressed = 0;
    send_no_wait(input_events, &pressed);
  }
  else
  {
    pressed = 1;
  }
}

void ButtonHandler(void)
{
  if ((*AT91C_PIOA_ISR & (1 << 14)) == (1 << 14))
  {
    pressed = 0;
    send_no_wait(input_events, &pressed);
  }
}

// MAIN
int main(void)
{

  setup();
  SysTick_Config(83999);

  init_kernel();

  create_task(task_1, task_1_deadline);
  create_task(task_2, task_2_deadline);
  create_task(task_3, task_3_deadline);
  create_task(task_4, task_4_deadline);

  run();

  while (1)
  {
  }
  return 0;
}