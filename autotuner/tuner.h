#ifndef TUNER_H
#define TUNER_H

enum _MYBARS_ {
    TESTBAR1 = 0,
    TESTBAR2,
};

enum _MYBUTTONS_ {
    BT_MANUELL = 0,
    BT_AUTO,
    BT_SETUP,
    BT_CALIB,
    BT__RETURN,
    BT_CALIB_DIG,
    BT_CALIB_ADC,
    BT_CALIB_VAL,
    BT_CALIB_RETURN,
};

enum _MENUNUM_ {
    MENU_MAIN = 0,
    MENU_MANUELL,
    MENU_AUTO,
    MENU_SETUP,
    MENU_CALIB,
};

void setup_bargraphs();
void setup_touchbuttons();
void show_menu(int menunum);
void show_values(int menunum, int inf);

extern int act_menunum;

#endif
