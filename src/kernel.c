#include "headers.h"

extern char kern_end;
extern uint8_t _binary_build_initrd_start;
extern uint8_t _binary_build_initrd_end;
extern void pmm_init(uint32_t mem_size_kb);
extern void heap_init(void *start, uint32_t size);
extern void vfs_init(uint8_t *image, uint32_t size);
extern void timer_init(uint32_t freq);
extern void scheduler_run(void);

/* Colors */
#define C_BLACK   0
#define C_BLUE    1
#define C_WHITE   2
#define C_GRAY    3
#define C_DGRAY   4
#define C_GREEN   5
#define C_YELLOW  6
#define C_RED     7
#define DESKTOP_W 320
#define DESKTOP_H 200
#define TASKBAR_H 24
#define TASKBAR_Y (DESKTOP_H - TASKBAR_H)

/* App IDs */
#define APP_DESKTOP 0
#define APP_LOGIN   1
#define APP_NOTEPAD 2
#define APP_SNAKE   3
#define APP_SETTINGS 4

static int current_app = APP_LOGIN;
static uint8_t desktop_bg = C_BLUE;

static int mouse_x = 160;
static int mouse_y = 100;
static int mouse_old_x = -1;
static int mouse_old_y = -1;

void draw_string(int x, int y, const char *s, uint8_t fg, uint8_t bg);
void draw_char(int x, int y, char c, uint8_t fg, uint8_t bg);

void draw_desktop(void) {
    gfx_clear(desktop_bg);

    /* Taskbar */
    gfx_rect(0, TASKBAR_Y, DESKTOP_W, TASKBAR_H, C_DGRAY);
    gfx_line(0, TASKBAR_Y, DESKTOP_W - 1, TASKBAR_Y, C_WHITE);

    /* Start button */
    gfx_rect(4, TASKBAR_Y + 4, 40, 16, C_GRAY);
    gfx_line(4, TASKBAR_Y + 4, 44, TASKBAR_Y + 4, C_WHITE);
    gfx_line(4, TASKBAR_Y + 4, 4, TASKBAR_Y + 20, C_WHITE);
    gfx_line(44, TASKBAR_Y + 4, 44, TASKBAR_Y + 20, C_GRAY);
    gfx_line(4, TASKBAR_Y + 20, 44, TASKBAR_Y + 20, C_GRAY);

    const char *start = "Start";
    int sx = 4 + 3;
    int sy = TASKBAR_Y + 4 + 2;
    draw_string(sx, sy, start, C_WHITE, C_GRAY);

    /* App hints */
    draw_string(60, TASKBAR_Y + 6, "1:Notepad  2:Snake  3:Settings", C_WHITE, C_DGRAY);
}

