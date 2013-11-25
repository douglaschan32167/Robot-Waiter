#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 

int main(int argc, char *argv[]) {
	int sockfd = 0, n = 0;
	char recvBuff[512];
	struct sockaddr_in serv_addr;
	
	memset(recvBuff, '0', sizeof(serv_addr));
	    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    } 

	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(5000);
	printf("set port");
	
	if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
		printf("\n inet_pton error occurred\n");
		return 1;
	}
		
	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		printf("\n Error : Connect Failed \n");
		return 1;
	}
//	while ((n = read(sockfd, recvBuff, sizeof(recvBuff)-1)) > 0) {
//		recvBuff[n] = 0;
//		if (fputs(recvBuff, stdout) == EOF) {
//			printf("\n Error : Fputs error\n");
//		}
//	}
	recvBuff[0] = 'h';
	recvBuff[1]= 'i';
	write(sockfd, recvBuff, 2);
	printf("%s", recvBuff);
	if(n < 0) {
		printf("\1n Read error \n");
	}

	return 0;
}