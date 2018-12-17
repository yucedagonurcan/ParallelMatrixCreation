#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>

int generate_random_number(int max);
void *generate_thread_func(void *args);
int **create_matrix(int rows, int cols);
void print_matrix(int **matrix, int rows, int cols);

int **global_matrix;
int last_generated_submatrix_index = 0;
int total_sub_matrix;
pthread_mutex_t incrementer_mutex;

int **create_matrix(int rows, int cols)
{
    int **matrix = (int **)malloc(rows * sizeof(int *));
    for (size_t i = 0; i < rows; i++)
    {
        matrix[i] = (int *)malloc(cols * sizeof(int));
    }
    return matrix;
}
void print_matrix(int **matrix, int rows, int cols)
{

    for (size_t i = 0; i < rows; i++)
    {
        printf("\n");

        for (size_t j = 0; j < cols; j++)
        {
            printf("%d ", matrix[i][j]);
        }
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

        printf("\nTrying to locked by thread %d\n", *myThreadID);
        pthread_mutex_lock(&incrementer_mutex);
        printf("\nLocked by thread: %d and thread_number: %d\n", *myThreadID, last_generated_submatrix_index);

        if (total_sub_matrix > last_generated_submatrix_index)
        {
            for (int j = 0; j < 5; j++)
            {
                global_matrix[last_generated_submatrix_index][j] = vector[j];
            }
            last_generated_submatrix_index++;
            printf("\nIncremented thread_number by thread: %d and thread_number: %d\n", *myThreadID, last_generated_submatrix_index);
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
    ///

    total_sub_matrix = (N / 5) * (N / 5);
    global_matrix = create_matrix(total_sub_matrix, 5);

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
    print_matrix(global_matrix, total_sub_matrix, 5);

    for (size_t i = 0; i < total_sub_matrix; i++)
    {
        free(global_matrix[i]);
    }

    free(global_matrix);
    return 0;
}