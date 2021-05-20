typedef struct _TOUCHINFO_ {
    int scaledX;
    int scaledY;
} TOUCHINFO;

#define MAXTOUCHPOSANZ 50

void touch_init();
int touch_read(TOUCHINFO *ptp);
