#ifndef __MY_UTILS_H__
#define __MY_UTILS_H__

#include <stdint.h>
#include <libpq-fe.h>

#include <cJSON.h>

#define MAX_CONF_STRING_LEN 1024
#define MAX_STRING_LEN 1024

struct User {
  char firstname[64];
  char lastname[64];
  uint16_t age;
};

struct UserPack {
  struct User **pack;
  int count;
};

void freeUserPack(struct UserPack *pack);

char *getConnectString(const char *jsonString);
char *getStringByName(const char *name, const char *jsonString);
char *getJSONfromFile(const char *filename);

int create_lstn_socket(int port);

PGconn *getPostgreConn(const char *conn_string);

struct UserPack getUsersData(const char *data);

int putDataToDB(PGconn *conn, const struct UserPack userPack);
#endif //__MY_UTILS_H__
