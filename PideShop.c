#include "pideLib.h"

CookThreadPool * cook_pool;

DeliveryThreadPool * delivery_pool;

int moto_speed;

int p;
int q;

sem_t log_sem;

char * logName = "PideLog.txt";

char * get_timestamp() {
    time_t now = time(NULL);
    return asctime(localtime( & now));
}

void write_to_log(char * message) {

    sem_wait( & log_sem);
    int logfile = open(logName, WRITE_FLAGS, PERMS);
    if (logfile == -1) {
        perror("Failed to open serverFifo");
        sem_post( & log_sem);
        exit(EXIT_FAILURE);
    }

    char * time = get_timestamp();

    write(logfile, time, strlen(time));
    write(logfile, message, strlen(message));
    write(logfile, "\n", 1);
    if (close(logfile) == -1) {
        perror("\nclientFifo close error\n");
        exit(EXIT_FAILURE);
    }

    sem_post( & log_sem);
}

void delivery_threadpool_shutdown(DeliveryThreadPool * pool) {
    pthread_mutex_lock( & pool -> lock);
    pool -> shutdown = true;
    pthread_mutex_unlock( & pool -> lock);

    pthread_cond_broadcast( & pool -> task_available);

    for (int i = 0; i < pool -> thread_count; ++i) {
        pthread_join(pool -> threads[i], NULL);
    }

    free(pool -> threads);

    pthread_mutex_destroy( & pool -> lock);
    pthread_cond_destroy( & pool -> task_available);
}

void calculatePseudoInverse(int row, int col) {
    double complex matrix[row][col];
    double complex pseudoInverse[col][row];

    srand(time(NULL));

    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            double realPart = ((double) rand() / RAND_MAX) * 10.0;
            double imagPart = ((double) rand() / RAND_MAX) * 10.0;

            matrix[i][j] = realPart + imagPart * I;
        }
    }

    double transposedMatrix[col][row];
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            transposedMatrix[j][i] = matrix[i][j];
        }
    }

    double product[row][row];

    for (int i = 0; i < row; i++) {
        for (int j = 0; j < row; j++) {
            for (int k = 0; k < col; k++) {
                product[i][j] += matrix[i][k] * transposedMatrix[k][j];
            }
        }
    }

    double inverse[row][row];
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < row; j++) {
            if (i == j) {
                inverse[i][j] = 1.0 / product[i][j];
            } else {
                inverse[i][j] = 0.0;
            }
        }
    }

    for (int i = 0; i < col; i++) {
        for (int j = 0; j < row; j++) {
            pseudoInverse[i][j] = 0.0;
            for (int k = 0; k < row; k++) {
                pseudoInverse[i][j] += transposedMatrix[i][k] * inverse[k][j];
            }
        }
    }
}

void handler(int signum) {

    int status;
    char log[200];

    log[0] = '\0';

    pid_t pid = getpid();

    if (signum == SIGINT) {
        printf("\n\nAll orders cancelled. Exiting...\n\n");
        sprintf(log, "All orders cancelled. Server exiting...");
        write_to_log(log);
        memset(log, '\0', sizeof(log));

        //cook_threadpool_shutdown(cook_pool);
        // delivery_threadpool_shutdown(delivery_pool);
        free(cook_pool);
        free(delivery_pool);
        sem_destroy( & log_sem);

        exit(EXIT_SUCCESS);
    }
}

