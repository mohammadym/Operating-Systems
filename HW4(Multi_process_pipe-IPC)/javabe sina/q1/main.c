#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <assert.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#define NUM_PROCESSES 8
#define NUM_PRIMES 100

typedef struct prime
{
    int value;   // value of the prime number
    int counter; // a counter to decrement (at the start counter = value)
} prime_t;

bool is_prime(int);
prime_t **find_primes(int);
void handle(int);
int main()
{
    int parent_fd[2];
    int fd[2 * NUM_PROCESSES];
    pid_t child[NUM_PROCESSES];

    pipe(parent_fd);

    for (size_t i = 0; i < NUM_PROCESSES; i++)
    {
        pipe(&fd[2 * i]);
    }
    for (size_t i = 0; i < NUM_PROCESSES; i++)
    {
        child[i] = fork();
        assert(child[i] >= 0 && "failed in forking new processes!");
        if (child[i] == 0)
        {
            signal(SIGINT, handle);
            close(parent_fd[0]);
            prime_t p;
            while (1)
            {
                int n = read(fd[2 * i], &p, sizeof(prime_t));
                p.value += i;
                p.counter--;
                int nextpid = 2 * ((i + 1) % NUM_PROCESSES);
                if (p.counter == 0)
                    n = write(parent_fd[1], &p, sizeof(prime_t));
                else
                    n = write(fd[nextpid + 1], &p, sizeof(prime_t));
            }
            return 0;
        }
    }

    // prime numbers
    prime_t **primes = find_primes(NUM_PRIMES);

    close(parent_fd[1]);
    // Assign the prime numbers to PUs
    for (size_t i = 0; i < NUM_PRIMES; i++)
    {
        prime_t *p = primes[i];
        int pid = p->value % NUM_PROCESSES;
        int n = write(fd[2 * pid + 1], p, sizeof(prime_t));
    }
    prime_t p;
    for (size_t i = 0; i < 100; i++)
    {
        int n = read(parent_fd[0], &p, sizeof(prime_t));
        printf("Done %d:%d\n", p.value, p.counter);
    }

    for (size_t i = 0; i < NUM_PROCESSES; i++)
    {
        kill(child[i], SIGINT);
    }

    for (size_t i = 0; i < NUM_PRIMES; i++)
    {
        free(primes[i]);
    }

    free(primes);

    return 0;
}

void handle(int signum)
{
    printf("pid: %d interrupted\n", getpid());
    exit(EXIT_SUCCESS);
}

bool is_prime(int num)
{
    for (int i = 2; i * i <= num; i++)
    {
        if (num % i == 0)
            return false;
    }

    return true;
}

prime_t **find_primes(int n)
{
    int i = 3;
    int count = 1;
    prime_t **primes = malloc(sizeof(prime_t *) * n);
    primes[count - 1] = malloc(sizeof(prime_t));
    primes[count - 1]->value = 2;
    primes[count - 1]->counter = 2;

    while (count != n)
    {
        if (is_prime(i))
        {
            primes[count] = malloc(sizeof(prime_t));
            primes[count]->value = i;
            primes[count]->counter = i;
            count++;
        }
        i += 2;
    }

    return primes;
}