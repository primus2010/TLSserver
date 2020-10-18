#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include "utils.h"

void init_openssl() {
  SSL_load_error_strings();
  OpenSSL_add_ssl_algorithms();
}

void cleanup_openssl() {
  EVP_cleanup();
}

SSL_CTX *create_context() {
  const SSL_METHOD *method;
  SSL_CTX *ctx;

  method = TLSv1_2_server_method();
  ctx = SSL_CTX_new(method);
  if (!ctx) {
    perror("Unable to create SSL context");
    ERR_print_errors_fp(stderr);
    exit(EXIT_FAILURE);
  }
  return ctx;
}

int configure_context(SSL_CTX *ctx, const char *cert_filename, const char *key_filename) {
  SSL_CTX_set_ecdh_auto(ctx, 1);
  if (SSL_CTX_use_certificate_file(ctx, cert_filename, SSL_FILETYPE_PEM) <= 0) {
    ERR_print_errors_fp(stderr);
    return -1;
  }
  if (SSL_CTX_use_PrivateKey_file(ctx, key_filename, SSL_FILETYPE_PEM) <= 0) {
    ERR_print_errors_fp(stderr);
    return -1;
  }
  return 0;
}

int main(int argc, char **argv) {
  PGconn *db_conn;
  int sock;
  SSL_CTX *ctx;
  char *conf_file_name;


  if (argc == 2) {
    conf_file_name = argv[1];
  } else {
    conf_file_name = "./conf/server_cfg.json";
  }
  char *json_string = getJSONfromFile(conf_file_name);
  if (json_string == NULL) {
    fprintf(stderr, "Unable get JSON string\n");
    exit (EXIT_FAILURE);
  }

  char *conn_string = getConnectString(json_string);
  if (NULL == conn_string) {
    fprintf(stderr, "Bad PG connection string\n");
    exit (EXIT_FAILURE);
  } 

  db_conn = getPostgreConn(conn_string);
  free(conn_string);
  if (NULL == db_conn) {
    fprintf(stderr, "Unable connect to DB\n");
    exit(EXIT_FAILURE);
  }


  init_openssl();
  ctx = create_context();
  char *cert_file = getStringByName("cert_file", json_string);
  char *key_file = getStringByName("key_file", json_string);
  int context_result = configure_context(ctx, cert_file, key_file);
  free(key_file);
  free(cert_file);
  free(json_string);

  sock = create_lstn_socket(4433);
  if (sock < 0) {
    fprintf(stderr, "Unable create socket");
    exit (EXIT_FAILURE);
  }


  while (true) {
    struct sockaddr_in addr;
    uint len = sizeof(addr);
    SSL *ssl;
    int client = accept(sock, (struct sockaddr*)&addr, &len);
    if (client < 0) {
      perror("Unable to accept");
      exit(EXIT_FAILURE);
    } else {
      printf("Socket accepted\n");
    }

    ssl = SSL_new(ctx);
    if (NULL == ssl) {
      fprintf(stderr,"Unable to get new ssl\n");
      ERR_print_errors_fp(stderr);
      exit(EXIT_FAILURE);
    } 
    SSL_set_fd(ssl, client);
    if (SSL_accept(ssl) <= 0) {
      ERR_print_errors_fp(stderr);
    } else {
      
      //Read all the data request from socket here
      char *buf = malloc(MAX_STRING_LEN);
      char tmp_buf[MAX_STRING_LEN];
      long cur_buf_size =  MAX_STRING_LEN;
      int msg_len = 0;
      int n = 0;
      while ((n = SSL_read(ssl, tmp_buf, MAX_STRING_LEN)) > 0) {
        if (cur_buf_size - msg_len <= n) {
          void *ptr = realloc(buf, cur_buf_size + MAX_STRING_LEN);
          if (NULL != ptr) {
            buf = ptr;
          }
          cur_buf_size += MAX_STRING_LEN;
        }
        memcpy(buf + msg_len, tmp_buf, n);
        msg_len += n;
      }
      buf[msg_len] = 0;
      //get users data and put it to DB
      struct UserPack userPack = getUsersData(buf);
      int count = putDataToDB(db_conn, userPack);
      printf("%d users inserted to DB\n", count);
      freeUserPack(&userPack);
      free(buf);
      //SSL_write(ssl, reply, strlen(reply));
    }
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(client);
  }
  close(sock);
  SSL_CTX_free(ctx);
  cleanup_openssl();
}
