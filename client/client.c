
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int main(int argc, char **argv)
{
	char *uname = NULL;
	char *ip = NULL;
	char *port = NULL;

	/* get args */
	int c;
	while ((c = getopt(argc, argv, "u:s:p:")) != -1)
	{
		switch (c)
		{
		case 'u':
			uname = optarg;
			break;
		case 'i':
			ip = optarg;
			break;
		case 'p':
			port = optarg;
			break;
		case '?':
			usage(argv[0]);
			return -1;
		}
	}

	/* check if essential args are NULL */
	if (!(uname && ip))
	{
		usage(argv[0]);
		return -1;
	}

	/* validate args */
	if (!valid_uname(uname) || !valid_ip(ip) || !valid_port(port))
	{
		return -1;
	}

	/* connect to server */
	
}


bool valid_uname(char* uname)
{
	int len = strlen(uname);
	if (len > UNAMELEN)
	{
		fprintf(stderr, "user name too long (16 characters maximum)\n");
		return false;
	}

	for (int i = 0; i < len; i++)
	{
		char curr = uname[i];
		if (!isprint(curr) || isspace(curr) || ispunct(curr))
		{
			fprintf(stderr, "username contains invalid characters\n");
			return false;
		}
	}

	return true;
}


bool valid_ip(char* ip)
{
	struct sockaddr_in sa;

	if (ip && inet_pton(AF_INET, ip, &(sa.sin_addr)) == 0)
	{
		fprintf(stderr, "invalid ip address\n");
		return false;
	}

	return true;
}


bool valid_port(char* port)
{
	if (port == NULL) return true;

	unsigned p = strtol(port, NULL, 10);

	if (p < 1 || p > 65535)
	{
		fprintf(stderr, "invalid port\n");
		return false;
	}

	return true;
}


void usage(char *name)
{
	fprintf(stderr, "usage: %s -u <username> -i <ip> [-p <port>]\n", name);
}


