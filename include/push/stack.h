/* stack.h - Stacks
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

#ifndef _PUSH_STACK_H_
#define _PUSH_STACK_H_


#include <glib.h>


typedef GQueue push_stack_t;


#include "push/types.h"
#include "push/val.h"


#define push_stack_push_new(push, stack, type, ...)  push_stack_push(stack, push_val_new(push, type, __VA_ARGS__))
#define push_stack_pop_code(push)                    push_val_make_code(push, push_stack_pop((push)->code))
#define push_stack_peek_code(push)                   push_val_make_code(push, push_stack_peek((push)->code))
#define push_stack_is_empty(stack)                   (stack->head == NULL)


push_stack_t *push_stack_new(void);
void push_stack_destroy(push_stack_t *stack);
void push_stack_push(push_stack_t *stack, push_val_t *val);
void push_stack_push_nth(push_stack_t *stack, push_int_t n, push_val_t *val);
push_val_t *push_stack_pop(push_stack_t *stack);
push_val_t *push_stack_pop_nth(push_stack_t *stack, push_int_t n);
push_val_t *push_stack_peek(push_stack_t *stack);
push_val_t *push_stack_peek_nth(push_stack_t *stack, push_int_t n);
int push_stack_length(push_stack_t *stack);
void push_stack_flush(push_stack_t *stack);


#endif /* _PUSH_CODE_H_ */

