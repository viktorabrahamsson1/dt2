#include <stdio.h>
#include <stdlib.h>
#include "../includes/kernel_functions.h"

#define PASS 1
#define FAIL 0

int create_mailbox_zero_test()
{

  mailbox *mBox = create_mailbox(0, 0);
  if (mBox != NULL)
    return FAIL;

  return PASS;
}

int create_mailbox_test()
{
  uint nMessages = 3;
  uint nDataSize = 4;

  mailbox *mBox = create_mailbox(nMessages, nDataSize);
  if (mBox == NULL)
    return FAIL;

  if (mBox->nMaxMessages != nMessages)
    return FAIL;
  if (mBox->nDataSize != nDataSize)
    return FAIL;
  if (mBox->pHead != NULL)
    return FAIL;
  if (mBox->pTail != NULL)
    return FAIL;

  return PASS;
}

int remove_mailbox_test(void)
{
  mailbox *mBox = create_mailbox(2, 4);
  exception ret = remove_mailbox(mBox);

  if (ret == FAIL)
    return FAIL;
  if (ret == NOT_EMPTY)
    return FAIL;

  return PASS;
}

int main(void)
{
  int g1 = create_mailbox_zero_test();
  if (g1 == FAIL)
  {
    while (1)
    {
      // No use in going further
    }
  }

  int g2 = create_mailbox_test();
  if (g2 == FAIL)
  {
    while (1)
    {
      // No use in going further
    }
  }

  int g3 = remove_mailbox_test();
  if (g3 == FAIL)
  {
    while (1)
    {
      // No use in going further
    }
  }
}
