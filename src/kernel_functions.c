#include "../includes/kernel_functions.h"
#include "../includes/globals.h"
#include "../includes/linked_list.h"

// Initiera kernel och initiiera listor
exception init_kernel(void)
{
  Ticks = 0;
  ReadyList = init_list();
  TimerList = init_list();
  WaitingList = init_list();

  // kolla om listorna har en adress
  if (!ReadyList || !WaitingList || !TimerList)
  {
    return FAIL;
  }

  KernelMode = INIT;
  exception idle_status = create_task(idle_task, UINT_MAX);

  // kolla om idle task failar
  if (!idle_status)
  {
    return FAIL;
  }

  return OK;
}

exception create_task(void (*task_body)(), uint deadline)
{
  TCB *new_tcb;
  new_tcb = (TCB *)calloc(1, sizeof(TCB));

  // Kolla om calloc retunerade en address
  if (new_tcb == NULL)
  {
    return FAIL;
  }

  new_tcb->PC = task_body;
  new_tcb->SPSR = 0x21000000;
  new_tcb->Deadline = deadline;

  new_tcb->StackSeg[STACK_SIZE - 2] = 0x21000000;
  new_tcb->StackSeg[STACK_SIZE - 3] = (unsigned int)task_body;
  new_tcb->SP = &(new_tcb->StackSeg[STACK_SIZE - 9]);

  // skapa ett list objekt
  listobj *pNewNode = create_listobj(new_tcb);

  // Check if list node allocation failed
  if (pNewNode == NULL)
  {
    free(new_tcb); // Free the previously allocated TCB
    return FAIL;   // Return failure status
  }

  // Check if the kernel is currently in "start-up" mode.
  if (KernelMode == INIT)
  {
    // lägger till nya tasken till ReadyList
    insert_sorted(ReadyList, pNewNode);
  }
  // KernelMode == RUNNING
  else
  {
    // Disable interrupts to protect list manipulation
    isr_off();

    // Save the task that is currently running (at head)
    PreviousTask = ReadyList->pHead->pTask;

    // Insert the new task into the sorted (priority queue) ReadyList
    insert_sorted(ReadyList, pNewNode);

    // Check which task is now at the head of the list
    NextTask = ReadyList->pHead->pTask;

    // If new task has a smaller deadline (is now Head) perform context switch
    if (NextTask != PreviousTask)
    {
      // Automatically re-enables interrupts inside the context switch
      SwitchContext();
    }
    else
    {
      // Re-enable interrupts after list manipulation
      isr_on();
    }
  }

  // Return success status
  return OK;
}
void terminate(void)
{

  listobj *leavingObj;
  isr_off();
  leavingObj = extract(ReadyList, ReadyList->pHead);
  NextTask = ReadyList->pHead->pTask;
  switch_to_stack_of_next_task();

  free(leavingObj->pTask);
  free(leavingObj);
  LoadContext_In_Terminate();
}
void run(void)
{
  Ticks = 0;
  KernelMode = RUNNING;
  NextTask = ReadyList->pHead->pTask;
  LoadContext_In_Run();
}

void idle_task(void)
{
  while (1)
  {
    __asm__("nop");
  }
}