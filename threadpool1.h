#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include<bits/stdc++.h>
#include<unistd.h>
#include<queue>
template<class T>
struct tasks{
    T *(*function)(void *arg);
    void *arg;
};



enum thread_stat {need_increase,need_decrease, no_need};

template<class T>
class threadpool{
    public:
        int min_thread_number;
        int thread_number;
        int task_number;
        int waiting_task_number;
        tasks<T> *task;      //任务队列，数组 
        std::queue<tasks<T>> que;
        int front,rear;   //任务队列首尾下标
        int task_num;
        int working_thread_num;  //忙线程

        thread_stat stat;
        pthread_t *thread;  //线程，数组
        pthread_t  producer; //  生产者线程
        pthread_t  manager; //管理者线程
        pthread_cond_t task_is_empty;   //pthread_cond_t多线程条件变量，用于控制线程等待和就绪的条件
        pthread_cond_t queue_empty;
        pthread_cond_t task_change;
        pthread_mutex_t lock;
        pthread_mutex_t lock_manager;
       threadpool(int init_thread_number,int task_max);
       ~threadpool();
       void add_task(T *(*function)(void *arg),void *arg);
       //static void *work(void *thread_pool);//////////////////////////////
};


template<class T>
void* work(void *thread_pool){
    threadpool<T> *pool=(threadpool<T> *)thread_pool; 
    while(1){  
        pthread_mutex_lock(&(pool->lock));
        pool->working_thread_num--;
        while(pool->task_num==0){            //任务队列为空 ///////////////////////////////////////////////
            if(pool->working_thread_num==0){   //同时线程全空闲，通知producer线程继续生产
               pthread_cond_signal(&pool->queue_empty);
            }
            pthread_cond_wait(&(pool->task_is_empty),&(pool ->lock));

        }
        pool->working_thread_num++;

        //判断是否线程都在工作，扩容
        // if(pool->working_thread_num==pool->thread_number){
        //     pool->stat=need_increase;
        //     pthread_cond_signal(&pool->task_change);
        // }

        //从任务队列拿出任务
        tasks<T> temp;
        temp.function=(pool->task+pool->front)->function;
        temp.arg=(pool->task+pool->front)->arg;

        pool->front=(pool->front+1)%pool->task_number;
        pool->task_num--;
         //std::cout<<"task  "<<pool->task_num<<std::endl;
        

        //执行任务
  
        pthread_mutex_unlock(&(pool->lock));
        (*temp.function)(temp.arg);
    }
    return nullptr;
}

template<class T>
void* manage_thread(void *thread_pool){
    threadpool<T> *pool=(threadpool<T> *)thread_pool; 
    while (1){
        pthread_mutex_lock(&(pool->lock));
        while(pool->stat==no_need){ 
            pthread_cond_wait(&(pool->task_change),&(pool ->lock));
        }
        int incre,decre;
        if(pool->stat==need_increase){
                            //////////////////////////////////////超过最大
            incre=pool->thread_number*1.5;
            for(int i=pool->thread_number;i<incre;i++){
                pthread_create(&pool->thread[i],nullptr,work<T>,pool);
                pool->working_thread_num++;
            }
        pool->thread_number=incre;

        pool->stat=no_need;
        pthread_mutex_unlock(&(pool->lock));
    }
    return nullptr;
 }
}

template<class T>
threadpool<T>::~threadpool(){
    delete []thread;
    delete []task;
}

template<class T>
void* produce(void *thread_pool){
    threadpool<T> *pool=(threadpool<T> *)thread_pool; 
     while (1){
        pthread_mutex_lock(&(pool->lock));
        while(pool->que.size()==0||pool->task_num==pool->task_number){ 
            if(pool->task_num>0){
                pthread_cond_broadcast(&pool->task_is_empty);
            }
            pthread_cond_wait(&(pool->queue_empty),&(pool ->lock));

        }
        //将队列中的task加入到缓冲任务队列
        tasks<T> temp=pool->que.front();
        pool->rear=(pool->rear+1)%pool->task_number;
        (pool->task+pool->rear)->function=temp.function;
        (pool->task+pool->rear)->arg=temp.arg;
        pool->que.pop();
        pool->task_num++;
 
        pthread_mutex_unlock(&(pool->lock));
    }
    return nullptr;
}


template<class T>
threadpool<T>::threadpool(int init_thread_number,int task_max){
    
    this->thread_number=init_thread_number;
    this->task_number=task_max;

    this->task=new tasks<T>[task_number];
    this->thread=new pthread_t[thread_number];

    this->working_thread_num=init_thread_number;
    this->min_thread_number=init_thread_number;
    this->stat=no_need;
    this->front=0;
    this->rear=-1;
    this->task_num=0;

    this->waiting_task_number=0;
    
    pthread_mutex_init(&this->lock,nullptr);
    pthread_mutex_init(&this->lock_manager,nullptr);
    pthread_cond_init(&this->task_is_empty,nullptr);
    pthread_cond_init(&this->task_change,nullptr);
    pthread_cond_init(&this->queue_empty,nullptr);
    for(int i=0;i<thread_number;i++){
        pthread_create(&thread[i],nullptr,work<T>,this);
    }

    pthread_create(&producer,nullptr,produce<T>,this);

    //创建管理者线程
    pthread_create(&manager,nullptr,manage_thread<T>,this);
}





template<class T>
void threadpool<T>::add_task(T *(*function)(void *),void *arg){////////arg   改成int
        //加入queue
     pthread_mutex_lock(&lock);
    tasks<T> temp;
    temp.function=function;
    temp.arg=arg;

    que.push(temp);

    pthread_cond_signal(&queue_empty);
    //唤醒producer线程
     pthread_mutex_unlock(&lock);
  
}


