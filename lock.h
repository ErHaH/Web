#ifndef LOCK_H
#define LOCK_H
#include <pthread.h>
#include <exception>
#include <semaphore.h>
#include <iostream>

class Locker {
public:
    Locker();
    ~Locker();

    bool lock();//加锁
    bool unlock();//解锁
    pthread_mutex_t* GetLock();//获取锁

private:
    pthread_mutex_t m_mutex_;
};

class Cond {
public:
    Cond();
    ~Cond();

    bool wait(pthread_mutex_t* m_mutex);
    bool timewait(pthread_mutex_t* m_mutex, timespec& t);    
    bool signal();
    bool broadcast();
private:
    pthread_cond_t m_cond_;
};

class Sem
{
public:
    Sem() = default;
    Sem(int num);
    ~Sem();

    bool wait();
    bool post();
private:
    sem_t m_sem_;
};

#endif