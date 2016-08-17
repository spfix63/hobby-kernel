#include "colors.h"
#include "Console.h"

using namespace IO;

volatile int Console::cursorX = 0;
volatile int Console::cursorY = 0;


void Console::kclearScreen()
{
	unsigned char *videoram = (unsigned char *) COLOR_TEXT_MODE;
	int pos;
	int i;
	int j;
	for (j = 0; j < SCREEN_HEIGHT; j++)
	{
		for (i = 0; i < SCREEN_WIDTH; i++)
		{
			pos = 2*(i + j*SCREEN_WIDTH);
			videoram[pos] = '\0';
			videoram[pos + 1] = COLOR_BLACK;
		}
	}
	cursorX = 0;
	cursorY = 0;
	kprintCursor();
}


void Console::kprint(const char *msg)
{
	unsigned char *videoram = (unsigned char *) COLOR_TEXT_MODE;
	int pos;
	int i;
	for (i = 0; msg[i] != '\0'; i++)
	{
		if (cursorY == SCREEN_HEIGHT)
		{
			kscroll();
			kscroll();
			kscroll();
		}
		if (cursorX == SCREEN_WIDTH)
		{
			cursorY++;
			cursorX = 0;
			i--;
			continue;
		}
		if (msg[i] == '\n')
		{
			cursorY++;
			cursorX = 0;
			continue;
		}
		else if (msg[i] == '\r')
		{
			cursorX = 0;
			continue;
		}
		else if (msg[i] == '\b')
		{
			if (cursorX > 0)
				cursorX--;
			else if (cursorY > 0)
			{
				cursorY--;
				cursorX = SCREEN_WIDTH - 1;
			}
			pos = 2*(cursorY*SCREEN_WIDTH + cursorX);
			videoram[pos] = 0;
			videoram[pos + 1] = COLOR_WHITE;
			continue;
		}
		pos = 2*(cursorY*SCREEN_WIDTH + cursorX);
		videoram[pos] = msg[i];
		videoram[pos + 1] = COLOR_WHITE;
		cursorX++;
	}
}


void Console::kprint(char c)
{
	char ca[2] = { '\0', '\0' };
	ca[0] = c;
	kprint(ca);
}


void Console::kprint(unsigned int c)
{
	char ats[11] = { '\0' };
	char *ptr = ats + 9;
	if (c == 0)
		*(--ptr) = '0';
	while (c > 0)
	{
		*(--ptr) = static_cast<char>(c%10) + 0x30;
		c /= 10;
	}
	kprint(ptr);
}

void Console::kprint(int c)
{
	kprint((unsigned int)c);
}

void Console::kprinthex(unsigned char c)
{
	char ca[3] = { '\0' };
	int n = c >> 4;
	if (n >= 10)
		n += 55;
	else
		n += 48;
	ca[0] = static_cast<char>(n);
	n = c & 0x0F;
	if (n >= 10)
		n += 55;
	else
		n += 48;
	ca[1] = static_cast<char>(n);
	kprint(ca);
}


void Console::kprinthex(int c)
{
	unsigned char *ptr =(unsigned char *) &c;
	kprinthex(*ptr);
	kprinthex(*(ptr+1));
	kprinthex(*(ptr+2));
	kprinthex(*(ptr+3));
}


void Console::kprintln(const char *msg)
{
	kprint(msg);
	kprint("\r\n");
	kprintCursor();
}

void Console::kprintln()
{
	kprintln("");
}

void Console::kprintCursor()
{
	kprint(">");
}


void Console::kscroll()
{
	unsigned char *videoram = (unsigned char *) COLOR_TEXT_MODE;
	int pos1;
	int pos2;
	int i;
	int j;
	for (j = 1; j < SCREEN_HEIGHT; j++)
	{
		for (i = 0; i < SCREEN_WIDTH; i++)
		{
			pos1 = 2*(i + (j - 1)*SCREEN_WIDTH);
			pos2 = 2*(i + j*SCREEN_WIDTH);
			videoram[pos1] = videoram[pos2];
			videoram[pos1 + 1] = videoram[pos2 + 1];
		}
	}
	for (i = 0; i < SCREEN_WIDTH; i++)
	{
			pos2 = 2*(i + SCREEN_WIDTH*(SCREEN_HEIGHT - 1));
			videoram[pos2] = '\0';
			videoram[pos2 + 1] = COLOR_BLACK;
	}
	cursorY--;
}


