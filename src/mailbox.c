#include "../includes/mailbox.h"
#include "../includes/kernel_functions.h"
#include "../includes/linked_list.h"
#include "../includes/globals.h"

// HJÄLP FUNKTIONER
static void push_tail(mailbox *m, msg *x);
static msg *pop_head(mailbox *m);
static void unlink_msg(mailbox *m, msg *x);

mailbox *create_mailbox(uint nMessages, uint nDataSize)
{
  mailbox *mailbox_p = calloc(1, sizeof(mailbox));

  if (nMessages == 0 || nDataSize == 0)
    return NULL;

  if (!mailbox_p)
    return NULL;

  mailbox_p->nMaxMessages = nMessages;
  mailbox_p->nDataSize = nDataSize;
  mailbox_p->pHead = NULL;
  mailbox_p->pTail = NULL;
  mailbox_p->nBlockedMsg = 0;
  mailbox_p->nMessages = 0;
  return mailbox_p;
}

exception remove_mailbox(mailbox *mBox)
{

  if (!mBox)
    return FAIL;

  if (mBox->nMessages != 0 || mBox->nBlockedMsg != 0 || mBox->pHead != NULL)
    return NOT_EMPTY;

  free(mBox);
  return OK;
}

exception send_wait(mailbox *mBox, void *pData)
{
  if (!mBox || !pData)
    return FAIL;

  isr_off();

  listobj *current_running_task = ReadyList->pHead;

  // Fall 1: det finns en msg objekt med status RECIVER i mailboxen
  msg *current = mBox->pHead;
  while (current != NULL)
  {
    if (current->Status == RECEIVER)
    {
      memcpy(current->pData, pData, mBox->nDataSize);
      current->Status = OK;

      PreviousTask = ReadyList->pHead->pTask;

      extract(WaitingList, current->pBlock);
      extract(TimerList, current->pBlock);
      current->pBlock->pMessage = NULL;
      insert_sorted(ReadyList, current->pBlock);

      mBox->nBlockedMsg--;
      unlink_msg(mBox, current);
      mBox->nMessages--;
      free(current);

      isr_on();
      NextTask = ReadyList->pHead->pTask;
      SwitchContext();
      return OK;
    }
    current = current->pNext;
  }

  // Fall 2: det finns inte ett msg objekt med status RECIVER i mailboxen
  msg *sender_message = calloc(1, sizeof(msg));
  if (!sender_message)
  {
    isr_on();
    return FAIL;
  }

  sender_message->Status = SENDER;
  sender_message->pData = pData;
  sender_message->pBlock = current_running_task;

  mBox->nBlockedMsg++;
  push_tail(mBox, sender_message);
  mBox->nMessages++;

  PreviousTask = ReadyList->pHead->pTask;
  // Blockera nuvarande running task
  extract(ReadyList, current_running_task);
  // Sätt den nuvarande running task i waitinglist
  insert_sorted(WaitingList, current_running_task);
  // Sätt den nuvarande running task också i timerlist
  current_running_task->pMessage = sender_message;
  insert_sorted(TimerList, current_running_task);
  NextTask = ReadyList->pHead->pTask;
  isr_on();
  SwitchContext();

  if (sender_message->Status == DEADLINE_REACHED)
  {
    isr_off();
    unlink_msg(mBox, sender_message);
    mBox->nMessages--;
    mBox->nBlockedMsg--;
    free(sender_message);
    isr_on();
    return DEADLINE_REACHED;
  }

  return sender_message->Status;
}

