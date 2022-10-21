#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include "smallsh.h"
/*
 * Defines a simple singlely linked list to keep track of background pid
 * Did this to avoid resizing an array everytime I remove a background pid
 * Adapted structure from my CS261 notes
 */


// Inserts a node to the front of the list
void insert_node(pid_t val, node **head) {
  struct node *new_node = (node *) malloc(sizeof(node*));
  new_node->val = val;
  new_node->next = *head;
  *head = new_node;
}

// Removes a node from the list and frees memory
void remove_node(node **head, pid_t val) {
  node *current = *head;
  node *prev = *head;
  
  // if first link contains val
  if (current != NULL && current->val == val) {
    *head = current->next;
    free(current);
    return;
  }

  while (current->val != val) {
    prev = current;
    current = current->next;
  }
  prev->next = current->next;
  free(current);
}

