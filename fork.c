#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/wait.h>

char webpage[] ="HTTP/1.1 200 OK\r\n"
		"Content-Type: text/html; charset=UTF-8\r\n\r\n"
		"<!DOCTYPE html>\r\n"
		"<html><head><title>My Website</title></head>\r\n"
		"<center><h1>Meow~</h1><br>\r\n"
		"<img src=\"cat.jpg\"></center>\r\n"
		"</html>\r\n";

int main(int argc, char *argv[]){
	struct sockaddr_in server_addr, client_addr;
	int fd_server , fd_client;
	char buf[2048];
	int fdimg, pid;
	int on = 1;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(80);
	fd_server = socket(AF_INET, SOCK_STREAM, 0);
	if(fd_server < 0){
		perror("socket");
		exit(1);
	}
	setsockopt(fd_server, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));
	if(bind(fd_server, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1){
		perror("bind");
		close(fd_server);
		exit(1);
	}
	if(listen(fd_server, 256) == -1) {
		perror("listen");
		close(fd_server);
		exit(1);
	}
	while(1){
		socklen_t ca_len = sizeof(client_addr);
		fd_client = accept(fd_server, (struct sockaddr *) &client_addr, &ca_len);
		if(fd_client == -1){
			perror("Connection failed\n");
			continue;
		}
		printf("Got client connection\n");
		pid = fork();
		if(!pid){
			close(fd_server);
			memset(buf, 0, 2048);
			read(fd_client, buf, 2047);
			printf("%s\n", buf);
			if(!strncmp(buf, "GET /cat.jpg", 12)){
				fdimg = open("cat.jpg", O_RDONLY);
				sendfile(fd_client, fdimg, NULL, 9000);
				close(fdimg);
			}
			else
				write(fd_client, webpage, sizeof(webpage) -1);
			close (fd_client);
			printf("closing\n");
			exit(0);
		}
		if(pid) {
			wait(NULL);
			close(fd_client);
		}
	}
	return 0;
}
