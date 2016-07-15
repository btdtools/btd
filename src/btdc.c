#include <argp.h>
#include <error.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "misc.h"
#include "log.h"

const char *argp_program_version = "btdc 0.1";
const char *argp_program_bug_address = "<mart@martlubbers.net>";

static char doc[] = "btdc -- A BibTex daemon client";
static char args_doc[] = "[~/.config/btd/config]";

static struct argp_option options[] = {
	{"verbose",'v', 0, 0, "Increase verbosity", 0},
	{"quiet",'q', 0, 0, "Decrease verbosity", 0},
	{"socket", 's', "SOCKET", 0, "Manually set socket path", 0},
	{0}
};

typedef enum {LIST} btdccommand;

struct btdcopts {
	char *socketpath;
	btdccommand cmd;
};

void sig_handler(int signo)
{
	if (signo == SIGINT || signo == SIGTERM){
		fflush(stdout);
		depart("Signal %s caught\nQuitting...\n", strsignal(signo));
	}
}
/* Parse a single option. */
static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	struct btdcopts *opts = (struct btdcopts *)state->input;
	switch (key)
		{
		case 'q':
			btd_decr_log();
			break;
		case 'v':
			btd_incr_log();
			break;
		case 's':
			opts->socketpath = arg;
			break;
		case ARGP_KEY_NO_ARGS:
			argp_usage(state);
			break;
		case ARGP_KEY_ARG:
			if(strstr(arg, "list")){
				btd_log(2, "Command set to list");
				opts->cmd = LIST;
			} else {
				btd_log(0, "Unknown command: %s", arg);
				argp_usage(state);
			}
			break;
		default:
			return ARGP_ERR_UNKNOWN;
		}
	return 0;
}

static struct argp argp = {
	.options=options, 
	.parser=parse_opt,
	.args_doc=args_doc,
	.doc=doc,
	.children=NULL,
	.help_filter=NULL,
	.argp_domain=NULL};

void opts_populate(struct btdcopts *opts)
{
	opts->socketpath = safe_strdup("~/.btd/btd.socket");
	opts->cmd = LIST;
}

int main (int argc, char **argv)
{
	/* Register signal handlers */
	if(signal(SIGINT, sig_handler) == SIG_ERR){
		die("Can't catch SIGINT\n");
	}
	if(signal(SIGTERM, sig_handler) == SIG_ERR){
		die("Can't catch SIGTERM\n");
	}

	btd_init_log();
	struct btdcopts *opts = (struct btdcopts *)malloc(sizeof(struct btdcopts));
	opts_populate(opts);

	argp_parse(&argp, argc, argv, 0, 0, opts);
	
	opts->socketpath = resolve_tilde(opts->socketpath);
	switch(opts->cmd)
	{
		case LIST:
			btd_log(0, "List");
		default:
			break;
	}
}
