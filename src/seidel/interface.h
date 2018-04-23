#ifndef __interface_h
#define __interface_h

#define TRUE 1
#define FALSE 0

extern int triangulate_polygon(int, int *, float (*)[2], int (*)[3]);
extern int is_point_inside_polygon(float *);

#endif /* __interface_h */
