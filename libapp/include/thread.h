#ifndef __THREAD_H__
#define __THREAD_H__

int fork();
int create_thread();

typedef unsigned long int pthread_t;

struct sched_param {
	int __sched_priority;
};

typedef struct {
	void *(*start_routine) (void *);
	void * args;
} __ThreadArgs;

typedef struct {
	int		detachstate;			// 线程的分离状态
	int		schedpolicy;			// 线程调度策略
	struct sched_param schedparam;	// 线程的调度参数
	int		inheritsched;			// 线程的继承性
	int		scope;					// 线程的作用域
	size_t	guardsize;				// 线程栈末尾的警戒缓冲区大小
	int		stackaddr_set;			//
	void *	stackaddr;				// 线程栈的位置
	size_t	stacksize;				// 线程栈的大小
} pthread_attr_t;

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);
int pthread_join(pthread_t thread, void **retval);

#endif
