#include <cstdint>
#include <cstddef>
#include <cstring>

#include "Console.h"
#include "IPC.h"
#include "MemoryManagement.h"
#include "TaskManager.h"


IPC::IPC()
{
	msgs = 0;
}


IPC& IPC::getInstance()
{
	static IPC instance;
	return instance;
}

void IPC::sendMessage(uint32_t tid, const void *data, uint32_t cbData)
{
	asm("cli");
		
	TaskManager &tm = TaskManager::getInstance();
	if (tm.isRunning(tid))
	{
		struct dataq *msg = (struct dataq *)kernel_malloc(sizeof(struct dataq));
		msg->next = 0;
		msg->tid = tid;
		msg->cbData = cbData;
		msg->data = kernel_malloc(cbData);
		memcpy(msg->data, data, cbData);
		
		if (!msgs)
			msgs = msg;
		else
		{
			struct dataq *it = msgs;
			while (it->next) it = it->next;
			it->next = msg;
		}
	}
	asm("sti");
}

void *IPC::receiveMessage(uint32_t &cbData)
{
	asm("cli");
	
	TaskManager &tm = TaskManager::getInstance();
	uint32_t id = tm.currentThread();
	struct dataq *iterator = msgs;
	if (iterator && iterator->tid == id)
	{
		cbData = iterator->cbData;
		void *data = new unsigned char[cbData];
		memcpy(data, iterator->data, cbData);
		msgs = msgs->next;
		
		kernel_free(iterator);
		
		asm("sti");
		return data;
	}
	
	while (iterator->next)
	{
		if (iterator->next->tid == id)
		{
			cbData = iterator->next->cbData;
			void *data = new unsigned char[cbData];
			memcpy(data, iterator->next->data, cbData);
			
			struct dataq *tmp = iterator->next;
			iterator->next = tmp->next;
			kernel_free(tmp);
			asm("sti");
			return data;
		}
		iterator = iterator->next;
	}
	asm("sti");
	return NULL;
}

void IPC::clearMessages(uint32_t tid)
{
	asm("cli");
	
	struct dataq *iterator = msgs;
	if (iterator && iterator->tid == tid)
	{
		msgs = iterator->next;
		kernel_free (iterator);
	}

	while (iterator && iterator->next)
	{
		if (iterator->next->tid == tid)
		{
			struct dataq *tmp = iterator->next;
			iterator->next = tmp->next;
			kernel_free (tmp);
		}
		iterator = iterator->next;
	}
	
	asm("sti");
}
	