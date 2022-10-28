#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include "smallsh.h"
#include "llist.h"

/*
 * Defines a simple singlely linked list to keep track of background pid
 * Did this to avoid resizing an array everytime I remove a background pid
 * Adapted structure from my CS261 notes and followed 
 * https://www.learn-c.org/en/Linked_lists for guidance on C,
 * paticularly on how to properly free memory
 */


// Inserts a node to the front of the list
void insert_node(pid_t val, node **head) {
  // Create new node
  node *new_node = (node*) malloc(sizeof(node));
  new_node->val = val;
  new_node->next = NULL;
  if (*head == NULL) {
    *head = new_node;
  } else {
    node *tmp = *head;
    while (tmp->next != NULL) {
      tmp = tmp->next;
    }
    tmp->next = new_node;
  }
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