void * threadpool_delivery(void * arg) {
    int thread_id = * ((int * ) arg);
    free(arg);
    while (1) {
        pthread_mutex_lock( & delivery_pool -> lock);
        while (delivery_pool -> current_order >= delivery_pool -> order_size && !delivery_pool -> shutdown) {
            pthread_cond_wait( & delivery_pool -> task_available, & delivery_pool -> lock);
        }

        if (delivery_pool -> shutdown) {
            pthread_mutex_unlock( & delivery_pool -> lock);
            pthread_exit(NULL);
        }

        int deliveryTime;

        if (delivery_pool -> current_order < delivery_pool -> order_size) {
            sem_wait( & delivery_pool -> order_in_bag);

            delivery_pool -> orders[0] = cook_pool -> orders[delivery_pool -> current_order];

            delivery_pool -> current_order++;

            printf("Moto %d put order %d in their bag\n", thread_id, delivery_pool -> current_order);

            char log[200];

            log[0] = '\0';

            sprintf(log, "Moto %d put order %d in their bag\n", thread_id, delivery_pool -> current_order);
            write_to_log(log);
            memset(log, '\0', sizeof(log));

            if (delivery_pool -> current_order < delivery_pool -> order_size) {
                sem_wait( & delivery_pool -> order_in_bag);

                delivery_pool -> orders[1] = cook_pool -> orders[delivery_pool -> current_order];

                delivery_pool -> current_order++;

                printf("Moto %d put order %d in their bag\n", thread_id, delivery_pool -> current_order);

                log[0] = '\0';

                sprintf(log, "Moto %d put order %d in their bag\n", thread_id, delivery_pool -> current_order);
                write_to_log(log);
                memset(log, '\0', sizeof(log));

                if (delivery_pool -> current_order < delivery_pool -> order_size) {
                    sem_wait( & delivery_pool -> order_in_bag);

                    delivery_pool -> orders[2] = cook_pool -> orders[delivery_pool -> current_order];

                    delivery_pool -> current_order++;

                    printf("Moto %d put order %d in their bag\n", thread_id, delivery_pool -> current_order);

                    log[0] = '\0';

                    sprintf(log, "Moto %d put order %d in their bag\n", thread_id, delivery_pool -> current_order);
                    write_to_log(log);
                    memset(log, '\0', sizeof(log));

                    pthread_mutex_unlock( & delivery_pool -> lock);

                    printf("Moto %d is delivering order: %d\n", thread_id, delivery_pool -> orders[0].num);

                    log[0] = '\0';

                    sprintf(log, "Moto %d is delivering order: %d\n", thread_id, delivery_pool -> orders[0].num);
                    write_to_log(log);
                    memset(log, '\0', sizeof(log));

                    deliveryTime = (fabs(delivery_pool -> orders[0].p - (p / 2.0)) + fabs(delivery_pool -> orders[0].q - (q / 2.0))) / moto_speed;

                    usleep(deliveryTime * 10);

                    printf("Moto %d is delivering order: %d\n", thread_id, delivery_pool -> orders[1].num);

                    log[0] = '\0';

                    sprintf(log, "Moto %d is delivering order: %d\n", thread_id, delivery_pool -> orders[1].num);
                    write_to_log(log);
                    memset(log, '\0', sizeof(log));

                    deliveryTime = (fabs(delivery_pool -> orders[1].p - delivery_pool -> orders[0].p) + fabs(delivery_pool -> orders[1].q - delivery_pool -> orders[0].p)) / moto_speed;

                    usleep(deliveryTime * 10);

                    printf("Moto %d is delivering order: %d\n", thread_id, delivery_pool -> orders[2].num);

                    log[0] = '\0';

                    sprintf(log, "Moto %d is delivering order: %d\n", thread_id, delivery_pool -> orders[2].num);
                    write_to_log(log);
                    memset(log, '\0', sizeof(log));

                    deliveryTime = (fabs(delivery_pool -> orders[2].p - delivery_pool -> orders[1].p) + fabs(delivery_pool -> orders[2].q - delivery_pool -> orders[1].p)) / moto_speed;

                    usleep(deliveryTime * 10);

                    printf("Moto %d delivered all orders, returning to PideShop\n", thread_id);

                    log[0] = '\0';

                    sprintf(log, "Moto %d delivered all orders, returning to PideShop\n", thread_id);
                    write_to_log(log);
                    memset(log, '\0', sizeof(log));

                    deliveryTime = (fabs(delivery_pool -> orders[2].p - (p / 2.0)) + fabs(delivery_pool -> orders[2].q - (q / 2.0))) / moto_speed;

                    usleep(deliveryTime * 10);

                    printf("Moto %d has returned to PideShop\n", thread_id);

                    log[0] = '\0';

                    sprintf(log, "Moto %d has returned to PideShop\n", thread_id);
                    write_to_log(log);
                    memset(log, '\0', sizeof(log));

                    sem_post( & delivery_pool -> order_in_bag);
                    sem_post( & delivery_pool -> order_in_bag);
                    sem_post( & delivery_pool -> order_in_bag);
                } else {

                    pthread_mutex_unlock( & delivery_pool -> lock);

                    printf("Moto %d is delivering order: %d\n", thread_id, delivery_pool -> orders[0].num);

                    log[0] = '\0';

                    sprintf(log, "Moto %d is delivering order: %d\n", thread_id, delivery_pool -> orders[0].num);
                    write_to_log(log);
                    memset(log, '\0', sizeof(log));

                    deliveryTime = (fabs(delivery_pool -> orders[0].p - (p / 2.0)) + fabs(delivery_pool -> orders[0].q - (q / 2.0))) / moto_speed;

                    usleep(deliveryTime * 10);

                    printf("Moto %d is delivering order: %d\n", thread_id, delivery_pool -> orders[1].num);

                    log[0] = '\0';

                    sprintf(log, "Moto %d is delivering order: %d\n", thread_id, delivery_pool -> orders[1].num);
                    write_to_log(log);
                    memset(log, '\0', sizeof(log));

                    deliveryTime = (fabs(delivery_pool -> orders[1].p - delivery_pool -> orders[0].p) + fabs(delivery_pool -> orders[1].q - delivery_pool -> orders[0].p)) / moto_speed;

                    usleep(deliveryTime * 10);

                    printf("Moto %d delivered all orders, returning to PideShop\n", thread_id);

                    log[0] = '\0';

                    sprintf(log, "Moto %d delivered all orders, returning to PideShop\n", thread_id);
                    write_to_log(log);
                    memset(log, '\0', sizeof(log));

                    deliveryTime = (fabs(delivery_pool -> orders[1].p - (p / 2.0)) + fabs(delivery_pool -> orders[1].q - (q / 2.0))) / moto_speed;

                    usleep(deliveryTime * 10);

                    printf("Moto %d has returned to PideShop\n", thread_id);

                    log[0] = '\0';

                    sprintf(log, "Moto %d has returned to PideShop\n", thread_id);
                    write_to_log(log);
                    memset(log, '\0', sizeof(log));

                    sem_post( & delivery_pool -> order_in_bag);
                    sem_post( & delivery_pool -> order_in_bag);
                }
            } else {

                pthread_mutex_unlock( & delivery_pool -> lock);

                printf("Moto %d is delivering order: %d\n", thread_id, delivery_pool -> orders[0].num);

                log[0] = '\0';

                sprintf(log, "Moto %d is delivering order: %d\n", thread_id, delivery_pool -> orders[0].num);
                write_to_log(log);
                memset(log, '\0', sizeof(log));

                deliveryTime = (fabs(delivery_pool -> orders[0].p - (p / 2.0)) + fabs(delivery_pool -> orders[0].q - (q / 2.0))) / moto_speed;

                usleep(deliveryTime * 10);

                printf("Moto %d delivered all orders, returning to PideShop\n", thread_id);

                log[0] = '\0';

                sprintf(log, "Moto %d delivered all orders, returning to PideShop\n", thread_id);
                write_to_log(log);
                memset(log, '\0', sizeof(log));

                deliveryTime = (fabs(delivery_pool -> orders[0].p - (p / 2.0)) + fabs(delivery_pool -> orders[0].q - (q / 2.0))) / moto_speed;

                usleep(deliveryTime * 10);

                printf("Moto %d has returned to PideShop\n", thread_id);

                log[0] = '\0';

                sprintf(log, "Moto %d has returned to PideShop\n", thread_id);
                write_to_log(log);
                memset(log, '\0', sizeof(log));

                sem_post( & delivery_pool -> order_in_bag);

            }
        }

    }

    return NULL;
}

