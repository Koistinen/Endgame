#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint64_t Bitboard;
const char *PIECECHARS="K";

main(int argc, char *argv[])
{
  int8_t *tb;
  int n;
  int i;
  int kings=0;
  
  if (2 != argc) {
    printf("usage: %s KXKY\n", argv[0]);
    exit(1);
  }
  
  n = strlen(argv[1]);
  for(i=n;i--;) {
    char piece = argv[1][i];
    if (NULL==strchr(PIECECHARS, piece)) {
      printf("Sorry, \"%c\" is an unknown piece type. Try one of \"%s\".\n", piece, PIECECHARS);
      exit(1);
    }
    if ('K' == piece) {
      ++kings;
    }
  }
  if (2!=kings) {
    printf("Sorry, endings with %d kings don't make sense to me. Try one with 2.\n", kings);
    exit(1);
  }
  printf("Ending %s has %d men!\n", argv[1], n);
}
