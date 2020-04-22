#include "queue.h"

void inc_index(int *index) {
  *index += 1;
  *index %= MAX_NODES;
}

void dec_index(int *index) {
  *index -= 1;
  *index %= MAX_NODES;
}

void Node_init(Node *n, int i) {
  n->id = i;
}

void Queue_init(Queue *q) {
  q->front = -1;
  q->rear  = -1;
  q->pivot = -1;
}

int Queue_add(Queue *q, Node *node) {
  
  if((q->front == 0 && q->rear == FINAL_NODE) ||
     (q->front == (q->rear+1))) {
    return QUEUE_ERR_FULL;
  }

  if(q->front == -1) {
    q->front = 0;
    q->rear  = 0;
    q->pivot = 0;
  }
  else {
    inc_index(&q->rear);
  }
  
  q->list[q->rear] = node;
    
  return QUEUE_OK;
}

int Queue_delete(Queue *q, Node *node) {
  if(q->front == -1) {
    return QUEUE_ERR_EMPTY;
  }

  if(q->front == q->rear) {
    if(node == q->list[q->front]) {
      q->list[q->front] = NULL;
      q->front = -1;
      q->rear  = -1;
      q->pivot = -1;
      return QUEUE_OK;
    }
    else {
      return QUEUE_ERR_NOT_FOUND;
    }
  }
  
  int found = -1;
  for(int i = q->front; i != q->rear; i = (i+1) % MAX_NODES) {
    if (node == q->list[i])
      found = 1;
  }
  if(found > 0) {
    for(int src = (found-1) % MAX_NODES, dst = found;
	src != q->front; dec_index(&dst), dec_index(&src)) {
      q->list[dst] = q->list[src];
    }
    q->list[q->front] = NULL;
    if (q->pivot = q->front) {
      inc_index(&q->pivot);
    }
    inc_index(&q->front);
    return QUEUE_OK;
  }
  else {
    return QUEUE_ERR_NOT_FOUND;
  }
}

Node *Queue_next(Queue *q) {
  Node *node = NULL;
  if (q->pivot != -1) {
    node = q->list[q->pivot];
    q->pivot = (q->pivot == FINAL_NODE) ? 0 : (q->pivot+1);
  }
  return node;
}
