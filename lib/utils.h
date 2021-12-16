#define POKE(a,b) (*((byte *)(a))=(byte)(b))
#define PEEK(a)   (*((byte *)(a)))

#define HIBYTE(c) (>(c))
#define LOBYTE(c) (<(c))

#define NOP asm { nop }