void draw_char(int x, int y, char c, uint8_t fg, uint8_t bg) {
    static const uint8_t font[96][7] = {
        {0x00,0x00,0x00,0x00,0x00,0x00,0x00}, /* space */
        {0x18,0x3C,0x3C,0x18,0x18,0x18,0x18}, /* ! */
        {0x36,0x36,0x00,0x00,0x00,0x00,0x00}, /* " */
        {0x36,0x36,0x7E,0x36,0x7E,0x36,0x36}, /* # */
        {0x0C,0x1E,0x3C,0x6C,0x3C,0x1E,0x0C}, /* $ */
        {0x00,0x66,0x3C,0x18,0x3C,0x66,0x00}, /* % */
        {0x1C,0x36,0x1C,0x0C,0x1E,0x36,0x1C}, /* & */
        {0x06,0x06,0x03,0x00,0x00,0x00,0x00}, /* ' */
        {0x0C,0x18,0x30,0x30,0x30,0x18,0x0C}, /* ( */
        {0x30,0x18,0x0C,0x0C,0x0C,0x18,0x30}, /* ) */
        {0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00}, /* * */
        {0x00,0x0C,0x1E,0x3C,0x1E,0x0C,0x00}, /* + */
        {0x00,0x00,0x00,0x00,0x00,0x0C,0x18}, /* , */
        {0x00,0x00,0x00,0x3C,0x00,0x00,0x00}, /* - */
        {0x00,0x00,0x00,0x00,0x00,0x30,0x30}, /* . */
        {0x00,0x06,0x0C,0x18,0x30,0x60,0x00}, /* / */
        {0x3C,0x66,0x6E,0x76,0x66,0x66,0x3C}, /* 0 */
        {0x18,0x38,0x18,0x18,0x18,0x18,0x3C}, /* 1 */
        {0x3C,0x66,0x06,0x0C,0x18,0x30,0x7E}, /* 2 */
        {0x3C,0x66,0x06,0x1C,0x06,0x66,0x3C}, /* 3 */
        {0x06,0x0E,0x16,0x26,0x7E,0x06,0x06}, /* 4 */
        {0x7E,0x60,0x60,0x7C,0x06,0x66,0x3C}, /* 5 */
        {0x3C,0x60,0x60,0x7C,0x66,0x66,0x3C}, /* 6 */
        {0x7E,0x66,0x06,0x0C,0x18,0x18,0x00}, /* 7 */
        {0x3C,0x66,0x66,0x3C,0x66,0x66,0x3C}, /* 8 */
        {0x3C,0x66,0x66,0x3E,0x06,0x0C,0x18}, /* 9 */
        {0x00,0x00,0x3C,0x00,0x3C,0x00,0x00}, /* : */
        {0x00,0x00,0x3C,0x00,0x3C,0x30,0x00}, /* ; */
        {0x00,0x06,0x0C,0x18,0x30,0x60,0x00}, /* < */
        {0x00,0x00,0x3C,0x00,0x3C,0x00,0x00}, /* = */
        {0x00,0x60,0x30,0x18,0x0C,0x06,0x00}, /* > */
        {0x3C,0x66,0x0C,0x18,0x18,0x00,0x18}, /* ? */
        {0x3C,0x66,0x6E,0x7E,0x7E,0x66,0x3C}, /* @ */
        {0x3C,0x66,0x66,0x7E,0x66,0x66,0x66}, /* A */
        {0x7C,0x66,0x66,0x7C,0x66,0x66,0x7C}, /* B */
        {0x3C,0x66,0x60,0x60,0x60,0x66,0x3C}, /* C */
        {0x78,0x6C,0x66,0x66,0x66,0x6C,0x78}, /* D */
        {0x7E,0x60,0x60,0x7C,0x60,0x60,0x7E}, /* E */
        {0x7E,0x60,0x60,0x7C,0x60,0x60,0x60}, /* F */
        {0x3C,0x66,0x60,0x6E,0x66,0x66,0x3C}, /* G */
        {0x66,0x66,0x66,0x7E,0x66,0x66,0x66}, /* H */
        {0x18,0x18,0x18,0x18,0x18,0x18,0x18}, /* I */
        {0x06,0x06,0x06,0x06,0x06,0x66,0x3C}, /* J */
        {0x66,0x66,0x3C,0x18,0x3C,0x66,0x66}, /* K */
        {0x60,0x60,0x60,0x60,0x60,0x66,0x3C}, /* L */
        {0x66,0x66,0x6E,0x76,0x66,0x66,0x66}, /* M */
        {0x66,0x66,0x76,0x76,0x6E,0x66,0x66}, /* N */
        {0x3C,0x66,0x66,0x66,0x66,0x66,0x3C}, /* O */
        {0x7C,0x66,0x66,0x7C,0x60,0x60,0x60}, /* P */
        {0x3C,0x66,0x66,0x66,0x66,0x6E,0x3E}, /* Q */
        {0x7C,0x66,0x66,0x7C,0x6C,0x66,0x66}, /* R */
        {0x3C,0x66,0x60,0x3C,0x06,0x66,0x3C}, /* S */
        {0x7E,0x18,0x18,0x18,0x18,0x18,0x18}, /* T */
        {0x66,0x66,0x66,0x66,0x66,0x66,0x3C}, /* U */
        {0x66,0x66,0x66,0x66,0x66,0x3C,0x18}, /* V */
        {0x66,0x66,0x66,0x6E,0x76,0x66,0x66}, /* W */
        {0x66,0x66,0x3C,0x18,0x3C,0x66,0x66}, /* X */
        {0x66,0x66,0x66,0x3C,0x18,0x18,0x18}, /* Y */
        {0x7E,0x66,0x0C,0x18,0x30,0x66,0x7E}, /* Z */
    };
    if (c < 32 || c > 126) c = '?';
    int idx = c - 32;
    for (int row = 0; row < 7; row++) {
        uint8_t line = font[idx][row];
        for (int col = 0; col < 5; col++) {
            if (line & (0x10 >> col)) {
                gfx_pixel(x + col, y + row, fg);
            } else {
                gfx_pixel(x + col, y + row, bg);
            }
        }
    }
}

void draw_string(int x, int y, const char *s, uint8_t fg, uint8_t bg) {
    while (*s) {
        draw_char(x, y, *s, fg, bg);
        x += 6;
        s++;
    }
}

void draw_mouse(int x, int y) {
    static const int shape[5][2] = {{0,0},{1,0},{2,0},{1,1},{1,2}};
    for (int i = 0; i < 5; i++) {
        gfx_pixel(x + shape[i][0], y + shape[i][1], C_WHITE);
    }
    gfx_pixel(x, y + 3, C_WHITE);
}

