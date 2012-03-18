/* dis.c - Default instruction set
 * NOTE: see http://hampshire.edu/lspector/push3-description.html for further
 * information.
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

#include <stddef.h>
#include <math.h>

#include <glib.h>

#include "push.h"


/* MOD in the LISP sense */
#define MOD(n, M)              (((n) % (M)) + (M)) % (M)

/* check if stack has enough items */
#define CH(stack, num)         (push_stack_length(stack) >= num)

/* define which stack to use for a POLY instruction */
#define STACK(stack)           offsetof(push_t, stack)
#define GETSTACK(push, offset) G_STRUCT_MEMBER(push_stack_t*, push, offset)


/* Struct for default instruction set
 * NOTE: last element has name = NULL
 */
struct push_dis_S {
  const char *name;
  void *func;
  /* actually offset of stack pointer in push structure */
  long stack;
};



/* POLY
 * NOTE: polymorph instructions (e.g. DUP). stack is passed as userdata
 */
static void push_instr_poly_equal(push_t *push, push_stack_t *stack) {
  push_val_t *val1, *val2;

  if (CH(stack, 2)) {
    val1 = push_stack_pop(stack);
    val2 = push_stack_pop(stack);
    push_stack_push_new(push, push->boolean, PUSH_TYPE_BOOL, push_val_equal(val1, val2));
  }
}

static void push_instr_poly_define(push_t *push, push_stack_t *stack) {
  push_val_t *val1, *val2;

  if (CH(stack, 1) && CH(push->name, 1)) {
    val1 = push_stack_pop(stack);
    val2 = push_stack_pop(push->name);
    push_define(push, val2->name, val1);
  }
}

static void push_instr_poly_dup(push_t *push, push_stack_t *stack) {
  push_val_t *val1;

  val1 = push_stack_peek(stack);
  if (val1 != NULL) {
    //push_stack_push_dup(push, stack, val1);
    push_stack_push(stack, val1);
  }
}

static void push_instr_poly_flush(push_t *push, push_stack_t *stack) {
  push_stack_flush(stack);
}

static void push_instr_poly_pop(push_t *push, push_stack_t *stack) {
  push_stack_pop(stack);
}

static void push_instr_poly_rot(push_t *push, push_stack_t *stack) {
  push_val_t *val1;

  if (CH(stack, 3)) {
    val1 = push_stack_pop_nth(stack, 2);
    push_stack_push(stack, val1);
  }
}

static void push_instr_poly_shove(push_t *push, push_stack_t *stack) {
  push_val_t *val1, *val2;

  if (CH(stack, 2) && CH(push->integer, 1)) {
    val1 = push_stack_pop(stack);
    val2 = push_stack_pop(push->integer);

    push_stack_push_nth(stack, push_stack_length(stack), val1);
  }
}

static void push_instr_poly_stackdepth(push_t *push, push_stack_t *stack) {
  push_stack_push_new(push, push->integer, PUSH_TYPE_INT, push_stack_length(stack));
}

static void push_instr_poly_swap(push_t *push, push_stack_t *stack) {
  push_val_t *val1;

  if (CH(stack, 2)) {
    val1 = push_stack_pop(stack);
    push_stack_push_nth(stack, 1, val1);
  }
}

static void push_instr_poly_yank(push_t *push, push_stack_t *stack) {
  push_val_t *val1, *val2;

  if (CH(push->integer, 1)) {
    val1 = push_stack_pop(push->integer);
    val2 = push_stack_pop_nth(stack, val1->integer);

    if (val2 != NULL) {
      push_stack_push(stack, val2);
    }
    else {
      /* failed: push back val1 */
      push_stack_push(push->integer, val1);
    }
  }
}

static void push_instr_poly_yankdup(push_t *push, push_stack_t *stack) {
  push_val_t *val1, *val2;

  if (CH(push->integer, 1)) {
    val1 = push_stack_pop(push->integer);
    val2 = push_stack_peek_nth(stack, val1->integer);
    if (val2 != NULL) {
      push_stack_push(stack, val2);
    }
    else {
      /* failed: push back val1 */
      push_stack_push(push->integer, val1);
    }
  }
}



/* BOOL */
static void push_instr_bool_and(push_t *push, void *userdata) {
  push_val_t *val1, *val2;

  if (CH(push->boolean, 2)) {
    val1 = push_stack_pop(push->boolean);
    val2 = push_stack_pop(push->boolean);
    push_stack_push_new(push, push->boolean, PUSH_TYPE_BOOL, val1->boolean && val2->boolean);
  }
}

static void push_instr_bool_fromint(push_t *push, void *userdata) {
  push_val_t *val1;

  if (CH(push->integer, 1)) {
    val1 = push_stack_pop(push->integer);
    push_stack_push_new(push, push->boolean, PUSH_TYPE_BOOL, val1->integer != 0);
  }
}

static void push_instr_bool_fromreal(push_t *push, void *userdata) {
  push_val_t *val1;

  if (CH(push->real, 1)) {
    val1 = push_stack_pop(push->real);
    push_stack_push_new(push, push->boolean, PUSH_TYPE_BOOL, val1->real != 0.0);
  }
}

static void push_instr_bool_not(push_t *push, void *userdata) {
  push_val_t *val1;

  val1 = push_stack_pop(push->boolean);
  if (val1 != NULL) {
    push_stack_push_new(push, push->boolean, PUSH_TYPE_BOOL, !val1->boolean);
  }
}

static void push_instr_bool_or(push_t *push, void *userdata) {
  push_val_t *val1, *val2;

  if (CH(push->boolean, 2)) {
    val1 = push_stack_pop(push->boolean);
    val2 = push_stack_pop(push->boolean);
    push_stack_push_new(push, push->boolean, PUSH_TYPE_BOOL, val1->boolean || val2->boolean);
  }
}

static void push_instr_bool_rand(push_t *push, void *userdata) {
  push_stack_push(push->boolean, push_rand_val(push, PUSH_TYPE_BOOL, NULL, PUSH_FALSE));
}


