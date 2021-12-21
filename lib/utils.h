#define POKE(a,b) (*((byte *)(a))=(byte)(b))
#define PEEK(a)   (*((byte *)(a)))

#define HIBYTE(c) (>(c))
#define LOBYTE(c) (<(c))

#define NOP asm { nop }

#ifndef __KICKC__
// KickC defaults these, while other C compilers do not
// this makes syntax highlight work in common C editors
typedef unsigned char byte;
typedef unsigned int  word;
#endif
