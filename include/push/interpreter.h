/* interpreter.h - The PUSH interpreter
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

#ifndef _PUSH_INTERPRETER_H_
#define _PUSH_INTERPRETER_H_


#include <glib.h>


typedef struct push_S push_t;


#include "push/types.h"
#include "push/stack.h"
#include "push/val.h"



#define PUSH_NAME_STORAGE_BLOCK_SIZE 1024


/* Interrupt handler type */
typedef void (*push_interrupt_handler_t)(push_t *push, push_int_t interrupt_flag, void *userdata);

/* Step hook function type */
typedef void (*push_step_hook_t)(push_t *push, void *userdata);


/* Interpreter */
struct push_S {
  /* stacks: push_val_t */
  push_stack_t *boolean;
  push_stack_t *code;
  push_stack_t *exec;
  push_stack_t *integer;
  push_stack_t *name;
  push_stack_t *real;

  /* Interrupt flag & handler */
  push_int_t interrupt_flag;
  push_interrupt_handler_t interrupt_handler;

  /* Step hook */
  push_step_hook_t step_hook;

  /* user data */
  void *userdata;

  /* Interpreter configuration */
  GHashTable *config;

  /* bindings: push_name_t -> push_val_t* */
  GHashTable *bindings;

  /* instructions */
  GHashTable *instructions;

  /* random number generator */
  GRand *rand;

  /* storage for interned strings */
  GStringChunk *names;

  /* Lock against concurrent execution */
  GStaticMutex mutex;

  /* garbage collector */
  struct {
    int generation;
    GList *values;
  } gc;
};


push_t *push_new_full(push_bool_t default_instructions, push_bool_t default_config, push_interrupt_handler_t interrupt_handler, push_step_hook_t step_hook);
push_t *push_new(void);
void push_destroy(push_t *push);
push_t *push_copy(push_t *push);
void push_flush(push_t *push);
push_name_t push_intern_name(push_t *push, const char *name);
void push_config_set(push_t *push, const char *key, push_val_t *val);
push_val_t *push_config_get(push_t *push, const char *key);
void push_define(push_t *push, push_name_t name, push_val_t *val);
void push_undef(push_t *push, push_name_t name);
push_val_t *push_lookup(push_t *push, push_name_t name);
void push_do_val(push_t *push, push_val_t *val);
push_bool_t push_step(push_t *push);
push_int_t push_run(push_t *push, push_int_t max_steps);
push_bool_t push_done(push_t *push);
char *push_dump_state(push_t *push);
void push_free(void *ptr);
void push_load_state(push_t *push, const char *xml);
void push_interrupt(push_t *push, push_int_t interrupt_flag);


#endif /* _PUSH_INTERPRETER_H_ */

