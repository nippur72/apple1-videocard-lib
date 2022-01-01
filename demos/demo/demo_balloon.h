#include <font8x8.h>

// the balloon sprite, manually copied from C64 User's guide
#include "cbm_balloon.h"

// a balloon is two 16x16 sprites, one above the other
typedef struct {
   int x;                 // x coordinate of the balloon
   int y;                 // y coordinate of the balloon
   int dx;                // x velocity of the balloon
   int dy;                // y velocity of the balloon
   byte color;            // color of the balloon
   byte sprite_number;    // sprite number (0-31) of the balloon
   tms_sprite upper;      // the upper sprite portion of the balloon
   tms_sprite lower;      // the lower sprite portion of the balloon   
} balloon;

// move ballon on the screen by its velocity
// and make it bounce over the borders
void animate_balloon(balloon *b) {     
   
   // use temporary variables as KickC has issues dealing with "->" operator
   int x = b->x;
   int y = b->y;
   int dx = b->dx;
   int dy = b->dy;

   // border collision detection
   if(x>=240 || x<=0) dx = -dx; 
   if(y>=172 || y<=0) dy = -dy; 

   // move the balloon
   x += dx;
   y += dy;

   // write back temporary variables
   b->x = x;
   b->y = y;
   b->dx = dx;
   b->dy = dy;

   // update the sprite part of the balloon
   b->upper.x = (byte) x;
   b->upper.y = (signed char) y;
   b->upper.name = 0;
   b->upper.color = b->color;   
   b->lower.x = b->upper.x;
   b->lower.y = b->upper.y + 16;      // 16 pixels below the upper sprite
   b->lower.name = b->upper.name + 4;  
   b->lower.color = b->color;   
   tms_set_sprite(b->sprite_number, &(b->upper));
   tms_set_sprite(b->sprite_number+1, &(b->lower));   
}

void demo_balloon() {
   tms_init_regs(SCREEN2_TABLE);   

   // we use only 4 sprites, two for each ot the two balloons on the screen
   tms_set_total_sprites(4);
   
   // fake C64 bootup screen colors
   tms_set_color(COLOR_LIGHT_BLUE);   
   byte text_color = FG_BG(COLOR_GREY,COLOR_DARK_BLUE);  // alas, COLOR_LIGHT_BLUE doesn't fit well so we use GREY instead

   screen2_init_bitmap(text_color);

   // C64-like screen text              
   screen2_puts("*** COMMODORE-APPLE BASIC V2 ***", 0, 0, text_color);
   screen2_puts("38911 BASIC BYTES FREE"          , 0, 2, text_color);
   screen2_puts("READY."                          , 0, 4, text_color);
   
   // copy the ballon graphic to VRAM
   tms_copy_to_vram(cbm_balloon, 4*8*2, TMS_SPRITE_PATTERNS);

   tms_set_sprite_double_size(1);   // set 16x16 sprites   
   tms_set_sprite_magnification(0); // set single pixel sprites

   // we have two balloons bouncing around the screen

   // first balloon
   balloon b1;   
   b1.x = 20;
   b1.y = 20;
   b1.dx = 1;
   b1.dy = 1;
   b1.color = COLOR_LIGHT_YELLOW;
   b1.sprite_number = 0;

   // second balloon
   balloon b2;   
   b2.x = 150;
   b2.y = 150;
   b2.dx = -1;
   b2.dy = -1;
   b2.color = COLOR_LIGHT_RED;
   b2.sprite_number = 2;

   // counter for a fake blinking cursor
   byte blink_counter=0;

   for(;;) {

      // delay the animation
      for(int delay=0; delay<400; delay++) {
         delay = delay+1;
         delay = delay-1;
      }

      // RETURN key ends the demo
      if(apple1_readkey()==0x0d) break;

      // if there's a collision invert the motion of the balloons
      if(COLLISION_BIT(TMS_READ_CTRL_PORT)) {      
         int temp;
         temp = b1.dx; b1.dx = -temp;   // b1.dx = -b1.dx; // due to KickC issue
         temp = b1.dy; b1.dy = -temp;   // b1.dy = -b1.dy; // due to KickC issue
         temp = b2.dx; b2.dx = -temp;   // b2.dx = -b2.dx; // due to KickC issue
         temp = b2.dy; b2.dy = -temp;   // b2.dy = -b2.dy; // due to KickC issue
      }

      // move the two balloons
      animate_balloon(&b1);      
      animate_balloon(&b2);  

      // since a balloon is made of two sprites, sometimes they overlap 
      // by one line during the move, causing a false collision      
      // so we clear the collision bit by simply reading the status register
      tms_clear_collisions();

      // fake a blinking cursor
      blink_counter++;
      if(blink_counter == 16) { screen2_puts(" ", 0, 5, FG_BG(COLOR_DARK_BLUE,COLOR_GREY)); }
      if(blink_counter == 32) { screen2_puts(" ", 0, 5, FG_BG(COLOR_GREY,COLOR_DARK_BLUE)); blink_counter = 0; }      
   }
}

