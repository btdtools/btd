#include <stdlib.h> 
#include <string.h> 

#include "misc.h"
#include "log.h"

static char *get_file(char *home, char *file)
{
	char *l[3] = {home, "/", file};
	char *path = safe_strcat(l, 3);
	char *resolved_path = resolve_tilde(path);
	free(path);
	return resolved_path;
}

static char *get_file_if_exist(char *home, char *file)
{
	char *path = get_file(home, file);
	if(path_exists(path)){
		return path;
	}
	free(path);
	return NULL;
}

static char *safe_getenv(char *env, char *def)
{
	char *t;
	if((t = getenv(env)) == NULL){
		return def;
	}
	return t;
}

char *get_config_path()
{
	char *cf;
   
	/* Check user xdg config file */
	btd_log(2, "Checking XDG_CONFIG_HOME/btd/config\n");
	if((cf = get_file_if_exist(
		safe_getenv("XDG_CONFIG_HOME", "~/.config"), "/btd/config")) != NULL){
		btd_log(2, "Found!\n");
		return cf;
	}

	/* Check system xdg config files */
	btd_log(2, "Checking XDG_CONFIG_DIRS\n");
	char *systempaths = safe_getenv("XDG_CONFIG_DIRS", "/etc/xdg");
	char *token;
	while((token = strsep(&systempaths, ":")) != NULL){
		btd_logf(2, "Checking %s/btd/config\n", token);
		if((cf = get_file_if_exist(token, "/btd/config")) != NULL){
			btd_log(2, "Found!\n");
			return cf;
		}
	}
	
	/* No config file found */
	die("No config file found...\n");
}

char *get_data_path(){
	char *df, *home = safe_getenv("XDG_DATA_HOME", "~/.local/share");

	/* Check user data dir */
	btd_log(2, "Checking XDG_DATA_HOME/btd\n");
	if((df = get_file_if_exist(home, "/btd")) != NULL){
		btd_log(2, "Found!\n");
		return df;
	}

	/* Check system xdg config files */
	btd_log(2, "Checking XDG_DATA_DIRS\n");
	char *systempaths = safe_strdup(
		safe_getenv("XDG_DATA_DIRS", "/usr/local/share:/usr/share"));
	char *token;
	while((token = strsep(&systempaths, ":")) != NULL){
		btd_logf(2, "Checking %s/btd\n", token);
		if((df = get_file_if_exist(token, "/btd")) != NULL){
			btd_log(2, "Found!\n");
			return df;
		}
	}

	/* No data found, thus going for the default */
	df = get_file(home, "/btd");
	btd_logf(2, "No existing data found, falling back to %s\n", df);
	return df;
}
