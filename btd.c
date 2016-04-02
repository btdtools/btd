#include <btparse.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "misc.h"

void sig_handler(int signo)
{
	if (signo == SIGINT || signo == SIGTERM){
			printf("received %s\n", strsignal(signo));
			depart("Quitting...");
	}
}

int main (int argc, char **argv)
{
	struct btd_config config;

	if(signal(SIGINT, sig_handler) == SIG_ERR){
		die("Can't catch SIGINT");
	}
	if(signal(SIGTERM, sig_handler) == SIG_ERR){
		die("Can't catch SIGTERM");
	}

	btd_config_populate(&config, argc, argv);
	btd_config_print(&config, stdout);
	exit(EXIT_SUCCESS);
}