/* CODE */

static void push_instr_code_append(push_t *push, void *userdata) {
  push_val_t *val1, *val2;

  if (CH(push->code, 2)) {
    val1 = push_stack_pop_code(push);
    val2 = push_stack_pop_code(push);
    push_stack_push_new(push, push->code, PUSH_TYPE_CODE, push_code_concat(val1->code, val2->code));
  }
}

static void push_instr_code_atom(push_t *push, void *userdata) {
  push_val_t *val1;

  if (CH(push->code, 2)) {
    val1 = push_stack_pop(push->code);
    push_stack_push_new(push, push->boolean, PUSH_TYPE_BOOL, push_check_code(val1));
  }
}

static void push_instr_code_car(push_t *push, void *userdata) {
  push_val_t *val1, *val2;

  val1 = push_stack_pop(push->code);

  if (val1 != NULL) {
    if (push_check_code(val1)) {
      val2 = push_code_peek(val1->code);
    }
    else {
      val2 = val1;
    }

    if (val2 != NULL) {
      push_stack_push(push->code, val2);
    }
  }
}

static void push_instr_code_cdr(push_t *push, void *userdata) {
  push_val_t *val1, *val2;

  val1 = push_stack_pop(push->code);

  if (val1 != NULL) {
    if (push_check_code(val1)) {
      val2 = push_val_code_dup(push, val1);
      push_code_pop(val2->code);
      push_stack_push(push->code, val2);
    }
    else {
      push_stack_push_new(push, push->code, PUSH_TYPE_CODE, NULL);
    }
  }
}

static void push_instr_code_cons(push_t *push, void *userdata) {
  push_val_t *val1, *val2, *val3;

  if (CH(push->code, 2)) {
    val1 = push_stack_pop_code(push);
    val2 = push_stack_pop(push->code);

    val3 = push_val_code_dup(push, val1);
    push_code_prepend(val3->code, val2);

    push_stack_push(push->code, val3);
  }
}

static void push_instr_code_container(push_t *push, void *userdata) {
  push_val_t *val1, *val2;

  if (CH(push->code, 2)) {
    val1 = push_stack_pop_code(push);;
    val2 = push_stack_pop(push->code);

    push_stack_push_new(push, push->code, PUSH_TYPE_CODE, push_code_container(val1->code, val2));
  }
}

static void push_instr_code_contains(push_t *push, void *userdata) {
  push_val_t *val1, *val2;

  if (CH(push->code, 2)) {
    val1 = push_stack_pop_code(push);
    val2 = push_stack_pop(push->code);

    push_stack_push_new(push, push->boolean, PUSH_TYPE_BOOL, push_code_container(val1->code, val2) != NULL);
  }
}

static void push_instr_code_definition(push_t *push, void *userdata) {
  push_val_t *val1, *val2;

  if (CH(push->name, 1)) {
    val1 = push_stack_pop(push->name);
    val2 = push_lookup(push, val1->name);
    if (val2 != NULL) {
      push_stack_push(push->code, val2);
    }
  }
}

static void push_instr_code_discrepancy(push_t *push, void *userdata) {
  push_val_t *val1, *val2;

  if (CH(push->code, 2)) {
    val1 = push_stack_pop_code(push);
    val2 = push_stack_pop_code(push);

    push_stack_push_new(push, push->integer, PUSH_TYPE_INT, push_code_discrepancy(val1->code, val2->code));
  }
}

static void push_instr_code_do(push_t *push, void *userdata) {
  push_val_t *val1;

  if (CH(push->code, 1)) {
    val1 = push_stack_peek_code(push);

    /* first push an instruction that pops the top-most code from code stack */
    push_stack_push_new(push, push->exec, PUSH_TYPE_INSTR, push_instr_lookup(push, "CODE.POP"));

    /* then push the code itself onto EXEC stack */
    push_stack_push(push->exec, val1);
  }
}

static void push_instr_code_do_(push_t *push, void *userdata) {
  push_val_t *val1;

  if (CH(push->code, 1)) {
    val1 = push_stack_pop_code(push);
    push_stack_push(push->exec, val1);
  }
}

static void push_instr_code_do_count(push_t *push, void *userdata) {
  push_val_t *val1, *val2;
  push_code_t *code;

  if (CH(push->code, 1) && CH(push->integer, 1)) {
    val1 = push_stack_pop(push->code);
    val2 = push_stack_pop(push->integer);

    if (val2->integer > 0) {
      /* construct ( 0 <1 - IntegerArg> CODE.QUOTE <CodeArg> CODE.DO*RANGE ) */
      code = push_code_new();
      push_code_append(code, push_val_new(push, PUSH_TYPE_INT, 0));
      val2->integer = 1 - val2->integer;
      push_code_append(code, val2);
      push_code_append(code, push_val_new(push, PUSH_TYPE_INSTR, push_instr_lookup(push, "CODE.QUOTE")));
      push_code_append(code, val1);
      push_code_append(code, push_val_new(push, PUSH_TYPE_INSTR, push_instr_lookup(push, "CODE.DO*RANGE")));

      /* push generated code */
      push_stack_push_new(push, push->exec, PUSH_TYPE_CODE, code);
    }
  }
}

static void push_instr_code_do_range(push_t *push, void *userdata) {
  push_val_t *val1, *val2, *val3;
  push_code_t *code;
  int step;

  if (CH(push->code, 1) && CH(push->integer, 2)) {
    val1 = push_stack_pop(push->code);    /* loop body */
    val2 = push_stack_pop(push->integer); /* destination index */
    val3 = push_stack_pop(push->integer); /* current index */

    /* push current index */
    push_stack_push(push->integer, val3);

    if (val2->integer != val3->integer) {
      /* push recursive call to DO*RANGE: (<DestIndex> <CurrentIndex+-1> CODE.QUOTE <CodeArg> CODE.DO*RANGE ) */
      code = push_code_new();
      push_code_append(code, val2);
      step = val2->integer > val3->integer ? 1 : -1;
      push_code_append(code, push_val_new(push, PUSH_TYPE_INT, val3->integer + step));
      push_code_append(code, push_val_new(push, PUSH_TYPE_INSTR, push_instr_lookup(push, "CODE.QUOTE")));
      push_code_append(code, val1);
      push_code_append(code, push_val_new(push, PUSH_TYPE_INSTR, push_instr_lookup(push, "CODE.DO*RANGE")));
      push_stack_push_new(push, push->exec, PUSH_TYPE_CODE, code);
    }

    /* push loop body */
    push_stack_push(push->exec, val1);
  }
}

