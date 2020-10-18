#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "cJSON/cJSON.h"
#include "utils.h"

int main (int argc, char **argv) {
  
  char *jsonString = getJSONfromFile("../conf/server_cfg.json");
  if (NULL == jsonString) {
    printf("Error get json from file\n");
  } else {
    printf("json file: \n%s\n", jsonString);
  }

  char *result = NULL;
  result = getConnectString(jsonString);
  if (NULL == result) {
    printf("Error parse conf\n");
  } else {
    printf("dbstring: %s\n", result);
  }
/*
  PGconn *conn = getPostgreConn(result);
  if (conn != NULL) {
    printf("Connected to DB\n");
    PQfinish(conn);
  } 
  free(result);
*/

  result = getStringByName("cert_file", jsonString);
  if (NULL == result) {
    printf("Error parse cert_file\n");
  } else {
    printf("cert_file: %s\n", result);
    free(result);
  }

  result = getStringByName("key_file", jsonString);
  if (NULL == result) {
    printf("Error parse key_file\n");
  } else {
    printf("key_file: %s\n", result);
    free(result);
  }

  free(jsonString);

  printf("Hi test\n");

  return 0;
}

