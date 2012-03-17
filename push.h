/* push.h - Definitions and declarations for the PUSH interpreter
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

#ifndef _PUSH_PUSH_H_
#define _PUSH_PUSH_H_


/* TODO: - Replace usages of standard C types (e.g. int) with PUSH basic types (e.g. push_int_t)
 *       - Beim Laden von DIS überprüfen ob die Instruktion per Config geladen werden soll
 *       - Stack size limits
 *       - Don't intern config keys
 *       - Load from config: random seed, NEW-ERC-NAME-PROPABILITY
 */

#include <glib.h>



#define PUSH_VERSION 0

#define PUSH_GC_INTERVAL 128

#define PUSH_NAME_STORAGE_BLOCK_SIZE 1024


/* Typedef's of structs */
typedef struct push_S push_t;
typedef struct push_val_S push_val_t;
typedef struct push_instr_S push_instr_t;
typedef struct push_proc_S push_proc_t;
typedef struct push_vm_S push_vm_t;


/* Basic types */
#define PUSH_FALSE 0
#define PUSH_TRUE  1
typedef int push_bool_t;
typedef int push_int_t;
typedef char *push_name_t; /* NOTE: interned string */
typedef double push_real_t;


/* Instructions */
typedef void (*push_instr_func_t)(push_t *push, void *userdata);
struct push_instr_S {
  push_name_t name;
  push_instr_func_t func;
  void *userdata;
};


/* Code objects */
typedef GQueue push_code_t;



/* Dynamic value: Container for different types
 * NOTE: inmutable! make a copy if you want to change them
 */
#define PUSH_TYPE_NONE  0
#define PUSH_TYPE_BOOL  1
#define PUSH_TYPE_CODE  2
#define PUSH_TYPE_INT   3
#define PUSH_TYPE_INSTR 4
#define PUSH_TYPE_NAME  5
#define PUSH_TYPE_REAL  6

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
  };

  /* garbage collector */
  int gc;
};



/* A stack */
typedef GQueue push_stack_t;



/* PUSH interpreter state
 * NOTE: all names that are used as hash table keys strings are interned
 */
struct push_S {
  /* stacks: push_val_t */
  push_stack_t *boolean;
  push_stack_t *code;
  push_stack_t *exec;
  push_stack_t *integer;
  push_stack_t *name;
  push_stack_t *real;

  /* Interrupt flag */
  push_int_t interrupt;

  /* Interpreter configuration */
  GHashTable *config;

  /* bindings: push_name_t -> push_val_t* */
  GHashTable *bindings;

  /* instructions */
  GHashTable *instructions;

  /* random number generator */
  GRand *rand;

  /* storage for interned strings */
  GStringChunk *names;

  /* garbage collector */
  struct {
    int generation;
    GList *values;
  } gc;
};



/* Function declarations & macros */

int push_version(void);
push_t *push_new(void);
void push_destroy(push_t *push);
push_name_t push_intern_name(push_t *push, const char *name);
void push_define(push_t *push, push_name_t name, push_val_t *val);
void push_undef(push_t *push, push_name_t name);
push_val_t *push_lookup(push_t *push, push_name_t name);
void push_do_val(push_t *push, push_val_t *val);
push_bool_t push_step(push_t *push);
push_int_t push_steps(push_t *push, push_int_t n);
void push_run(push_t *push);
push_bool_t push_done(push_t *push);
char *push_dump_state(push_t *push);
void push_free(void *ptr);
void push_load_state(push_t *push, const char *xml);

void push_instr_reg(push_t *push, const char *name, push_instr_func_t func, void *userdata);
push_instr_t *push_instr_lookup(push_t *push, const char *name);
void push_instr_call(push_t *push, push_instr_t *instr);

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
#define push_stack_push_new(push, stack, type, ...)  push_stack_push(stack, push_val_new(push, type, __VA_ARGS__))
#define push_stack_pop_code(push)                    push_val_make_code(push, push_stack_pop((push)->code))
#define push_stack_peek_code(push)                   push_val_make_code(push, push_stack_peek((push)->code))
#define push_stack_is_empty(stack)                   (stack->head == NULL)

push_val_t *push_val_new(push_t *push, int type, ...);
void push_val_destroy(push_val_t *val);
push_bool_t push_val_equal(push_val_t *val1, push_val_t *val2);
push_val_t *push_val_make_code(push_t *push, push_val_t *val);
#define push_check_none(v)            ((v)->type == PUSH_TYPE_NONE)
#define push_check_bool(v)            ((v)->type == PUSH_TYPE_BOOL)
#define push_check_code(v)            ((v)->type == PUSH_TYPE_CODE)
#define push_check_int(v)             ((v)->type == PUSH_TYPE_INT)
#define push_check_instr(v)           ((v)->type == PUSH_TYPE_INSTR)
#define push_check_name(v)            ((v)->type == PUSH_TYPE_NAME)
#define push_check_real(v)            ((v)->type == PUSH_TYPE_REAL)
#define push_val_max(v1, v2, t)       ((v1)->t > (v2)->t ? v1 : v2)
#define push_val_min(v1, v2, t)       ((v1)->t < (v2)->t ? v1 : v2)
#define push_val_code_dup(push, val)  push_val_new(push, PUSH_TYPE_CODE, push_code_dup((val)->code));

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
push_code_t *push_code_dup(push_code_t *code);
push_bool_t push_code_equal(push_code_t *code1, push_code_t *code2);
push_code_t *push_code_concat(push_code_t *code1, push_code_t *code2);
push_code_t *push_code_container(push_code_t *haystack, push_val_t *needle);
int push_code_discrepancy(push_code_t *code1, push_code_t *code2);
push_val_t *push_code_extract(push_code_t *code, int point);
int push_code_index(push_code_t *haystack, push_val_t *needle);
int push_code_size(push_code_t *code);
push_code_t *push_code_replace(push_code_t *_code, push_int_t i, push_val_t *val);
void push_code_push_elements(push_code_t *code, push_stack_t *stack);

void push_serialize_val(GString *xml, int ident_count, push_val_t *val);
void push_serialize_code(GString *xml, int ident_count, push_code_t *code);
void push_serialize_stack(GString *xml, int ident_count, const char *name, push_stack_t *stack);
void push_serialize(GString *xml, int ident_count, push_t *push);
void push_unserialize_parse(push_t *push, const char *xml_data);

void push_rand_set_seed(push_t *push, int seed);
push_bool_t push_rand_bool(push_t *push);
push_code_t *push_rand_code(push_t *push, push_int_t *size, push_bool_t force_size);
push_int_t push_rand_int(push_t *push);
push_instr_t *push_rand_instr(push_t *push);
push_name_t push_rand_name(push_t *push);
push_name_t push_rand_bound_name(push_t *push);
push_real_t push_rand_real(push_t *push);
push_val_t *push_rand_val(push_t *push, int type, push_int_t *size, push_bool_t force_size);

void push_config_set_interned(push_t *push, push_name_t key, push_val_t *val);
void push_config_set(push_t *push, const char *key, push_val_t *val);
push_val_t *push_config_get_interned(push_t *push, push_name_t key);
push_val_t *push_config_get(push_t *push, const char *key);

void push_gc_init(push_t *push);
void push_gc_destroy(push_t *push);
void push_gc_collect(push_t *push, int force);

void push_add_dis(push_t *push);

#define g_return_if_null(ptr) g_return_if_fail(ptr != NULL)
#define g_return_val_if_null(ptr, val) g_return_val_if_fail(ptr != NULL, val)


#endif /* _PUSH_PUSH_H_ */