static void push_instr_code_do_times(push_t *push, void *userdata) {
  push_val_t *val1, *val2, *val3;
  push_code_t *code;

  if (CH(push->code, 1) && CH(push->integer, 1)) {
    val1 = push_stack_pop_code(push);
    val2 = push_stack_pop(push->integer);

    if (val2->integer > 0) {
      /* construct ( 0 <1 - IntegerArg> CODE.QUOTE INT.POP::<CodeArg> CODE.DO*RANGE ) */
      code = push_code_new();
      push_code_append(code, push_val_new(push, PUSH_TYPE_INT, 0));
      push_code_append(code, push_val_new(push, PUSH_TYPE_INT, val2->integer - 1));
      push_code_append(code, push_val_new(push, PUSH_TYPE_INSTR, push_instr_lookup(push, "CODE.QUOTE")));

      val3 = push_val_new(push, PUSH_TYPE_CODE, push_code_dup(val1->code));
      push_code_prepend(val3->code, push_val_new(push, PUSH_TYPE_INSTR, push_instr_lookup(push, "INT.POP")));
      push_code_append(code, val3);
      push_code_append(code, push_val_new(push, PUSH_TYPE_INSTR, push_instr_lookup(push, "CODE.DO*RANGE")));

      /* push generated code */
      push_stack_push_new(push, push->exec, PUSH_TYPE_CODE, code);
    }
  }
}

static void push_instr_code_extract(push_t *push, void *userdata) {
  push_val_t *val1, *val2;
  push_int_t p;

  if (CH(push->code, 1) && CH(push->integer, 1)) {
    val1 = push_stack_pop(push->code);
    val2 = push_stack_pop(push->integer);

    if (push_check_code(val1) && push_code_size(val1->code) > 0) {
      p = MOD(val2->integer, push_code_size(val1->code));
    }
    else {
      p = 0;
    }

    if (p == 0) {
      push_stack_push(push->code, val1);
    }
    else {
      push_stack_push(push->code, push_code_extract(val1->code, p - 1));
    }
  }
}

static void push_instr_code_frombool(push_t *push, void *userdata) {
  push_val_t *val1;

  if (CH(push->boolean, 1)) {
    val1 = push_stack_pop(push->boolean);
    push_stack_push(push->code, val1);
  }
}

static void push_instr_code_fromint(push_t *push, void *userdata) {
  push_val_t *val1;

  if (CH(push->integer, 1)) {
    val1 = push_stack_pop(push->integer);
    push_stack_push(push->code, val1);
  }
}

static void push_instr_code_fromname(push_t *push, void *userdata) {
  push_val_t *val1;

  if (CH(push->name, 1)) {
    val1 = push_stack_pop(push->name);
    push_stack_push(push->code, val1);
  }
}

static void push_instr_code_fromreal(push_t *push, void *userdata) {
  push_val_t *val1;

  if (CH(push->real, 1)) {
    val1 = push_stack_pop(push->real);
    push_stack_push(push->code, val1);
  }
}

static void push_instr_code_if(push_t *push, void *userdata) {
  push_val_t *val1, *val2, *val3;

  if (CH(push->code, 2) && CH(push->boolean, 1)) {
    val1 = push_stack_pop(push->code);
    val2 = push_stack_pop(push->code);
    val3 = push_stack_pop(push->boolean);

    push_stack_push(push->exec, val3->boolean ? val2 : val1);
  }
}

static void push_instr_code_insert(push_t *push, void *userdata) {
  push_val_t *val1, *val2, *val3;
  push_code_t *code;

  if (CH(push->code, 2) && CH(push->integer, 1)) {
    /* insert val2 into val1 at pos val3 */
    val1 = push_stack_pop_code(push);
    val2 = push_stack_pop(push->code);
    val3 = push_stack_pop(push->integer);

    push_stack_push(push->code, push_code_replace(push, val1->code, MOD(val3->integer, push_code_size(val1->code) + 1), val2));
  }
}

static void push_instr_code_instructions(push_t *push, void *userdata) {
  push_code_t *list;
  GHashTableIter iter;
  push_instr_t *instr;

  list = push_code_new();
  g_hash_table_iter_init(&iter, push->instructions);

  while (g_hash_table_iter_next(&iter, NULL, (void*)&instr)) {
    push_code_append(list, push_val_new(push, PUSH_TYPE_INSTR, instr));
  }

  push_stack_push_new(push, push->code, PUSH_TYPE_CODE, list);
}

static void push_instr_code_length(push_t *push, void *userdata) {
  push_val_t *val1;

  if (CH(push->code, 1)) {
    val1 = push_stack_pop_code(push);

    push_stack_push_new(push, push->integer, PUSH_TYPE_INT, val1->code->length);
  }
}

static void push_instr_code_list(push_t *push, void *userdata) {
  push_val_t *val1, *val2;
  push_code_t *list;

  if (CH(push->code, 2)) {
    val1 = push_stack_pop(push->code);
    val2 = push_stack_pop(push->code);

    list = push_code_new();
    push_code_append(list, val1);
    push_code_append(list, val2);

    push_stack_push_new(push, push->code, PUSH_TYPE_CODE, list);
  }
}

static void push_instr_code_member(push_t *push, void *userdata) {
  push_val_t *val1, *val2;

  if (CH(push->code, 2)) {
    val1 = push_stack_pop_code(push);
    val2 = push_stack_pop(push->code);

    push_stack_push_new(push, push->boolean, PUSH_TYPE_BOOL, g_queue_find(val1->code, val2) != NULL);
  }
}

