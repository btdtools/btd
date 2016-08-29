#ifndef LOG_H
#define LOG_H

int get_btd_log_level();
void btd_init_log();
void btd_log(int lvl, char *msg, ...);
void btd_decr_log();
void btd_incr_log();

#endif
