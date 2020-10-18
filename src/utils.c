#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <error.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <cJSON/cJSON.h>
#include "utils.h"

int create_lstn_socket(int port) {
  int s;
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0) {
    perror("Unable to create socket");
    return -1;
  }
  if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    perror("Unable to bind");
    return -1;
  }
  if (listen(s, 1) < 0) {
    perror("Unable to listen");
    return -1;
  }
  return s;
}


char *getConnectString(const char *jsonString) {
  char *result = malloc(MAX_CONF_STRING_LEN);
  memset(result, 0, MAX_CONF_STRING_LEN);
  cJSON *root = NULL;
  cJSON *pSub = NULL;
  root = cJSON_Parse(jsonString);
  if (NULL == root) {
    free(result);
    return NULL;
  }
  pSub = cJSON_GetObjectItem(root, "db_user");
  if (NULL == pSub) {
    cJSON_Delete(root);
    free(result);
    return NULL;
  }
  strcat(result, "user=");
  strcat(result, pSub->valuestring);
  strcat(result, " ");

  pSub = cJSON_GetObjectItem(root, "db_password");
  if (NULL == pSub) {
    cJSON_Delete(root);
    free(result);
    return NULL;
  }
  strcat(result, "password=");
  strcat(result, pSub->valuestring);
  strcat(result, " ");
  pSub = cJSON_GetObjectItem(root, "db_name");
  if (NULL == pSub) {
    cJSON_Delete(root);
    free(result);
    return NULL;
  }
  strcat(result, "dbname=");
  strcat(result, pSub->valuestring);
  cJSON_Delete(root);
  return result;
}


char *getStringByName(const char *name, const char *jsonString) {
  cJSON *root = NULL;
  cJSON *pSub = NULL;
  root = cJSON_Parse(jsonString);
  if (NULL == root) {
    return NULL;
  }
  pSub = cJSON_GetObjectItem(root, name);
  if (NULL == pSub) {
    cJSON_Delete(root);
    return NULL;
  }
  char *result = malloc(strlen(pSub->valuestring) + 1);
  strcpy(result, pSub->valuestring);
  cJSON_Delete(root);
  return result;
}

char *getJSONfromFile(const char *filename) {
  int fd;
  char *buf = NULL;
  fd = open(filename, O_RDONLY);
  if (fd < 0) {
    perror("ERROR opening file");
    fprintf(stderr, "File name: %s\n", filename);
    return NULL;
  }
  long len = lseek(fd, 0L, SEEK_END);
  lseek(fd, 0L, SEEK_SET);
  buf = malloc(len+1);
  char *tmp_buf = malloc(len+1);
  long rbytes = 0;
  long n = 0;
  while (rbytes < len) {
    n  = read(fd, tmp_buf, len);
    if (n < 0) {
      perror("Unable read json file");
      free(buf);
      free(tmp_buf);
      return NULL;
    }
    memcpy(buf + rbytes, tmp_buf, n);
    rbytes += n;
  }
  buf[len] = 0;
  free(tmp_buf);
  return buf;
}

PGconn *getPostgreConn(const char *conn_string) {
  PGconn *db_conn = PQconnectdb(conn_string);
  if (PQstatus(db_conn) != CONNECTION_OK) {
    fprintf(stderr, "Unable connect to DB: %s",
    PQerrorMessage(db_conn));
    PQfinish(db_conn);
    return NULL;
  }
  return db_conn;
}


struct User *getOneUser(const cJSON *jsn_user) {
  //printf("USER: %s\n", cJSON_Print(user)); 
  char *firstname;
  char *lastname;
  int age;
  struct User *user = malloc(sizeof(struct User));
  cJSON *field;
  field = cJSON_GetObjectItem(jsn_user, "firstname");
  if (NULL == field) {
    free(user);
    return NULL;
  } else {
    strcpy(user->firstname, field->valuestring);
  }
  field = cJSON_GetObjectItem(jsn_user, "lastname");
  if (NULL == field) {
    free(user);
    return NULL;
  } else {
    strcpy(user->lastname, field->valuestring);
  }
  field = cJSON_GetObjectItem(jsn_user, "age");
  if (NULL == field) {
    free(user);
    return NULL;
  } else {
    user->age = field->valueint;
  }
  return user;
}

struct UserPack getUsersData(const char *data) {
  cJSON *root = NULL;
  cJSON *field = NULL;
  struct UserPack result = {
    .pack = NULL,
    .count = 0
  };
  root = cJSON_Parse(data);
  if (NULL == root) {
    fprintf (stderr, "Bad client data\n");
    return result;
  }
  //check array
  field = cJSON_GetObjectItem(root, "users");
  if (NULL != field) {
    if (cJSON_IsArray(field)) {
      //proc every user
      int size = cJSON_GetArraySize(field);
      result.pack = malloc(sizeof(struct User) * size);

      for (int i = 0; i < size; i++) {
        cJSON *cjson_user;
        cjson_user = cJSON_GetArrayItem(field, i);
        struct User *user = getOneUser(cjson_user);
        if (NULL != user) {
          result.pack[result.count] = user;
          result.count += 1;
        }
      }
    } else {
      fprintf(stderr, "Bad client data. \"users\" isn't array\n");
      cJSON_Delete(root);
      return result;
    }
  
  } else { //just one user
    field = cJSON_GetObjectItem(root, "user");
    if (NULL == field) {
      fprintf (stderr, "Bad client data. There isn't \"user\" item\n");
      cJSON_Delete(root);
      return result;
    } else {
     struct User *user = getOneUser(field);
      result.pack = malloc(sizeof(struct User));
     result.pack[0] = user;
     result.count = 1;
    } 
  }
  cJSON_Delete(root);
  return result;
}

void freeUserPack(struct UserPack *pack) {
  for (int i = 0; i < pack->count; i++) {
    free(pack->pack[i]);
  }
  free(pack->pack);
}

int putDataToDB(PGconn *conn, const struct UserPack userPack) {
  int result = 0;
  for (int i = 0; i < userPack.count; i++) {
    char query[MAX_STRING_LEN] = {0};
    sprintf(query, "INSERT INTO USERS (firstname, lastname, age) VALUES ('%s', '%s', %d)",
        userPack.pack[i]->firstname,
        userPack.pack[i]->lastname,
        userPack.pack[i]->age);
    PGresult *res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
      fprintf(stderr, "ERROR while inserting USER data\n");
    } else {
      result += 1;
    }
  }
  //return userPack.count;
  return result;
}