static void push_instr_code_noop(push_t *push, void *userdata) {
}

static void push_instr_code_nth(push_t *push, void *userdata) {
  push_val_t *val1, *val2;

  if (CH(push->code, 1) && CH(push->integer, 1)) {
    val1 = push_stack_pop(push->integer);
    val2 = push_stack_pop_code(push);

    if (val2->code->length > 0) {
      push_stack_push(push->code, push_code_pop_nth(val2->code, MOD(val1->integer, val2->code->length)));
    }
  }
}

static void push_instr_code_nthcdr(push_t *push, void *userdata) {
  push_val_t *val1, *val2;
  GList *link;
  int n;

  if (CH(push->code, 1) && CH(push->integer, 1)) {
    val1 = push_stack_pop(push->integer);
    val2 = push_stack_pop_code(push);

    if (push_code_length(val2->code) > 0) {
      n = MOD(val1->integer, push_code_length(val2->code));

      if (n > 0) {
        link = g_queue_peek_nth_link(val2->code, n);
        push_stack_push_new(push, push->code, PUSH_TYPE_CODE, push_code_dup_ext(val2->code, link, NULL, NULL, NULL));
      }
    }
    else {
      push_stack_push(push->code, val2);
    }
  }
}

static void push_instr_code_null(push_t *push, void *userdata) {
  push_val_t *val1;

  val1 = push_stack_pop(push->code);

  if (val1 != NULL) {
    push_stack_push_new(push, push->boolean, PUSH_TYPE_BOOL, push_check_code(val1) && val1->code->length == 0);
  }
}

static void push_instr_code_position(push_t *push, void *userdata) {
  push_val_t *val1, *val2;

  if (CH(push->code, 2)) {
    val1 = push_stack_pop_code(push);
    val2 = push_stack_pop(push->code);

    push_stack_push_new(push, push->boolean, PUSH_TYPE_BOOL, push_code_index(val1->code, val2));
  }
}

static void push_instr_code_quote(push_t *push, void *userdata) {
  push_val_t *val1;

  val1 = push_stack_pop(push->exec);
  if (val1 != NULL) {
    push_stack_push(push->code, val1);
  }
}

static void push_instr_code_rand(push_t *push, void *userdata) {
  push_val_t *val1, *val2;
  push_int_t size;

  val1 = push_stack_pop(push->integer);
  if (val1 != NULL) {
    size = val1->integer;
    val2 = push_config_get(push, "MAX-POINTS-IN-RANDOM-EXPRESSIONS");
    if (val2 != NULL && push_check_int(val2) && val2->integer < size) {
      size = val2->integer;
    }

    push_stack_push(push->code, push_rand_val(push, size == 1 ? PUSH_TYPE_NONE: PUSH_TYPE_CODE, &size, PUSH_TRUE));
  }
}

static void push_instr_code_size(push_t *push, void *userdata) {
  push_val_t *val1;

  if (CH(push->code, 1)) {
    val1 = push_stack_pop(push->code);

    push_stack_push_new(push, push->integer, PUSH_TYPE_INT, push_check_code(val1) ? push_code_size(val1->code) + 1: 1);
  }
}

static void push_instr_code_subst(push_t *push, void *userdata) {
  // TODO
}

/* EXEC */

static void push_instr_exec_do_count(push_t *push, void *userdata) {
  push_val_t *val1, *val2;
  push_code_t *code;

  if (CH(push->exec, 1) && CH(push->integer, 1)) {
    val1 = push_stack_pop(push->exec);
    val2 = push_stack_pop(push->integer);

    if (val2->integer > 0) {
      /* construct ( 0 <1 - IntegerArg> EXEC.DO*RANGE <ExecArg> ) */
      code = push_code_new();
      push_code_append(code, push_val_new(push, PUSH_TYPE_INT, 0));
      val2->integer = 1 - val2->integer;
      push_code_append(code, val2);
      push_code_append(code, push_val_new(push, PUSH_TYPE_INSTR, push_instr_lookup(push, "EXEC.DO*RANGE")));
      push_code_append(code, val1);

      /* push generated code */
      push_stack_push_new(push, push->exec, PUSH_TYPE_CODE, code);
    }
  }
}

static void push_instr_exec_do_range(push_t *push, void *userdata) {
  push_val_t *val1, *val2, *val3;
  push_code_t *code;
  int step;

  if (CH(push->exec, 1) && CH(push->integer, 2)) {
    val1 = push_stack_pop(push->exec);    /* loop body */
    val2 = push_stack_pop(push->integer); /* destination index */
    val3 = push_stack_pop(push->integer); /* current index */

    /* push current index */
    push_stack_push(push->integer, val3);

    if (val2->integer != val3->integer) {
      /* push recursive call to EXEC.DO*RANGE: (<DestIndex> <CurrentIndex+-1> EXEC.DO*RANGE <EXECArg> ) */
      code = push_code_new();
      push_code_append(code, val2);
      step = val2->integer > val3->integer ? 1 : -1;
      push_code_append(code, push_val_new(push, PUSH_TYPE_INT, val3->integer + step));
      push_code_append(code, push_val_new(push, PUSH_TYPE_INSTR, push_instr_lookup(push, "EXEC.DO*RANGE")));
      push_code_append(code, val1);
      push_stack_push_new(push, push->exec, PUSH_TYPE_CODE, code);
    }

    /* push loop body */
    push_stack_push(push->exec, val1);
  }
}