exception recive_wait(mailbox *mBox, void *pData)
{
  if (!mBox || !pData)
    return FAIL;

  isr_off();

  listobj *current_running_task = ReadyList->pHead;

  // Fall 1, det finns en sender i mailboxen.
  msg *current = mBox->pHead;
  while (current != NULL)
  {

    if (current->Status == SENDER)
    {
      memcpy(pData, current->pData, mBox->nDataSize);
      mBox->nBlockedMsg--;

      PreviousTask = ReadyList->pHead->pTask;

      if (current->pBlock != NULL)
      {
        extract(WaitingList, current->pBlock);
        extract(TimerList, current->pBlock);
        current->pBlock->pMessage = NULL;
        insert_sorted(ReadyList, current->pBlock);
      }
      else
      {
        free(current->pData);
      }

      unlink_msg(mBox, current);
      mBox->nMessages--;
      free(current);

      NextTask = ReadyList->pHead->pTask;
      isr_on();
      SwitchContext();
      return OK;
    }

    current = current->pNext;
  }

  // Fall 2: det finns inte ett msg objekt med status SENDER i mailboxen
  msg *reciver_message = calloc(1, sizeof(msg));
  if (!reciver_message)
  {
    isr_on();
    return FAIL;
  }

  reciver_message->Status = RECEIVER;
  reciver_message->pData = pData;
  reciver_message->pBlock = current_running_task;

  mBox->nBlockedMsg++;
  push_tail(mBox, reciver_message);
  mBox->nMessages++;

  // Blockera current running task genom att ta bort den ur ready list och sätta in den i waiting list
  PreviousTask = ReadyList->pHead->pTask;
  extract(ReadyList, current_running_task);
  insert_sorted(WaitingList, current_running_task);

  // Sätt också tasken i timerlist
  current_running_task->pMessage = reciver_message;
  insert_sorted(TimerList, current_running_task);

  isr_on();
  SwitchContext();

  if (reciver_message->Status == DEADLINE_REACHED)
  {
    isr_off();
    unlink_msg(mBox, reciver_message);
    mBox->nMessages--;
    mBox->nBlockedMsg--;
    free(reciver_message);
    isr_on();
    return DEADLINE_REACHED;
  }

  return reciver_message->Status;
}

exception send_no_wait(mailbox *mBox, void *pData)
{
  if (!mBox || !pData)
    return FAIL;

  isr_off();

  // Fall 1: det finns en reciver
  msg *current = mBox->pHead;
  while (current != NULL)
  {

    if (current->Status == RECEIVER)
    {
      memcpy(current->pData, pData, mBox->nDataSize);
      current->Status = OK;

      PreviousTask = ReadyList->pHead->pTask;

      extract(WaitingList, current->pBlock);
      extract(TimerList, current->pBlock);
      current->pBlock->pMessage = NULL;
      insert_sorted(ReadyList, current->pBlock);

      mBox->nBlockedMsg--;
      unlink_msg(mBox, current);
      mBox->nMessages--;
      free(current);

      NextTask = ReadyList->pHead->pTask;
      isr_on();
      SwitchContext();
      return OK;
    }

    current = current->pNext;
  }

  // Fall 2: ingen reciver finns ingen blockning: retunera FAIL

  msg *new_msg = calloc(1, sizeof(msg));
  if (!new_msg)
    return FAIL;

  new_msg->pData = calloc(1, mBox->nDataSize);
  memcpy(new_msg->pData, pData, mBox->nDataSize);

  if (mBox->nMessages == mBox->nMaxMessages)
  {
    msg *oldest = pop_head(mBox);
    mBox->nMessages--;
    free(oldest->pData);
    free(oldest);
  }
  push_tail(mBox, new_msg);
  mBox->nMessages++;

  isr_on();
  return OK;
}

int receive_no_wait(mailbox *mBox, void *pData)
{
  if (!mBox || !pData)
    return FAIL;

  isr_off();

  msg *current = mBox->pHead;
  while (current != NULL)
  {
    if (current->Status == SENDER)
    {
      memcpy(pData, current->pData, mBox->nDataSize);

      if (current->pBlock != NULL) // send_wait typ
      {
        PreviousTask = ReadyList->pHead->pTask;
        mBox->nBlockedMsg--;
        extract(WaitingList, current->pBlock);
        extract(TimerList, current->pBlock);
        current->pBlock->pMessage = NULL;
        insert_sorted(ReadyList, current->pBlock);
        NextTask = ReadyList->pHead->pTask;
        unlink_msg(mBox, current);
        mBox->nMessages--;
        free(current);
        isr_on();
        SwitchContext();
      }
      else // send_no_wait typ
      {
        free(current->pData);
        unlink_msg(mBox, current);
        mBox->nMessages--;
        free(current);
        isr_on();
      }
      return OK;
    }
    current = current->pNext;
  }

  isr_on();
  return FAIL;
}

