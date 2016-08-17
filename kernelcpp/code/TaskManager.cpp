#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <new>

#include "Console.h"
#include "IPC.h"
#include "MemoryManagement.h"
#include "TaskManager.h"
#include "mem/heap.h"
#include "mem/vmm.h"

thread_list_t *current_thread = NULL;
void thread_start(void *launchable);
void thread_exit();
	
TaskManager::TaskManager() 
{
	ready_queue = NULL;
	tid = 0;
}
	
TaskManager& TaskManager::getInstance()
{
	static TaskManager instance;
	return instance;
}	
	
void TaskManager::init()
{
	thread_t *initial_thread = (thread_t *)kernel_malloc(sizeof(thread_t));
	initial_thread->id = tid++;
	initial_thread->stack = NULL;
	initial_thread->heap = (Heap *)kernel_malloc(sizeof(Heap));
	new (initial_thread->heap) Heap();
	Heap::currentHeap = initial_thread->heap;
	
	initial_thread->page_directory = current_page_directory;

	current_thread = (thread_list_t *)kernel_malloc(sizeof(thread_list_t));
	current_thread->thread = initial_thread;
	current_thread->next = NULL;
	current_thread->remove = false;
	ready_queue = NULL;
	IO::Console::kprintln("init--");
}

int TaskManager::createProcess(Launchable *child)
{
	//has to be kernel page directory
	
	IO::Console::kprintln("++pd");
	
	uint32_t *pd = create_page_directory();
	IO::Console::kprint("pd: ");
	IO::Console::kprinthex((int)pd);
	IO::Console::kprintln();
	
	Heap *heap = (Heap *)kernel_malloc(sizeof(Heap));
	
	//heapo konstruktorius pirma puslapi sumappina iskart..
	switch_page_directory(pd);	
	new (heap) Heap();
	
	IO::Console::kprint("Heap: ");
	IO::Console::kprinthex((int)heap);
	IO::Console::kprintln();
	
	//kursim thread'o stacka and thread'o heapo.
	Heap::currentHeap = heap;
	int result = createThread(child, pd, heap);
	Heap::currentHeap = current_thread->thread->heap;
	
	switch_page_directory(current_thread->thread->page_directory);
	
	return result;
}

int TaskManager::createThread(Launchable *child)
{
	return createThread(child, current_thread->thread->page_directory, current_thread->thread->heap);
}
	
int TaskManager::createThread(Launchable *child, uint32_t *pd, Heap *heap)
{
	// uint32_t *stack = new uint32_t[0x100] + 0xFC;
	
	IO::Console::kprint("PD: ");
	IO::Console::kprinthex((int)current_page_directory);
	IO::Console::kprintln();
	
	//thread'a alokuojam ant kernelio heap'o
	thread_t *thread = (thread_t *)kernel_malloc(sizeof(thread_t));
	memset(thread, 0, sizeof (thread_t));
	thread->id = tid++;
	thread->page_directory = pd;
	// thread->page_directory = current_page_directory;
	thread->heap = heap;
	
	IO::Console::kprintln("2");
	//thread'o stacka alokuojam ant threado heapo
	thread->stack = new uint32_t[0x100];
	uint32_t *stack = thread->stack + 0xFC;

	*--stack = (uint32_t)child;
	*--stack = (uint32_t)&thread_exit;
	*--stack = (uint32_t)&thread_start;

	thread->esp = (uint32_t)stack;
	thread->ebp = 0;
	thread->eflags = 0x200; // Interrupts enabled.
	
	addThread(thread);
	
	IO::Console::kprintln("create--");
	return 0;
}


void TaskManager::addThread(thread_t *t)
{
	// Create a new list item for the new thread.
	thread_list_t *item = (thread_list_t *)kernel_malloc(sizeof(thread_list_t));;
	item->thread = t;
	item->next = 0;
	item->remove = false;

	if (!ready_queue)
	{
		// Special case if the ready queue is empty.
		ready_queue = item;
		
		IO::Console::kprintln("addThread--");
	}
	else
	{
		// Iterate through the ready queue to the end.
		thread_list_t *iterator = ready_queue;
		while (iterator->next)
			iterator = iterator->next;
		// Add the item.
		iterator->next = item;
	}
}

void TaskManager::removeThread(thread_t *t)
{
	// Attempt to find the thread in the ready queue.
	thread_list_t *iterator = ready_queue;

	// Special case if the thread is first in the queue.
	if (iterator->thread == t)
	{
		ready_queue = iterator->next;
		kernel_free (iterator);
		return;
	}

	while (iterator->next)
	{
		if (iterator->next->thread == t)
		{
			thread_list_t *tmp = iterator->next;
			iterator->next = tmp->next;
			kernel_free (tmp);
		}
		iterator = iterator->next;
	}
}	


bool TaskManager::isRunning(uint32_t id)
{
	if (current_thread->thread->id == id)
		return true;
	thread_list_t *iterator = ready_queue;
	while (iterator)
	{
		if (iterator->thread->id == id)
			return true;
		iterator = iterator->next;
	}
	return false;
}

uint32_t TaskManager::currentThread()
{
	return current_thread->thread->id;
}

void TaskManager::handleTimerEvent()
{
	//allocated on stack, accesible to all processes
	TaskManager &tm = getInstance();
	if (!tm.ready_queue)
		return;
			
	if (current_thread->remove)
	{
		/*thread stack is allocated in it's virtual space 
		and gets destroyed by destroying the page table*/
		/*if (current_thread->thread->stack)
			delete[] current_thread->thread->stack;*/
		//atlaisvinti heapa!
		IPC &ipc = IPC::getInstance();
		ipc.clearMessages(current_thread->thread->id);
		
		current_thread->thread->heap->~Heap();
		kernel_free(current_thread->thread->heap);
		kernel_free(current_thread->thread);
		tm.removeThread(current_thread->thread);
		current_thread = NULL;
	}
	else
	{
		// Iterate through the ready queue to the end.
		thread_list_t *iterator = tm.ready_queue;
		while (iterator->next)
			iterator = iterator->next;

		// Add the old thread to the end of the queue, and remove it from the start.
		iterator->next = current_thread;
		current_thread->next = 0;
	}
	thread_list_t *new_thread = tm.ready_queue;
	tm.ready_queue = tm.ready_queue->next;
	
	current_thread->thread->heap = Heap::currentHeap;
	Heap::currentHeap = new_thread->thread->heap;
	// Switch to the new thread.
	switch_thread (new_thread);
}

void thread_start(void *launchable)
{
	Launchable *l = (Launchable *)launchable;
	l->launch();
}

void thread_exit ()
{
	IO::Console::kprintln("Thread returned ");
	
	//Galima ir cia istrint threada.
	current_thread->remove = true;
	for (;;) IO::Console::kprint("A");;
}

