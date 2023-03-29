#ifndef THREADPOOL_H
#define THREADPOOL_H

//#include <memory>
#include <pthread.h>
#include <exception>
#include <list>
#include "locker.h"

template <class T>
class threadpool {
public:
    threadpool(int thread_number = 8, int max_requests = 10000);
    ~threadpool();
    bool append(T *request);
private:
    void run();
    static void *worker(void *arg);
private:
    //std::unique_ptr<pthread_t> m_threads(new pthread_t);
    pthread_t *m_threads;
    int m_max_requests;
    int m_thread_number;
    std::list<T *> m_workqueue;
    locker m_queuelock;
    sem m_queuesem;
    bool m_stop;
};

template <class T>
threadpool<T>::threadpool(int thread_number, int max_requests) :
    m_thread_number(thread_number), m_max_requests(max_requests), m_stop(false), m_threads(NULL) {
    if((thread_number <= 0) || (max_requests <= 0)) {
        throw std::exception();
    }

    m_threads = new pthread_t[m_thread_number];
    if(m_threads == nullptr) {
        throw std::exception();
    }

    for(int i = 0; i < m_thread_number; ++i) {
        printf( "create the %dth thread\n", i);
        if(pthread_create(m_threads + i, NULL, worker, this) != 0) {
            delete [] m_threads;
            throw std::exception();
        }

        if(pthread_detach(m_threads[i])) {
            delete [] m_threads;
            throw std::exception();
        }
    }
}

template <class T>
threadpool<T>::~threadpool() {
    delete [] m_threads;
    m_stop = true;
}

template <class T>
bool threadpool<T>::append(T* request) {
    m_queuelock.lock();
    if(m_workqueue.size() > m_max_requests) {
        m_queuelock.unlock();
        return false;
    }
    m_workqueue.emplace_back(request);
    m_queuelock.unlock();
    m_queuesem.post();
    return true;
}

template <class T>
void* threadpool<T>::worker(void* arg) {
    threadpool* pool = (threadpool*)arg;
    pool->run();
    return pool;
}

template <class T>
void threadpool<T>::run() {
    while(m_stop == false) {
        m_queuesem.wait();
        m_queuelock.lock();
        if(m_workqueue.empty()) {
            m_queuelock.unlock();
            continue;
        }
        T* request = m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelock.unlock();
        if(request == nullptr) {
            continue;
        }
        request->process();
    }
}

#endif