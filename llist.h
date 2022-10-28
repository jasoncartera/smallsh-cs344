#ifndef LLIST
#define LLIST

// define linked list node
typedef struct node {
    pid_t val;
    struct node *next;
  } node;

  // Linked list methods for keeping track of background PIDs
void insert_node(pid_t val, node **head);

void remove_node(node **head, pid_t val);

#endif 

