#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
int generate_random_number(int max);
void *generate_thread_func(void *args);

int **global_matrix;
int current_thread_number = 0;
void print_matrix(int **matrix, int rows, int cols);
pthread_mutex_t incrementer_mutex;

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
    int vector[5];
    for (int i = 0; i < 5; i++)
    {
        int random_number = generate_random_number(100);
        vector[i] = random_number;
    }

    printf("\nTrying to locked by thread %d\n", *myThreadID);
    pthread_mutex_lock(&incrementer_mutex);
    printf("\n Locked by thread: %d and thread_number: %d\n", *myThreadID, current_thread_number);

    for (int j = 0; j < 5; j++)
    {
        global_matrix[current_thread_number][j] = vector[j];
    }

    current_thread_number++;
    printf("\n Incremented thread_number by thread: %d and thread_number: %d\n", *myThreadID, current_thread_number);

    pthread_mutex_unlock(&incrementer_mutex);
    return NULL;
}

int generate_random_number(int max)
{
    return rand() % max;
}
int main(void)
{

    int N = 30;
    int total_sub_matrix = (N / 5) * (N / 5);
    global_matrix = (int **)malloc(total_sub_matrix * sizeof(int *));
    for (size_t i = 0; i < total_sub_matrix; i++)
    {
        global_matrix[i] = (int *)malloc(5 * sizeof(int));
    }

    if (pthread_mutex_init(&incrementer_mutex, NULL) != 0)
    {
        printf("\n Mutex init has failed\n");
        return 1;
    }
    // Initialization, should only be called once.
    srand(time(NULL));
    // It will change!
    int num_of_generate_threads = total_sub_matrix;

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

    free(global_matrix);
    return 0;
}