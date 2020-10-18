#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>

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

  //method = SSLv23_client_method();
  method = TLS_client_method();
  ctx = SSL_CTX_new(method);
  if (!ctx) {
    fprintf(stderr, "Unable to create SSL context");
    ERR_print_errors_fp(stderr);
    exit(EXIT_FAILURE);
  }
  return ctx;
}

int main(int argc, char **argv) {
  SSL_CTX *ctx;
  init_openssl();
  ctx = create_context();

  SSL *ssl;
  ssl = SSL_new(ctx);

  SSL_set_connect_state(ssl);

  BIO *s_bio = BIO_new(BIO_f_ssl());

  if (!s_bio) {
    fprintf(stderr, "Unable create SSL bio");
    ERR_print_errors_fp(stderr);
    exit(EXIT_FAILURE);
  }

  BIO_set_ssl(s_bio, ssl, BIO_CLOSE);

  BIO *bio = BIO_new_connect("127.0.0.1:4433");
  if(BIO_do_connect(bio) <=0) {
    fprintf(stderr, "Unable connect to server\n");
    ERR_print_errors_fp(stderr);
    exit(EXIT_FAILURE);
  }

  BIO_push(s_bio, bio);

  char buf[MAX_STRING_LEN];
  int n = 0;
  //Open json file and send it's conten to server
  int fd;
  char *filename;
  if (argc > 1) {
    filename = argv[1];
  } else {
    filename = "./conf/server_cfg.json";
  }

  fd = open(filename, O_RDONLY);
  if (fd < 0) {
    perror("ERROR opening file");
    fprintf(stderr, "File name: %s\n", filename);
    exit(EXIT_FAILURE);
  }
  while ( (n = read(fd, buf, MAX_STRING_LEN)) > 0) {
    BIO_write(s_bio, buf, n);
  }
  close(fd);



  

  /*
  while( (n = BIO_read(s_bio, buf, BUFSIZE - 1)) > 0) {
    buf[n] = 0;
    printf("%s", buf);
  }
  */
  BIO_free_all(s_bio);
  SSL_CTX_free(ctx);
  cleanup_openssl();
  return EXIT_SUCCESS;
}