void erase_mouse(int x, int y) {
    gfx_pixel(x, y, desktop_bg);
    gfx_pixel(x + 1, y, desktop_bg);
    gfx_pixel(x + 2, y, desktop_bg);
    gfx_pixel(x + 1, y + 1, desktop_bg);
    gfx_pixel(x + 1, y + 2, desktop_bg);
    gfx_pixel(x, y + 3, desktop_bg);
}

void draw_window(int x, int y, int w, int h, const char *title, uint8_t bg_color) {
    gfx_rect(x, y, w, h, C_GRAY);
    gfx_rect(x + 1, y + 1, w - 2, h - 2, bg_color);
    gfx_line(x, y, x + w - 1, y, C_WHITE);
    gfx_line(x, y, x, y + h - 1, C_WHITE);
    draw_string(x + 3, y + 2, title, C_WHITE, C_GRAY);
}

static char notepad_buf[256];
static int notepad_len = 0;
static int notepad_cursor_x = 0;

void notepad_app(void) {
    draw_window(40, 30, 240, 140, "Notepad", C_BLACK);
    draw_string(46, 44, "Text:", C_WHITE, C_BLACK);
    draw_string(46, 56, notepad_buf, C_WHITE, C_BLACK);
    draw_char(46 + notepad_cursor_x * 6, 56, '_', C_WHITE, C_BLACK);
    draw_string(46, 160, "ESC=back  ENTER=newline", C_GRAY, C_BLACK);
}

static int snake_x[100], snake_y[100];
static int snake_len = 5;
static int snake_dir = 0; /* 0=right,1=down,2=left,3=up */
static int snake_food_x, snake_food_y;
static int snake_score = 0;
static int snake_game_over = 0;

void snake_init(void) {
    for (int i = 0; i < snake_len; i++) {
        snake_x[i] = 100 - i * 4;
        snake_y[i] = 100;
    }
    snake_food_x = 150;
    snake_food_y = 100;
    snake_dir = 0;
    snake_score = 0;
    snake_game_over = 0;
}

void snake_app(void) {
    if (snake_len == 5 && snake_x[0] == 100) {
        snake_init();
    }

    draw_window(40, 30, 240, 140, "Snake", C_BLACK);
    char score_buf[32];
    snake_score = snake_len - 5;
    score_buf[0] = 'S'; score_buf[1] = 'c'; score_buf[2] = 'o'; score_buf[3] = 'r'; score_buf[4] = ':'; score_buf[5] = ' ';
    score_buf[6] = '0' + snake_score / 10;
    score_buf[7] = '0' + snake_score % 10;
    score_buf[8] = 0;
    draw_string(46, 44, score_buf, C_WHITE, C_BLACK);

    if (snake_game_over) {
        draw_string(100, 90, "GAME OVER", C_RED, C_BLACK);
        draw_string(90, 102, "Press R to restart", C_WHITE, C_BLACK);
    } else {
        for (int i = 0; i < snake_len; i++) {
            gfx_pixel(snake_x[i], snake_y[i], C_GREEN);
        }
        gfx_pixel(snake_food_x, snake_food_y, C_RED);
    }

    draw_string(46, 160, "Arrows=move  R=reset  ESC=back", C_GRAY, C_BLACK);
}

static uint8_t settings_color = C_BLUE;

void settings_app(void) {
    draw_window(40, 30, 240, 140, "Settings", C_BLACK);
    draw_string(46, 44, "Background Color:", C_WHITE, C_BLACK);
    const char *colors[] = {"Blue", "Green", "Red", "Black", "Gray"};
    for (int i = 0; i < 5; i++) {
        gfx_rect(46 + i * 44, 60, 40, 20, i);
        if ((uint8_t)i == desktop_bg) {
            gfx_line(46 + i * 44, 60, 46 + i * 44 + 40, 60, C_WHITE);
        }
        draw_string(46 + i * 44 + 2, 66, colors[i], C_WHITE, (uint8_t)i);
    }
    draw_string(46, 100, "1-5=select  ESC=back", C_GRAY, C_BLACK);
}

void login_screen(void) {
    gfx_clear(C_BLUE);
    draw_string(100, 80, "nOS Login", C_WHITE, C_BLUE);
    draw_string(90, 100, "Username:", C_WHITE, C_BLUE);
    draw_string(150, 100, "administrator", C_GREEN, C_BLUE);
    draw_string(90, 120, "Password:", C_WHITE, C_BLUE);
    draw_string(150, 120, "********", C_GREEN, C_BLUE);
    draw_string(100, 150, "Press any key to login", C_WHITE, C_BLUE);
    gfx_flush();
}

