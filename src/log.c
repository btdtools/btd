#define BTD_MAX_LOG 2
#define BTD_MIN_LOG 0

#include <stdarg.h>
#include <stdio.h>

#include "log.h"

int btd_log_level;

int get_btd_log_level()
{
	return btd_log_level;
}

void btd_init_log()
{
	btd_log_level = 1;
}

void btd_log(int lvl, char *msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	if (lvl <= btd_log_level)
		vprintf(msg, ap);
	va_end(ap);
}

void btd_decr_log()
{
	if (btd_log_level > BTD_MIN_LOG)
		btd_log_level--;
}

void btd_incr_log()
{
	if (btd_log_level < BTD_MAX_LOG)
		btd_log_level++;
}