void * threadpool_cook(void * arg) {
    int thread_id = * ((int * ) arg);

    while (1) {
        pthread_mutex_lock( & cook_pool -> lock);

        while (cook_pool -> current_order >= cook_pool -> order_size && !cook_pool -> shutdown) {
            pthread_cond_wait( & cook_pool -> task_available, & cook_pool -> lock);
        }

        if (cook_pool -> shutdown) {
            pthread_mutex_unlock( & cook_pool -> lock);
            pthread_exit(NULL);
        }

        Order order = cook_pool -> orders[cook_pool -> current_order];
        cook_pool -> current_order++;

        pthread_mutex_unlock( & cook_pool -> lock);

        printf("Cook %d is preparing order %d\n", thread_id, cook_pool -> current_order);

        char log[200];

        log[0] = '\0';

        sprintf(log, "Cook %d is preparing order %d\n", thread_id, cook_pool -> current_order);
        write_to_log(log);
        memset(log, '\0', sizeof(log));

        calculatePseudoInverse(30, 40);

        printf("Cook %d is checking for free aparatus for order %d\n", thread_id, cook_pool -> current_order);

        log[0] = '\0';

        sprintf(log, "Cook %d is checking for free aparatus for order %d\n", thread_id, cook_pool -> current_order);
        write_to_log(log);
        memset(log, '\0', sizeof(log));

        sem_wait( & cook_pool -> oven.aparatus);

        sem_post( & cook_pool -> oven.aparatus);

        printf("Cook %d is checking for free opening for order %d\n", thread_id, cook_pool -> current_order);

        log[0] = '\0';

        sprintf(log, "Cook %d is checking for free opening for order %d\n", thread_id, cook_pool -> current_order);
        write_to_log(log);
        memset(log, '\0', sizeof(log));

        sem_wait( & cook_pool -> oven.opening);

        sem_post( & cook_pool -> oven.opening);

        printf("Cook %d is checking number of pides already in the oven for order %d\n", thread_id, cook_pool -> current_order);

        log[0] = '\0';

        sprintf(log, "Cook %d is checking number of pides already in the oven for order %d\n", thread_id, cook_pool -> current_order);
        write_to_log(log);
        memset(log, '\0', sizeof(log));

        sem_wait( & cook_pool -> oven.pide);

        sem_post( & cook_pool -> oven.pide);

        printf("Cook %d is cooking order %d\n", thread_id, cook_pool -> current_order);

        log[0] = '\0';

        sprintf(log, "Cook %d is cooking order %d\n", thread_id, cook_pool -> current_order);
        write_to_log(log);
        memset(log, '\0', sizeof(log));

        calculatePseudoInverse(15, 40);

        sem_post( & delivery_pool -> order_in_bag);

        printf("Cook %d finished order %d\n", thread_id, cook_pool -> current_order);

        log[0] = '\0';

        sprintf(log, "Cook %d finished order %d\n", thread_id, cook_pool -> current_order);
        write_to_log(log);
        memset(log, '\0', sizeof(log));

    }

    return NULL;
}

