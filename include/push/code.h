/* code.h - The code datatype
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

#ifndef _PUSH_CODE_H_
#define _PUSH_CODE_H_


#include <glib.h>


typedef GQueue push_code_t;


#include "push/types.h"
#include "push/interpreter.h"
#include "push/stack.h"
#include "push/val.h"


push_code_t *push_code_new(void);
void push_code_destroy(push_code_t *code);
void push_code_append(push_code_t *code, push_val_t *val);
void push_code_prepend(push_code_t *code, push_val_t *val);
void push_code_insert(push_code_t *code, int n, push_val_t *val);
push_val_t *push_code_pop(push_code_t *code);
push_val_t *push_code_pop_nth(push_code_t *code, int n);
push_val_t *push_code_peek(push_code_t *code);
push_val_t *push_code_peek_nth(push_code_t *code, int n);
int push_code_length(push_code_t *code);
void push_code_flush(push_code_t *code);
push_code_t *push_code_dup_ext(push_code_t *code, GList *first_link, GList *last_link, GList *replace_link, push_val_t *replace_with);
push_code_t *push_code_dup(push_code_t *code);
push_bool_t push_code_equal(push_code_t *code1, push_code_t *code2);
push_code_t *push_code_concat(push_code_t *code1, push_code_t *code2);
push_code_t *push_code_container(push_code_t *haystack, push_val_t *needle);
int push_code_discrepancy(push_code_t *code1, push_code_t *code2);
push_val_t *push_code_extract(push_code_t *code, push_int_t point);
int push_code_index(push_code_t *haystack, push_val_t *needle);
int push_code_size(push_code_t *code);
push_val_t *push_code_replace(push_t *push, push_code_t *code, push_int_t point, push_val_t *val);
void push_code_push_elements(push_code_t *code, push_stack_t *stack);


#endif /* _PUSH_CODE_H_ */

