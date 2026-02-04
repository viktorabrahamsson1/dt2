#include "linked_list.h"

list *init_list(void)
{
  return (list *)calloc(1, sizeof(list));
}

listobj *create_listobj(TCB *pTask)
{
  listobj *liObj = (listobj *)calloc(1, sizeof(listobj));
  if (liObj)
  {
    liObj->pTask = pTask;
    return liObj;
  }
}
listobj *extract(list *li, listobj *node)
{
  if (!li || !node)
    return NULL;

  // Om node är head: flytta head
  if (node->pPrevious == NULL)
  {
    li->pHead = node->pNext;
  }
  else
  {
    node->pPrevious->pNext = node->pNext;
  }

  // Om node är tail: flytta tail
  if (node->pNext == NULL)
  {
    li->pTail = node->pPrevious;
  }
  else
  {
    node->pNext->pPrevious = node->pPrevious;
  }

  // Om listan blev tom måste både head och tail vara NULL
  if (li->pHead == NULL)
  {
    li->pTail = NULL;
  }
  else if (li->pTail == NULL)
  {
    li->pTail = li->pHead; // defensivt, men ofta onödigt
  }

  node->pPrevious = NULL;
  node->pNext = NULL;
  return node;
}

void insert_tail(list *li, listobj *node)
{

  if (!li || !node)
  {
    return;
  }

  if (li->pHead == NULL && li->pTail == NULL)
  {
    li->pHead = node;
    li->pTail = node;
    node->pNext = NULL;
    node->pPrevious = NULL;
  }
  else
  {
    node->pPrevious = li->pTail;
    node->pNext = NULL;
    li->pTail->pNext = node;
    li->pTail = node;
  }
}

void insert_sorted(list *li, listobj *node)
{

  if (!li || !node)
  {
    return;
  }

  if (li->pHead == NULL)
  {
    li->pHead = node;
    li->pTail = node;
    node->pNext = NULL;
    node->pPrevious = NULL;
    return;
  }

  listobj *current = li->pHead;
  while (current != NULL)
  {
    if (node->pTask->Deadline <= current->pTask->Deadline)
    {
      listobj *temp = current->pPrevious;
      current->pPrevious = node;
      node->pPrevious = temp;
      node->pNext = current;
      return;
    }
    current = current->pNext;
  }

  li->pTail->pNext = node;
  node->pPrevious = li->pTail;
  node->pNext = NULL;
  li->pTail = node;
}