void delivery_threadpool_init(DeliveryThreadPool * pool, int thread_count, int order_size, Order * orders) {
    pool -> thread_count = thread_count;
    pool -> order_size = order_size;
    pool -> current_order = 0;
    pool -> shutdown = false;

    pool -> threads = (pthread_t * ) malloc(thread_count * sizeof(pthread_t));
    if (pool -> threads == NULL) {
        perror("Failed to allocate memory for threads");
        exit(EXIT_FAILURE);
    }

    pool -> thread_ids = (int * ) malloc(thread_count * sizeof(int));
    if (pool -> thread_ids == NULL) {
        perror("Failed to allocate memory for thread IDs");
        free(pool -> threads);
        exit(EXIT_FAILURE);
    }

    if (pthread_mutex_init( & pool -> lock, NULL) != 0) {
        perror("Failed to initialize mutex");
        free(pool -> threads);
        free(pool -> thread_ids);
        exit(EXIT_FAILURE);
    }

    if (pthread_cond_init( & pool -> task_available, NULL) != 0) {
        perror("Failed to initialize condition variable");
        pthread_mutex_destroy( & pool -> lock);
        free(pool -> threads);
        free(pool -> thread_ids);
        exit(EXIT_FAILURE);
    }

    if (sem_init( & pool -> order_in_bag, 0, MAX_ORDER_IN_BAG) != 0) {
        perror("Failed to initialize semaphores");
        pthread_cond_destroy( & pool -> task_available);
        pthread_mutex_destroy( & pool -> lock);
        free(pool -> threads);
        free(pool -> thread_ids);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < thread_count; ++i) {
        pool -> thread_ids[i] = i + 1;
        int * thread_id = malloc(sizeof(int));
        * thread_id = pool -> thread_ids[i];
        int rc = pthread_create( & pool -> threads[i], NULL, threadpool_delivery, (void * ) thread_id);
        if (rc != 0) {
            fprintf(stderr, "Error: pthread_create failed with code %d\n", rc);
            for (int j = 0; j < i; ++j) {
                pthread_cancel(pool -> threads[j]);
            }
            pthread_cond_destroy( & pool -> task_available);
            pthread_mutex_destroy( & pool -> lock);
            free(pool -> threads);
            free(pool -> thread_ids);
            exit(EXIT_FAILURE);
        }
    }
}

