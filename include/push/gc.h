/* gc.h - A multi-threaded mark-and-sweep garbage collector for PUSH values
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

#ifndef _PUSH_GC_H_
#define _PUSH_GC_H_


#include <glib.h>


typedef struct push_gc_S push_gc_t;
typedef struct push_gc_val_S push_gc_val_t;


#include "push/types.h"
#include "push/interpreter.h"
#include "push/val.h"


/* Undefine this, if it causes errors */
#define GC_USE_ATEXIT 1

/* Collecting interval */
#define PUSH_GC_WAIT_USEC 100000 /* 1 s */


struct push_gc_S {
  /* GC thread */
  GThread *thread;

  /* Message queue */
  GAsyncQueue *queue;
};


struct push_gc_val_S {
  push_int_t mark;
  push_bool_t untrack;
};


push_gc_t *push_gc_new(void);
void push_gc_destroy(push_gc_t *gc);
void push_gc_add_interpreter(push_gc_t *gc, push_t *push);
void push_gc_remove_interpreter(push_gc_t *gc, push_t *push);
void push_gc_add_val(push_gc_t *gc, push_val_t *val, push_bool_t recursive);
void push_gc_remove_val(push_gc_t *gc, push_val_t *val, push_bool_t recursive);
push_gc_t *push_gc_global(void);

#endif /* _PUSH_GC_H_ */

