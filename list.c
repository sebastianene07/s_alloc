#include "list.h"

/**
 * list_sort - sort a list
 * @priv: private data, opaque to list_sort(), passed to @cmp
 * @head: the list to sort
 * @cmp: the elements comparison function
 *
 * This function implements "insertion sort", which has O(N^2)
 * complexity.
 *
 * The comparison function @cmp must return a negative value if @a
 * should sort before @b, and a positive value if @a should sort after
 * @b. If @a and @b are equivalent, and their original relative
 * ordering is to be preserved, @cmp must return 0.
 */
void list_sort(void *priv,
               struct list_head *head,
               comparator_cb cb)
{

  struct list_head *pos, *prev, *next_next;
  int ret;
  int is_order_wrong;

  if (list_empty(head) || list_length(head) == 1)
    return;

  do
  {
    /* reset the flag to indicate that the list is in order */

    is_order_wrong = 0;

    list_for_each(pos, head)
    {
      struct list_head *next_node = pos->next;
      if (next_node == head)
        break;
 
      ret = cb(priv, pos, next_node);
      if (ret > 0)
      {
        /* The current node is greater than the next_node
         * Let's move the current node at the end..
         */

        prev      = pos->prev; 
        next_next = next_node->next;

        prev->next      = next_node;
        next_node->prev = prev; 

        next_node->next = pos;
        pos->prev       = next_node;

        pos->next       = next_next;
        next_next->prev = pos;

        is_order_wrong = 1;
        break;
      }
    }
  } while (is_order_wrong);
}
