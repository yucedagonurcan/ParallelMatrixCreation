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
struct Vector **create_matrix(int rows, int cols);
void print_vector(struct Vector vector, int length);
void print_queue(struct Queue *queue, int rows, int cols);

// function to create a queue of given capacity.
// It initializes size of queue as 0
struct Queue *createQueue(unsigned capacity)
{
    struct Queue *queue = (struct Queue *)malloc(sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity - 1; // This is important, see the enqueue
    queue->vector_array = (struct Vector *)malloc(capacity * sizeof(struct Vector));
    return queue;
}

// Queue is full when size becomes equal to the capacity
int isFull(struct Queue *queue)
{
    return (queue->size == queue->capacity);
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
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->vector_array[queue->rear].array = (int *)malloc(5 * sizeof(int));

    for (size_t i = 0; i < 5; i++)
    {
        queue->vector_array[queue->rear].array[i] = vector[i];
    }

    queue->size = queue->size + 1;
    queue->vector_array[queue->rear].index = index;

    printf("\nVector created for the %d th index is: \n", index);
    print_vector(queue->vector_array[queue->rear], 5);
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

int **global_matrix;
int last_generated_submatrix_index = 0;
int total_sub_matrix;
pthread_mutex_t incrementer_mutex;

struct Queue *generate_threads_queue;

struct Vector **create_matrix(int rows, int cols)
{
    struct Vector **matrix = (struct Vector **)malloc(rows * sizeof(struct Vector *));
    for (size_t i = 0; i < rows; i++)
    {
        matrix[i] = (struct Vector *)malloc(cols * sizeof(int));
    }
    return matrix;
}
void print_queue(struct Queue *queue, int rows, int cols)
{

    for (size_t i = 0; i < rows; i++)
    {
        print_vector(queue->vector_array[i], cols);
    }
}
void print_vector(struct Vector vector, int length)
{
    printf("\n");
    for (size_t j = 0; j < length; j++)
    {
        printf("%d ", vector.array[j]);
    }
}

void *generate_thread_func(void *args)
{
    int *myThreadID = (int *)args;

    while (total_sub_matrix > last_generated_submatrix_index)
    {
        int vector[5];
        for (int i = 0; i < 5; i++)
        {
            int random_number = generate_random_number(100);
            vector[i] = random_number;
        }

        //printf("\nTrying to locked by thread %d\n", *myThreadID);
        pthread_mutex_lock(&incrementer_mutex);
        //printf("\nLocked by thread: %d and thread_number: %d\n", *myThreadID, last_generated_submatrix_index);

        if (total_sub_matrix > last_generated_submatrix_index)
        {
            enqueue(generate_threads_queue, vector, last_generated_submatrix_index);
            last_generated_submatrix_index++;
            //printf("\nIncremented thread_number by thread: %d and thread_number: %d\n", *myThreadID, last_generated_submatrix_index);
        }

        pthread_mutex_unlock(&incrementer_mutex);
    }
    return NULL;
}

int generate_random_number(int max)
{
    return rand() % max;
}
int main(int argc, char *argv[])
{
    /// Get command line arguments.
    int N = atoi(argv[1]);
    int num_of_generate_threads = atoi(argv[2]);
    /// Get command line arguments.

    // Initialization of variables.
    total_sub_matrix = (N / 5) * (N / 5);
    generate_threads_queue = createQueue(total_sub_matrix);

    //generated_matrix_queue = malloc(sizeof(matrix_queue));

    struct Vector new_ = rear(generate_threads_queue);
    // Initialization of variables.

    if (pthread_mutex_init(&incrementer_mutex, NULL) != 0)
    {
        printf("\nMutex init has failed\n");
        return 1;
    }
    // Initialization, should only be called once.
    srand(time(NULL));

    pthread_t generate_threads[num_of_generate_threads];

    for (int i = 0; i < num_of_generate_threads; i++)
    {
        pthread_create(&(generate_threads[i]), NULL, generate_thread_func, (void *)&generate_threads[i]);
    }
    // Wait for all generate threads.
    for (int i = 0; i < num_of_generate_threads; i++)
    {
        pthread_join(generate_threads[i], NULL);
    }
    pthread_mutex_destroy(&incrementer_mutex);
    print_queue(generate_threads_queue, total_sub_matrix, 5);

    return 0;
}