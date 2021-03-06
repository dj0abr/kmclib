#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

int start_timer(int mSec, void(*timer_func_handler)(void));
void stop_timer(int timer);
void sleep_ms(int ms);
char *get_utctime();
char *get_utcdate();
