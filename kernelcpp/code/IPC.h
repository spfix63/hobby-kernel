#ifndef IPC_H
#define IPC_H

struct dataq
{
	uint32_t tid;
	uint32_t cbData;
	void *data;
	struct dataq *next;
};

class IPC
{
public:
	static IPC& getInstance();
	
	void sendMessage(uint32_t tid, const void *data, uint32_t cbData);
	void *receiveMessage(uint32_t &cbData);
	void clearMessages(uint32_t tid);
	
private:
	IPC();
	IPC(IPC const &);
	void operator=(IPC const &);
		
	struct dataq *msgs;
};

#endif
