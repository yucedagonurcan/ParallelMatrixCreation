#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <limits.h>

// A structure to represent matrix that each of the Queue's array indices has.
struct QueueMatrix
{
    int index;
    int **matrix;
};

// A structure to represent a queue that holds QueueMatrix.
struct Queue
{
    int front, rear, size;
    unsigned capacity;
    struct QueueMatrix *queue_matrix;
};

void *generate_thread_func(void *args);
void *log_thread_func(void *args);

void print_queue(struct Queue *queue, int total_sub_matrix);
void print_matrix(struct QueueMatrix queue_matrix, int length);
void print_array(int *arr, int length);

int isFull(struct Queue *queue);
int isEmpty(struct Queue *queue);

int generate_random_number(int max);
int **create_matrix(int rows, int cols);
struct Queue *createQueue(unsigned capacity);

void enqueue(struct Queue *queue, int matrix[5][5], int index);
struct QueueMatrix dequeue(struct Queue *queue);
struct QueueMatrix front(struct Queue *queue);
struct QueueMatrix rear(struct Queue *queue);

int **log_thread_matrix;
int last_generated_submatrix_index = 0;
int total_sub_matrix;
pthread_mutex_t incrementer_generate_mutex;
pthread_mutex_t incrementer_log_mutex;

struct Queue *generate_threads_queue;
int last_computed_submatrix_index = 0;
int N;

// function to create a queue of given capacity.
// It initializes size of queue as 0
struct Queue *createQueue(unsigned capacity)
{
    struct Queue *queue = (struct Queue *)malloc(sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity - 1; // This is important, see the enqueue
    queue->queue_matrix = (struct QueueMatrix *)malloc(capacity * sizeof(struct QueueMatrix));

    for (size_t i = 0; i < capacity; i++)
    {
        queue->queue_matrix[i].matrix = (int **)malloc(5 * sizeof(int *));

        for (size_t j = 0; j < 5; j++)
        {
            queue->queue_matrix[i].matrix[j] = (int *)malloc(5 * sizeof(int));
        }
    }

    return queue;
}

// Queue is full when size becomes equal to the capacity
int isFull(struct Queue *queue)
{
    return (queue->size == (queue->capacity));
}

// Queue is empty when size is 0
int isEmpty(struct Queue *queue)
{
    return (queue->size == 0);
}

// Function to add an vector to the queue.
// It changes rear and size
void enqueue(struct Queue *queue, int matrix[5][5], int index)
{
    if (isFull(queue))
        return;

    queue->rear = (queue->rear + 1) % queue->capacity;
    // queue->queue_matrix[queue->rear].matrix = (int *)malloc(5 * sizeof(int));
    queue->queue_matrix[queue->rear].index = index;
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            queue->queue_matrix[queue->rear].matrix[i][j] = matrix[i][j];
        }
    }

    queue->size = queue->size + 1;
    // queue->queue_matrix[queue->rear].index = index;
}

// Function to remove an vector from queue.
// It changes front and size
struct QueueMatrix dequeue(struct Queue *queue)
{
    if (isEmpty(queue))
    {
        struct QueueMatrix null_vec;
        null_vec.matrix = NULL;
        return null_vec;
    }
    struct QueueMatrix item = queue->queue_matrix[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}

// Function to get front of queue
struct QueueMatrix front(struct Queue *queue)
{
    if (isEmpty(queue))
    {
        struct QueueMatrix null_vec;
        null_vec.matrix = NULL;
        return null_vec;
    }
    return queue->queue_matrix[queue->front];
}

// Function to get rear of queue
struct QueueMatrix rear(struct Queue *queue)
{
    if (isEmpty(queue))
    {
        struct QueueMatrix null_vec;
        null_vec.matrix = NULL;
        return null_vec;
    }

