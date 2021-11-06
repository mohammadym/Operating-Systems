

Sina Shabani Kumeleh - 97521351
## Cross Memory Attach VS other IPC methods
Pipes and shared memory live on the kernel and are managed by the kernel. But in CMA after you transferred required addresses with one of the traditional IPC methods i.e. pipes, you can directly transfer data to other processes that have the required address in user space without the help of kernel.

## The safety of the pipe mechanism
According to the man page [pipe(7)](https://man7.org/linux/man-pages/man7/pipe.7.html) as long as you write less than **PIPE_BUF** the atomicity of the read and write operations are guaranteed. But the overall safety depends on the number of bytes we're trying to read or write, the file descriptor working in a nonblocking fashion or not, and the count of the writers. Now we go over four possible scenarios.
1. **O_NONBLOCK** disabled, _n_ <= **PIPE_BUF**
_n_ bytes are written atomically. and if the pipe is full, having data from other writes maybe, the write will block and write the rest after there is enough room in the pipe.
2. **O_NONBLOCK** enabled, _n_ <= **PIPE_BUF**
this is safe like the last one. If there is room in the pipe _n_ bytes are written, if not, write will fail with errno being **EAGAIN** which means that resource is not available at this moment.
3. **O_NONBLOCK** disabled, _n_ > **PIPE_BUF**
_n_ is bigger than **PIPE_BUFF** so the write is not atomic and data may be "interleaved" by other processes performing the write operation. this means that it's not safe this way.
4. **O_NONBLOCK** enabled, _n_ > **PIPE_BUF**
if the pipe is full write fails like the second scenario, setting errno to **EAGAIN** otherwise, 1 to n bytes may be written so we have to keep track of the return value from write to check bytes written. This is not safe also and data may get corrupted by other processes performing the write operation.

## Sharing pthread_mutex between processes
The short answer is yes, you can use mutexes between threads of multiple processes. But for the long answer, your mutex needs to be in the memory shared with other processes and it needs to be initialized in a way to allow other processes accessing the lock. For this to work all mutexes must use **PTHREAD_PROCESS_SHARED** attribute as said in [pthread_mutexattr_setpshared(3)](https://man7.org/linux/man-pages/man3/pthread_mutexattr_setpshared.3.html). With this attribute any thread that has access to the shared memory can acquire or release the lock. But it cannot work like a semaphore because only the owner of a lock can unlock it. There is a type of mutex called **Recursive Mutex** that has a count like semaphores but it's only for one thread. This means that if a thread locks a mutex and the type is **Recursive** ( you can set the type by setting the attribute), the same thread can lock it again and again without resulting in a deadlock. However, this is a bad practice because you are using a lock to keep your data safe and while you have acquired the lock why the heck you want to lock it again?! looks to me like you designed your code poorly and need to rethink your design.
