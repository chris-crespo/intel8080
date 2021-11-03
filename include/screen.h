#ifndef SCREEN_H
#define SCREEN_H

void screen_init(void);
void screen_clear(void);
void screen_draw(u8 *memory);
void screen_draw_bottom(u8 *memory);
void screen_draw_top(u8 *memory);
void screen_quit(void);

#endif
