#ifndef _POLECART_H_
#define _POLECART_H_

#include <glib.h>


typedef enum {
  PC_STATUS_READY,
  PC_STATUS_LOST
} pc_status_t;

typedef struct {
  pc_status_t status;
  int allow_F_0;
  double th;
  double th_v;
  double x;
  double x_v;
  double F;
  double t;
  double tau;
  double g;
  double h_min;
  double h_max;
  double r_min;
  double r_max;
  double l;
  double m_c;
  double m_p;
  double F_max;
  double F_min;
} pc_t;


pc_t *pc_new(GRand *_rand);
void pc_destroy(pc_t *pc);
int pc_set_input(pc_t *pc, double F);
int pc_get_output(pc_t *pc, double *th, double *th_v, double *x, double *x_v);
pc_status_t pc_get_status(pc_t *pc);
int pc_step(pc_t *pc);
const char *pc_secho(pc_t *pc);
void pc_echo(pc_t *pc);

#endif /* _POLECART_H_ */
