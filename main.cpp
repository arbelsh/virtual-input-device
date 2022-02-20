#include "vkeyboard.h"
#include <unistd.h>
#include <iostream>
#include <stdexcept>
#include <cerrno>
#include <cstdio>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define C_PORT (5000)
#define C_MAX_BUF_LEN (1460)

int main(int argc, char* argv[])
{
	int sockfd;
	struct sockaddr_in addr = {0};
	struct sockaddr_in from = {0};
	socklen_t from_len = sizeof(from);
	uint8_t buf[C_MAX_BUF_LEN] = {0};
	ssize_t buf_len = 0;
	
	try
	{
		CVKeyboard kb("vkeyboard"); // create the virtual keyboard
		sleep(2);
		if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) // create a socket for instructions
		{
			std::perror("socket() failed");
			exit(-1);
		}
		
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = INADDR_ANY;
		addr.sin_port = htons(C_PORT);
		
		if (bind(sockfd, (const struct sockaddr *)&addr, sizeof(addr)) < 0 ) // bind the socket
		{
			std::perror("bind() failed");
			exit(-1);
		}
		
		while(1)
		{
			from_len = sizeof(from);
			buf_len = recvfrom(sockfd, buf, sizeof(buf)-1, 0, (struct sockaddr *)&from, &from_len); // listen for instructions
			if(buf_len > 0)
			{
				kb.Parse(buf, buf_len); // pass the data to the virtual keyboard for parsing
			}
		}
	}
	catch(std::runtime_error ex)
	{
		std::cout <<  "Exception: " << ex.what() << std::endl;
		return -1;
	}
	return 0;
}