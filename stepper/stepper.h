int create_stepper(int sp, int dp, int ep, int pol, int spd, int refp, int refdir);
void move_stepper(int ID, int steps, int dir, int wait);
void move_cond_stepper(int ID, int steps, int dir , int wait, int (*stepcond)(int step, int dir));
void gopos_stepper(int ID, int pos, int wait);
int getpos_stepper(int ID);
void ref_stepper(int ID);
void disable_stepper();
