
#include "common.h"
#include "../Console.h"

// Write a byte out to the specified port.
void outb(uint16_t port, uint8_t value)
{
    asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}

uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
    return ret;
}

uint16_t inw(uint16_t port)
{
    uint16_t ret;
    asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
    return ret;
}

// Copy len bytes from src to dest.
void memcpy(uint8_t *dest, const uint8_t *src, uint32_t len)
{
    const uint8_t *sp = (const uint8_t *)src;
    uint8_t *dp = (uint8_t *)dest;
    for(; len != 0; len--) *dp++ = *sp++;
}

// Write len copies of val into dest.
extern "C" void memset(uint8_t *dest, uint8_t val, uint32_t len)
{
    uint8_t *temp = (uint8_t *)dest;
    for ( ; len != 0; len--) *temp++ = val;
}

// Compare two strings. Should return -1 if 
// str1 < str2, 0 if they are equal or 1 otherwise.
int strcmp(char *str1, char *str2)
{
      int i = 0;
      int failed = 0;
      while(str1[i] != '\0' && str2[i] != '\0')
      {
          if(str1[i] != str2[i])
          {
              failed = 1;
              break;
          }
          i++;
      }
      // why did the loop exit?
      if( (str1[i] == '\0' && str2[i] != '\0') || (str1[i] != '\0' && str2[i] == '\0') )
          failed = 1;
  
      return failed;
}

// Copy the NULL-terminated string src into dest, and
// return dest.
char *strcpy(char *dest, const char *src)
{
    do
    {
      *dest++ = *src++;
    }
    while (*src != 0);
	return dest;
}

// Concatenate the NULL-terminated string src onto
// the end of dest, and return dest.
char *strcat(char *dest, const char *src)
{
    while (*dest != 0)
    {
        dest++;
    }

    do
    {
        *dest++ = *src++;
    }
    while (*src != 0);
    return dest;
}

int strlen(char *src)
{
    int i = 0;
    while (*src++)
        i++;
    return i;
}

extern void panic(const char *message)
{
    // We encountered a massive problem and have to stop.
    asm volatile("cli"); // Disable interrupts.

	IO::Console::kprint("PANIC(");
	IO::Console::kprint(message);
    IO::Console::kprint(")");
    IO::Console::kprintln();
	
	/* 
    monitor_write("PANIC(");
    monitor_write(message);
    monitor_write(") at ");
    monitor_write(file);
    monitor_write(":");
    monitor_write_dec(line);
    monitor_write("\n"); */
    // Halt by going into an infinite loop.
    for(;;);
}

extern void panic_assert(const char *file, uint32_t line, const char *desc)
{
    // An assertion failed, and we have to panic.
    asm volatile("cli"); // Disable interrupts.	
	
	IO::Console::kprint("ASSERTION-FAILED(");
	IO::Console::kprint(desc);
    IO::Console::kprint(") at ");
    IO::Console::kprint(file);
    IO::Console::kprint(":");
    IO::Console::kprint((int)line);
    IO::Console::kprintln();
	
    /* monitor_write("ASSERTION-FAILED(");
    monitor_write(desc);
    monitor_write(") at ");
    monitor_write(file);
    monitor_write(":");
    monitor_write_dec(line);
    monitor_write("\n"); */
    // Halt by going into an infinite loop.
    for(;;);
}
