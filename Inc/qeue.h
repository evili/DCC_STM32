#ifndef __QUEUE_H__
#define __QUEUE_H__

#define MAX_NODES  120
#define FINAL_NODE 119 // MAX_NODES-1)

#define QUEUE_OK             (0)
#define QUEUE_ERR_FULL      (-1)
#define QUEUE_ERR_EMPTY     (-2)
#define QUEUE_ERR_NOT_FOUND (-3)

typedef struct Node {
  int id;
} Node;

typedef struct Queue {
  Node *list[MAX_NODES];
  int front;
  int rear;
  int pivot ;
} Queue;

void Node_init(Node *n, int i);
void Queue_init(Queue *q);
int Queue_add(Queue *q, Node *node);
int Queue_delete(Queue *q, Node *node);
Node *Queue_next(Queue *q);

#endif // __QUEUE_H__
