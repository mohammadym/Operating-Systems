#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <semaphore.h>

#define USE_SEMAPHORE 1

struct prime
{
    int value;
    int counter;
};

int main(int argc, char *argv[])
{
    sem_t *mutex;
    if (USE_SEMAPHORE) 
    {
        mutex = (sem_t *) mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
        sem_init(mutex, 1, 0);
    }
    const int SIZE = 8192;
    const char *name = "/SHM";
    int fd;
    char *ptr;
    for(int i=0;i<8;i++)
    {
        int producer;
        if((producer = fork()) < 0)
        {
            printf("fork failed\n");
            exit(-1);
        }
        else if((producer = fork()) > 0)
        {
            fd = shm_open(NULL, O_CREAT | O_RDWR, 0666);
            if (fd < 0)
            {
                printf("Error in producer\n");
            }
            ftruncate(fd, SIZE);
            struct prime *queue;
            ptr = (struct prime *) mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            if (ptr == MAP_FAILED)
                printf("failed in mapping\n");
            int count = 0;
            while(count <= 1000)
            {
                for(int j=2;;j++)
                {
                    int flag = 0;
                    for(int k=2;k<=j/2;k++)
                    {
                        int r = j%k;
                        if(r == 0)
                        {
                            flag = 1;
                            break;
                        }
                    }
                    if(flag == 0)
                    {
                        queue->value = j;
                        count++;
                        queue->counter = count;
                    }
                }
                ptr += i * 125;
                if (USE_SEMAPHORE) 
                {
                    sem_post(mutex);
                }
            }   
        }
        else 
        {
            int copy = i;
            if (USE_SEMAPHORE) 
            {
                sem_wait(mutex);
            } 
            else
            {
                sleep(5);
            }
            fd = shm_open(name, O_RDONLY, 06666);
            if (fd < 0)
            {
                printf("Error in consumer\n");
            }
            ptr = (struct prime *) mmap(NULL, SIZE, PROT_READ, MAP_SHARED, fd, 0);
            ptr += copy * 125;
            while((((struct prime *)ptr)->counter) > 0)
            {
                ((struct prime *)ptr)->value + copy;
                ((struct prime *)ptr)->counter--;
                copy++;
                ptr += copy * 125;
            }
        }
        if(producer != 0)
            wait(NULL);
    } 
    for(int i=0;i<8;i++)
        wait(NULL);
    return 0;

}