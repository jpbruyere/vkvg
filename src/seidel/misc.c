#include "triangulate.h"
#include <sys/time.h>
#include <math.h>

#ifdef __STDC__
extern double log2(double);
#else
extern double log2();
#endif

static int choose_idx;
static int permute[SEGSIZE];


/* Generate a random permutation of the segments 1..n */
int generate_random_ordering(n)
     int n;
{
  struct timeval tval;
  struct timezone tzone;
  register int i;
  int m, st[SEGSIZE], *p;

  choose_idx = 1;
  gettimeofday(&tval, &tzone);
  srand48(tval.tv_sec);

  for (i = 0; i <= n; i++)
    st[i] = i;

  p = st;
  for (i = 1; i <= n; i++, p++)
    {
      m = lrand48() % (n + 1 - i) + 1;
      permute[i] = p[m];
      if (m != 1)
    p[m] = p[1];
    }
  return 0;
}


/* Return the next segment in the generated random ordering of all the */
/* segments in S */
int choose_segment()
{
  int i;

#ifdef DEBUG
  fprintf(stderr, "choose_segment: %d\n", permute[choose_idx]);
#endif
  return permute[choose_idx++];
}


#ifdef STANDALONE

/* Read in the list of vertices from infile */
int read_segments(filename, genus)
     char *filename;
     int *genus;
{
  FILE *infile;
  int ccount;
  register int i;
  int ncontours, npoints, first, last;

  if ((infile = fopen(filename, "r")) == NULL)
    {
      perror(filename);
      return -1;
    }

  fscanf(infile, "%d", &ncontours);
  if (ncontours <= 0)
    return -1;

  /* For every contour, read in all the points for the contour. The */
  /* outer-most contour is read in first (points specified in */
  /* anti-clockwise order). Next, the inner contours are input in */
  /* clockwise order */

  ccount = 0;
  i = 1;

  while (ccount < ncontours)
    {
      int j;

      fscanf(infile, "%d", &npoints);
      first = i;
      last = first + npoints - 1;
      for (j = 0; j < npoints; j++, i++)
    {
      fscanf(infile, "%lf%lf", &seg[i].v0.x, &seg[i].v0.y);
      if (i == last)
        {
          seg[i].next = first;
          seg[i].prev = i-1;
          seg[i-1].v1 = seg[i].v0;
        }
      else if (i == first)
        {
          seg[i].next = i+1;
          seg[i].prev = last;
          seg[last].v1 = seg[i].v0;
        }
      else
        {
          seg[i].prev = i-1;
          seg[i].next = i+1;
          seg[i-1].v1 = seg[i].v0;
        }

      seg[i].is_inserted = FALSE;
    }

      ccount++;
    }

  *genus = ncontours - 1;
  return i-1;
}

#endif


/* Get log*n for given n */
int math_logstar_n(n)
     int n;
{
  register int i;
  double v;

  for (i = 0, v = (double) n; v >= 1; i++)
    v = log2(v);

  return (i - 1);
}


int math_N(n, h)
     int n;
     int h;
{
  register int i;
  double v;

  for (i = 0, v = (int) n; i < h; i++)
    v = log2(v);

  return (int) ceil((double) 1.0*n/v);
}
