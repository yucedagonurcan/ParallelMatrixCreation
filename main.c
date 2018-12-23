#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <limits.h>
#include <getopt.h>

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

// Thread Functions.
void *log_thread_func(void *args);
void *generate_thread_func(void *args);
void *add_thread_func(void *args);
void *mod_thread_func(void *args);

// Printing and Writing Functions.
void print_array(int *arr, int length);
void print_matrixDP(int **queue_matrix, int length);
void print_matrix(int queue_matrix[][5], int length);
void print_queue(struct Queue *queue, int total_sub_matrix);
void WriteMatrixAndSumToFile(char *fileName);

// Queue Functions.
int isFull(struct Queue *queue);
int isEmpty(struct Queue *queue);

// Helper Functions for Queue and matrices.
int find_row(int N, int index);
int find_col(int N, int index);
int generate_random_number(int max);
int **create_matrix(int rows, int cols);
struct Queue *createQueue(unsigned capacity);

// Queue build-in functions.
struct QueueMatrix rear(struct Queue *queue);
struct QueueMatrix front(struct Queue *queue);
struct QueueMatrix dequeue(struct Queue *queue);
void enqueueDP(struct Queue *queue, int **matrix, int index);
void enqueue(struct Queue *queue, int matrix[5][5], int index);

// Global Variables and Mutexes.
int total_sub_matrix;
int **log_thread_matrix;
pthread_mutex_t adder_mutex;
pthread_mutex_t printer_mutex;
pthread_mutex_t incrementer_mod_mutex;
pthread_mutex_t incrementer_log_mutex;
pthread_mutex_t incrementer_add_mutex;
pthread_mutex_t incrementer_generate_mutex;

int N;
long long int global_sum = 0;
struct Queue *mod_threads_queue;
int last_mod_submatrix_index = 0;
int last_summed_submatrix_index = 0;
int last_logged_submatrix_index = 0;
struct Queue *generate_threads_queue;
int last_generated_submatrix_index = 0;

// Function to create a queue of given capacity.
// It initializes size of queue as 0
struct Queue *createQueue(unsigned capacity)
{
    // Create a memory space for Queue.
    struct Queue *queue = (struct Queue *)malloc(sizeof(struct Queue));
    // Initialize Queue variables.
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity - 1;
    // Initialize the ->queue_matrix with the size of the capacity.
    queue->queue_matrix = (struct QueueMatrix *)malloc(capacity * sizeof(struct QueueMatrix));

    // Create memory space for each matrix and each matrix's row and column in the queue_matrix.
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

// Function to add an matrix to the queue.
// It changes rear and size
// It is a function for enqueue the matrix type of matrix[5][5].
void enqueue(struct Queue *queue, int matrix[5][5], int index)
{
    if (isFull(queue))
        return;

    // Change the Struct variables wrt new element for the Queue.
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->queue_matrix[queue->rear].index = index;
    // Copy all the element in the given matrix to the next free place in the queue_matrix.
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            queue->queue_matrix[queue->rear].matrix[i][j] = matrix[i][j];
        }
    }
    // Increment the size by 1.
    queue->size = queue->size + 1;
}
// Function to add an matrix to the queue.
// It changes rear and size
// It is a function for enqueue the matrix type of **matrix.
// DP: Double Pointer.
void enqueueDP(struct Queue *queue, int **matrix, int index)
{
    if (isFull(queue))
        return;

    // Change the Struct variables wrt new element for the Queue.
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->queue_matrix[queue->rear].index = index;
    // Copy all the element in the given matrix to the next free place in the queue_matrix.
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            queue->queue_matrix[queue->rear].matrix[i][j] = matrix[i][j];
        }
    }
    // Increment the size by 1.
    queue->size = queue->size + 1;
}

