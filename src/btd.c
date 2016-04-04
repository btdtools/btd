#include <btparse.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sqlite3.h>

#include "config.h"
#include "misc.h"
#include "log.h"
#include "db.h"

struct btd_config config;
int socket_fd;

void cleanup()
{
	btd_log(2, "Closing socket\n");
	close(socket_fd);
	btd_log(2, "Unlinking socket\n");
	unlink(config.socket);
	btd_log(2, "Closing database\n");
	db_close();
}

void sig_handler(int signo)
{
	if (signo == SIGINT || signo == SIGTERM){
		cleanup();
		depart("Signal %s caught\nQuitting...\n", strsignal(signo));
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

	btd_init_log();

	/* Register signal handlers */
	if(signal(SIGINT, sig_handler) == SIG_ERR){
		die("Can't catch SIGINT\n");
	}
	if(signal(SIGTERM, sig_handler) == SIG_ERR){
		die("Can't catch SIGTERM\n");
	}

	/* Parse args and config */
	btd_config_populate(&config, argc, argv);
	btd_config_print(&config, stdout);

	/* Init db */
	db_init(config.db);

	/* Setup socket */
	btd_log(2, "Registering socket\n");
	socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if(socket_fd < 0) {
		perror("socket");
		die("Bye\n");
	} 
	btd_log(2, "Registered socket\n");
	memset(&address, 0, sizeof(struct sockaddr_un));

	address.sun_family = AF_UNIX;
	strcpy(address.sun_path, config.socket);

	if(bind(socket_fd, (struct sockaddr *) &address, 
			sizeof(struct sockaddr_un)) != 0) {
		perror("bind");
		die("Bye\n");
	}
	btd_log(2, "Bound socket\n");

	if(listen(socket_fd, 5) != 0) {
		perror("listen");
		die("Bye\n");
	}
	btd_log(2, "Listening to socket\n");

	btd_log(1, "Waiting for a client to connect\n");
	fflush(stdout);
	while((connection_fd = accept(socket_fd, (struct sockaddr *) &address,
			&address_length)) > -1) {
		btd_log(1, "Client connected...\n");
		child = fork();
		if(child == 0) {
			return connection_handler(connection_fd);
		}
		close(connection_fd);
	}
	depart("Bye...\n");
	cleanup();
}
