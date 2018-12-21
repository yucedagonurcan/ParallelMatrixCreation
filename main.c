#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <limits.h>

struct Vector
{
    int index;
    int *array;
};

// A structure to represent a queue
struct Queue
{
    int front, rear, size;
    unsigned capacity;
    struct Vector *vector_array;
};

int generate_random_number(int max);
void *generate_thread_func(void *args);
void print_vector(struct Vector vector, int length, int deneme);
void print_queue(struct Queue *queue, int rows, int cols);
int **create_matrix(int rows, int cols);
void print_array(int *arr, int length);

// function to create a queue of given capacity.
// It initializes size of queue as 0
struct Queue *createQueue(unsigned capacity)
{
    struct Queue *queue = (struct Queue *)malloc(sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity - 1; // This is important, see the enqueue
    queue->vector_array = (struct Vector *)malloc(capacity * sizeof(struct Vector));

    for (size_t i = 0; i < capacity; i++)
    {
        queue->vector_array[i].array = (int *)malloc(5 * sizeof(int));
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
void enqueue(struct Queue *queue, int *vector, int index)
{
    if (isFull(queue))
        return;

    if (index == 0)
    {
        printf("Index is zero\n");
    }
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->vector_array[queue->rear].array = (int *)malloc(5 * sizeof(int));
    queue->vector_array[queue->rear].index = index;
    for (size_t i = 0; i < 5; i++)
    {
        queue->vector_array[queue->rear].array[i] = vector[i];
    }

    queue->size = queue->size + 1;
    queue->vector_array[queue->rear].index = index;
}

// Function to remove an vector from queue.
// It changes front and size
struct Vector dequeue(struct Queue *queue)
{
    if (isEmpty(queue))
    {
        struct Vector null_vec;
        null_vec.array = NULL;
        return null_vec;
    }
    struct Vector item = queue->vector_array[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}

// Function to get front of queue
struct Vector front(struct Queue *queue)
{
    if (isEmpty(queue))
    {
        struct Vector null_vec;
        null_vec.array = NULL;
        return null_vec;
    }
    return queue->vector_array[queue->front];
}

// Function to get rear of queue
struct Vector rear(struct Queue *queue)
{
    if (isEmpty(queue))
    {
        struct Vector null_vec;
        null_vec.array = NULL;
        return null_vec;
    }

    return queue->vector_array[queue->rear];
}

int **log_thread_matrix;
int last_generated_submatrix_index = 0;
int total_sub_matrix;
pthread_mutex_t incrementer_mutex;
struct Queue *generate_threads_queue;

void print_queue(struct Queue *queue, int rows, int cols)
{

    for (size_t i = 0; i < rows; i++)
    {
        print_vector(queue->vector_array[i], cols, i);
        printf(" ==> Index: %d\n", queue->vector_array[i].index);
    }
}
void print_vector(struct Vector vector, int length, int deneme)
{
    printf("\n");
    for (size_t j = 0; j < length; j++)
    {
        printf("%d ", vector.array[j]);
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
        // Creating the current vector for the current thread-in-run.
        int vector[5];
        for (int i = 0; i < 5; i++)
        {
            int random_number = generate_random_number(100);
            vector[i] = random_number;
        }

        int slot_to_take;

        pthread_mutex_lock(&incrementer_mutex);
        if (total_sub_matrix > last_generated_submatrix_index)
        {
            slot_to_take = last_generated_submatrix_index;
            last_generated_submatrix_index++;
        }
        else
        {
            pthread_mutex_unlock(&incrementer_mutex);
            pthread_exit(0);
            return NULL;
        }
        pthread_mutex_unlock(&incrementer_mutex);

        // printf("\nThread: %d, will enqueue the %d slot, last_generated: %d", *myThreadID,
        //        slot_to_take, last_generated_submatrix_index);
        enqueue(generate_threads_queue, vector, slot_to_take);
    }
    return NULL;
}
int last_computed_submatrix_index = 0;
int N;
pthread_mutex_t incrementer_log_mutex;

void *log_thread_func(void *args)
{
    int *myThreadID = (int *)args;
    while (generate_threads_queue->size >= last_computed_submatrix_index)
    {

        pthread_mutex_lock(&incrementer_log_mutex);
        int slot_index_taken = last_computed_submatrix_index;
        int real_index = generate_threads_queue->vector_array[last_computed_submatrix_index].index;
        last_computed_submatrix_index++;
        pthread_mutex_unlock(&incrementer_log_mutex);

        int row;
        if (real_index <= (N / 5))
        {
            row = 0;
        }
        else
        {
            int left_trail = (real_index * 5 * 5); // If N = 10, real_index=3 => left_trail = 15

            row = ((real_index * 5) / (N)) * N;
        }
        int col = real_index % (N);

        if (last_computed_submatrix_index >= last_generated_submatrix_index)
        {
            pthread_exit(0);
            return NULL;
        }

        else
        {
            for (size_t i = col; i < col + 5; i++)
            {
                log_thread_matrix[row][i] = generate_threads_queue->vector_array[slot_index_taken].array[i % 5];
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

    if (pthread_mutex_init(&incrementer_mutex, NULL) != 0)
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

    // Wait for all generate threads.
    for (int i = 0; i < num_of_generate_threads; i++)
    {
        pthread_join(generate_threads[i], NULL);
    }

    for (int i = 0; i < num_of_log_threads; i++)
    {
        pthread_create(&(log_threads[i]), NULL, log_thread_func, (void *)&log_threads[i]);
    }
    for (int i = 0; i < num_of_log_threads; i++)
    {
        pthread_join(log_threads[i], NULL);
    }

    //print_queue(generate_threads_queue, total_sub_matrix, 5);

    printf("\n Printing the LOG Matrix:\n");
    for (size_t i = 0; i < N; i++)
    {

        for (size_t j = 0; j < N; j++)
        {
            printf("%.2d ", log_thread_matrix[i][j]);
        }
        printf("\n");
    }

    pthread_mutex_destroy(&incrementer_mutex);

    for (size_t i = 0; i < total_sub_matrix; i++)
    {
        free(generate_threads_queue->vector_array[i].array);
    }
    free(generate_threads_queue->vector_array);
    free(generate_threads_queue);

    return 0;
}