void cook_threadpool_init(CookThreadPool * pool, int thread_count, int order_size, Order * orders) {
    pool -> thread_count = thread_count;
    pool -> order_size = order_size;
    pool -> current_order = 0;
    pool -> shutdown = false;

    pool -> threads = (pthread_t * ) malloc(thread_count * sizeof(pthread_t));
    if (pool -> threads == NULL) {
        perror("Failed to allocate memory for threads");
        exit(EXIT_FAILURE);
    }

    pool -> thread_ids = (int * ) malloc(thread_count * sizeof(int));
    if (pool -> thread_ids == NULL) {
        perror("Failed to allocate memory for thread IDs");
        free(pool -> threads);
        exit(EXIT_FAILURE);
    }

    pool -> orders = orders;

    if (pthread_mutex_init( & pool -> lock, NULL) != 0) {
        perror("Failed to initialize mutex");
        free(pool -> threads);
        free(pool -> thread_ids);
        exit(EXIT_FAILURE);
    }

    if (pthread_cond_init( & pool -> task_available, NULL) != 0) {
        perror("Failed to initialize condition variable");
        pthread_mutex_destroy( & pool -> lock);
        free(pool -> threads);
        free(pool -> thread_ids);
        exit(EXIT_FAILURE);
    }

    if (sem_init( & pool -> oven.aparatus, 0, MAX_APARATUS) != 0 ||
        sem_init( & pool -> oven.opening, 0, MAX_OPENING) != 0 ||
        sem_init( & pool -> oven.pide, 0, MAX_PIDE) != 0) {
        perror("Failed to initialize semaphores");
        pthread_cond_destroy( & pool -> task_available);
        pthread_mutex_destroy( & pool -> lock);
        free(pool -> threads);
        free(pool -> thread_ids);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < thread_count; ++i) {
        pool -> thread_ids[i] = i + 1;
        int * thread_id = malloc(sizeof(int));
        * thread_id = pool -> thread_ids[i];
        int rc = pthread_create( & pool -> threads[i], NULL, threadpool_cook, (void * ) thread_id);
        if (rc != 0) {
            fprintf(stderr, "Error: pthread_create failed with code %d\n", rc);
            for (int j = 0; j < i; ++j) {
                pthread_cancel(pool -> threads[j]);
            }
            pthread_cond_destroy( & pool -> task_available);
            pthread_mutex_destroy( & pool -> lock);
            free(pool -> threads);
            free(pool -> thread_ids);
            exit(EXIT_FAILURE);
        }
    }
}

void cook_threadpool_shutdown(CookThreadPool * pool) {
    pthread_mutex_lock( & pool -> lock);
    pool -> shutdown = true;
    pthread_mutex_unlock( & pool -> lock);

    pthread_cond_broadcast( & pool -> task_available);

    for (int i = 0; i < pool -> thread_count; ++i) {
        pthread_join(pool -> threads[i], NULL);
    }

    free(pool -> threads);

    pthread_mutex_destroy( & pool -> lock);
    pthread_cond_destroy( & pool -> task_available);
}

