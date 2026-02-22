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

      extract(WaitingList, current->pBlock);
      extract(TimerList, current->pBlock);
      current->pBlock->pMessage = NULL;
      insert_sorted(ReadyList, current->pBlock);

      mBox->nBlockedMsg--;
      unlink_msg(mBox, current);
      mBox->nMessages--;
      free(current);

      isr_on();

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

  // Blockera nuvarande running task
  extract(ReadyList, current_running_task);
  // Sätt den nuvarande running task i waitinglist
  insert_sorted(WaitingList, current_running_task);
  // Sätt den nuvarande running task också i timerlist
  current_running_task->pMessage = sender_message;
  insert_sorted(TimerList, current_running_task);

  isr_on();
  SwitchContext();

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

      extract(WaitingList, current->pBlock);
      extract(TimerList, current->pBlock);
      current->pBlock->pMessage = NULL;

      insert_sorted(ReadyList, current->pBlock);

      unlink_msg(mBox, current);
      mBox->nMessages--;
      free(current);

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
  extract(ReadyList, current_running_task);
  insert_sorted(WaitingList, current_running_task);

  // Sätt också tasken i timerlist
  current_running_task->pMessage = reciver_message;
  insert_sorted(TimerList, current_running_task);

  isr_on();
  SwitchContext();

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

      extract(WaitingList, current->pBlock);
      extract(TimerList, current->pBlock);
      current->pBlock->pMessage = NULL;
      insert_sorted(ReadyList, current->pBlock);

      mBox->nBlockedMsg--;
      unlink_msg(mBox, current);
      mBox->nMessages--;
      free(current);

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

  memcpy(new_msg->pData, pData, mBox->nDataSize);

  if (mBox->nMessages == mBox->nMaxMessages)
  {
    if (mBox->pHead == NULL)
    {
      mBox->pHead = new_msg;
      mBox->pTail = new_msg;
    }
    else
    {
      new_msg->pNext = mBox->pHead->pNext;
      mBox->pHead->pNext->pPrevious = new_msg;
      mBox->pHead = new_msg;
      new_msg->pPrevious = NULL;
    }
  }
  else
  {
    push_tail(mBox, new_msg);
  }

  isr_on();
  return OK;
}

int receive_no_wait(mailbox *mBox, void *pData)
{
}

exception wait(uint nTicks)
{
  isr_off();
  listobj *current_running_task = ReadyList->pHead;
  current_running_task->nTCnt = Ticks + nTicks;
  extract(ReadyList, current_running_task);
  insert_sorted(TimerList, current_running_task);

  isr_on();
  SwitchContext();
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
