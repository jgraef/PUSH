/* gp.c - Genetic programming
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

#include <glib.h>

#include "push.h"



push_gp_t *push_gp_new(push_int_t population_size, push_int_t init_prog_size, push_int_t max_threads, push_int_t max_steps, push_int_t seed,
                       push_gp_func_t init_func, push_gp_func_t prepare_func, push_gp_fitness_func_t fitness_func,
                       push_gp_selection_func_t selection_func, push_gp_func_t mutation_func, push_gp_crossover_func_t crossover_func) {
  push_gp_t *gp;
  push_int_t i;
  push_gp_prog_t *prog;

  g_return_val_if_fail(population_size > 0, NULL);
  g_return_val_if_fail(init_prog_size > 0, NULL);
  g_return_val_if_null(fitness_func, NULL);

  gp = g_slice_new(push_gp_t);

  gp->vm = push_vm_new(max_threads, max_steps, NULL);
  gp->rand = seed != 0 ? g_rand_new_with_seed(seed) : g_rand_new();
  gp->init_func = init_func;
  gp->prepare_func = prepare_func;
  gp->fitness_func = fitness_func;
  gp->selection_func = selection_func == NULL ? push_gp_selection_roulette_wheel_linear: selection_func;
  gp->mutation_func = mutation_func == NULL ? push_gp_mutation_func: mutation_func;
  gp->crossover_func = crossover_func == NULL ? push_gp_crossover_one_point: crossover_func;

  /* initialize random population */
  gp->pop = g_ptr_array_sized_new(population_size);
  for (i = 0; i < population_size; i++) {
    prog = g_slice_new(push_gp_prog_t);
    g_ptr_array_add(gp->pop, prog);
    push_gp_init_program(gp, prog, init_prog_size, NULL);
  }

  return gp;
}


void push_gp_destroy(push_gp_t *gp) {
  push_gp_prog_t *prog;
  push_int_t i;

  g_return_if_null(gp);

  push_vm_destroy(gp->vm, TRUE);

  /* destroy all programs */
  for (i = 0; i < gp->pop->len; i++) {
    prog = push_gp_get_nth(gp, i);
    push_destroy(prog->push);
    g_slice_free(push_gp_prog_t, prog);
  }
  g_ptr_array_free(gp->pop, TRUE);

  g_slice_free(push_gp_t, gp);
}


push_int_t push_gp_get_num(push_gp_t *gp) {
  return gp->pop->len;
}


push_gp_prog_t *push_gp_get_nth(push_gp_t *gp, push_int_t i) {
  return (push_gp_prog_t*)g_ptr_array_index(gp->pop, i);
}


void push_gp_init_program(push_gp_t *gp, push_gp_prog_t *prog, push_int_t size, push_t *push_template) {
  push_t *push;

  push = push_template == NULL ? push_new() : push_copy(push_template);
  push->userdata = prog;
  push_rand_set_seed(push, g_rand_int(gp->rand));

  prog->push = push;
  prog->code = push_rand_val(push, PUSH_TYPE_CODE, &size, TRUE);
  prog->fitness = 0.0;
  prog->userdata = NULL;

  if (gp->init_func != NULL) {
    gp->init_func(gp, prog);
  }
}


void push_gp_run_program(push_gp_t *gp, push_gp_prog_t *prog) {
  /* flush all stacks and remove all bindings */
  push_flush(prog->push);

  push_stack_push(prog->push->code, prog->code);
  push_stack_push(prog->push->exec, prog->code);

  if (gp->prepare_func != NULL) {
    gp->prepare_func(gp, prog);
  }

  push_vm_run(gp->vm, prog->push);
}


void push_gp_eval(push_gp_t *gp) {
  push_int_t i;
  push_gp_prog_t *prog;

  /* run all programs */
  for (i = 0; i < gp->pop->len; i++) {
    prog = push_gp_get_nth(gp, i);

    if (!prog->eval) {
      push_gp_run_program(gp, prog);
    }
  }

  /* wait until all programs finished */
  push_vm_wait(gp->vm);

  /* set eval flag and call fitness function */
  for (i = 0; i < gp->pop->len; i++) {
    if (!prog->eval) {
      prog->eval = TRUE;
      /* TODO: 3rd argument: pass number of steps */
      prog->fitness = gp->fitness_func(gp, prog, 0);
    }
  }
}


void push_gp_generation(push_gp_t *gp) {
  GList *selection;
  push_gp_prog_t *prog1, *prog2;

  /* evaluate all programs */
  push_gp_eval(gp);

  /* select pairs of programs and crossover them */
  while ((selection = gp->selection_func(gp, 2, FALSE)) != NULL) {
    prog1 = (push_gp_prog_t*)g_list_nth_data(selection, 0);
    prog2 = (push_gp_prog_t*)g_list_nth_data(selection, 1);

    /* crossover programs */
    gp->crossover_func(gp, prog1, prog2);

    /* mutate programs */
    gp->mutation_func(gp, prog1);
    gp->mutation_func(gp, prog2);

    /* reset eval flags & fitnesses */
    prog1->eval = FALSE;
    prog2->eval = FALSE;
  }

}


