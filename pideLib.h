#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <errno.h>

#include <pthread.h>

#include <sys/time.h>

#include <sys/types.h>

#include <signal.h>

#include <dirent.h>

#include <unistd.h>

#include <fcntl.h>

#include <sys/stat.h>

#include <netinet/in.h>

#include <sys/socket.h>

#include <arpa/inet.h>

#include <complex.h>

#include <math.h>

#include <semaphore.h>

#include <time.h>

#include <stdbool.h>

#define READ_FLAGS O_RDONLY
#define WRITE_FLAGS (O_WRONLY | O_CREAT | O_APPEND)
#define PERMS (S_IRUSR | S_IWUSR | S_IWGRP)

#define MAX_APARATUS 3
#define MAX_OPENING 2
#define MAX_PIDE 6

#define MAX_ORDER_IN_BAG 3


typedef enum {
        PLACED,
        TAKEN,
        PREPARED,
        COOKED,
        DELIVERED
}
State;

typedef struct {
        State state;
        int num;
        int p;
        int q;
}
Order;

typedef struct {
        sem_t aparatus; // [0-3]
        sem_t opening; // [0-2]
        sem_t pide; // [0-6]
}
Oven;

typedef struct {
        pthread_t manager_thread;
        pthread_cond_t task_available;
        pthread_mutex_t lock;
        bool shutdown; 
        pthread_t * threads; 
        int * thread_ids;
        int thread_count; 
        Order orders[3];
        int current_order;
        int order_size; 
	sem_t order_in_bag;
	int total_order;
}
DeliveryThreadPool;

typedef struct {
        pthread_t manager_thread;
        pthread_cond_t task_available;
        pthread_mutex_t lock; 
        Oven oven;
        bool shutdown; 
        pthread_t * threads; 
        int * thread_ids;
        int thread_count; 
        Order * orders;
        int current_order;
        int order_size; 
	int total_order;
}
CookThreadPool;