    return queue->queue_matrix[queue->rear];
}

void print_queue(struct Queue *queue, int total_sub_matrix)
{
    for (size_t i = 0; i < total_sub_matrix; i++)
    {

        printf("\n ==> Index: %d\n", queue->queue_matrix[i].index);
        print_matrix(queue->queue_matrix[i], 5);
    }
}
void print_matrix(struct QueueMatrix queue_matrix, int length)
{
    printf("\n");

    for (size_t i = 0; i < length; i++)
    {
        printf("\n");
        for (size_t j = 0; j < length; j++)
        {
            printf("%d ", queue_matrix.matrix[i][j]);
        }
        printf("\n");
    }
}
void print_array(int *arr, int length)
{
    printf("\n");
    for (size_t j = 0; j < length; j++)
    {
        printf("%d ", arr[j]);
    }
}

void *generate_thread_func(void *args)
{
    int *myThreadID = (int *)args;

    while (total_sub_matrix > last_generated_submatrix_index)
    {
        // Creating the current matrix for the current thread-in-run.
        int matrix[5][5];
        for (int i = 0; i < 5; i++)
        {

            for (size_t j = 0; j < 5; j++)
            {
                matrix[i][j] = generate_random_number(100);
            }
        }

        int slot_to_take;

        pthread_mutex_lock(&incrementer_generate_mutex);
        if (total_sub_matrix > last_generated_submatrix_index)
        {
            slot_to_take = last_generated_submatrix_index;
            last_generated_submatrix_index++;
        }
        else
        {
            pthread_mutex_unlock(&incrementer_generate_mutex);
            pthread_exit(0);
            return NULL;
        }
        pthread_mutex_unlock(&incrementer_generate_mutex);

        printf("\nThread: %d, will enqueue the %d slot, last_generated: %d", *myThreadID,
               slot_to_take, last_generated_submatrix_index);
        enqueue(generate_threads_queue, matrix, slot_to_take);
    }
    return NULL;
}

void *log_thread_func(void *args)
{
    int *myThreadID = (int *)args;
    while (generate_threads_queue->size >= last_computed_submatrix_index)
    {

        pthread_mutex_lock(&incrementer_log_mutex);
        int slot_index_taken = last_computed_submatrix_index;
        int real_index = generate_threads_queue->queue_matrix[last_computed_submatrix_index].index;
        last_computed_submatrix_index++;
        pthread_mutex_unlock(&incrementer_log_mutex);

        int row = ((real_index / (N / 5)) % N) * 5;
        int col = real_index % (N / 5) * 5;

        if (last_computed_submatrix_index > last_generated_submatrix_index)
        {
            pthread_exit(0);
            return NULL;
        }

        else
        {

            for (size_t i = row; i < row + 5; i++)
            {
                for (size_t j = col; j < col + 5; j++)
                {
                    log_thread_matrix[i][j] = generate_threads_queue->queue_matrix[slot_index_taken].matrix[i - row][j - col];
                }
            }
            printf("\nLog thread %d for indices: (%d,%d): \n", real_index, row, col);
            print_array(log_thread_matrix[row], 5);
        }
    }
    return NULL;
}
int generate_random_number(int max)
{
    return rand() % max;
}
int **create_matrix(int rows, int cols)
{
    int **matrix = (int **)malloc(rows * sizeof(int *));
    for (size_t i = 0; i < rows; i++)
    {
        matrix[i] = (int *)malloc(cols * sizeof(int));
    }
    return matrix;
}
int main(int argc, char *argv[])
{
    /// Get command line arguments.
    N = atoi(argv[1]);
    int num_of_generate_threads = atoi(argv[2]);
    int num_of_log_threads = atoi(argv[3]);
    /// Get command line arguments.

    // Initialization of variables.
    total_sub_matrix = (N / 5) * (N / 5);

    generate_threads_queue = createQueue(total_sub_matrix);
    log_thread_matrix = create_matrix(N, N);
    // Initialization of variables.

    if (pthread_mutex_init(&incrementer_generate_mutex, NULL) != 0)
    {
        printf("\nMutex init has failed\n");
        return 1;
    }
    if (pthread_mutex_init(&incrementer_log_mutex, NULL) != 0)
    {
        printf("\nMutex init has failed\n");
        return 1;
    }
    // Initialization, should only be called once.
    srand(time(NULL));

    pthread_t generate_threads[num_of_generate_threads];
    pthread_t log_threads[num_of_log_threads];

    for (int i = 0; i < num_of_generate_threads; i++)
    {
        pthread_create(&(generate_threads[i]), NULL, generate_thread_func, (void *)&generate_threads[i]);
    }

    for (int i = 0; i < num_of_log_threads; i++)
    {
        pthread_create(&(log_threads[i]), NULL, log_thread_func, (void *)&log_threads[i]);
    }
    // Wait for all generate threads.
    for (int i = 0; i < num_of_generate_threads; i++)
    {
        pthread_join(generate_threads[i], NULL);
    }

    for (int i = 0; i < num_of_log_threads; i++)
    {
        pthread_join(log_threads[i], NULL);
    }

    print_queue(generate_threads_queue, total_sub_matrix);

    printf("\n Printing the LOG Matrix:\n");
    for (size_t i = 0; i < N; i++)
    {

        for (size_t j = 0; j < N; j++)
        {
            printf("%.2d ", log_thread_matrix[i][j]);
        }
        printf("\n");
    }

    pthread_mutex_destroy(&incrementer_generate_mutex);

    for (size_t i = 0; i < total_sub_matrix; i++)
    {
        free(generate_threads_queue->queue_matrix[i].matrix);
    }

    for (size_t i = 0; i < num_of_log_threads; i++)
    {
        free(log_thread_matrix[i]);
    }
    free(log_thread_matrix);
    free(generate_threads_queue->queue_matrix);
    free(generate_threads_queue);

    return 0;
}