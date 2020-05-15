
#ifndef _SERVER_H_
#define _SERVER_H_

void usage(char *name);

bool valid_port(char *port);

int server(char *port_str);

#endif

