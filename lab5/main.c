#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <stddef.h>
#include <pthread.h>

#define DATA_MAX (((256 + 3) / 4) * 4)
#define MSG_MAX 4096
#define CHILD_MAX 1024

short is_go_head = 0;

int CURRENT_COUNT_MSG = MSG_MAX;
short do_flag = 1;
int head_addition = 0;

int my_sem = 0;
pthread_mutex_t my_mutex;

typedef struct {
    char type;
    short hash;
    unsigned char size;
    char data[DATA_MAX];
} msg;

typedef struct {
    int add_count;
    int extract_count;

    int msg_count;

    int head;
    int tail;
    msg buffer[MSG_MAX];
} msg_queue;

typedef struct _handler_arg {
    int *mySem;
} handler_arg;

void msg_queue_init();

int put_msg(msg *_msg);

int get_msg(msg *_msg);

void show_menu();

void init();

void end();

void my_sem_post(int *sem);

void my_sem_wait(int *sem);

void create_producer();

void remove_producer();

void produce_msg(msg *_msg);

void *produce_handler(void *arg);

void create_consumer();

void remove_consumer();

void *consume_handler(void *arg);

msg_queue queue;
pthread_mutex_t mutex;

sem_t free_space;
sem_t items;

pthread_t producers[CHILD_MAX];
int producers_amount;

pthread_t consumers[CHILD_MAX];
int consumers_amount;

void my_sem_set(int value) {
    while (pthread_mutex_lock(&my_mutex) != 0) {
        sleep(1);
    };
    my_sem = value;
    pthread_mutex_unlock(&my_mutex);
}

void my_sem_post(int *sem) {
    while (pthread_mutex_lock(&my_mutex) != 0) {
        sleep(1);
    };
    *sem = *sem + 1;
    pthread_mutex_unlock(&my_mutex);
}

void my_sem_wait(int *sem) {
    int flag = 1;
    while (flag) {
        while (pthread_mutex_lock(&my_mutex) != 0) {
            sleep(1);
        }
        if (*sem > 0) {
            *sem = *sem - 1;
            flag = 0;
        }
        pthread_mutex_unlock(&my_mutex);
    }
}

void show_menu() {
    printf("Menu\n");
    printf("p - create producer\n");
    printf("d - delete producer\n");
    printf("c - create consumer\n");
    printf("r - delete consumer\n");
    printf("s - change size of queue\n");
    printf("q - quit\n");
}

void init(void) {
    msg_queue_init();

    if (pthread_mutex_init(&mutex, NULL) != 0 || pthread_mutex_init(&my_mutex, NULL)) {
        fprintf(stderr, "Failed mutex init \n");
        exit(1);
    }

    my_sem_set(MSG_MAX);

    if (sem_init(&free_space, 0, MSG_MAX) == -1 || sem_init(&items, 0, 0) == -1) {
        fprintf(stderr, "Failed to initialize semaphore\n");
        exit(1);
    }
}

void end() {
    do_flag = 0;
    usleep(500000);
    int i = 0;
    for (; i < producers_amount; ++i) {
        pthread_cancel(producers[i]);
    }
    i = 0;
    for (; i < consumers_amount; ++i) {
        pthread_cancel(consumers[i]);
    }

    pthread_join(consumers[i], NULL);

    if (pthread_mutex_destroy(&mutex) != 0) {
        fprintf(stderr, "Failed mutex destroy\n");
        exit(1);
    }

    if (sem_destroy(&free_space) != 0) {
        fprintf(stderr, "Failed to destroy semaphore 'free_space'\n");
        exit(1);
    }

    if (sem_destroy(&items) != 0) {
        fprintf(stderr, "Failed to destroy semaphore 'items'\n");
        exit(1);
    }
    exit(1);
}

void msg_queue_init() {
    memset(&queue, 0, sizeof(queue));
}

int put_msg(msg *_msg) {
    if (queue.msg_count == CURRENT_COUNT_MSG - 1) {
        fprintf(stderr, "Queue buffer overflow\n");
        queue.buffer[queue.tail] = *_msg;
        queue.tail++;
        queue.msg_count++;
        return ++queue.add_count;
    }

    if (queue.tail >= CURRENT_COUNT_MSG) {
        queue.tail = 0;
    }

    queue.buffer[queue.tail] = *_msg;

    queue.tail++;
    queue.msg_count++;

    return ++queue.add_count;
}

int get_msg(msg *_msg) {
    if (queue.msg_count == 0) {
        fprintf(stderr, "Queue buffer is empty\n");
        return queue.extract_count;
    }

    if ((queue.head == CURRENT_COUNT_MSG && head_addition == 0) ||
        (queue.head > CURRENT_COUNT_MSG && head_addition == 0)) {
        queue.head = 0;
    }

    *_msg = queue.buffer[queue.head];

    if (is_go_head) {
        head_addition--;
    }
    if (head_addition == 0) {
        is_go_head = 0;
    }
    queue.head++;
    queue.msg_count--;

    return ++queue.extract_count;
}