static void push_instr_exec_do_times(push_t *push, void *userdata) {
  push_val_t *val1, *val2, *val3;
  push_code_t *code;

  if (CH(push->exec, 1) && CH(push->integer, 1)) {
    val1 = push_val_make_code(push, push_stack_pop(push->exec));
    val2 = push_stack_pop(push->integer);

    if (val2->integer > 0) {
      /* construct ( 0 <1 - IntegerArg> EXEC.DO*RANGE INT.POP::<CodeArg> ) */
      code = push_code_new();
      push_code_append(code, push_val_new(push, PUSH_TYPE_INT, 0));
      push_code_append(code, push_val_new(push, PUSH_TYPE_INT, val2->integer - 1));
      push_code_append(code, push_val_new(push, PUSH_TYPE_INSTR, push_instr_lookup(push, "EXEC.DO*RANGE")));
      val3 = push_val_new(push, PUSH_TYPE_CODE, push_code_dup(val1->code));
      push_code_prepend(val3->code, push_val_new(push, PUSH_TYPE_INSTR, push_instr_lookup(push, "INT.POP")));
      push_code_append(code, val3);

      /* push generated code */
      push_stack_push_new(push, push->exec, PUSH_TYPE_CODE, code);
    }
  }
}

static void push_instr_exec_if(push_t *push, void *userdata) {
  push_val_t *val1;

  if (CH(push->exec, 2) && CH(push->boolean, 1)) {
    val1 = push_stack_pop(push->boolean);

    push_stack_pop_nth(push->exec, val1->boolean ? 1 : 0);
  }
}

static void push_instr_exec_k(push_t *push, void *userdata) {
  push_stack_pop_nth(push->exec, 1);
}

static void push_instr_exec_s(push_t *push, void *userdata) {
  push_val_t *val1, *val2, *val3;
  push_code_t *code;

  if (CH(push->exec, 3)) {
    val1 = push_stack_pop(push->exec);
    val2 = push_stack_pop(push->exec);
    val3 = push_stack_pop(push->exec);

    code = push_code_new();
    push_code_append(code, val2);
    push_code_append(code, val3);
    push_stack_push_new(push, push->exec, PUSH_TYPE_CODE, code);
    push_stack_push(push->exec, val3);
    push_stack_push(push->exec, val1);
  }
}

static void push_instr_exec_y(push_t *push, void *userdata) {
  push_val_t *val1;
  push_code_t *code;

  if (CH(push->exec, 1)) {
    val1 = push_stack_peek(push->exec);

    code = push_code_new();
    push_code_append(code, push_val_new(push, PUSH_TYPE_INSTR, push_instr_lookup(push, "EXEC.Y")));
    push_code_append(code, val1);

    push_stack_push_nth(push->exec, 1, push_val_new(push, PUSH_TYPE_CODE, code));
  }
}

/* INT */

static void push_instr_int_mod(push_t *push, void *userdata) {
  push_val_t *val1, *val2;

  if (CH(push->integer, 2)) {
    val1 = push_stack_pop(push->integer);
    val2 = push_stack_pop(push->integer);

    if (val1->integer != 0) {
      push_stack_push_new(push, push->integer, PUSH_TYPE_INT, MOD(val2->integer, val1->integer));
    }
  }
}

static void push_instr_int_mul(push_t *push, void *userdata) {
  push_val_t *val1, *val2;

  if (CH(push->integer, 2)) {
    val1 = push_stack_pop(push->integer);
    val2 = push_stack_pop(push->integer);

    push_stack_push_new(push, push->integer, PUSH_TYPE_INT, val2->integer * val1->integer);
  }
}

static void push_instr_int_add(push_t *push, void *userdata) {
  push_val_t *val1, *val2;

  if (CH(push->integer, 2)) {
    val1 = push_stack_pop(push->integer);
    val2 = push_stack_pop(push->integer);

    push_stack_push_new(push, push->integer, PUSH_TYPE_INT, val2->integer + val1->integer);
  }
}

static void push_instr_int_sub(push_t *push, void *userdata) {
  push_val_t *val1, *val2;

  if (CH(push->integer, 2)) {
    val1 = push_stack_pop(push->integer);
    val2 = push_stack_pop(push->integer);

    push_stack_push_new(push, push->integer, PUSH_TYPE_INT, val2->integer - val1->integer);
  }
}

static void push_instr_int_div(push_t *push, void *userdata) {
  push_val_t *val1, *val2;

  if (CH(push->integer, 2)) {
    val1 = push_stack_pop(push->integer);
    val2 = push_stack_pop(push->integer);

    if (val1->integer != 0) {
      push_stack_push_new(push, push->integer, PUSH_TYPE_INT, val2->integer / val1->integer);
    }
  }
}

static void push_instr_int_less(push_t *push, void *userdata) {
  push_val_t *val1, *val2;

  if (CH(push->integer, 2)) {
    val1 = push_stack_pop(push->integer);
    val2 = push_stack_pop(push->integer);

    push_stack_push_new(push, push->boolean, PUSH_TYPE_BOOL, val2->integer < val1->integer);
  }
}

static void push_instr_int_greater(push_t *push, void *userdata) {
  push_val_t *val1, *val2;

  if (CH(push->integer, 2)) {
    val1 = push_stack_pop(push->integer);
    val2 = push_stack_pop(push->integer);

    push_stack_push_new(push, push->boolean, PUSH_TYPE_BOOL, val2->integer > val1->integer);
  }
}

static void push_instr_int_frombool(push_t *push, void *userdata) {
  push_val_t *val1;

  val1 = push_stack_pop(push->boolean);

  if (val1 != NULL) {
    push_stack_push_new(push, push->integer, PUSH_TYPE_INT, val1->boolean ? 1 : 0);
  }
}

static void push_instr_int_fromreal(push_t *push, void *userdata) {
  push_val_t *val1;

  val1 = push_stack_pop(push->real);

  if (val1 != NULL) {
    push_stack_push_new(push, push->integer, PUSH_TYPE_INT, (push_int_t)val1->real);
  }
}

static void push_instr_int_max(push_t *push, void *userdata) {
  push_val_t *val1, *val2;

  if (CH(push->integer, 2)) {
    val1 = push_stack_pop(push->integer);
    val2 = push_stack_pop(push->integer);

    push_stack_push(push->integer, push_val_max(val1, val2, integer));
  }
}