// Function to remove a matrix from queue.
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
// It is a function to print all the matrices in a given Queue.
void print_queue(struct Queue *queue, int total_sub_matrix)
{
    // Loop through all the matrices in the queue and send them to the print_matrixDP.
    for (size_t i = 0; i < total_sub_matrix; i++)
    {

        printf("\n ==> Index: %d\n", queue->queue_matrix[i].index);
        print_matrixDP(queue->queue_matrix[i].matrix, 5);
    }
}
// It is a printer function for the matrices that have the type of **queue_matrix.
// DP: Double Pointer.
void print_matrixDP(int **queue_matrix, int length)
{
    // Loop through all the elements in the matrix and print them.
    printf("\n");
    for (size_t i = 0; i < length; i++)
    {
        printf("\n");
        for (size_t j = 0; j < length; j++)
        {
            printf("%.2d ", queue_matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}
// It is a printer function for the matrices that have the type of queue_matrix[5][5].
void print_matrix(int queue_matrix[][5], int length)
{
    // Loop through all the elements in the matrix and print them.
    printf("\n");
    for (size_t i = 0; i < length; i++)
    {
        printf("\n");
        for (size_t j = 0; j < length; j++)
        {
            printf("%.2d ", queue_matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}
// It is a function for printing an array.
void print_array(int *arr, int length)
{
    printf("\n");
    for (size_t j = 0; j < length; j++)
    {
        printf("%d ", arr[j]);
    }
}
void WriteMatrixAndSumToFile(char *fileName)
{
    // Create a new_file with the corresponding given name.
    FILE *new_file;
    new_file = fopen(fileName, "w");

    fprintf(new_file, "The matrix is: \n\n");
    for (size_t i = 0; i < N; i++)
    {
        for (size_t j = 0; j < N; j++)
        {
            fprintf(new_file, "%.2d ", log_thread_matrix[i][j]);
        }
        fprintf(new_file, "\n");
    }

    fprintf(new_file, "\nThe global sum is: %lld", global_sum);
    fclose(new_file);
}
// Helper function to return a random number with given range.
int generate_random_number(int max)
{
    return rand() % max;
}
// Function that finds the starting row for the given index in the NxN matrix with builded up with the 5x5 submatrices.
int find_row(int N, int index)
{
    int row = ((index / (N / 5)) % N) * 5;
    return row;
}
// Function that finds the starting col for the given index in the NxN matrix with builded up with the 5x5 submatrices.
int find_col(int N, int index)
{
    int col = index % (N / 5) * 5;
    return col;
}
// Function that creates a Double Pointed matrix and returns the memory address.
int **create_matrix(int rows, int cols)
{
    int **matrix = (int **)malloc(rows * sizeof(int *));
    for (size_t i = 0; i < rows; i++)
    {
        matrix[i] = (int *)malloc(cols * sizeof(int));
    }
    return matrix;
}
// Add Thread's function.
void *add_thread_func(void *args)
{
    // Get the Thread ID for the given thread.
    int *myThreadID = (int *)args;
    // Check if the Add Threads computed all the indices in the mod_queue's queue_matrix.
    while (last_mod_submatrix_index >= last_summed_submatrix_index)
    {
        // Lock the mutex because it will get a slot index.
        // We are getting a slot first because we want to enqueue concurently later in the code.
        // Multiple threads can update the Queue in this way.
        pthread_mutex_lock(&incrementer_add_mutex);
        int slot_taken = last_summed_submatrix_index;
        // Increment the last_summed_submatrix_index by 1.
        last_summed_submatrix_index++;
        // Real index is the index that comes from Mod threads, also comes from Generate Threads
        // It is the index that shows which submatrix index we want to implement the current matrix.
        int real_index = mod_threads_queue->queue_matrix[slot_taken].index;
        // Unlock the mutex.
        pthread_mutex_unlock(&incrementer_add_mutex);

        // Local_sum variable that will store the matrix element's summation for the current turn.
        int local_sum = 0;

        // We will check again because it could be change since we are out of the locker.
        if (last_mod_submatrix_index >= last_summed_submatrix_index)
        {
            // Compute the local_sum.
            for (size_t i = 0; i < 5; i++)
            {
                for (size_t j = 0; j < 5; j++)
                {
                    local_sum += mod_threads_queue->queue_matrix[slot_taken].matrix[i][j];
                }
            }
        }
        else
        {
            // If we are already done the work, we will rollback the value that we've just incremented and exit the thread since we don't need it anymore.
            last_summed_submatrix_index--;
            pthread_exit(0);
            return NULL;
        }
        // Add the local_sum into the global_sum.
        // Since it is a global variable we will lock the mutexes to make sure only one thread is changing this variable.
        pthread_mutex_lock(&incrementer_add_mutex);
        long long int current_global_sum = global_sum;
        global_sum += local_sum;
        pthread_mutex_unlock(&incrementer_add_mutex);

        // Locking the printer mutex because without it we will get some results that we don't expect.
        // Other threads can change the stdout. That's why we use mutexes.
        pthread_mutex_lock(&printer_mutex);
        printf("\n------------------------------------------------------------------------------------------------");
        printf("\nAdd_%d | has local sum: %d by [%d,%d] submatrix, global sum before/after update: %lld/%lld\n", *myThreadID, local_sum,
               (find_row(N, real_index) / 5), (find_col(N, real_index) / 5), current_global_sum, (current_global_sum + local_sum));
        printf("------------------------------------------------------------------------------------------------\n");
        pthread_mutex_unlock(&printer_mutex);
    }
    return NULL;
}
// Generate Thread's function.
void *generate_thread_func(void *args)
{
    // Get the Thread ID for the given thread.
    int *myThreadID = (int *)args;
    // Check if the Generate Threads generated all the indices that we need from total_sub_matrix.
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
        // We are getting a slot first because we want to enqueue concurently later in the code.
        int slot_to_take;

        pthread_mutex_lock(&incrementer_generate_mutex);
        // Check again because last_generated_submatrix_index could be changed.
        if (total_sub_matrix > last_generated_submatrix_index)
        {
            // Only the first Generate Thread is creating the Queue.
            if (last_generated_submatrix_index == 0)
            {
                generate_threads_queue = createQueue(total_sub_matrix);
            }
            // Get the slot and incremet the last_generated_submatrix_index by 1.
            slot_to_take = last_generated_submatrix_index;
            last_generated_submatrix_index++;
        }
        else
        {
            // If we are already done the work, exit the thread since we don't need it anymore.
            pthread_mutex_unlock(&incrementer_generate_mutex);
            pthread_exit(0);
            return NULL;
        }
        pthread_mutex_unlock(&incrementer_generate_mutex);

        // Enqueue the current matrix into the generate_threads_queue concurently.
        enqueue(generate_threads_queue, matrix, slot_to_take);

        // Locking the printer mutex because without it we will get some results that we don't expect.
        // Other threads can change the stdout. That's why we use mutexes.
        pthread_mutex_lock(&printer_mutex);
        printf("\n--------------------------------------------------------------------");
        printf("\nGenerate_%d: |   Generate_%d generated following matrix: ", *myThreadID, *myThreadID);
        print_matrix(matrix, 5);
        printf("This matrix is [%d,%d] submatrix\n", find_row(N, slot_to_take) / 5, find_col(N, slot_to_take) / 5);
        printf("-------------------------------\n");

        pthread_mutex_unlock(&printer_mutex);
    }
    return NULL;
}

void *log_thread_func(void *args)
{
    // Get the Thread ID for the given thread.
    int *myThreadID = (int *)args;
    // Check if the Log Threads generated all the indices that we need from the size of generate_thread's size.
    while (generate_threads_queue->size >= last_logged_submatrix_index)
    {
        // Get the slot and incremet the last_logged_submatrix_index by 1.
        // Also get the index of the current queue_matrix's index as real_index.
        // It will show us where to put the current matrix in the global log matrix.
        pthread_mutex_lock(&incrementer_log_mutex);

        // Only the first Log Thread is creating the matrix.
        if (last_logged_submatrix_index == 0)
        {
            log_thread_matrix = create_matrix(N, N);
        }
        int slot_index_taken = last_logged_submatrix_index;
        int real_index = generate_threads_queue->queue_matrix[last_logged_submatrix_index].index;
        last_logged_submatrix_index++;
        pthread_mutex_unlock(&incrementer_log_mutex);

        // Compute the starting points for the row and column with given index.
        int row = find_row(N, real_index);
        int col = find_col(N, real_index);

        // Check again because it could be changed.
        if (last_logged_submatrix_index > last_generated_submatrix_index)
        {
            // If so exit the thread, since we don't need it anymore.
            pthread_exit(0);
            return NULL;
        }

        else
        {
            // Copy the current queue_matrix into the log_thread_matrix with the corresponding row and column values.
            for (size_t i = row; i < row + 5; i++)
            {
                for (size_t j = col; j < col + 5; j++)
                {
                    log_thread_matrix[i][j] = generate_threads_queue->queue_matrix[slot_index_taken].matrix[i - row][j - col];
                }
            }
            // Locking the printer_mutex.
            pthread_mutex_lock(&printer_mutex);
            printf("\n-------------------------------------------------------------");
            printf("\nLog_%d |   Log_%d just grabbed the [%d,%d] submatrix\n", *myThreadID, *myThreadID, row / 5, col / 5);
            printf("-------------------------------------------------------------\n");
            pthread_mutex_unlock(&printer_mutex);
        }
    }
    return NULL;
}
// Helper function for the creating a Mod Matrix for the given Queue_Matrix.
int **GenerateModMatrix(struct QueueMatrix queue_matrix)
{
    // Declare and initialize a 5x5 matrix to fill in later.
    int **mod_matrix;
    mod_matrix = create_matrix(5, 5);
    // Get the first number.
    int first_number = queue_matrix.matrix[0][0];

    for (size_t i = 0; i < 5; i++)
    {

        for (size_t j = 0; j < 5; j++)
        {
            // Check if the first number is 0 or less.
            // If so, we will get error because x % 0 is infinity.
            if (first_number > 0)
            {
                mod_matrix[i][j] = queue_matrix.matrix[i][j] % first_number;
            }
            // If it is 0 or less, don't compute the modulo since they couldn't be computed.
            else
            {
                mod_matrix[i][j] = queue_matrix.matrix[i][j];
            }
        }
    }
    // Return the mod_matrix that we generated.
    return mod_matrix;
}
void *mod_thread_func(void *args)
{
    // Get the Thread ID for the given thread.
    int *myThreadID = (int *)args;

    // Check if the Mod Threads computed all the indices that we need from last_mod_submatrix_index as max value.
    while (generate_threads_queue->size >= last_mod_submatrix_index)
    {

        // Lock the mutex because current mod thread will acquire a slot
        // And increment the last_mod_submatrix_index.
        pthread_mutex_lock(&incrementer_mod_mutex);
        // If current thread is the first one, it will create the Queue with the createQueue() function.
        if (last_mod_submatrix_index == 0)
        {
            mod_threads_queue = createQueue(total_sub_matrix);
        }
        // Take the slot.
        int slot_index_taken = last_mod_submatrix_index;
        // Gather the index of the current indexed matrix, it's index value hasbeen given in Generate Thread section.
        int real_index = generate_threads_queue->queue_matrix[last_mod_submatrix_index].index;
        last_mod_submatrix_index++;
        pthread_mutex_unlock(&incrementer_mod_mutex);

        // If when current thread is computed, there are other threads that increment the last_mod_submatrix_index
        // Decrement the last_mod_submatrix_index that we've just incremented.
        // And exit the current_thread.
        if (last_mod_submatrix_index > last_generated_submatrix_index)
        {
            last_mod_submatrix_index--;
            pthread_exit(0);
            return NULL;
        }

        else
        {
            // Create the mod matrix with GenerateModMatrix function.
            int **mod_matrix;
            mod_matrix = GenerateModMatrix(generate_threads_queue->queue_matrix[slot_index_taken]);
            // Enqueue the current mod_matrix into the mod_threads_queue.
            // We are using enqueueDP because we have double pointer matrix.
            enqueueDP(mod_threads_queue, mod_matrix, real_index);

            // Lock the printer mutex.
            pthread_mutex_lock(&printer_mutex);
            printf("\n---------------------------------------------------------");
            printf("\nMod_%d   Mod_%d generated following matrix: ", *myThreadID, *myThreadID);
            print_matrixDP(mod_matrix, 5);
            printf("This matrix is generated by [%d,%d] submatrix\n", find_row(N, real_index) / 5, find_col(N, real_index) / 5);
            printf("------------------------------------------------\n");

            pthread_mutex_unlock(&printer_mutex);
        }
    }
    return NULL;
}
int main(int argc, char *argv[])
{
    // Get command line arguments.
    int option;
    int num_of_generate_threads;
    int num_of_log_threads;
    int num_of_add_threads;
    int num_of_mod_threads;
    while ((option = getopt(argc, argv, "d:n:")) != -1)
    {
        switch (option)
        {
        case 'd':
            N = atoi(optarg);
            if ((N % 5) != 0)
            {
                printf("\nError, N is not a scalar of 5, N:%d\n", N);
                exit(0);
                break;
            }

            break;
        case 'n':
            num_of_generate_threads = atoi(optarg);
            num_of_log_threads = atoi(argv[5]);
            num_of_mod_threads = atoi(argv[6]);
            num_of_add_threads = atoi(argv[7]);
            break;

        default:
            printf("\nError, option is not recognized %c\n", option);
            exit(0);
            break;
        }
    }

    printf("\n\t\t======= Program is starting =======\n\n");
    // Initialization of total_sub_matrix.
    total_sub_matrix = (N / 5) * (N / 5);

    // Initialize the mutexes.
    if (pthread_mutex_init(&incrementer_generate_mutex, NULL) != 0)
    {
        printf("\nGenerate Mutex init has failed\n");
        return 1;
    }
    if (pthread_mutex_init(&incrementer_log_mutex, NULL) != 0)
    {
        printf("\nLog Mutex init has failed\n");
        return 1;
    }
    if (pthread_mutex_init(&incrementer_mod_mutex, NULL) != 0)
    {
        printf("\nMod Mutex init has failed\n");
        return 1;
    }
    if (pthread_mutex_init(&printer_mutex, NULL) != 0)
    {
        printf("\nPrinter Mutex init has failed\n");
        return 1;
    }
    if (pthread_mutex_init(&incrementer_add_mutex, NULL) != 0)
    {
        printf("\nAdd Mutex init has failed\n");
        return 1;
    }
    if (pthread_mutex_init(&adder_mutex, NULL) != 0)
    {
        printf("\nAdder Mutex init has failed\n");
        return 1;
    }

    // Initialization of randomness, should only be called once.
    srand(time(NULL));

    //Create the arrays for the threads with the corresponding number of threads for each type.
    pthread_t generate_threads[num_of_generate_threads];
    pthread_t log_threads[num_of_log_threads];
    pthread_t mod_threads[num_of_mod_threads];
    pthread_t add_threads[num_of_add_threads];

    // Create and initialize the threads.
    for (int i = 0; i < num_of_generate_threads; i++)
    {
        pthread_create(&(generate_threads[i]), NULL, generate_thread_func, (void *)&generate_threads[i]);
    }

    for (int i = 0; i < num_of_generate_threads; i++)
    {
        pthread_join(generate_threads[i], NULL);
    }
    for (int i = 0; i < num_of_log_threads; i++)
    {
        pthread_create(&(log_threads[i]), NULL, log_thread_func, (void *)&log_threads[i]);
    }
    for (int i = 0; i < num_of_mod_threads; i++)
    {
        pthread_create(&(mod_threads[i]), NULL, mod_thread_func, (void *)&mod_threads[i]);
    }
    for (int i = 0; i < num_of_add_threads; i++)
    {
        pthread_create(&(add_threads[i]), NULL, add_thread_func, (void *)&add_threads[i]);
    }

    // Wait for all threads.
    for (int i = 0; i < num_of_log_threads; i++)
    {
        pthread_join(log_threads[i], NULL);
    }
    for (int i = 0; i < num_of_mod_threads; i++)
    {
        pthread_join(mod_threads[i], NULL);
    }
    for (int i = 0; i < num_of_add_threads; i++)
    {
        pthread_join(add_threads[i], NULL);
    }

    // Print the output that comes from Log Threads.
    printf("\n\n=== Printing the LOG Matrix ===\n");
    for (size_t i = 0; i < N; i++)
    {

        for (size_t j = 0; j < N; j++)
        {
            printf("%.2d ", log_thread_matrix[i][j]);
        }
        printf("\n");
    }

    WriteMatrixAndSumToFile("output.txt");

    // Destroy all the mutexes.
    pthread_mutex_destroy(&incrementer_generate_mutex);
    pthread_mutex_destroy(&incrementer_log_mutex);
    pthread_mutex_destroy(&incrementer_mod_mutex);
    pthread_mutex_destroy(&printer_mutex);
    pthread_mutex_destroy(&adder_mutex);
    pthread_mutex_destroy(&incrementer_add_mutex);

    // Free the malloc'd variables.
    for (size_t i = 0; i < total_sub_matrix; i++)
    {
        free(generate_threads_queue->queue_matrix[i].matrix);
    }
    for (size_t i = 0; i < total_sub_matrix; i++)
    {
        free(mod_threads_queue->queue_matrix[i].matrix);
    }

    for (size_t i = 0; i < num_of_log_threads; i++)
    {
        free(log_thread_matrix[i]);
    }
    free(log_thread_matrix);
    free(generate_threads_queue->queue_matrix);
    free(generate_threads_queue);
    free(mod_threads_queue->queue_matrix);
    free(mod_threads_queue);

    return 0;
}
