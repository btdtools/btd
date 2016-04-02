#include <btparse.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "config.h"
#include "misc.h"

struct btd_config config;
int socket_fd;

void cleanup()
{
	close(socket_fd);
	unlink(config.socket);
}

void sig_handler(int signo)
{
	if (signo == SIGINT || signo == SIGTERM){
		printf("Signal %s caught\n", strsignal(signo));
		cleanup();
		depart("Quitting...");
	}
}

int connection_handler(int connection_fd)
{
	int nbytes;
	char buffer[256];

	nbytes = read(connection_fd, buffer, 256);
	buffer[nbytes] = '\0';

	printf("MESSAGE FROM CLIENT: %s\n", buffer);
	nbytes = snprintf(buffer, 256, "hello from the server");
	write(connection_fd, buffer, nbytes);

	close(connection_fd);
	return 0;
}

int main (int argc, char **argv)
{
	struct sockaddr_un address;
	int connection_fd;
	socklen_t address_length;
	pid_t child;

	/* Register signal handlers */
	if(signal(SIGINT, sig_handler) == SIG_ERR){
		die("Can't catch SIGINT");
	}
	if(signal(SIGTERM, sig_handler) == SIG_ERR){
		die("Can't catch SIGTERM");
	}

	/* Parse args and config */
	btd_config_populate(&config, argc, argv);
	btd_config_print(&config, stdout);

	/* Setup socket */
	if(path_exists(config.socket)){
		die("Socket already in use");
	}
	socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if(socket_fd < 0) {
		die("socket() failed");
	} 
	unlink(config.socket);

	memset(&address, 0, sizeof(struct sockaddr_un));

	address.sun_family = AF_UNIX;
	strcpy(address.sun_path, config.socket);

	if(bind(socket_fd, (struct sockaddr *) &address, 
			sizeof(struct sockaddr_un)) != 0) {
		die("bind() failed");
	}

	if(listen(socket_fd, 5) != 0) {
		die("listen() failed");
	}

	while((connection_fd = accept(socket_fd, (struct sockaddr *) &address,
			&address_length)) > -1) {
		child = fork();
		if(child == 0) {
			return connection_handler(connection_fd);
		}
		close(connection_fd);
	}
	cleanup();
	depart("Bye...");
}
