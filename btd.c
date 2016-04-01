#include <stdio.h>
#include <argp.h>
#include <btparse.h>

const char *argp_program_version = "btd 0.1";
const char *argp_program_bug_address = "<mart@martlubbers.net>";

static char doc[] = "btd -- A BibTex daemon";
static char args_doc[] = "[~/.config/btd/config]";

static struct argp_option options[] = {
	{"socket", 's', "SOCKET", 0, "Path of the socket", 0},
	{"verbose",'v', 0, 0, "Increase verbosity", 0},
	{"quiet",'q', 0, 0, "Decrease verbosity", 0},
	{"output", 'o', "FILE", 0, "Output to FILE instead of standard output", 0},
	{0}
};

struct config
{
	char *configpath;
	int verbose;
	char *output_file;
};

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	struct config *config = state->input;

	switch (key)
	{
		case 'q':
			config->verbose -= config->verbose != 0;
			break;
		case 'v':
			config->verbose += config->verbose != 5;
			break;
		case 'o':
			config->output_file = arg;
			break;
		case ARGP_KEY_ARG:
			if (state->arg_num >= 1) {
				argp_usage (state);
			}
			config->configpath = arg;
			break;
		case ARGP_KEY_END:
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

/* Our argp parser. */
static struct argp argp = {
	.options=options, 
	.parser=parse_opt,
	.args_doc=args_doc,
	.doc=doc,
	.children=NULL,
	.help_filter=NULL,
	.argp_domain=NULL};

int main (int argc, char **argv)
{
	struct config config;

	config.configpath = NULL;
	config.verbose = 1;
	config.output_file = "-";
	argp_parse(&argp, argc, argv, 0, 0, &config);

	printf("CONFIG = %s\n"
			"OUTPUT_FILE = %s\n"
			"VERBOSE = %d\n",
			config.configpath == NULL ? "null" : config.configpath,
			config.output_file,
			config.verbose);
	return 0;
}
