/* instr.c - Instruction datatype
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


void push_instr_reg(push_t *push, const char *name, push_instr_func_t func, void *userdata) {
  push_instr_t *instr;

  g_return_if_null(push);
  g_return_if_null(name);
  g_return_if_null(func);

  instr = g_slice_new(push_instr_t);
  instr->name = push_intern_name(push, name);
  instr->func = func;
  instr->userdata = userdata;

  g_hash_table_insert(push->instructions, instr->name, instr);
}


void push_instr_destroy(push_instr_t *instr) {
  g_slice_free(push_instr_t, instr);
}


push_instr_t *push_instr_lookup(push_t *push, const char *name) {
  g_return_val_if_null(push, NULL);

  return (push_instr_t*)g_hash_table_lookup(push->instructions, push_intern_name(push, name));
}


void push_call_instr(push_t *push, push_instr_t *instr) {
  g_return_if_null(instr);

  //g_debug("%s: instr=%p, name=%s, func=%p, userdata=%p", __func__, instr, instr->name, instr->func, instr->userdata);
  instr->func(push, instr->userdata);
  //g_debug("%s: done", __func__);
}