push_gp_prog_t *push_gp_best_program(push_gp_t *gp) {
  push_int_t i;
  push_gp_prog_t *prog, *best = NULL;

  best = push_gp_get_nth(gp, 0);

  for (i = 1; i < gp->pop->len; i++) {
    prog = push_gp_get_nth(gp, i);
    if (prog->eval && (best == NULL || prog->fitness > best->fitness)) {
      best = prog;
    }
  }

  return best;
}


static int push_gp_compare_programs(push_gp_prog_t **_prog1, push_gp_prog_t **_prog2) {
  push_real_t f1, f2;

  f1 = (*_prog1)->fitness;
  f2 = (*_prog1)->fitness;

  if (f1 == f2) {
    return -1;
  }
  else if (f1 < f2) {
    return -1;
  }
  else {
    return 1;
  }
}


static GList *push_gp_selection_roulette_wheel(push_gp_t *gp, push_int_t num, push_bool_t inverse, push_bool_t ranked) {
  GList *buckets = NULL, *candidates = NULL;
  push_gp_prog_t *prog;
  push_int_t i, n;
  push_real_t F, x;
  GList *link;

  g_return_val_if_null(gp, NULL);
  g_return_val_if_fail(num > 0, NULL);

  /* put all evaluated programs into buckets, also sum fitness and count programs */
  for (i = 0; i < gp->pop->len; i++) {
    prog = push_gp_get_nth(gp, i);

    if (prog->eval) {
      buckets = g_list_prepend(buckets, prog);
      F += prog->fitness;
      n++;
    }
  }

  /* return null, if there aren't enough evaluated programs */
  if (n < num) {
    return NULL;
  }

  if (ranked) {
    /* sort buckets by fitness */
    buckets = g_list_sort(buckets, (GCompareFunc)push_gp_compare_programs);

    /* adjust fitnesses */
    F = 0.5 * n * (n + 1); /* gauss */
    for (i = 0; i < gp->pop->len; i++) {
      prog = push_gp_get_nth(gp, i);

      if (prog->eval) {
        prog->fitness = (float)(i + 1);
      }
    }
  }

  /* select programs */
  for (i = 0; i < num; i++) {
    x = F * g_rand_double(gp->rand);

    /* find program in x-bucket */
    prog = NULL;
    for (link = buckets; link != NULL; link = link->next) {
      prog = (push_gp_prog_t*)link->data;
      x -= prog->fitness;
      if (x < 0) {
        break;
      }
    }
    /* link should not be NULL! TODO: do a few tests, but it really should not happen */
    g_assert(link == NULL);

    /* move program from buckets to candidates and decrease fitness sum */
    buckets = g_list_delete_link(buckets, link);
    candidates = g_list_prepend(candidates, prog);
    F -= x;
  }

  return candidates;
}

GList *push_gp_selection_roulette_wheel_linear(push_gp_t *gp, push_int_t num, push_bool_t inverse) {
  return push_gp_selection_roulette_wheel(gp, num, inverse, FALSE);
}

GList *push_gp_selection_roulette_wheel_ranked(push_gp_t *gp, push_int_t num, push_bool_t inverse) {
  return push_gp_selection_roulette_wheel(gp, num, inverse, TRUE);
}


void push_gp_mutation_func(push_gp_t *gp, push_gp_prog_t *prog) {
  // TODO
}


void push_gp_crossover_one_point(push_gp_t *gp, push_gp_prog_t *prog1, push_gp_prog_t *prog2) {
  push_int_t size1, size2, p1, p2;
  push_code_t *code1, *code2;
  push_val_t *val1, *val2;

  g_return_if_fail(push_check_code(prog1->code));
  g_return_if_fail(push_check_code(prog2->code));

  code1 = prog1->code->code;
  code2 = prog2->code->code;

  /* sample random crossover points */
  size1 = push_code_size(code1);
  size2 = push_code_size(code2);
  p1 = g_rand_int_range(gp->rand, 0, size1);
  p2 = g_rand_int_range(gp->rand, 0, size2);

  /* swap values in code */
  val1 = push_code_extract(code1, p1);
  val2 = push_code_extract(code2, p2);
  push_code_replace(prog1->push, code1, p1, val2);
  push_code_replace(prog1->push, code2, p2, val1);

  /* and swap values in garbage collector
   * NOTE: Since val1 is replaced in code1, there is no need of copying or destroying anything
   */
  push_gc_untrack(prog1->push, val1, TRUE);
  push_gc_untrack(prog2->push, val2, TRUE);
  push_gc_track(prog1->push, val2, TRUE);
  push_gc_track(prog1->push, val1, TRUE);
}

