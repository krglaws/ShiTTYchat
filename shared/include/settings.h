
#ifndef _SETTINGS_H_
#define _SETTINGS_H_

/* default port */
#define DEFAULT_PORT (42069)

/* maximum clients allowed to connect */
#define MAXCLIENTS (10)

/* Maximum username length */
#define UNAMELEN (16)

/* maximum message length from client */
#define MAXMSGLEN (256)

/* buffer length for broadcasted messages (+32 for time stamp, spaces, etc.) */
#define BROADCASTLEN (MAXMSGLEN + UNAMELEN + 32)

/* RSA key base encoding */
#define RSAKEYENC (62)

/* number of bits needed to represent one digit in base RSAKEYENC */
#define BITSPERDIG (6)

/* RSA key bit length */
#define RSAKEYLEN (1024)

/* maximum number of chunks(1) allowed */
#define NUMCHUNKS (4)

/* Size of buffer for receiving raw data */
#define RECVBUFFLEN (NUMCHUNKS * (RSAKEYLEN / BITSPERDIG))

#endif