static void push_instr_int_min(push_t *push, void *userdata) {
  push_val_t *val1, *val2;

  if (CH(push->integer, 2)) {
    val1 = push_stack_pop(push->integer);
    val2 = push_stack_pop(push->integer);

    push_stack_push(push->integer, push_val_min(val1, val2, integer));
  }
}

static void push_instr_int_rand(push_t *push, void *userdata) {
  push_stack_push(push->integer, push_rand_val(push, PUSH_TYPE_INT, NULL, PUSH_FALSE));
}

/* NAME */

static void push_instr_name_quote(push_t *push, void *userdata) {
  push_val_t *val1;

  if (CH(push->exec, 1)) {
    val1 = push_stack_pop(push->exec);
    push_stack_push(push_check_name(val1) ? push->name : push->exec, val1);
  }
}

static void push_instr_name_rand(push_t *push, void *userdata) {
  push_stack_push_new(push, push->name, PUSH_TYPE_NAME, push_rand_name(push));
}

static void push_instr_name_randboundname(push_t *push, void *userdata) {
  push_name_t name;

  name = push_rand_bound_name(push);
  if (name != NULL) {
    push_stack_push_new(push, push->name, PUSH_TYPE_NAME, name);
  }
}

/* REAL */

static void push_instr_real_mod(push_t *push, void *userdata) {
  push_val_t *val1, *val2;

  if (CH(push->real, 2)) {
    val1 = push_stack_pop(push->real);
    val2 = push_stack_pop(push->real);

    if (val2->real != 0.0) {
      push_stack_push_new(push, push->real, PUSH_TYPE_REAL, fmod(val2->real, val1->real));
    }
  }
}

static void push_instr_real_mul(push_t *push, void *userdata) {
  push_val_t *val1, *val2;

  if (CH(push->real, 2)) {
    val1 = push_stack_pop(push->real);
    val2 = push_stack_pop(push->real);

    push_stack_push_new(push, push->real, PUSH_TYPE_REAL, val2->real * val1->real);
  }
}

static void push_instr_real_add(push_t *push, void *userdata) {
  push_val_t *val1, *val2;

  if (CH(push->real, 2)) {
    val1 = push_stack_pop(push->real);
    val2 = push_stack_pop(push->real);

    push_stack_push_new(push, push->real, PUSH_TYPE_REAL, val2->real + val1->real);
  }
}

static void push_instr_real_sub(push_t *push, void *userdata) {
  push_val_t *val1, *val2;

  if (CH(push->real, 2)) {
    val1 = push_stack_pop(push->real);
    val2 = push_stack_pop(push->real);

    push_stack_push_new(push, push->real, PUSH_TYPE_REAL, val2->real - val1->real);
  }
}

static void push_instr_real_div(push_t *push, void *userdata) {
  push_val_t *val1, *val2;

  if (CH(push->real, 2)) {
    val1 = push_stack_pop(push->real);
    val2 = push_stack_pop(push->real);

    if (val2->real != 0.0) {
      push_stack_push_new(push, push->real, PUSH_TYPE_REAL, val2->real / val1->real);
    }
  }
}

static void push_instr_real_less(push_t *push, void *userdata) {
  push_val_t *val1, *val2;

  if (CH(push->real, 2)) {
    val1 = push_stack_pop(push->real);
    val2 = push_stack_pop(push->real);

    push_stack_push_new(push, push->boolean, PUSH_TYPE_BOOL, val2->real < val1->real);
  }
}

static void push_instr_real_greater(push_t *push, void *userdata) {
  push_val_t *val1, *val2;

  if (CH(push->real, 2)) {
    val1 = push_stack_pop(push->real);
    val2 = push_stack_pop(push->real);

    push_stack_push_new(push, push->boolean, PUSH_TYPE_BOOL, val2->real > val1->real);
  }
}

static void push_instr_real_cos(push_t *push, void *userdata) {
  push_val_t *val1;

  val1 = push_stack_pop(push->real);

  if (val1 != NULL) {
    push_stack_push_new(push, push->real, PUSH_TYPE_REAL, cos(val1->real));
  }
}

static void push_instr_real_exp(push_t *push, void *userdata) {
  push_val_t *val1;

  val1 = push_stack_pop(push->real);

  if (val1 != NULL) {
    push_stack_push_new(push, push->real, PUSH_TYPE_REAL, exp(val1->real));
  }
}

static void push_instr_real_frombool(push_t *push, void *userdata) {
  push_val_t *val1;

  val1 = push_stack_pop(push->boolean);

  if (val1 != NULL) {
    push_stack_push_new(push, push->real, PUSH_TYPE_REAL, val1->boolean ? 1.0 : 0.0);
  }
}

static void push_instr_real_fromint(push_t *push, void *userdata) {
  push_val_t *val1;

  val1 = push_stack_pop(push->integer);

  if (val1 != NULL) {
    push_stack_push_new(push, push->real, PUSH_TYPE_REAL, (push_real_t)val1->integer);
  }
}

static void push_instr_real_log(push_t *push, void *userdata) {
  push_val_t *val1;

  val1 = push_stack_pop(push->real);

  if (val1 != NULL) {
    push_stack_push_new(push, push->real, PUSH_TYPE_REAL, log(val1->real));
  }
}

static void push_instr_real_max(push_t *push, void *userdata) {
  push_val_t *val1, *val2;

  if (CH(push->real, 2)) {
    val1 = push_stack_pop(push->real);
    val2 = push_stack_pop(push->real);

    push_stack_push(push->real, push_val_max(val1, val2, real));
  }
}

static void push_instr_real_min(push_t *push, void *userdata) {
  push_val_t *val1, *val2;

  if (CH(push->real, 2)) {
    val1 = push_stack_pop(push->real);
    val2 = push_stack_pop(push->real);

    push_stack_push(push->real, push_val_min(val1, val2, real));
  }
}

static void push_instr_real_rand(push_t *push, void *userdata) {
  push_stack_push(push->real, push_rand_val(push, PUSH_TYPE_REAL, NULL, PUSH_FALSE));
}

