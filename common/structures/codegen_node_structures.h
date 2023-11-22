/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 * @author Adam Månsson
 *
 * code generation macros for node based structures like stacks and queues
 *==========================================================*/

#ifndef _CODEGEN_H_
#define _CODEGEN_H_

#include <stdlib.h>
#include <stdio.h>
/**
 * value_type: the type parameter
 * node_name: the name of the structure typedef
 * postfix: a postfix used in function names
 *df
 * stack: a lifo structure, simpler than queue
 * pop: returns and removes the most recently aded value_type from the stack
 * push: appends a value to the stack
 *
 * queue: a lifo structure, simpler than queue
 * enqueue: appends a value_type to the end of the queue
 * dequeue: returns and removes the first value_type of the queue
 */
#define CODEGEN_QUEUE(value_type, node_name, postfix)					\
typedef struct node_name##_s { value_type val; struct node_name##_s *next; } node_name;	\
static node_name *prefix_name##_new_queue_node_(value_type val)				\
{											\
	node_name *res;									\
	res = malloc(sizeof(node_name));						\
	res->val = val;									\
	res->next = NULL;								\
	return res;									\
}											\
void enqueue##postfix(node_name **n, value_type val)					\
{											\
	node_name *head, *new;								\
	if (*n == NULL) { /* empty queue */						\
		*n = prefix_name##_new_queue_node_(val);				\
		(*n)->next = *n;							\
		return;									\
	}										\
	if ((*n)->next == *n) { /* single element queue */				\
		head = *n;								\
		new = prefix_name##_new_queue_node_(val);				\
		head->next = new;							\
		new->next = head;							\
		*n = new;								\
		return;									\
	}										\
	head = (*n)->next;								\
	new = prefix_name##_new_queue_node_(val);					\
	new->next = head;								\
	(*n)->next = new;								\
	*n = new;									\
}											\
value_type dequeue##postfix(node_name **n)						\
{											\
	value_type val;									\
	node_name *new_head;								\
	if (*n == NULL) { /* empty queue */						\
		fprintf(stderr, "Tried getting from empty queue, %s :%i\n",		\
			__FILE__, __LINE__);						\
		exit(-1);								\
	}										\
	if ((*n)->next == *n) { /* single element queue */				\
		val = (*n)->val;							\
		free(*n);								\
		*n = NULL;								\
		return val;								\
	}										\
	val = (*n)->next->val;								\
	new_head = (*n)->next->next;							\
	free((*n)->next);								\
	(*n)->next = new_head;								\
	return val;									\
}

#define CODEGEN_STACK(value_type, node_name, postfix)					\
typedef struct node_name##_s { value_type val; struct node_name##_s *next; } node_name;	\
static node_name *prefix_name##_new_stack_node_(value_type val)				\
{											\
	node_name *res;									\
	res = malloc(sizeof(node_name));						\
	res->val = val;									\
	res->next = NULL;								\
	return res;									\
}											\
void push##postfix(node_name **n, value_type val)					\
{											\
	node_name *head;								\
	head = prefix_name##_new_stack_node_(val);					\
	head->next = *n;								\
	*n = head;									\
}											\
value_type pop##postfix(node_name **n)							\
{											\
	node_name *head;								\
	value_type res;									\
	if (*n == NULL) { /* empty stack */						\
		fprintf(stderr, "Tried poping empty stack, %s :%i\n",			\
			__FILE__, __LINE__);						\
		exit(-1);								\
	}										\
	head = *n;									\
	*n = head->next;								\
	res = head->val;								\
	free(head);									\
	return res;									\
}

#define CODEGEN_STATIC_QUEUE(value_type, node_name, postfix)				\
typedef struct node_name##_s { value_type val; struct node_name##_s *next; } node_name;	\
static node_name *prefix_name##_new_queue_node_(value_type val)				\
{											\
	node_name *res;									\
	res = malloc(sizeof(node_name));						\
	res->val = val;									\
	res->next = NULL;								\
	return res;									\
}											\
static void enqueue##postfix(node_name **n, value_type val)				\
{											\
	node_name *head, *new;								\
	if (*n == NULL) { /* empty queue */						\
		*n = prefix_name##_new_queue_node_(val);				\
		(*n)->next = *n;							\
		return;									\
	}										\
	if ((*n)->next == *n) { /* single element queue */				\
		head = *n;								\
		new = prefix_name##_new_queue_node_(val);				\
		head->next = new;							\
		new->next = head;							\
		*n = new;								\
		return;									\
	}										\
	head = (*n)->next;								\
	new = prefix_name##_new_queue_node_(val);					\
	new->next = head;								\
	(*n)->next = new;								\
	*n = new;									\
}											\
static value_type dequeue##postfix(node_name **n)					\
{											\
	value_type val;									\
	node_name *new_head;								\
	if (*n == NULL) { /* empty queue */						\
		fprintf(stderr, "Tried getting from empty queue, %s :%i\n",		\
			__FILE__, __LINE__);						\
		exit(-1);								\
	}										\
	if ((*n)->next == *n) { /* single element queue */				\
		val = (*n)->val;							\
		free(*n);								\
		*n = NULL;								\
		return val;								\
	}										\
	val = (*n)->next->val;								\
	new_head = (*n)->next->next;							\
	free((*n)->next);								\
	(*n)->next = new_head;								\
	return val;									\
}

#define CODEGEN_STATIC_STACK(value_type, node_name, postfix)				\
typedef struct node_name##_s { value_type val; struct node_name##_s *next; } node_name;	\
static node_name *prefix_name##_new_stack_node_(value_type val)				\
{											\
	node_name *res;									\
	res = malloc(sizeof(node_name));						\
	res->val = val;									\
	res->next = NULL;								\
	return res;									\
}											\
static void push##postfix(node_name **n, value_type val)				\
{											\
	node_name *head;								\
	head = prefix_name##_new_stack_node_(val);					\
	head->next = *n;								\
	*n = head;									\
}											\
static value_type pop##postfix(node_name **n)						\
{											\
	node_name *head;								\
	value_type res;									\
	if (*n == NULL) { /* empty stack */						\
		fprintf(stderr, "Tried poping empty stack, %s :%i\n",			\
			__FILE__, __LINE__);						\
		exit(-1);								\
	}										\
	head = *n;									\
	*n = head->next;								\
	res = head->val;								\
	free(head);									\
	return res;									\
}
#endif /* _CODEGEN_H_ */