/* interpreter.c - The PUSH interpreter
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


static push_destroy_instr(push_instr_t *instr) {
  g_slice_free(push_instr_t, instr);
}


push_t *push_new_full(push_interrupt_handler_t interrupt_handler) {
  push_t *push;

  push = g_slice_new(push_t);

  /* set interrupt handler */
  push->interrupt_handler = interrupt_handler;

  /* initialize garbage collection */
  push_gc_init(push);

  /* initialize execution mutex */
  g_static_mutex_init(&push->mutex);

  /* create hash tables */
  push->config = g_hash_table_new(NULL, NULL);
  push->bindings = g_hash_table_new(NULL, NULL);
  push->instructions = g_hash_table_new_full(NULL, NULL, NULL, (GDestroyNotify)push_destroy_instr);

  /* initialize stacks */
  push->boolean = push_stack_new();
  push->code = push_stack_new();
  push->exec = push_stack_new();
  push->integer = push_stack_new();
  push->name = push_stack_new();
  push->real = push_stack_new();

  /* initialize random number generator */
  push->rand = g_rand_new();

  /* initialize storage for interned strings */
  push->names = g_string_chunk_new(PUSH_NAME_STORAGE_BLOCK_SIZE);

  /* default configuration */
  push_config_set(push, "MIN-RANDOM-INT", push_val_new(push, PUSH_TYPE_INT, -100));
  push_config_set(push, "MAX-RANDOM-INT", push_val_new(push, PUSH_TYPE_INT, 100));
  push_config_set(push, "MIN-RANDOM-REAL", push_val_new(push, PUSH_TYPE_REAL, 0.0));
  push_config_set(push, "MAX-RANDOM-REAL", push_val_new(push, PUSH_TYPE_REAL, 1.0));
  push_config_set(push, "MIN-RANDOM-NAME-LENGTH", push_val_new(push, PUSH_TYPE_INT, 2));
  push_config_set(push, "MAX-RANDOM-NAME-LENGTH", push_val_new(push, PUSH_TYPE_INT, 16));
  push_config_set(push, "MAX-POINTS-IN-RANDOM-EXPRESSIONS", push_val_new(push, PUSH_TYPE_INT, 100));

  return push;
}


push_t *push_new(void) {
  return push_new_full(NULL);
}


void push_destroy(push_t *push) {
  g_return_if_null(push);

  /* clear stacks */
  push_stack_flush(push->boolean);
  push_stack_flush(push->code);
  push_stack_flush(push->exec);
  push_stack_flush(push->integer);
  push_stack_flush(push->name);
  push_stack_flush(push->real);

  /* clear hash tables */
  g_hash_table_remove_all(push->bindings);
  g_hash_table_remove_all(push->config);

  /* collect everything & destroy GC */
  push_gc_collect(push, PUSH_TRUE);
  push_gc_destroy(push);

  /* destroy stacks */
  push_stack_destroy(push->boolean);
  push_stack_destroy(push->code);
  push_stack_destroy(push->exec);
  push_stack_destroy(push->integer);
  push_stack_destroy(push->name);
  push_stack_destroy(push->real);

  /* destroy hash tables */
  g_hash_table_destroy(push->bindings);
  g_hash_table_destroy(push->config);
  g_hash_table_destroy(push->instructions);

  /* destroy random number generator */
  g_rand_free(push->rand);

  /* destroy storage for interned strings */
  g_string_chunk_free(push->names);

  /* destroy execution mutex */
  g_static_mutex_free(&push->mutex);

  g_slice_free(push_t, push);
}



push_name_t push_intern_name(push_t *push, const char *name) {
  g_return_val_if_null(push, NULL);

  return g_string_chunk_insert_const(push->names, name);
}


void push_define(push_t *push, push_name_t name, push_val_t *val) {
  g_return_if_null(push);

  if (!push_check_name(val) && !push_check_name(val)) {
    g_hash_table_insert(push->bindings, name, val);
  }
}


void push_undef(push_t *push, push_name_t name) {
  push_val_t *val;

  g_return_if_null(push);

  val = g_hash_table_lookup(push->bindings, name);
  if (val != NULL) {
    g_hash_table_remove(push->bindings, name);
  }
}


push_val_t *push_lookup(push_t *push, push_name_t name) {
  g_hash_table_lookup(push->bindings, name);
}


void push_do_val(push_t *push, push_val_t *val) {
  push_val_t *val2;

  g_return_if_null(push);

  switch (val->type) {
    case PUSH_TYPE_BOOL:
      push_stack_push(push->boolean, val);
      break;

    case PUSH_TYPE_CODE:
      g_return_if_null(val);

      /* push all values of code object onto exec stack in reverse order */
      push_code_push_elements(val->code, push->exec);
      break;

    case PUSH_TYPE_INT:
      push_stack_push(push->integer, val);
      break;

    case PUSH_TYPE_INSTR:
      g_return_if_null(val);

      push_call_instr(push, val->instr);
      break;

    case PUSH_TYPE_NAME:
      g_return_if_null(val);

      val2 = push_lookup(push, val->name);
      if (val2 != NULL) {
        push_stack_push(push->exec, val2);
      }
      else {
        push_stack_push(push->name, val);
      }
      break;

    case PUSH_TYPE_REAL:
      push_stack_push(push->real, val);
      break;

    default:
      g_warning("Unknown value type: %d", val->type);
      break;
  }
}


/* Do one single step
 * NOTE: Doesn't clear the interrupt flag
 * NOTE: Doesn't check execution mutex
 */
push_bool_t push_step(push_t *push) {
  push_val_t *val;

  val = push_stack_pop(push->exec);
  if (val != NULL) {
    push_do_val(push, val);
  }

  push_gc_collect(push, PUSH_FALSE);

  if (push->interrupt_flag != 0) {
    /* call the interrupt handler */
    if (push->interrupt_handler != NULL && push->interrupt_flag > 0) {
      push->interrupt_handler(push, push->interrupt_flag, push->userdata);
    }
    return PUSH_FALSE;
  }

  return val != NULL;
}


push_int_t push_steps(push_t *push, push_int_t n) {
  int i;

  g_return_if_null(push);

  g_static_mutex_lock(&push->mutex);

  /* clear interrupt flag */
  push->interrupt_flag = 0;

  /* do steps */
  for (i = 0; i < n && push_step(push); i++);

  g_static_mutex_unlock(&push->mutex);

  return i;
}


void push_run(push_t *push) {
  g_return_if_null(push);

  g_static_mutex_lock(&push->mutex);

  /* clear interrupt flag */
  push->interrupt_flag = 0;

  /* run until EXEC stack is empty or an interrupt occured */
  while (push_step(push));

  g_static_mutex_unlock(&push->mutex);
}


push_bool_t push_done(push_t *push) {
  g_return_if_null(push);

  return push_stack_is_empty(push->exec);
}


void push_interrupt(push_t *push, push_int_t interrupt_flag) {
  push->interrupt_flag = interrupt_flag;
}


/* NOTE: free returned string with g_free or push_free */
char *push_dump_state(push_t *push) {
  GString *xml;

  xml = g_string_new("<?xml version=\"1.0\" ?>\n");

  g_static_mutex_lock(&push->mutex);
  push_serialize(xml, 0, push);
  g_static_mutex_unlock(&push->mutex);

  return g_string_free(xml, 0);
}
void push_free(void *ptr) {
  g_free(ptr);
}

void push_load_state(push_t *push, const char *xml) {
  g_static_mutex_lock(&push->mutex);
  push_unserialize_parse(push, xml);
  g_static_mutex_unlock(&push->mutex);
}

