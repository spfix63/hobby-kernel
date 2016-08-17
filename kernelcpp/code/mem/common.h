
#ifndef COMMON_H
#define COMMON_H

#include <cstdint>

// Some nice typedefs, to standardise sizes across platforms.
// These typedefs are written for 32-bit X86.
/* typedef unsigned int   uint32_t;
typedef          int   int32_t;
typedef unsigned short uint16_t;
typedef          short s16int;
typedef unsigned char  uint8_t;
typedef          char  int8_t; */

/* void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port); */

#define PANIC(msg) panic(msg);
#define ASSERT(b) ((b) ? (void)0 : panic_assert(__FILE__, __LINE__, #b))

extern void panic (const char *msg);
extern void panic_assert(const char *file, uint32_t line, const char *desc);

#endif // COMMON_H
