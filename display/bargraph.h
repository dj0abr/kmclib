
#define LABELTEXTLEN    256
#define LABELANZ        20
#define MAXBARGRAPHS    100

typedef struct _BARLABEL_ {
	int posx;
	char text[LABELTEXTLEN];
} t_barlabel;

typedef struct _BARGRAPH_ {
    // Position and Size
	int minx,width;          // rectangle specifying the position and size
	int miny,height;

    // Title
	int titlefont;          // font names see display.h
	int titlex, titley;     // title position relativ to 0/0 of the bargraph
	int titlesize;			// font size
	char titletext[LABELTEXTLEN];	    // title (or NULL)

    // x-Axis
	char labeltext[LABELTEXTLEN];       // meaning of X-axis
	int labelfont;                      // font names see display.h
	int labelsize;						// font size of the label
	int labelypos;						// vertical pos
	t_barlabel barlabel[LABELANZ];      // up to LABELANZ label-markers on x-axis
	int labelanz;                       // number of label-markers on x-axis

	// Position of value-display right of the bar
    int showvalue;          // 0 or 1
	int valuefont;          // font names see display.h
	int valuefontsize;
	int valuex;             // title position relativ to 0/0 of the bargraph
	int valuey;
	int bgx,bgw,bgy,bgh;	// background rectange to be cleared relativ to 0/0 of the bargraph
	COLOR bgcolor;			// background color
	void (*barvaluefunc)(char *, int, double);	// callback to calculate real values (NULL if not used)

    // min/max value for the bargraph
    double minval, maxval;
} t_bargraph;

int create_bargraph(t_bargraph *settings);
void putvalue_bargraph(int id, double value, int redraw);
