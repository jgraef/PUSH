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


push_instr_t *push_instr_lookup(push_t *push, const char *name) {
  g_return_val_if_null(push, NULL);

  return (push_instr_t*)g_hash_table_lookup(push->instructions, push_intern_name(push, name));
}


void push_call_instr(push_t *push, push_instr_t *instr) {
  g_return_if_null(instr);

  g_debug("%s: instr=0x%lx, name=%s, func=0x%lx, userdata=0x%lx", __func__, (long)instr, instr->name, (long)instr->func, (long)instr->userdata);
  instr->func(push, instr->userdata);
  g_debug("%s: done", __func__);
}

