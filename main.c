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
const char *PIECECHARS="KR";

typedef struct {
  int64_t piece[33]; /* position of pieces */
  int64_t wtm; /* flag saying if white is to move */
  char *type; /* piece types, KQ etc. */
  int64_t side[33]; /* 0: black, 1: white */
  int n; /* number of pieces */
  int64_t kings[2]; /* black at 0, white at 1 */
} Position;

Bitboard
bit(int64_t n) {
  return 1ull<<n;
}

void
BB_print(Bitboard bb) {
  int64_t rank;
  int64_t file;
  printf("========\n");
  for (rank=8;rank--;) {
    for (file=0;file<8;++file)
      printf("%c", "*-"[!(bb&bit(rank*8+file))]);
    printf("\n");
  }
}

Bitboard
occupied(Position *p)
{
  Bitboard oc = 0;
  int64_t i;

  for(i=p->n;i--;)
    oc |= bit(p->piece[i]);

  return oc;
}

int64_t
pos_offset(Position *p)
{
  /* compute table index for position */
  int64_t i;
  int64_t value = p->wtm<<(6*p->n);
  for(i=p->n;i--;) {
    value += p->piece[i]<<(6*i);
  }
  return value;
}

Bitboard
gen_moves(Position *p, int64_t mover, Bitboard oc)
{
  Bitboard mv = 0;
  Bitboard bb;
  int64_t from = p->piece[mover];
  
  switch (p->type[mover]) {
  case 'K':
    mv = bit(from);
    if (7!=(from&7)) /* not next to left side */
      mv |= mv << 1;
    if (from&7) /* not next to right side */
      mv |= (mv >> 1);
    mv |= (mv<<8) | (mv>>8); /* up or down no problem as moves off board are also shifted out */
    mv ^= bit(from); /* remove move to same square */
    break;
  case 'R':
    /* checked king at a1 (square 0) is not a problem as it will block no move */
    for (bb = bit(from); (bb>>=1) & 0x7f7f7f7f7f7f7f7full;) { /* moving west */
      mv |= bb;
      if (bb&oc) break; /* not moving through pieces */
    }
    for (bb = bit(from); (bb<<=1) & 0xfefefefefefefefeull;) { /* moving east */
      mv |= bb;
      if (bb&oc) break; /* not moving through pieces */
    }
    for (bb = bit(from); bb>>=8;) { /* moving south */
      mv |= bb;
      if (bb&oc) break; /* not moving through pieces */
    }
    for (bb = bit(from); bb<<=8;) { /* moving north */
      mv |= bb;
      if (bb&oc) break; /* not moving through pieces */
    }
    break;
    default:
    printf("Internal error! Can't generate moves for unknown piece \"%c\".\n", p->type[mover]);
    exit(1);
  }
  return mv;
}

void
mark_check(Byte *tb, Position *p, int64_t mover, int64_t checked, int64_t value)
{
  int64_t ind;
  int64_t sq;
  Bitboard oc = occupied(p);
  Bitboard mv = gen_moves(p, mover, oc);

  for (sq=64;sq--;) {
    if (mv&bit(sq)) {
      p->piece[checked] = sq;
      ind = pos_offset(p);
      if (-1 != tb[ind]) /* don't remove illegal position value */
	tb[ind] = value;
    }
  }
}

void
count_positions(Byte *tb, int64_t n) {
  int64_t count[256];
  int64_t i;

  for (i=256;i--;) count[i] = 0;
  for (i=n;i--;) ++count[0xff&tb[i]];
  for (i=256;i--;)
    if (count[i])
      printf("%3d: %15ld\n", (Byte)i, count[i]);
}

void
mark_legal(Byte *tb, Position *p)
{
  int64_t i;
  int64_t illegal_count = 0;
  int64_t legal_count = 0;
  int64_t position_count = 0;

  for(p->wtm=2;p->wtm--;) { /* iterate over wtm and btm */
    /* init piece table */
    p->piece[p->n]=1; /* extra piece to mark end */
    for(i=p->n;i--;) p->piece[i] = 077;

    /* for each position */
    while(p->piece[p->n]) {
      {
	Byte value = 101; /* legal position */
	Bitboard oc=0;
	for(i=p->n;i--;) {
	  Bitboard bb = bit(p->piece[i]);
	  if(oc&bb) value = -1; /* illegal */
	  oc |= bb;
	}
	if (101 == value) ++legal_count;
	if (-1 == value) ++illegal_count;
	++position_count;
	tb[pos_offset(p)] = value;
      }
      if (0 == p->piece[p->kings[!p->wtm]]) { /* nonmoving king at square 0 */
	  for (i=p->n;i--;) {
	    if (p->side[i]==p->wtm) { /* piece i to move */
	      mark_check(tb, p, i, p->kings[!p->wtm], -1); /* illegal for king not on move to be in check */
	    }
	  }
	  p->piece[p->kings[!p->wtm]] = 0; /* reset nonmoving kings position */
	}

      /* decrement to next position */
      for(i=0;!p->piece[i]--;) {
	p->piece[i++] = 077;
      }
    }
  }
  printf("%ld legal and %ld illegal out of %ld positions when initializing.\n",
	 legal_count, illegal_count, position_count);
}

int
main(int argc, char *argv[])
{
  Byte *tb;
  int64_t i;
  int kings=0;
  int fd;
  size_t sz;
  Position *p;
  

  if (2 != argc) {
    printf("usage: %s KXKY\n", argv[0]);
    exit(1);
  }
  
  p = malloc(sizeof(Position));
  p->type = argv[1];
  p->n = strlen(argv[1]);
  if (32 < p->n) {
    printf("Sorry, no way I can compute endings with %d pieces.\n", p->n);
    exit(1);
  }

  for(i=p->n;i--;) {
    char piece = argv[1][i];
    if (NULL==strchr(PIECECHARS, piece)) {
      printf("Sorry, \"%c\" is an unknown piece type. Try one of \"%s\".\n", piece, PIECECHARS);
      exit(1);
    }
    p->side[i] = kings; /* 0: black, 1: white */
    if ('K' == piece) {
      p->kings[kings] = i;
      ++kings;
    }
  }
  if (2!=kings) {
    printf("Sorry, endings with %d kings don't make sense to me. Try one with 2.\n", kings);
    exit(1);
  }
  printf("Ending %s has %d men!\n", argv[1], p->n);
  sz = sizeof(Byte)<<(1+6*p->n);
  tb = malloc(sz);
  printf("Table has %ld bytes.\n", sz);

  /* compute */
  mark_legal(tb, p);
  
  count_positions(tb, sz);
  /* write result */
  fd = creat(argv[1], 00666);
  while (sz>(1<<24)) {
    if ((1<<24)!=write(fd, tb, 1<<24)) {
      printf("Error: Failed to write the %s table properly.\n", argv[1]);
      exit(1);
    }
    sz -= 1<<24; tb += 1<<24;
  }
  if (sz!=(size_t)write(fd, tb, sz)) {
    printf("Error: Failed to write the %s table properly.\n", argv[1]);
    exit(1);
  }

  return 0;
}
