/* Dijkstra's shortest-path algorithm, 2-D */
/*
  Copyright (C) 2004 University of Texas at Austin
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <float.h>

#include <rsf.h>
/*^*/

#include "dijkstra.h"

typedef struct Step {
    int ud, lr;
    struct Step* next;
} *step;

static int n1, n2, np, **status;
static float **cost;
static step **path, here;
static const float big_number = FLT_MAX;

void dijkstra_init(int m1, int m2)
/*< initialize with model size >*/
{
    int i2, i1;

    n1=m1;
    n2=m2;

    cost = sf_floatalloc2(n1,n2);
    status = sf_intalloc2(n1,n2);

    path = (step**) sf_alloc(n2,sizeof(step*));
    path[0] = (step*) sf_alloc(n1*n2,sizeof(step));
    for (i2=0; i2 < n2; i2++) {
	if (i2) path[i2] = path[0] + i2*n1;
	for (i1=0; i1 < n1; i1++) {
	    cost[i2][i1] = big_number;
	    status[i2][i1] = SF_OUT;
	    path[i2][i1] = NULL;
	}
    }
    sf_pqueue_init (2*(n1+n2));
}

void dijkstra_close(void)
/*< free allocated storage >*/
{
    int i2, i1;

    sf_pqueue_close ();
    for (i2=0; i2 < n2; i2++) {
	for (i1=0; i1 < n1; i1++) {
	    if (NULL != path[i2][i1]) free (path[i2][i1]);
	}
    }
    free (path[0]);
    free (path);
    free(cost[0]);
    free(cost);
    free(status[0]);
    free(status);
    free(path[0]);
    free(path);
}

static void fix_neighbor(int s1, int s2 /* location */, 
			 int ud, int lr /* up-down, left-right */, 
			 float shift    /* cost */)
/* process a sample */
{
    float *neighbor, newcost, oldstatus;
    step st;

    oldstatus = status[s2+lr][s1+ud];

    if (oldstatus == SF_IN) return;

    neighbor = cost[s2+lr]+s1+ud;
    newcost = cost[s2][s1] + shift;

    if (oldstatus == SF_OUT) {
	*neighbor = newcost;
	status[s2+lr][s1+ud] = SF_FRONT;
	sf_pqueue_insert (neighbor);
	np++;
	st = path[s2+lr][s1+ud] = sf_alloc(1,sizeof(struct Step));	
    } else if (newcost < *neighbor) {
	*neighbor = newcost;
	st = path[s2+lr][s1+ud];
    } else {
	return;
    }

    st->ud=ud;
    st->lr=lr;
    st->next = path[s2][s1];
}

static void neighbors(int s1, int s2, float **ud, float **lr)
/* process neighbors */
{
    if (s1 < n1-1) { fix_neighbor(s1,s2,+1,0,ud[s2][s1  ]); }
    if (s1 > 0)    { fix_neighbor(s1,s2,-1,0,ud[s2][s1-1]); }
    if (s2 < n2-1) { fix_neighbor(s1,s2,0,+1,lr[s2  ][s1]); }
    if (s2 > 0)    { fix_neighbor(s1,s2,0,-1,lr[s2-1][s1]); }
}

void dijkstra(int s1, int s2         /* source location */, 
	      float **ud, float **lr /* up-down and left-right cost */) 
/*< run the algorithm >*/
{
    int s;
    float *p;

    /* Intialize source */
    cost[s2][s1] = 0.;
    status[s2][s1] = SF_IN;
    sf_pqueue_start (); 
    np = 0.;
    neighbors(s1,s2,ud,lr);

    while (np > 0) {
	/* Extract smallest */
	p = sf_pqueue_extract(); 
	np--;

	if (p == NULL) {
	    sf_warning("%s: heap exausted!",__FILE__);
	    break;
	}

	s = p - cost[0];
	s2 = s/n1;
	s1 = s - s2*n1;

	status[s2][s1] = SF_IN;
	neighbors(s1,s2,ud,lr);
    }
}

void dijkstra_start(int s1, int s2)
/*< intialize path >*/
{
    here = path[s2][s1];
}

bool dijkstra_next(int *ud, int *lr) 
/*< find the next shift (up-down, left-right), return true on success >*/
{
    if (NULL == here) return false;

    *ud = here->ud;
    *lr = here->lr;

    here = here->next;

    return true;
}
