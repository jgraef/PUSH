/* val.h - Dynamically-typed PUSH values
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

#ifndef _PUSH_VAL_H_
#define _PUSH_VAL_H_


typedef struct push_val_S push_val_t;


#include "push/types.h"
#include "push/interpreter.h"
#include "push/instr.h"
#include "push/code.h"
#include "push/gc.h"



#define push_check_none(v)            ((v)->type == PUSH_TYPE_NONE)
#define push_check_bool(v)            ((v)->type == PUSH_TYPE_BOOL)
#define push_check_code(v)            ((v)->type == PUSH_TYPE_CODE)
#define push_check_int(v)             ((v)->type == PUSH_TYPE_INT)
#define push_check_instr(v)           ((v)->type == PUSH_TYPE_INSTR)
#define push_check_name(v)            ((v)->type == PUSH_TYPE_NAME)
#define push_check_real(v)            ((v)->type == PUSH_TYPE_REAL)
#define push_val_max(v1, v2, t)       ((v1)->t > (v2)->t ? v1 : v2)
#define push_val_min(v1, v2, t)       ((v1)->t < (v2)->t ? v1 : v2)
#define push_val_code_dup(push, val)  push_val_new(push, PUSH_TYPE_CODE, push_code_dup((val)->code))


#define PUSH_TYPE_NONE  0
#define PUSH_TYPE_BOOL  1
#define PUSH_TYPE_CODE  2
#define PUSH_TYPE_INT   3
#define PUSH_TYPE_INSTR 4
#define PUSH_TYPE_NAME  5
#define PUSH_TYPE_REAL  6


/* Dynamic value: Container for different types
 * NOTE: inmutable! make a copy if you want to change them
 */
struct push_val_S {
  /* type */
  int type;

  /* actual value */
  union {
    push_bool_t boolean;
    push_code_t *code;
    push_int_t integer;
    push_instr_t *instr;
    push_name_t name;
    push_real_t real;
    long _value;
  };

  /* garbage collector */
  push_gc_val_t gc;
};


push_val_t *push_val_new(push_t *push, int type, ...);
void push_val_destroy(push_val_t *val);
push_val_t *push_val_copy(push_val_t *val, push_t *to_push);
push_bool_t push_val_equal(push_val_t *val1, push_val_t *val2);
push_val_t *push_val_make_code(push_t *push, push_val_t *val);


#endif /* _PUSH_VAL_H_ */

