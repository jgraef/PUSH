/* vm.h - Multi-threaded execution of PUSH interpreters
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

#ifndef _PUSH_VM_H_
#define _PUSH_VM_H_


#include <glib.h>


typedef struct push_vm_S push_vm_t;


#include "push/types.h"
#include "push/interpreter.h"



#define PUSH_VM_INTERRUPT_KILL -1


typedef void (*push_vm_done_callback_t)(push_vm_t *vm, push_t *push);


struct push_vm_S {
  /* Processes: push_t */
  GList *processes;

  /* Thread pool for running processes */
  GThreadPool *threads;

  /* Max number of steps per process */
  push_int_t max_steps;

  /* Callback when process is done */
  push_vm_done_callback_t done_callback;

  /* Mutex */
  GMutex *mutex;

  /* Condition for waiting until all processes are done */
  GCond *wait_cond;
};


push_vm_t *push_vm_new(push_int_t num_threads, push_int_t max_steps, push_vm_done_callback_t done_callback);
void push_vm_destroy(push_vm_t *vm, push_bool_t kill_all);
void push_vm_run(push_vm_t *vm, push_t *push);
push_int_t push_vm_num_processes(push_vm_t *vm);
push_int_t push_vm_num_queued(push_vm_t *vm);
void push_vm_interrupt_all(push_vm_t *vm, push_int_t interrupt_flag);
void push_vm_kill_all(push_vm_t *vm);
void push_vm_wait(push_vm_t *vm);


#endif /* _PUSH_VM_H_ */

