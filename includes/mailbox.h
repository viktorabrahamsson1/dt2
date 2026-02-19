#ifndef MAILBOX_H
#define MAILBOX_H

#include "../includes/kernel_functions.h"

mailbox *create_mailbox(uint nMessages, uint nDataSize);
exception remove_mailbox(mailbox *mBox);

exception send_wait(mailbox *mBox, void *pData);
exception receive_wait(mailbox *mBox, void *pData);
exception recive_wait(mailbox *mBox, void *pData);

exception wait(uint nTicks);

void set_ticks(uint nTicks);
uint ticks(void);

uint deadline(void);
void set_deadline(uint deadline);

#endif