#ifndef PORTIO_H
#define PORTIO_H

namespace IO
{

namespace PortIO
{
	void outb(unsigned short port, unsigned char b);
	void outw(unsigned short port, unsigned short w);
	unsigned char inb(unsigned short port);
	unsigned short inw(unsigned short port);
	void io_wait(void);
}

}

#endif
