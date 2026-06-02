
#include "ylib.h"

char* basename(char* original) {
  int len = strlen(original);
  char* bn = calloc(len + 1, sizeof(char));

  int i = len - 1;

  while (i >= 1 && original[i - 1] != '/') i--;

  strncpy(bn, original + i, len + 1);
  bn[len] = '\0';

  return bn;
}

