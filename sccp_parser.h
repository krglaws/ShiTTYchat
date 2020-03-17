
#ifndef _SCCP_PARSER_H_
#define _SCCP_PARSER_H_


/* ShiTTYchat Communication Protocol field list */
#define SCCP_VERSION "Version: %s;\n"
#define CONN_REQ "Message-Type: %s;\n"
#define REASON "Reason: %s;\n"
#define RSA_EXP "RSA-Key: %s; %s;\n"
#define UUID "UUID: %s;\n"
#define UNAME "User-Name: %s;\n"
#define TEXT_LEN "Text-Length: %s;\n"

// For the "Connection-Request" field, only two values are permitted:
#define IN_REQ_TYPE_STR "In-Socket"
#define OUT_REQ_TYPE_STR "Out-Socket"

// All the other fields can contain any value


// version info struct
struct
{
  unsigned short major;
  unsigned short minor;
  unsigned short patch;
} v_info;

// request type enum
enum message_type
{
  INFO,
  CLIENT_MSG,
  IN_REQ,
  OUT_REQ
};

// reason type enum
enum reason_type
{
  NONE,
  BAD_REQUEST,
  INV_VERSION
};


typedef struct
{
  // version info
  struct v_info version;

  // message type
  enum request_type req_type;
  char *divisor;
  char *exponent;
  unsigned base;
  
} sccp_message_t;


#endif