static void push_instr_real_sin(push_t *push, void *userdata) {
  push_val_t *val1;

  val1 = push_stack_pop(push->real);

  if (val1 != NULL) {
    push_stack_push_new(push, push->real, PUSH_TYPE_REAL, sin(val1->real));
  }
}

static void push_instr_real_tan(push_t *push, void *userdata) {
  push_val_t *val1;

  val1 = push_stack_pop(push->real);

  if (val1 != NULL) {
    push_stack_push_new(push, push->real, PUSH_TYPE_REAL, tan(val1->real));
  }
}


/* Builtin default instruction set 
 */
struct push_dis_S push_dis[] = {
  /* BOOL */
  { "BOOL.=",             push_instr_poly_equal        , STACK(boolean)      },
  { "BOOL.AND",           push_instr_bool_and                                },
  { "BOOL.DEFINE",        push_instr_poly_define       , STACK(boolean)      },
  { "BOOL.DUP",           push_instr_poly_dup          , STACK(boolean)      },
  { "BOOL.FLUSH",         push_instr_poly_flush        , STACK(boolean)      },
  { "BOOL.FROMINT",       push_instr_bool_fromint                            },
  { "BOOL.FROMREAL",      push_instr_bool_fromreal                           },
  { "BOOL.NOT",           push_instr_bool_not                                },
  { "BOOL.OR",            push_instr_bool_or                                 },
  { "BOOL.POP",           push_instr_poly_pop          , STACK(boolean)      },
  { "BOOL.RAND",          push_instr_bool_rand                               },
  { "BOOL.ROT",           push_instr_poly_rot          , STACK(boolean)      },
  { "BOOL.SHOVE",         push_instr_poly_shove        , STACK(boolean)      },
  { "BOOL.STACKDEPTH",    push_instr_poly_stackdepth   , STACK(boolean)      },
  { "BOOL.SWAP",          push_instr_poly_swap         , STACK(boolean)      },
  { "BOOL.YANK",          push_instr_poly_yank         , STACK(boolean)      },
  { "BOOL.YANKDUP",       push_instr_poly_yankdup      , STACK(boolean)      },

  /* CODE */
  { "CODE.=",             push_instr_poly_equal        , STACK(code)         },
  { "CODE.APPEND",        push_instr_code_append                             },
  { "CODE.ATOM",          push_instr_code_atom                               },
  { "CODE.CAR",           push_instr_code_car                                },
  { "CODE.CDR",           push_instr_code_cdr                                },
  { "CODE.CONS",          push_instr_code_cons                               },
  { "CODE.CONTAINER",     push_instr_code_container                          },
  { "CODE.CONTAINS",      push_instr_code_contains                           },
  { "CODE.DEFINE",        push_instr_poly_define       , STACK(code)         },
  { "CODE.DEFINITION",    push_instr_code_definition                         },
  { "CODE.DISCREPANCY",   push_instr_code_discrepancy                        },
  { "CODE.DO",            push_instr_code_do                                 },
  { "CODE.DO*",           push_instr_code_do_                                },
  { "CODE.DO*COUNT",      push_instr_code_do_count                           },
  { "CODE.DO*RANGE",      push_instr_code_do_range                           },
  { "CODE.DO*TIMES",      push_instr_code_do_times                           },
  { "CODE.DUP",           push_instr_poly_dup          , STACK(code)         },
  { "CODE.EXTRACT",       push_instr_code_extract                            },
  { "CODE.FLUSH",         push_instr_poly_flush        , STACK(code)         },
  { "CODE.FROMBOOL",      push_instr_code_frombool                           },
  { "CODE.FROMREAL",      push_instr_code_fromreal                           },
  { "CODE.FROMINT",       push_instr_code_fromint                            },
  { "CODE.FROMNAME",      push_instr_code_fromname                           },
  { "CODE.IF",            push_instr_code_if                                 },
  { "CODE.INSERT",        push_instr_code_insert                             },
  { "CODE.INSTRUCTIONS",  push_instr_code_instructions                       },
  { "CODE.LENGTH",        push_instr_code_length                             },
  { "CODE.LIST",          push_instr_code_list                               },
  { "CODE.MEMBER",        push_instr_code_member                             },
  { "CODE.NOOP",          push_instr_code_noop                               },
  { "CODE.NTH",           push_instr_code_nth                                },
  { "CODE.NTHCDR",        push_instr_code_nthcdr                             },
  { "CODE.NULL",          push_instr_code_null                               },
  { "CODE.POP",           push_instr_poly_pop          , STACK(code)         },
  { "CODE.POSITION",      push_instr_code_position                           },
  { "CODE.QUOTE",         push_instr_code_quote                              },
  { "CODE.RAND",          push_instr_code_rand                               },
  { "CODE.ROT",           push_instr_poly_rot          , STACK(code)         },
  { "CODE.SHOVE",         push_instr_poly_shove        , STACK(code)         },
  { "CODE.SIZE",          push_instr_code_size                               },
  { "CODE.STACKDEPTH",    push_instr_poly_stackdepth   , STACK(code)         },
  //{ "CODE.SUBST",         push_instr_code_subst                              },
  { "CODE.SWAP",          push_instr_poly_swap         , STACK(code)         },
  { "CODE.YANK",          push_instr_poly_yank         , STACK(code)         },
  { "CODE.YANKDUP",       push_instr_poly_yankdup      , STACK(code)         },

