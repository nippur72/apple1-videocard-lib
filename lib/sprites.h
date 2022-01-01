#ifndef SPRITES_H
#define SPRITES_H

typedef struct {
   signed char y;
   byte x;
   byte name;
   byte color;
} tms_sprite;

#define SIZEOF_SPRITE 4
#define SPRITE_OFF_MARKER 0xD0
#define EARLY_CLOCK 128

void tms_set_total_sprites(byte num) {
   // write the value 0xD0 as Y coordinate of the first unused sprite
   word addr = TMS_SPRITE_ATTRS + (word) (num * SIZEOF_SPRITE);
   tms_set_vram_write_addr(addr);
   TMS_WRITE_DATA_PORT(SPRITE_OFF_MARKER);  // 0xD0 oi the sprite off indicator
}

void tms_set_sprite(byte sprite_num, tms_sprite *s) {
   word addr = TMS_SPRITE_ATTRS + (word) (sprite_num * SIZEOF_SPRITE);
   tms_set_vram_write_addr(addr);

   // TODO verify NOPs on real machine
   // TODO is it better to use tms_copy_to_vram() ?
   TMS_WRITE_DATA_PORT(s->y);     NOP; // y coordinate
   TMS_WRITE_DATA_PORT(s->x);     NOP; // x coordinate
   TMS_WRITE_DATA_PORT(s->name);  NOP; // name
   TMS_WRITE_DATA_PORT(s->color); NOP; // color
}

void tms_set_sprite_double_size(byte size) {
   byte regval = tms_regs_latch[1] & (REG1_SIZE_MASK ^ 0xFF);
   if(size) regval |= REG1_SIZE_MASK;
   tms_write_reg(1, regval);
}

void tms_set_sprite_magnification(byte m) {
   byte regval = tms_regs_latch[1] & (REG1_MAG_MASK ^ 0xFF);
   if(m) regval |= REG1_MAG_MASK;
   tms_write_reg(1, regval);
}

// clear the collision flag by reading the status register
inline void tms_clear_collisions() {
   asm { lda VDP_REG };
}
#endif
