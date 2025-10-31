/*
 * Copyright (C) 2025.  Yasuhiro Ohara <yasu1976@gmail.com>
 */

#ifndef _QUEUE_H_
#define _QUEUE_H_

#define QUEUE_DEBUG     0
#define QUEUE_INIT_SIZE 2

struct queue
{
  void **array;
  int array_size;
  int start;
  int end;
};

struct queue *queue_create ();
void queue_delete (struct queue *queue);
int queue_size (struct queue *queue);

void queue_enqueue (struct queue *queue, void *data);
void *queue_dequeue (struct queue *queue);

void queue_print (struct queue *queue);

#endif /*_QUEUE_H_*/

