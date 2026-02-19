#include "../includes/mailbox.h"
#include "../includes/kernel_functions.h"
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
  isr_off();
}

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

  x->pPrevious = m->pTail;
  m->pTail->pNext = x;
  m->pTail = x;
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
    return NULL;

  if (m->pHead == x)
  {
    pop_head(m);
    return;
  }
  if (m->pTail == x)
  {
    msg *temp = m->pTail;
    m->pTail = m->pTail->pPrevious;
    m->pTail->pNext = NULL;
    temp->pPrevious = NULL;
    return;
  }

  x->pNext->pPrevious = x->pPrevious;
  x->pPrevious->pNext = x->pNext;
  x->pPrevious = NULL;
  x->pNext = NULL;
}
