/* push.h - Include everything
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
 *
 *
 * TODO: - Replace usages of standard C types (e.g. int) with PUSH basic types (e.g. push_int_t)
 *       - Replace PUSH_TRUE/FALSE with TRUE/FALSE
 *       - Beim Laden von DIS überprüfen ob die Instruktion per Config geladen werden soll
 *       - Stack/code size limits
 *       - Don't intern instruction names?
 *       - Load from config: random seed/state
 *       - unserialize: Use GMarkup parser
 */

#ifndef _PUSH_H_
#define _PUSH_H_


/* Include all header files */
#include "push/code.h"
#include "push/gc.h"
#include "push/instr.h"
#include "push/rand.h"
#include "push/serialize.h"
#include "push/stack.h"
#include "push/types.h"
#include "push/unserialize.h"
#include "push/val.h"
#include "push/vm.h"


#define PUSH_VERSION 0

#define g_return_if_null(ptr) g_return_if_fail(ptr != NULL)
#define g_return_val_if_null(ptr, val) g_return_val_if_fail(ptr != NULL, val)



void push_add_dis(push_t *push);
int push_version(void);


#endif /* _PUSH_H_ */

