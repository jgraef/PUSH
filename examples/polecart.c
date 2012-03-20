#include <stdio.h>

#include <glib.h>

#include "push.h"
#include "polecart.h"


#define POPULATION_SIZE        100
#define INITIAL_PROGRAM_SIZE    50
#define MAX_THREADS              1
#define MAX_STEPS             1000
#define SEED                     0 /* random seed */
#define STEPS_PER_SEC           10
#define FITNESS_FACTOR        0.01
#define FITNESS_THRESHOLD     (FITNESS_FACTOR * ((push_real_t)MAX_STEPS / (push_real_t)STEPS_PER_SEC))


static void print_state(push_t *push) {
  push_gp_prog_t *prog = (push_gp_prog_t*)push->userdata;
  pc_t *pc = (pc_t*)prog->userdata;
  char *xml;

  /* print interpreter state */
  xml = push_dump_state(push);
  printf("PUSH interpreter:\n%s", xml);
  g_free(xml);

  /* print simulation state */
  printf("Pole cart simulation:\n");
  pc_echo(pc);
}


static void gp_pc_left(push_t *push, push_gp_prog_t *prog) {
  pc_t *pc = (pc_t*)prog->userdata;

  pc_set_input(pc, pc->F_min);
}


static void gp_pc_right(push_t *push, push_gp_prog_t *prog) {
  pc_t *pc = (pc_t*)prog->userdata;

  pc_set_input(pc, pc->F_max);
}


static void gp_pc_input(push_t *push, push_gp_prog_t *prog) {
  pc_t *pc = (pc_t*)prog->userdata;
  push_real_t th, th_v, x, x_v;

  pc_get_output(pc, &th, &th_v, &x, &x_v);

  push_stack_push_new(push, push->real, PUSH_TYPE_REAL, x_v);
  push_stack_push_new(push, push->real, PUSH_TYPE_REAL, x);
  push_stack_push_new(push, push->real, PUSH_TYPE_REAL, th_v);
  push_stack_push_new(push, push->real, PUSH_TYPE_REAL, th);
}


static push_bool_t gp_pc_step(push_t *push, push_gp_prog_t *prog) {
  pc_t *pc = (pc_t*)prog->userdata;

  pc_step(pc);

  /* abort execution if simulation is done */
  return pc_get_status(pc) == PC_STATUS_READY;
}


static void gp_init(push_gp_t *gp, push_gp_prog_t *prog) {
  push_t *push = prog->push;

  /* register instructions for controlling the pole cart */
  push_instr_reg(push, "PC.LEFT", (push_instr_func_t)gp_pc_left, prog);
  push_instr_reg(push, "PC.RIGHT", (push_instr_func_t)gp_pc_left, prog);
  push_instr_reg(push, "PC.INPUT", (push_instr_func_t)gp_pc_input, prog);

  /* register step hook for stepping pole cart simulation */
  push->step_hook = (push_step_hook_t)gp_pc_step;
}


static void gp_prepare(push_gp_t *gp, push_gp_prog_t *prog) {
  pc_t *pc;

  /* create pole cart simulation */
  pc = pc_new(gp->rand);
  pc->tau = 1.0 / (push_real_t)STEPS_PER_SEC;
  pc->allow_F_0 = FALSE;

  prog->userdata = pc;
}


static push_real_t gp_fitness(push_gp_t *gp, push_gp_prog_t *prog, push_int_t steps) {
  pc_t *pc = (pc_t*)prog->userdata;
  push_real_t fitness;

  /* calculate fitness from simulation time */
  fitness = FITNESS_FACTOR * pc->t;

  pc_destroy(pc);

  return fitness;
}


int main(int argc, char *argv[]) {
  push_gp_t *gp;
  push_gp_prog_t *best;

  gp = push_gp_new(POPULATION_SIZE, INITIAL_PROGRAM_SIZE, MAX_THREADS, MAX_STEPS, SEED, gp_init, gp_prepare, gp_fitness, NULL, NULL, NULL);

  do {
    push_gp_generation(gp);
    best = push_gp_best_program(gp);
    printf("%d: best: fitness = %f\n", best->fitness);
  } while (best->fitness >= FITNESS_THRESHOLD);

  printf("Fitness threshold reached.\n");

  return 0;
}
