/* instr.h - Instruction datatype
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

#ifndef _PUSH_INSTR_H_
#define _PUSH_INSTR_H_


typedef struct push_instr_S push_instr_t;


#include "push/types.h"
#include "push/interpreter.h"


/* Instruction handler function type */
typedef void (*push_instr_func_t)(push_t *push, void *userdata);


/* Instruction type */
struct push_instr_S {
  push_name_t name;
  push_instr_func_t func;
  void *userdata;
};


void push_instr_reg(push_t *push, const char *name, push_instr_func_t func, void *userdata);
push_instr_t *push_instr_lookup(push_t *push, const char *name);
void push_instr_call(push_t *push, push_instr_t *instr);


#endif /* _PUSH_INSTR_H_ */