int main(int argc, char
    const * argv[]) {

    if (argc != 5) {
        printf("Usage: PideShop [portnumber] [CookthreadPoolSize] [DeliveryPoolSize] [k]\n");
        return 1;
    }

    const char * portnumber = argv[1];
    const char * CookthreadPoolSize_string = argv[2];
    const char * DeliveryPoolSize_string = argv[3];
    const char * k = argv[4];

    printf("portnumber: %s\nCookthreadPoolSize: %s\nDeliveryPoolSize: %s\nk: %s\n\n", portnumber, CookthreadPoolSize_string, DeliveryPoolSize_string, k);

    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = handler;
    sigemptyset( & sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    if (sigaction(SIGINT, & sigIntHandler, NULL) != 0) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    moto_speed = atoi(k);

    int server_fd, new_socket;
    ssize_t valread;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    char buffer[1024] = {
        0
    };
    char * hello = "All orders done. Client exiting...";

    int port = atoi(portnumber);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET,
            SO_REUSEADDR | SO_REUSEPORT, & opt,
            sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    printf("PideShop active waitng for connection\n\n");

    if (bind(server_fd, (struct sockaddr * ) & address,
            sizeof(address)) <
        0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    int CookthreadPoolSize = atoi(CookthreadPoolSize_string);
    int DeliveryPoolSize = atoi(DeliveryPoolSize_string);

    char log[200];

    int numberOfClients;

    while (1) {

        if ((new_socket = accept(server_fd, (struct sockaddr * ) & address, &
                addrlen)) <
            0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        if (sem_init( & log_sem, 0, 1) != 0) {
            perror("Log file semaphore initialization failed");
            return 1;
        }

        log[0] = '\0';

        sprintf(log, "PideShop initialized, waiting for customers");
        write_to_log(log);
        memset(log, '\0', sizeof(log));

        valread = read(new_socket, & numberOfClients,
            sizeof(int));

        numberOfClients = ntohl(numberOfClients);

        valread = read(new_socket, & p,
            sizeof(int));

        p = ntohl(p);

        valread = read(new_socket, & q,
            sizeof(int));

        q = ntohl(q);

        printf("p: %d, q: %d\n\n", p, q);

        printf("%d new customers.. Serving\n\n", numberOfClients);

        log[0] = '\0';

        sprintf(log, "%d new customers.. Serving\n\n", numberOfClients);
        write_to_log(log);
        memset(log, '\0', sizeof(log));

        Order * orders;

        orders = (Order * ) malloc(sizeof(Order) * numberOfClients);

        for (int i = 0; i < numberOfClients; i++) {
            valread = read(new_socket, & orders[i], sizeof(Order));
            if (valread == -1) {
                perror("Failed to read from socket");
                free(orders);
                exit(EXIT_FAILURE);
            } else if (valread == 0) {
                printf("Client closed the connection\n");
                free(orders);
                exit(EXIT_FAILURE);
            } else if (valread < sizeof(Order)) {
                printf("Incomplete order received\n");
                free(orders);
                exit(EXIT_FAILURE);
            }

            orders[i].state = PLACED;
            orders[i].num = i + 1;

        }

        cook_pool = (CookThreadPool * ) malloc(sizeof(CookThreadPool));

        delivery_pool = (DeliveryThreadPool * ) malloc(sizeof(DeliveryThreadPool));

        cook_threadpool_init(cook_pool, CookthreadPoolSize, numberOfClients, orders);

        delivery_threadpool_init(delivery_pool, DeliveryPoolSize, numberOfClients, orders);

        pthread_mutex_lock( & cook_pool -> lock);
        pthread_mutex_lock( & delivery_pool -> lock);

        if (cook_pool -> current_order < cook_pool -> order_size) {
            pthread_cond_signal( & cook_pool -> task_available);
        } else {
            cook_pool -> shutdown = true;
        }

        if (delivery_pool -> current_order % 3 == 0 && delivery_pool -> current_order < delivery_pool -> order_size) {
            pthread_cond_signal( & delivery_pool -> task_available);
        } else {
            delivery_pool -> shutdown = true;
        }

        while (cook_pool -> current_order >= cook_pool -> order_size && !cook_pool -> shutdown) {
            pthread_cond_wait( & cook_pool -> task_available, & cook_pool -> lock);
        }

        while (delivery_pool -> current_order >= delivery_pool -> order_size && !delivery_pool -> shutdown) {
            pthread_cond_wait( & delivery_pool -> task_available, & delivery_pool -> lock);
        }

        if (cook_pool -> shutdown) {
            pthread_mutex_unlock( & cook_pool -> lock);
            break;
        }

        if (delivery_pool -> shutdown) {
            pthread_mutex_unlock( & delivery_pool -> lock);
            break;
        }

        pthread_mutex_unlock( & cook_pool -> lock);
        pthread_mutex_unlock( & delivery_pool -> lock);

        free(orders);

        send(new_socket, hello, strlen(hello), 0);

    }

    cook_threadpool_shutdown(cook_pool);
    delivery_threadpool_shutdown(cook_pool);

    free(cook_pool);
    free(delivery_pool);

    sem_destroy( & log_sem);

    close(new_socket);
    close(server_fd);
    return 0;
}
