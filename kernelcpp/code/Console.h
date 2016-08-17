#ifndef CONSOLE_H
#define CONSOLE_H

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

namespace IO
{

class Console
{
public:
	static void kclearScreen();
	static void kprint(const char *msg);
	static void kprint(char c);
	static void kprint(unsigned int c);
	static void kprint(int c);
	static void kprinthex(unsigned char c);
	static void kprinthex(int c); //prints the way int is in memory.
	static void kprintln(const char *msg);
	static void kprintln();
	
	
private:
	static void kscroll();
	static void kprintCursor();
	
	static volatile int cursorX;
	static volatile int cursorY;
};

}
#endif