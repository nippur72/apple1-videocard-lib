#include <tms9918.h>
#include <fastmultiply.h>

void main(void) {
    mulf_init();
    tms_init_regs(SCREEN2_TABLE);
    byte text_color = FG_BG(COLOR_WHITE,COLOR_BLACK);
    tms_set_color(COLOR_BLACK);
    screen2_init_bitmap(text_color);
    //screen2_puts("*** P-LAB  VIDEO CARD SYSTEM ***", 0, 0, text_color);

    screen2_plot_mode = PLOT_MODE_SET;
    screen2_line(0, 0, (unsigned char) mulf8u(50,3), 50);
    // TODO: THIS DOES NOT WORK screen2_ellipse_rect(10, 10, 150, 100);
    screen2_circle(128,76,30);
}
