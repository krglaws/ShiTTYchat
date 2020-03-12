
#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

/* ShiTTYchat Communication Protocol field list */
#define SCCP_VERSION "SCCP: %s;\n"
#define CONN_REQ "Connection-Request: %s;\n"
#define REASON "Reason: %s;\n"
#define RSA_EXP "RSA-Key: %s; %s;\n"
#define UUID "UUID: %s;\n"
#define UNAME "User-Name: %s;\n"
#define TEXT_LEN "Text-Length: %s;\n"

// For the "Connection-Request" field, only two values are permitted:
#define IN_REQ_TYPE "In-Socket"
#define OUT_REQ_TYPE "Out-Socket"

// All the other fields can contain any value

#endif

