#ifdef __linux__
#include <pthread.h>
#include <semaphore.h>

typedef pthread_t LzyThread; 
typedef pthread_mutex_t LzyMutex;
typedef sem_t LzySemaphore;

#endif