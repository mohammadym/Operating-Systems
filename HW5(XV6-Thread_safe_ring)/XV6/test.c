#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "user.h"
#include "x86.h"

// pass process count as argument

int main(int argc, const char *argv[])
{
    if (argc != 2)
    {
        printf(2, "wrong argument...\n");
        exit();
    }

    const int pcount = atoi(argv[1]);

    if (pcount < 0 || pcount > 64)
    {
        printf(2, "process count must be positive and smaller than 64...\n");
        exit();
    }

    int len;
    int pids[pcount];
    unsigned short i;
    struct proc_info *procs = NULL;

    // creating child processes
    for (i = 0; i < pcount; ++i)
    {
        if ((pids[i] = fork()) == 0)
        {
            char *data = malloc(sizeof(char) * (i + 1) * 45056);
            memset(data, 65, sizeof(char) * (i + 1) * 45056);
            while (1)
                ;

            exit();
        }
    }

    sleep(50);

    // getting process info
    procs = malloc(sizeof(struct proc_info) * NPROC);
    pinfo(procs, &len);
    printf(1, "pid\tsize\n");

    for (i = 0; i < len; ++i)
        printf(1, "%d\t%d\n", procs[i].pid, procs[i].sz);

    // killing and waiting for child processes to prevent zomibe processes
    for (i = 0; i < pcount; ++i)
    {
        kill(pids[i]);
        wait();
    }

    free(procs);

    exit();
}
