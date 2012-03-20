/* gp.h - Genetic programming
 *
 * Copyright (c) 2012 Janosch Gräf <janosch.graef@gmx.net>
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef _PUSH_GP_H_
#define _PUSH_GP_H_


#include <glib.h>


typedef struct push_gp_S push_gp_t;
typedef struct push_gp_prog_S push_gp_prog_t;


#include "push/types.h"
#include "push/vm.h"


typedef void (*push_gp_func_t)(push_gp_t *gp, push_gp_prog_t *prog);
typedef push_real_t (*push_gp_fitness_func_t)(push_gp_t *gp, push_gp_prog_t *prog, push_int_t steps);
typedef GList *(*push_gp_selection_func_t)(push_gp_t *gp, push_int_t num, push_bool_t inverse);
typedef void (*push_gp_crossover_func_t)(push_gp_t *gp, push_gp_prog_t *prog1, push_gp_prog_t *prog2);


struct push_gp_prog_S {
  /* PUSH interpreter
   * NOTE: The userdata field is used by GP
   */
  push_t *push;

  /* Program code */
  push_val_t *code;

  /* If this programs has been evaluated yet */
  push_bool_t eval;

  /* Fitness after evaluation */
  push_real_t fitness;

  /* User data */
  void *userdata;
};


struct push_gp_S {
  /* PUSH VM */
  push_vm_t *vm;

  /* Population
   * TODO: Could also be a list
   */
  GPtrArray *pop;

  /* Random number generator */
  GRand *rand;

  /* Prepare function: Called after a interpreter was created */
  push_gp_func_t init_func;

  /* Prepare function: Called right before a program is run; or NULL */
  push_gp_func_t prepare_func;

  /* Fitness function: Called after a program was run and returns a fitness value
   * NOTE: This callback should also clean up everything setup before evaluation
   */
  push_gp_fitness_func_t fitness_func;

  /* Selection function */
  push_gp_selection_func_t selection_func;

  /* Mutation function */
  push_gp_func_t mutation_func;

  /* Crossover function */
  push_gp_crossover_func_t crossover_func;

  /* User data */
  void *userdata;
};


push_gp_t *push_gp_new(push_int_t population_size, push_int_t init_prog_size, push_int_t max_threads, push_int_t max_steps, push_int_t seed,
                       push_gp_func_t init_func, push_gp_func_t prepare_func, push_gp_fitness_func_t fitness_func,
                       push_gp_selection_func_t selection_func, push_gp_func_t mutation_func, push_gp_crossover_func_t crossover_func);
void push_gp_destroy(push_gp_t *gp);
push_int_t push_gp_get_num(push_gp_t *gp);
push_gp_prog_t *push_gp_get_nth(push_gp_t *gp, push_int_t i);
void push_gp_init_program(push_gp_t *gp, push_gp_prog_t *prog, push_int_t size, push_t *push_template);
void push_gp_run_program(push_gp_t *gp, push_gp_prog_t *prog);
void push_gp_eval(push_gp_t *gp);
void push_gp_generation(push_gp_t *gp);
push_gp_prog_t *push_gp_best_program(push_gp_t *gp);
GList *push_gp_selection_roulette_wheel_linear(push_gp_t *gp, push_int_t num, push_bool_t inverse);
GList *push_gp_selection_roulette_wheel_ranked(push_gp_t *gp, push_int_t num, push_bool_t inverse);
void push_gp_mutation_func(push_gp_t *gp, push_gp_prog_t *prog);
void push_gp_crossover_one_point(push_gp_t *gp, push_gp_prog_t *prog1, push_gp_prog_t *prog2);


#endif /* _PUSH_GP_H_ */

