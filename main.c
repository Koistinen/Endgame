#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

typedef uint64_t Bitboard;
typedef int8_t Byte;
const char *PIECECHARS="K";

void
mark_legal(Byte *tb, int n, char *piece_types)
{
  int piece[33]; /* position of pieces */
  int wtm;
  int i;

  for(wtm=2;wtm--;) {
    piece[n]=1;
    for(i=n;i--;) piece[i] = 077;
    while(piece[n]) {
      int pos = wtm<<(6*n);
      for(i=n;i--;) {
	pos += piece[i]<<(6*i);
      }
      {
	tb[pos] = 0;
      }
      for(i=0;!piece[i]--;) {
	piece[i++] = 077;
      }
    }
  }
}

int
main(int argc, char *argv[])
{
  Byte *tb;
  int n;
  int i;
  int kings=0;
  int fd;
  int sz;

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
  sz = 2<<(6*n);
  tb = malloc(sz);
  printf("Table has %d bytes.\n", sz);

  /* compute */
  mark_legal(tb, n, argv[1]);
  
  fd = creat(argv[1], 00666);
  if (sz!=write(fd, tb, sz)) {
    printf("Error: Failed to write the table properly.\n");
    exit(1);
  }

  return 0;
}
