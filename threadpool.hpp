#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <iostream>
#include <list>
#include <pthread.h>
#include <exception>
#include "lock.h"

template <class T>
class ThreadPool {
public:
    ThreadPool(int thread_num = 8, int max_request = 10000);
    ~ThreadPool();
    bool AppendRequest(T* request);

private:
    static void* Worker(void* arg);
    void RunTask();

private:
    //线程数组
    pthread_t* m_thread_;

    //线程数量
    int m_thread_num_;

    //工作队列
    std::list<T*> m_workqueue_;

    //最大工作队列数
    int m_workqueue_maxnum_;

    Locker m_queuelock_;
    Sem m_queuesem_;
    bool m_stop_;
};

template <class T>
ThreadPool<T>::ThreadPool(int thread_num, int max_request):
    m_thread_num_(thread_num), m_workqueue_maxnum_(max_request), m_stop_(false), m_thread_(NULL) {
    if(m_thread_num_ <= 0 || m_workqueue_maxnum_ <= 0) {
        throw std::exception();
    }
    else{
        m_thread_ = new pthread_t[m_thread_num_];
        if(!m_thread_) {
            throw std::exception();
        }
    }

    for(int i = 0; i < m_thread_num_; ++i) {
        if(pthread_create(m_thread_ + i, NULL, Worker, this) != 0) {
            delete[] m_thread_;
            throw std::exception();
        }

        if(pthread_detach(*(m_thread_ + i)) != 0) {
            delete[] m_thread_;
            throw std::exception();            
        }

        std::cout << "Create " << i + 1 << "th thread..." << std::endl;
    }
}

template <class T>
ThreadPool<T>::~ThreadPool() {
    delete[] m_thread_;
    m_stop_ = true;
}

template <class T>
bool ThreadPool<T>::AppendRequest(T* request) {
    m_queuelock_.lock();
    if(m_workqueue_.size() >= m_workqueue_maxnum_) {
        m_queuelock_.unlock();
        return false;
    }
    m_workqueue_.emplace_back(request);
    m_queuelock_.unlock();
    m_queuesem_.post();
    return true;
}

template <class T>
void* ThreadPool<T>::Worker(void* arg) {
    ThreadPool* pool = (ThreadPool*) arg;
    pool->RunTask();
    return pool;
}

template <class T>
void ThreadPool<T>::RunTask() {
    while(!m_stop_) {
        m_queuesem_.wait();
        m_queuelock_.lock();
        if(m_workqueue_.empty()) {
            m_queuelock_.unlock();
            continue;
        }
        T* request = m_workqueue_.front();
        m_workqueue_.pop_front();
        m_queuelock_.unlock();
        if(!request) {
            continue;
        }
        std::cout << "I am runing..." << std::endl;
    }
}

#endif