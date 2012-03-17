/* val.c - Implementation of dynamically-typed PUSH values
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
#include <string.h>
#include <stdarg.h>

#include "push.h"



push_val_t *push_val_new(push_t *push, int type, ...) {
  va_list ap;
  push_val_t *val;
  push_code_t *code;

  /* create dynamically-typed value */
  val = g_slice_new(push_val_t);
  val->type = type;

  /* set value */
  va_start(ap, type);
  switch (type) {
    case PUSH_TYPE_BOOL:
      val->boolean = va_arg(ap, push_bool_t);
      break;
    case PUSH_TYPE_CODE:
      code = va_arg(ap, push_code_t*);
      val->code = code == NULL ? push_code_new() : code;
      break;
    case PUSH_TYPE_INT:
      val->integer = va_arg(ap, push_int_t);
      break;
    case PUSH_TYPE_INSTR:
      val->instr = va_arg(ap, push_instr_t*);
      break;
    case PUSH_TYPE_NAME:
      val->name = va_arg(ap, push_name_t);
      break;
    case PUSH_TYPE_REAL:
      val->real = va_arg(ap, push_real_t);
      break;
    default:
      val->type = PUSH_TYPE_NONE;
      break;
  }
  va_end(ap);

  /* add to garbage collection */
  val->gc = push->gc.generation;
  push->gc.values = g_list_prepend(push->gc.values, val);

  return val;
}


void push_val_destroy(push_val_t *val) {
  g_return_if_null(val);

  if (push_check_code(val)) {
    push_code_destroy(val->code);
  }

  g_slice_free(push_val_t, val);
}


push_bool_t push_val_equal(push_val_t *val1, push_val_t *val2) {
  g_return_val_if_null(val1, PUSH_FALSE);
  g_return_val_if_null(val2, PUSH_FALSE);

  if (val1 == val2) {
    return PUSH_TRUE;
  }
  else if (val1->type != val2->type) {
    return PUSH_FALSE;
  }
  else {
    switch (val1->type) {
      case PUSH_TYPE_NONE:
        return 1;
      case PUSH_TYPE_BOOL:
        return val1->boolean == val2->boolean;
      case PUSH_TYPE_CODE:
        return push_code_equal(val1->code, val2->code);
      case PUSH_TYPE_INT:
        return val1->integer == val2->integer;
      case PUSH_TYPE_INSTR:
        return val1->instr == val2->instr;
      case PUSH_TYPE_NAME:
        return val1->name == val2->name;
      case PUSH_TYPE_REAL:
        return val1->real == val2->real;
      default:
        return PUSH_FALSE;
    }
  }
}


push_val_t *push_val_make_code(push_t *push, push_val_t *val) {
  push_val_t *val_new;

  g_return_val_if_null(val, NULL);

  if (push_check_code(val)) {
    return val;
  }
  else {
    val_new = push_val_new(push, PUSH_TYPE_CODE, NULL);
    push_code_append(val_new->code, val);
    return val_new;
  }
}

