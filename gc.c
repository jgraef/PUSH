/* gc.c - A simple mark-and-sweep garbage collector for Push values
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


static void push_gc_mark_val(push_val_t *val, push_t *push) {
  g_return_if_null(val);

  val->gc = push->gc.generation;

  if (push_check_code(val)) {
    g_queue_foreach(val->code, (GFunc)push_gc_mark_val, push);
  }
}


static void push_gc_mark_stack(push_t *push, push_stack_t *stack) {
  g_queue_foreach(stack, (GFunc)push_gc_mark_val, push);
}


static void push_gc_mark_hash_table(push_t *push, GHashTable *hash_table) {
  GHashTableIter iter;
  push_val_t *val;

  g_hash_table_iter_init(&iter, hash_table);

  while (g_hash_table_iter_next(&iter, NULL, (void*)&val)) {
    push_gc_mark_val(val, push);
  }
}


static int push_gc_find_sweep(push_val_t *val, push_t *push) {
  /* returns 0 if value is not marked */
  return (val->gc == push->gc.generation); 
}


void push_gc_init(push_t *push) {
  g_return_if_null(push);

  push->gc.generation = 0;
  push->gc.values = NULL;
}


void push_gc_destroy(push_t *push) {
  g_return_if_null(push);

  g_list_free(push->gc.values);  
}


void push_gc_collect(push_t *push, int force) {
  GList *link, *next_link;

  g_return_if_null(push);

  push->gc.generation++;

  if (push->gc.generation % PUSH_GC_INTERVAL ==  0 || force) {
    /* mark stacks positive */
    push_gc_mark_stack(push, push->boolean);
    push_gc_mark_stack(push, push->code);
    push_gc_mark_stack(push, push->exec);
    push_gc_mark_stack(push, push->integer);
    push_gc_mark_stack(push, push->name);
    push_gc_mark_stack(push, push->real);

    /* mark bindings positive */
    push_gc_mark_hash_table(push, push->bindings);

    /* mark config positive */
    push_gc_mark_hash_table(push, push->config);

    /* destroy negative marked values */
    next_link = push->gc.values;
    while ((link = g_list_find_custom(next_link, push, (GCompareFunc)push_gc_find_sweep)) != NULL) {
      next_link = link->next;
      push_val_destroy((push_val_t*)link->data);
      push->gc.values = g_list_delete_link(push->gc.values, link);
    }
  }
}


