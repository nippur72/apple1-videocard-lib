#ifndef UTILS_H
#define UTILS_H

#define POKE(a,b) (*((byte *)(a))=(byte)(b))
#define PEEK(a)   (*((byte *)(a)))

#ifdef KICKC_NIGHTLY
#define HIBYTE(c) (BYTE1(c))
#define LOBYTE(c) (BYTE0(c))
#define EXPORT __export
#else
#define HIBYTE(c) (>(c))
#define LOBYTE(c) (<(c))
#define EXPORT export
#endif

#define NOP asm { nop }

#ifndef __KICKC__
// KickC defaults these, while other C compilers do not
// this makes syntax highlight work in common C editors
typedef unsigned char byte;
typedef unsigned int  word;
#endif

#endif
