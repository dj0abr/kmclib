#define MAXTOUCHBUTTONS	100

typedef struct _TOUCHBUTTON {
	int active;
	int menu;
	int x;
	int y;
	int width;
	int height;
	COLOR button_color;
	COLOR text_color;
	char text[20];
	int font;
	int fontsize;
	int text_xoff;
	int text_yoff;
	void (*touchbutfunc)(int);	// callback on touching a button, parameter: button number
} TOUCHBUTTON;

int create_touchbutton(TOUCHBUTTON *ptb);
void draw_button(int id);
void deactivate_allbuttons();
