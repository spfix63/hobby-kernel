#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "mem/heap.h"

/*
TODO:
Reikia atskiro heapo kerneliui, kuris butu nuo 2 iki 4 mb kazkur,
ir butu identity sumappintas. Tada visiem procesam identity mappint
pirmus 4 mb. Visus threadu aprasymus, heapa, zinuciu siuntima daryt
per kernelio heapa. Tada nereikia switchintis direktoriju ir kito sh.
Viskas sugrazinta kaip buvo, direktoriju kurimas yra, taciau createThread
funckijoj neuzsettinama direktorija.
*/

class Launchable
{
public:
	virtual void launch() { };
	virtual const char *getName() { return 0; };
};

class Heap;
typedef struct
{
	uint32_t esp, ebp, ebx, esi, edi, eflags;
	uint32_t *page_directory;
	Heap *heap;
	uint32_t id;
	uint32_t *stack;
} thread_t;

typedef struct thread_list
{
	thread_t *thread;
	struct thread_list *next;
	bool remove;
} thread_list_t;

class TaskManager
{
friend class IDT;
public:
	static TaskManager& getInstance();
	
	void init();
	int createProcess(Launchable *);
	int createThread(Launchable *);
	int createThread(Launchable *child, uint32_t *page_directory, Heap *heap);
	
	void addThread(thread_t *t);	
	void removeThread(thread_t *t);	
	bool isRunning(uint32_t id);
	uint32_t currentThread();

	static void handleTimerEvent();
private:
	TaskManager();
	TaskManager(TaskManager const &);
	void operator=(TaskManager const &);
		
		
	int tid;
	thread_list_t *ready_queue;
};

extern "C" void switch_thread (thread_list_t *next);

#endif
