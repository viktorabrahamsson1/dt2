#include "../includes/mailbox.h"
#include "../includes/kernel_functions.h"

mailbox *create_mailbox(uint nMessages, uint nDataSize)
{
  mailbox *mailbox_p = calloc(1, sizeof(mailbox));

  if (!mailbox_p)
  {
    return NULL;
  }

  mailbox_p->nDataSize = nDataSize;
  mailbox_p->nMessages = nMessages;
  mailbox_p->pHead = NULL;
  mailbox_p->pTail = NULL;
  mailbox_p->nBlockedMsg = 0;
  mailbox_p->nMessages = 0;

  return mailbox_p;
}

exception remove_mailbox(mailbox *mBox)
{

  if (mBox->pHead || mBox->pTail)
  {
    return NOT_EMPTY;
  }

  free(mBox);
  return OK;
}

exception send_wait(mailbox *mBox, void *pData)
{
  isr_off();
}