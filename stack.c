/* stack.c - Stacks
 * NOTE: Just wrappers for the GLib GQueue functions
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



push_stack_t *push_stack_new(void) {
  return g_queue_new();
}

void push_stack_destroy(push_stack_t *stack) {
  g_queue_free(stack);
}

void push_stack_push(push_stack_t *stack, push_val_t *val) {
  g_return_if_null(val);

  g_queue_push_head(stack, val);
}

void push_stack_push_nth(push_stack_t *stack, push_int_t n, push_val_t *val) {
  g_return_if_null(val);

  g_queue_push_nth(stack, val, n);
}

push_val_t *push_stack_pop(push_stack_t *stack) {
  return (push_val_t*)g_queue_pop_head(stack);
}

push_val_t *push_stack_pop_nth(push_stack_t *stack, push_int_t n) {
  return (push_val_t*)g_queue_pop_nth(stack, n);
}

push_val_t *push_stack_peek(push_stack_t *stack) {
  return (push_val_t*)g_queue_peek_head(stack);
}

push_val_t *push_stack_peek_nth(push_stack_t *stack, push_int_t n) {
  return (push_val_t*)g_queue_peek_nth(stack, n);
}

int push_stack_length(push_stack_t *stack) {
  return stack->length;
}

void push_stack_flush(push_stack_t *stack) {
  g_queue_clear(stack);
}


push_stack_t *push_stack_copy(push_stack_t *stack, push_t *to_push) {
  push_stack_t *new_stack;
  GList *link;

  new_stack = g_queue_new();

  for (link = stack->head; link != NULL; link = link->next) {
    g_queue_push_tail(new_stack, push_val_copy((push_val_t*)link->data, to_push));
  }

  return new_stack;
}

