#define FLIPVERT

typedef unsigned char UTF8;
typedef int32_t UTF32;

typedef struct _COLOR_ {
    int r;
    int g;
    int b;
} COLOR;

enum _FONTNAMES_ {
    VERDANA = 0,
    VERDANA_BOLD,
    VERDANA_ITALIC,
    VERDANA_BOLD_ITALIC,

    LIBMONO,
    LIBMONO_BOLD,
    LIBMONO_ITALIC,
    LIBMONO_BOLD_ITALIC,

    COURIER,
    COURIER_BOLD,
    COURIER_ITALIC,
    COURIER_BOLD_ITALIC,
};

void display_init();
void display_clear();
void draw_pixel(int x, int y, COLOR rgb);
void TFT_Hor_Line(int xs, int xe, int ypos, COLOR rgb, COLOR brgb, int linewidth, uint32_t pattern);
void TFT_Vert_Line(int xpos, int ys, int ye, COLOR rgb, COLOR brgb, int linewidth, uint32_t pattern);
void TFT_DrawLine(int x0,int y0,int x1,int y1, COLOR rgb, int linewidth);
void TFT_Rectangle(int xs, int ys, int xe, int ye, COLOR rgb, int linewidth);
void TFT_Rectangle_filled(int xs, int ys, int xe, int ye, COLOR rgb);
void TFT_drawcircle(int x0, int y0, int radius,COLOR rgb, int linewidth);
void TFT_drawcircle_filled(int x0, int y0, int radius,COLOR rgb);
void TFT_drawJPGfile(char *filename, int x, int y);
void draw_font(char *word, int init_x, int init_y, int fontidx, COLOR rgb, int font_size);

extern int disp_width, disp_height;

extern COLOR WHITE;
extern COLOR BLACK;
extern COLOR RED;
extern COLOR GREEN;
extern COLOR BLUE;
extern COLOR YELLOW;
extern COLOR GREY;
extern COLOR DARKGREY;