  /* EXEC */
  { "EXEC.=",             push_instr_poly_equal        , STACK(exec)         },
  { "EXEC.DEFINE",        push_instr_poly_define       , STACK(exec)         },
  { "EXEC.DO*COUNT",      push_instr_exec_do_count                           },
  { "EXEC.DO*RANGE",      push_instr_exec_do_range                           },
  { "EXEC.DO*TIMES",      push_instr_exec_do_times                           },
  { "EXEC.DUP",           push_instr_poly_dup          , STACK(exec)         },
  { "EXEC.FLUSH",         push_instr_poly_flush        , STACK(exec)         },
  { "EXEC.IF",            push_instr_exec_if                                 },
  { "EXEC.K",             push_instr_exec_k                                  },
  { "EXEC.POP",           push_instr_poly_pop          , STACK(exec)         },
  { "EXEC.ROT",           push_instr_poly_rot          , STACK(exec)         },
  { "EXEC.S",             push_instr_exec_s                                  },
  { "EXEC.SHOVE",         push_instr_poly_shove        , STACK(exec)         },
  { "EXEC.STACKDEPTH",    push_instr_poly_stackdepth   , STACK(exec)         },
  { "EXEC.SWAP",          push_instr_poly_swap         , STACK(exec)         },
  { "EXEC.Y",             push_instr_exec_y                                  },
  { "EXEC.YANK",          push_instr_poly_yank         , STACK(exec)         },
  { "EXEC.YANKDUP",       push_instr_poly_yankdup      , STACK(exec)         },

  /* INT */
  { "INT.%",             push_instr_int_mod                                  },
  { "INT.*",             push_instr_int_mul                                  },
  { "INT.+",             push_instr_int_add                                  },
  { "INT.-",             push_instr_int_sub                                  },
  { "INT./",             push_instr_int_div                                  },
  { "INT.LESS",          push_instr_int_less                                 },
  { "INT.=",             push_instr_poly_equal         , STACK(integer)      },
  { "INT.GREATER",       push_instr_int_greater                              },
  { "INT.DEFINE",        push_instr_poly_define        , STACK(integer)      },
  { "INT.DUP",           push_instr_poly_dup           , STACK(integer)      },
  { "INT.FLUSH",         push_instr_poly_flush         , STACK(integer)      },
  { "INT.FROMBOOL",      push_instr_int_frombool                             },
  { "INT.FROMREAL",      push_instr_int_fromreal                             },
  { "INT.MAX",           push_instr_int_max                                  },
  { "INT.MIN",           push_instr_int_min                                  },
  { "INT.POP",           push_instr_poly_pop           , STACK(integer)      },
  { "INT.RAND",          push_instr_int_rand                                 },
  { "INT.ROT",           push_instr_poly_rot           , STACK(integer)      },
  { "INT.SHOVE",         push_instr_poly_shove         , STACK(integer)      },
  { "INT.STACKDEPTH",    push_instr_poly_stackdepth    , STACK(integer)      },
  { "INT.SWAP",          push_instr_poly_swap          , STACK(integer)      },
  { "INT.YANK",          push_instr_poly_yank          , STACK(integer)      },
  { "INT.YANKDUP",       push_instr_poly_yankdup       , STACK(integer)      },

  /* NAME */
  { "NAME.=",            push_instr_poly_equal         , STACK(name)         },
  { "NAME.DUP",          push_instr_poly_dup           , STACK(name)         },
  { "NAME.FLUSH",        push_instr_poly_flush         , STACK(name)         },
  { "NAME.POP",          push_instr_poly_pop           , STACK(name)         },
  { "NAME.QUOTE",        push_instr_name_quote                               },
  { "NAME.RAND",         push_instr_name_rand                                },
  { "NAME.ROT",          push_instr_poly_rot           , STACK(name)         },
  { "NAME.SHOVE",        push_instr_poly_shove         , STACK(name)         },
  { "NAME.STACKDEPTH",   push_instr_poly_stackdepth    , STACK(name)         },
  { "NAME.SWAP",         push_instr_poly_swap          , STACK(name)         },
  { "NAME.YANK",         push_instr_poly_yank          , STACK(name)         },
  { "NAME.YANKDUP",      push_instr_poly_yankdup       , STACK(name)         },

  /* REAL */
  { "REAL.%",             push_instr_real_mod                                },
  { "REAL.*",             push_instr_real_mul                                },
  { "REAL.+",             push_instr_real_add                                },
  { "REAL.-",             push_instr_real_sub                                },
  { "REAL./",             push_instr_real_div                                },
  { "REAL.LESS",          push_instr_real_less                               },
  { "REAL.=",             push_instr_poly_equal        , STACK(real)         },
  { "REAL.GREATER",       push_instr_real_greater                            },
  { "REAL.COS",           push_instr_real_cos                                },
  { "REAL.DEFINE",        push_instr_poly_define       , STACK(real)         },
  { "REAL.DUP",           push_instr_poly_dup          , STACK(real)         },
  { "REAL.EXP",           push_instr_real_exp                                },
  { "REAL.FLUSH",         push_instr_poly_flush        , STACK(real)         },
  { "REAL.FROMBOOL",      push_instr_real_frombool                           },
  { "REAL.FROMINT",       push_instr_real_fromint                            },
  { "REAL.LOG",           push_instr_real_log                                },
  { "REAL.MAX",           push_instr_real_max                                },
  { "REAL.MIN",           push_instr_real_min                                },
  { "REAL.POP",           push_instr_poly_pop          , STACK(real)         },
  { "REAL.RAND",          push_instr_real_rand                               },
  { "REAL.ROT",           push_instr_poly_rot          , STACK(real)         },
  { "REAL.SHOVE",         push_instr_poly_shove        , STACK(real)         },
  { "REAL.SIN",           push_instr_real_sin                                },
  { "REAL.STACKDEPTH",    push_instr_poly_stackdepth   , STACK(real)         },
  { "REAL.SWAP",          push_instr_poly_swap         , STACK(real)         },
  { "REAL.TAN",           push_instr_real_tan                                },
  { "REAL.YANK",          push_instr_poly_yank         , STACK(real)         },
  { "REAL.YANKDUP",       push_instr_poly_yankdup      , STACK(real)         },

  { NULL,                 NULL                                               }
};



void push_add_dis(push_t *push) {
  int i;

  /* add default instructions */
  for (i = 0; push_dis[i].name != NULL; i++) {
    push_instr_reg(push, push_dis[i].name, (push_instr_func_t)push_dis[i].func, GETSTACK(push, push_dis[i].stack));
  }
}

