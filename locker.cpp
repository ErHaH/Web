#include "lock.h"

Locker::Locker() {
    if(pthread_mutex_init(&m_mutex_, NULL) != 0) {
        throw std::exception();
    }
}

Locker::~Locker() {
    pthread_mutex_destroy(&m_mutex_);
}

bool Locker::lock() {
    return pthread_mutex_lock(&m_mutex_) == 0;
}

bool Locker::unlock() {
    return pthread_mutex_unlock(&m_mutex_) == 0;    
}

pthread_mutex_t* Locker::GetLock() {
    return &m_mutex_;
}


Cond::Cond() {
    if(pthread_cond_init(&m_cond_, NULL) != 0) {
        throw std::exception();
    }
}

Cond::~Cond() {
    pthread_cond_destroy(&m_cond_);
}

bool Cond::wait(pthread_mutex_t* m_mutex) {
    return pthread_cond_wait(&m_cond_, m_mutex) == 0;
}

bool Cond::timewait(pthread_mutex_t* m_mutex, timespec& t) {
    return pthread_cond_timedwait(&m_cond_, m_mutex, &t) == 0;
}  

bool Cond::signal() {
    return pthread_cond_signal(&m_cond_) == 0;
}
bool Cond::broadcast() {
    return pthread_cond_broadcast(&m_cond_) == 0;
}

Sem::Sem(int num) {
    if(num < 0) {
        std::cout << "ERR:参数num应大于0!" << std::endl;
    }
    else {
        if(sem_init(&m_sem_, 0, num) != 0) {
            throw std::exception();
        }
    }

}
Sem::~Sem() {
    sem_destroy(&m_sem_);
}

bool Sem::wait() {
    return sem_wait(&m_sem_) == 0;
}

bool Sem::post() {
    return sem_post(&m_sem_) == 0;
}