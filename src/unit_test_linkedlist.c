#include <stdio.h>
#include <stdlib.h>
#include "../includes/linked_list.h"

#define PASS 1
#define FAIL 0

static void free_list_and_nodes(list *li)
{
  if (!li)
    return;

  listobj *cur = li->pHead;
  while (cur != NULL)
  {
    listobj *next = cur->pNext;
    free(cur);
    cur = next;
  }
  free(li);
}

int init_list_test(void)
{
  list *li = init_list();

  if (li == NULL)
    return FAIL;
  if (li->pHead != NULL)
    return FAIL;
  if (li->pTail != NULL)
    return FAIL;

  free(li);
  return PASS;
}

int create_listobj_test(void)
{
  TCB task = {0};
  task.Deadline = 42;

  listobj *node = create_listobj(&task);

  if (node == NULL)
    return FAIL;
  if (node->pTask != &task)
    return FAIL;

  if (node->pNext != NULL)
    return FAIL;
  if (node->pPrevious != NULL)
    return FAIL;

  free(node);
  return PASS;
}

int extract_test(void)
{
  list *li = init_list();
  TCB task = {0};
  listobj *node = create_listobj(&task);

  if (extract(NULL, node) != NULL)
    return FAIL;
  if (extract(li, NULL) != NULL)
    return FAIL;
  if (extract(NULL, NULL) != NULL)
    return FAIL;

  free(node);
  free(li);
  return PASS;
}

int extract_head_test(void)
{
  list *li = init_list();

  TCB t1 = {0}, t2 = {0}, t3 = {0};
  t1.Deadline = 10;
  t2.Deadline = 20;
  t3.Deadline = 30;

  listobj *n1 = create_listobj(&t1);
  listobj *n2 = create_listobj(&t2);
  listobj *n3 = create_listobj(&t3);

  insert_tail(li, n1);
  insert_tail(li, n2);
  insert_tail(li, n3);

  listobj *ret = extract(li, n1);

  if (ret != n1)
    return FAIL;
  if (li->pHead != n2)
    return FAIL;
  if (li->pTail != n3)
    return FAIL;
  if (n1->pNext != NULL || n1->pPrevious != NULL)
    return FAIL;
  if (n2->pPrevious != NULL)
    return FAIL;

  free(n1);
  free_list_and_nodes(li);
  return PASS;
}

int extract_middle_test(void)
{
  list *li = init_list();

  TCB t1 = {0}, t2 = {0}, t3 = {0};
  t1.Deadline = 10;
  t2.Deadline = 20;
  t3.Deadline = 30;

  listobj *n1 = create_listobj(&t1);
  listobj *n2 = create_listobj(&t2);
  listobj *n3 = create_listobj(&t3);

  insert_tail(li, n1);
  insert_tail(li, n2);
  insert_tail(li, n3);

  listobj *ret = extract(li, n2);

  if (ret != n2)
    return FAIL;
  if (li->pHead != n1)
    return FAIL;
  if (li->pTail != n3)
    return FAIL;
  if (n1->pNext != n3)
    return FAIL;
  if (n3->pPrevious != n1)
    return FAIL;
  if (n2->pNext != NULL || n2->pPrevious != NULL)
    return FAIL;

  free(n2);
  free_list_and_nodes(li);
  return PASS;
}

int extract_tail_test(void)
{
  list *li = init_list();

  TCB t1 = {0}, t2 = {0}, t3 = {0};
  t1.Deadline = 10;
  t2.Deadline = 20;
  t3.Deadline = 30;

  listobj *n1 = create_listobj(&t1);
  listobj *n2 = create_listobj(&t2);
  listobj *n3 = create_listobj(&t3);

  insert_tail(li, n1);
  insert_tail(li, n2);
  insert_tail(li, n3);

  listobj *ret = extract(li, n3);

  if (ret != n3)
    return FAIL;
  if (li->pHead != n1)
    return FAIL;
  if (li->pTail != n2)
    return FAIL;
  if (n2->pNext != NULL)
    return FAIL;
  if (n3->pNext != NULL || n3->pPrevious != NULL)
    return FAIL;

  free(n3);
  free_list_and_nodes(li);
  return PASS;
}

int insert_sorted_test(void)
{
  list *li = init_list();
  TCB t1 = {0}, t2 = {0}, t3 = {0};
  t1.Deadline = 1000;
  t2.Deadline = 2000;
  t3.Deadline = 3000;

  listobj *obj3 = create_listobj(&t3);
  listobj *obj2 = create_listobj(&t2);
  listobj *obj1 = create_listobj(&t1);

  insert_sorted(li, obj3);
  insert_sorted(li, obj2);
  insert_sorted(li, obj1);

  if (li->pHead != obj1)
    return FAIL;
  if (li->pHead->pNext != obj2)
    return FAIL;
  if (li->pTail != obj3)
    return FAIL;
  if (obj2->pPrevious != obj1)
    return FAIL;
  if (obj3->pPrevious != obj2)
    return FAIL;
  if (obj1->pNext != obj2)
    return FAIL;
  if (obj2->pNext != obj3)
    return FAIL;
  if (obj1->pPrevious != NULL)
    return FAIL;
  if (obj3->pNext != NULL)
    return FAIL;

  return PASS;
}

int main(void)
{
  int g1 = init_list_test();
  if (g1 == FAIL)
  {
    while (1)
    {
      // No use in going further
    }
  }

  int g2 = create_listobj_test();
  if (g2 == FAIL)
  {
    while (1)
    {
      // No use in going further
    }
  }

  int g3 = extract_test();
  if (g3 == FAIL)
  {
    while (1)
    {
      // No use in going further
    }
  }

  int g4 = extract_head_test();
  if (g4 == FAIL)
  {
    while (1)
    {
      // No use in going further
    }
  }

  int g5 = extract_middle_test();
  if (g5 == FAIL)
  {
    while (1)
    {
      // No use in going further
    }
  }

  int g6 = extract_tail_test();
  if (g6 == FAIL)
  {
    while (1)
    {
      // No use in going further
    }
  }

  int g7 = insert_sorted_test();
  if (g7 == FAIL)
  {
    while (1)
    {
      // No use in going further
    }
  }

  while (1)
  {
    // Alles gut :)
  }
}