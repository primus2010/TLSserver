#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include "utils.h"

int main(int argc, char **argv) {
  char *jsonString;
  if (argc == 2) {
    jsonString = getJSONfromFile(argv[1]);
  } else {
    jsonString = getJSONfromFile("./users.json");
  }
  if (NULL == jsonString) {
    printf("Error get json from file\n");
    return EXIT_FAILURE;
  } 
  printf("DATA: %s\n", jsonString);
  struct UserPack users = getUsersData(jsonString);
  for (int i = 0; i < users.count; i++) {
    printf("fname: %s\tlname: %s\tage: %d\n",
        users.pack[i]->firstname,
        users.pack[i]->lastname,
        users.pack[i]->age);
  }

  freeUserPack(&users);
  
  return EXIT_SUCCESS;
}