static char scancode_to_ascii(uint8_t sc) {
    static const char lower[] = {
        0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 8,
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
        0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
        '*', 0, ' ', 0
    };
    if (sc < sizeof(lower)) return lower[sc];
    return 0;
}

void kernel_main(unsigned int magic, unsigned int addr) {
    (void)magic;
    (void)addr;

    gfx_init();
    gdt_install();
    idt_install_null();
    idt_install_full();
    kb_install();
    mouse_install();

    pmm_init(256 * 1024);
    heap_init((void *)&kern_end, 4 * 1024 * 1024);

    vfs_init((uint8_t *)&_binary_build_initrd_start,
             (uint32_t)(&_binary_build_initrd_end - &_binary_build_initrd_start));

    timer_init(100);

    /* Phase 7: Show login screen first */
    login_screen();

    /* Main loop */
    for (;;) {
        __asm__ volatile ("sti; hlt");

        if (mouse_has_packet()) {
            int mx = mouse_get_x();
            int my = mouse_get_y();
            if (mx < 0) mx = 0;
            if (my < 0) my = 0;
            if (mx >= DESKTOP_W) mx = DESKTOP_W - 1;
            if (my >= DESKTOP_H) my = DESKTOP_H - 1;

            if (mx != mouse_old_x || my != mouse_old_y) {
                if (mouse_old_x >= 0 && mouse_old_y >= 0) {
                    erase_mouse(mouse_old_x, mouse_old_y);
                }
                mouse_x = mx;
                mouse_y = my;
                draw_mouse(mouse_x, mouse_y);
                mouse_old_x = mouse_x;
                mouse_old_y = mouse_y;
                gfx_flush();
            }
        }

        if (kb_has_key()) {
            uint8_t sc = kb_read_scancode();

            if (current_app == APP_LOGIN) {
                current_app = APP_DESKTOP;
                draw_desktop();
                gfx_flush();
                continue;
            }

            if (current_app == APP_NOTEPAD) {
                if (sc == 1) { /* ESC */
                    current_app = APP_DESKTOP;
                    draw_desktop();
                    gfx_flush();
                } else if (sc == 28) { /* Enter */
                    if (notepad_len < 255) {
                        notepad_buf[notepad_len++] = '\n';
                        notepad_cursor_x = 0;
                    }
                } else if (sc == 14) { /* Backspace */
                    if (notepad_len > 0 && notepad_cursor_x > 0) {
                        notepad_len--;
                        notepad_cursor_x--;
                        notepad_buf[notepad_len] = 0;
                    }
                } else {
                    char c = scancode_to_ascii(sc);
                    if (c && notepad_len < 255) {
                        notepad_buf[notepad_len++] = c;
                        notepad_buf[notepad_len] = 0;
                        notepad_cursor_x++;
                    }
                }
                notepad_app();
                gfx_flush();
                continue;
            }

            if (current_app == APP_SNAKE) {
                if (sc == 1) { /* ESC */
                    current_app = APP_DESKTOP;
                    draw_desktop();
                    gfx_flush();
                } else if (sc == 19) { /* R */
                    snake_len = 5;
                    snake_game_over = 0;
                } else if (sc == 75) { /* Left */
                    if (snake_dir != 0) snake_dir = 2;
                } else if (sc == 77) { /* Right */
                    if (snake_dir != 2) snake_dir = 0;
                } else if (sc == 72) { /* Up */
                    if (snake_dir != 1) snake_dir = 3;
                } else if (sc == 80) { /* Down */
                    if (snake_dir != 3) snake_dir = 1;
                }
                snake_app();
                gfx_flush();
                continue;
            }

            if (current_app == APP_SETTINGS) {
                if (sc == 1) { /* ESC */
                    current_app = APP_DESKTOP;
                    draw_desktop();
                    gfx_flush();
                } else if (sc >= 2 && sc <= 6) { /* 1-5 */
                    desktop_bg = sc - 1;
                    settings_color = desktop_bg;
                    draw_desktop();
                }
                settings_app();
                gfx_flush();
                continue;
            }

            /* Desktop app switching */
            if (sc == 1) { /* ESC */
                current_app = APP_LOGIN;
                login_screen();
            } else if (sc == 2) { /* 1 */
                current_app = APP_NOTEPAD;
                notepad_len = 0;
                notepad_cursor_x = 0;
                notepad_buf[0] = 0;
                notepad_app();
            } else if (sc == 3) { /* 2 */
                current_app = APP_SNAKE;
                snake_len = 5;
                snake_game_over = 0;
                snake_app();
            } else if (sc == 4) { /* 3 */
                current_app = APP_SETTINGS;
                settings_app();
            }

            gfx_flush();
        }
    }
}
