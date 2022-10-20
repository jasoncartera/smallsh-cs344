#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include "smallsh.h"
/*
 * Defines a simple singlely linked list to keep track of background pid
 * Did this to avoid resizing an array everytime I remove a background pid
 * Adapted structure from my CS261 notes
 */

// Inserts a node to the list
void insert_node(pid_t val, node *head) {
  node *current = head;
  while (current->next != NULL){
    current = current->next;
  }
  
  struct node *new_node = (node *) malloc(sizeof(node));
  current->next = new_node;
  new_node->val = val;
  new_node->next = NULL; 
}

// Removes a node from the list and frees memory
void remove_node(node *head, int val) {
  node *current = head;
  while (current->next->val != val) {
    current = current->next;
  }
  current->next = current->next->next;
}