exception wait(uint nTicks)
{
  isr_off();
  listobj *current_running_task = ReadyList->pHead;
  current_running_task->nTCnt = Ticks + nTicks;

  PreviousTask = ReadyList->pHead->pTask;
  extract(ReadyList, current_running_task);
  insert_sorted(TimerList, current_running_task);
  NextTask = ReadyList->pHead->pTask;

  isr_on();
  SwitchContext();

  // Efter uppvakning, kolla hur tasken väcktes:
  if (Ticks >= current_running_task->pTask->Deadline)
  {
    return DEADLINE_REACHED;
  }
  return OK;
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

  ReadyList->pHead->pTask->Deadline = deadline;

  PreviousTask = ReadyList->pHead->pTask;

  // Extrahera och sätt in igen för att reschedula
  listobj *current = extract(ReadyList, ReadyList->pHead);
  insert_sorted(ReadyList, current);

  NextTask = ReadyList->pHead->pTask;

  isr_on();
  SwitchContext();
}

void TimerInt(void)
{
  Ticks++;

  // Kolla TimerList
  listobj *current = TimerList->pHead;
  while (current != NULL)
  {
    listobj *next = current->pNext;
    if (Ticks >= current->nTCnt || Ticks >= current->pTask->Deadline)
    {
      extract(TimerList, current);
      insert_sorted(ReadyList, current);
    }
    current = next;
  }

  // Kolla WaitingList
  listobj *current_wait = WaitingList->pHead;
  while (current_wait != NULL)
  {
    listobj *next = current_wait->pNext;
    if (current_wait->pTask->Deadline <= Ticks)
    {
      // Städa upp mailbox-meddelandet
      if (current_wait->pMessage != NULL)
      {
        current_wait->pMessage->Status = DEADLINE_REACHED;
        current_wait->pMessage->pBlock = NULL;
      }
      extract(WaitingList, current_wait);
      insert_sorted(ReadyList, current_wait);
    }
    current_wait = next;
  }
}

///////////////////
// HJÄLP FUNKTIONER
///////////////////

static void push_tail(mailbox *m, msg *x)
{
  if (!m || !x)
    return;

  x->pPrevious = NULL;
  x->pNext = NULL;

  if (m->pTail == NULL)
  {
    m->pHead = x;
    m->pTail = x;
  }
  else
  {
    x->pPrevious = m->pTail;
    m->pTail->pNext = x;
    m->pTail = x;
  }
}

static msg *pop_head(mailbox *m)
{
  if (!m || !m->pHead)
    return NULL;

  msg *head_msg = m->pHead;
  if (head_msg->pNext == NULL)
  {
    m->pHead = NULL;
    m->pTail = NULL;
  }
  else
  {
    m->pHead = head_msg->pNext;
    m->pHead->pPrevious = NULL;
  }

  head_msg->pNext = NULL;
  head_msg->pPrevious = NULL;
  return head_msg;
}

static void unlink_msg(mailbox *m, msg *x)
{
  if (!m || !x)
    return;

  if (x->pPrevious)
  {
    x->pPrevious->pNext = x->pNext;
  }
  else
  {
    m->pHead = x->pNext;
  }

  if (x->pNext)
  {
    x->pNext->pPrevious = x->pPrevious;
  }
  else
  {
    m->pTail = x->pPrevious;
  }

  x->pPrevious = NULL;
  x->pNext = NULL;
}
