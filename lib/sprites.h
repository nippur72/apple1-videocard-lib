#ifndef SPRITES_H
#define SPRITES_H

typedef struct {
   byte y;
   byte x;
   byte name;
   byte color;
} tms_sprite;

#define SIZEOF_SPRITE 4

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

// clears all the sprites
void tms_clear_sprites() {
   // fills first sprite pattern with 0
   tms_set_vram_write_addr(TMS_SPRITE_PATTERNS);  
   for(byte i=0;i<8;i++) {
      TMS_WRITE_DATA_PORT(0);
   }

   // set sprite coordinates to (0,0) and set pattern name 0
   tms_set_vram_write_addr(TMS_SPRITE_ATTRS);       
   for(byte i=0;i<32;i++) {
      TMS_WRITE_DATA_PORT(0); NOP; NOP; NOP; NOP; // y coordinate
      TMS_WRITE_DATA_PORT(0); NOP; NOP; NOP; NOP; // x coordinate
      TMS_WRITE_DATA_PORT(0); NOP; NOP; NOP; NOP; // name
      TMS_WRITE_DATA_PORT(0); NOP; NOP; NOP; NOP; // color
   }
}
#endif
