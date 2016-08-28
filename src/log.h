#ifndef LOG_H
#define LOG_H

int btd_log_level;

#define BTD_MAX_LOG 2
#define BTD_MIN_LOG 0

#define btd_log(lvl, ...) if(lvl <= btd_log_level) printf(__VA_ARGS__);

#define btd_init_log() btd_log_level = 1;
#define btd_decr_log() btd_log_level -= btd_log_level > BTD_MIN_LOG;
#define btd_incr_log() btd_log_level += btd_log_level < BTD_MAX_LOG;

#endif
