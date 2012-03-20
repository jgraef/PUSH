#include <math.h>
#include <stdio.h>

#include <glib.h>

#include "polecart.h"

static struct {
  int allow_F_0;
  double th_min;
  double th_max;
  double x_min;
  double x_max;
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
} pc_default = {
  .allow_F_0 = 0,
  .th_min = -M_PI/12.,
  .th_max = M_PI/12.,
  .x_min = -1.5,
  .x_max = 1.5,
  .tau = .02,
  .g = 9.80665,
  .h_min = -2.4,
  .h_max = 2.4,
  .r_min = -M_PI/15.,
  .r_max = M_PI/15.,
  .l = .5,
  .m_c = 1.,
  .m_p = .1,
  .F_min = -10.,
  .F_max = 10.,
};

pc_t *pc_new(GRand *_rand) {
  pc_t *pc;
  GRand *rand;

  pc = g_slice_new(pc_t);

  rand = _rand == NULL ? rand = g_rand_new() : _rand;

  pc->status = PC_STATUS_READY;
  pc->allow_F_0 = pc_default.allow_F_0;
  pc->th = g_double_rand_range(rand, pc_default.th_min, pc_default.th_max);
  pc->th_v = 0.;
  pc->x = g_double_rand_range(rand, pc_default.x_min, pc_default.x_max);
  pc->x_v = 0.;
  if (pc_default.allow_F_0) {
    pc->F = 0.;
  }
  else {
    pc->F = pc_default.F_max;
  }
  pc->t = 0.;
  pc->tau = pc_default.tau;
  pc->g = pc_default.g;
  pc->h_min = pc_default.h_min;
  pc->h_max = pc_default.h_max;
  pc->r_min = pc_default.r_min;
  pc->r_max = pc_default.r_max;
  pc->l = pc_default.l;
  pc->m_c = pc_default.m_c;
  pc->m_p = pc_default.m_p;
  pc->F_min = pc_default.F_min;
  pc->F_max = pc_default.F_max;

  if (_rand == NULL) {
    g_rand_free(rand);
  }

  return 0;
}

void pc_destroy(pc_t *pc) {
  g_slice_free(pc_t, pc);
}

int pc_set_input(pc_t *pc, double F) {
  if (F!=pc->F_min && F!=pc->F_max && (F!=0. || !pc->allow_F_0)) {
    return -1;
  }
  else {
    pc->F = F;
  }
}

int pc_get_output(pc_t *pc, double *th, double *th_v, double *x, double *x_v) {
  if (th!=NULL) {
    *th = pc->th;
  }
  if (th_v!=NULL) {
    *th_v = pc->th_v;
  }
  if (x!=NULL) {
    *x = pc->x;
  }
  if (x_v!=NULL) {
    *x_v = pc->x_v;
  }

  return 0;
}

pc_status_t pc_get_status(pc_t *pc) {
  return pc->status;
}

int pc_step(pc_t *pc) {
  double th_a, x_a;

  if (pc->status==PC_STATUS_READY) {
    th_a = pc->g*sin(pc->th)+cos(pc->th)*(-pc->F-pc->m_p*pc->l*pow(pc->th_v, 2.)*sin(pc->th)/(pc->m_c+pc->m_p))/(pc->l*(4./3.-pc->m_p*pow(cos(pc->th), 2.)/(pc->m_c+pc->m_p)));
    x_a = pc->F+pc->m_p*pc->l*(pow(pc->th_v, 2.)*sin(pc->th)-th_a*cos(pc->th))/(pc->m_c+pc->m_p);

    pc->th += pc->tau*pc->th_v;
    pc->th_v += pc->tau*th_a;
    //printf("theta: %f rad;  %f rad/s;  %f rad/s^2\n", pc->th, pc->th_v, th_a);

    pc->x += pc->tau*pc->x_v;
    pc->x_v += pc->tau*x_a;
    //printf("x:     %f m;  %f m/s;  %f m/s^2\n", pc->x, pc->x_v, x_a);

    pc->t += pc->tau;

    if (pc->x>pc->h_max || pc->x<pc->h_min || pc->th>pc->r_max || pc->th<pc->r_min) {
      pc->status = PC_STATUS_LOST;
    }

    return 0;
  }
  else {
    return -1;
  }
}

void pc_echo(pc_t *pc) {
  printf("Pole Cart:\n%s", pc_secho(pc));
}

const char *pc_secho(pc_t *pc) {
  static char buf[1024];

  snprintf(buf, 1024,
    "Status:    %s\n"
    "Allow F=0: %s\n"
    "theta:     %f rad\n"
    "theta*:    %f rad/s\n"
    "x:         %f m\n"
    "x*:        %f m/s\n"
    "F:         %f N\n"
    "t:         %f s\n"
    "tau:       %f s\n"
    "g:         %f m/s^2\n"
    "h_min:     %f m\n"
    "h_max:     %f m\n"
    "r_min:     %f rad\n"
    "r_max:     %f rad\n"
    "l:         %f m\n"
    "m_c:       %f kg\n"
    "m_p:       %f kg\n"
    "F_min:     %f kg\n"
    "F_max:     %f kg\n",
    pc->status==PC_STATUS_READY?"ready":"lost",
    pc->allow_F_0?"yes":"no",
    pc->th,
    pc->th_v,
    pc->x,
    pc->x_v,
    pc->F,
    pc->t,
    pc->tau,
    pc->g,
    pc->h_min,
    pc->h_max,
    pc->r_min,
    pc->r_max,
    pc->l,
    pc->m_c,
    pc->m_p,
    pc->F_min,
    pc->F_max
  );

  return buf;
}