void create_producer() {
    if (producers_amount == CHILD_MAX) {
        fprintf(stderr, "Max value of producers\n");
        return;
    }

    handler_arg *args = malloc(sizeof(handler_arg));
    (args->mySem) = &my_sem;

    if (pthread_create(&producers[producers_amount], NULL, produce_handler, (void *) args) != 0) {
        fprintf(stderr, "Failed to create producer\n");
        exit(1);
    }

    ++producers_amount;
}

void remove_producer() {
    if (producers_amount == 0) {
        fprintf(stderr, "No producers to delete\n");
        return;
    }
    usleep(500000);
    --producers_amount;
    pthread_cancel(producers[producers_amount]);
    pthread_join(producers[producers_amount], NULL);
}

void produce_msg(msg *message) {
    message->type = 'M';
    message->hash = 0;
    short sum = 0;
    int temp;
    message->size = rand() % 256 + 1;

    if (message->size == 256) {
        message->size = 0;
        message->data[0] = '\0';
    } else {
        for (int i = 0; i < message->size; i++) {
            short temp = rand() % 52;
            if (temp < 26) {
                message->data[i] = 'a' + temp;
            } else {
                message->data[i] = 'A' + temp - 26;
            }
        }
        message->data[message->size - 1] = '\0';
        temp = (message->size) - 1;
    }

    int padding = ((message->size + 3) / 4) * 4 - message->size;
    for (int i = 0; i < padding; i++) {
        temp += i + 1;
        message->data[temp] = '\0';
    }

    for (int i = 0; i <= temp; i++) {
        sum += (message->data[i]) / 4;
    }
    message->hash = sum;
}

_Noreturn void *produce_handler(void *arg) {
    handler_arg *args = (handler_arg *) arg;
    msg _msg;
    int add_count_local;
    while (1) {
        while (do_flag) {
            produce_msg(&_msg);

            my_sem_wait(args->mySem);
            pthread_mutex_lock(&mutex);

            add_count_local = put_msg(&_msg);

            pthread_mutex_unlock(&mutex);
            sem_post(&items);

            printf("%ld produce msg: hash=%d, add_count=%d\n",
                   pthread_self(), _msg.hash, add_count_local);

            sleep(5);
        }
    }
}

void create_consumer() {
    if (consumers_amount == CHILD_MAX) {
        fprintf(stderr, "Max value of consumers\n");
        return;
    }

    if (pthread_create(&consumers[consumers_amount], NULL,
                       consume_handler, NULL) != 0) {
        fprintf(stderr, "Failed to create producer\n");
        exit(1);
    }

    ++consumers_amount;
}

void remove_consumer() {
    if (consumers_amount == 0) {
        fprintf(stderr, "No consumers to delete\n");
        return;
    }

    consumers_amount--;
    pthread_cancel(consumers[consumers_amount]);
    pthread_join(consumers[consumers_amount], NULL);
}

_Noreturn void *consume_handler(void *arg) {
    msg _msg;
    int extract_count_local;
    while (1) {
        while (do_flag) {
            sem_wait(&items);
            pthread_mutex_lock(&mutex);

            extract_count_local = get_msg(&_msg);

            pthread_mutex_unlock(&mutex);
            my_sem_post(&my_sem);

            printf("%ld consume msg: hash=%d, extract_count=%d\n",
                   pthread_self(), _msg.hash, extract_count_local);

            sleep(5);
        }
    }
}

int get_number() {
    int num;
    char buffer[100];
    printf("Enter a number: ");
    while (1) {
        if (fgets(buffer, sizeof(buffer), stdin)) {
            if (sscanf(buffer, "%d", &num) == 1 && num >= 0 && num <= 4096) {
                return num;
            }
        }
    }
}

void change_msg_size() {
    do_flag = 0;

    CURRENT_COUNT_MSG = get_number();
    usleep(500000);

    printf("CURRENT_COUNT_MSG: %d, queue.msg_count: %d \n", CURRENT_COUNT_MSG, queue.msg_count);

    my_sem_set(CURRENT_COUNT_MSG - queue.msg_count);
    is_go_head = 1;
    if (queue.tail > queue.head) {
        head_addition = queue.tail - queue.head;
    }
    if ((queue.head > queue.tail)) {
        head_addition = queue.msg_count - queue.tail;
    }
    usleep(500000);
    do_flag = 1;
}

int main() {
    srand(time(NULL));
    init();
    show_menu();
    while (true) {
        switch (getchar()) {
            case 'p':
                create_producer();
                break;
            case 'd':
                remove_producer();
                break;
            case 'c':
                create_consumer();
                break;
            case 'r':
                remove_consumer();
                break;
            case 'q':
                end();
                return 0;
            case 's':
                change_msg_size();
            default:
                break;
        }
    }
}