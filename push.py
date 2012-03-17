# push.py - Python wrapper for PUSH
#
# Copyright (c) 2012 Janosch Gräf <janosch.graef@gmx.net>
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.


from ctypes import CDLL, c_char, c_void_p, c_uint, c_int, c_double, c_char_p, cast, POINTER, CFUNCTYPE, Structure, Union, byref
import zlib
from threading import Thread, Semaphore


__all__ = ["Interpreter", "VM", "syscall", "instr"]


# PUSH version
PUSH_VERSION = 0

# C's void
c_void = None

# Basic push types
push_bool_t = c_int
push_int_t = c_int
# NOTE: can't use c_char_p or ctypes will convert strings and make a mess
#       with interned strings!
push_name_t = c_void_p
push_real_t = c_double

# stack type (opaque)
push_stack_P = c_void_p

# PUSH interpreter type
class push_t(Structure):
    _fields_ = [("bool", push_stack_P),
                ("code", push_stack_P),
                ("exec", push_stack_P),
                ("int", push_stack_P),
                ("name", push_stack_P),
                ("real", push_stack_P),
                ("interrupt", push_int_t)]
push_P = POINTER(push_t)

# instruction type
push_instr_func_t = CFUNCTYPE(c_void, push_P, c_void_p)
class push_instr_t(Structure):
    _fields_ = [("name", push_name_t),
                ("func", push_instr_func_t),
                ("userdata", c_void_p)]
push_instr_P = POINTER(push_instr_t)

# code type (opaque)
push_code_P = c_void_p

# PUSH value type
PUSH_TYPE_NONE  = 0
PUSH_TYPE_BOOL  = 1
PUSH_TYPE_CODE  = 2
PUSH_TYPE_INT   = 3
PUSH_TYPE_INSTR = 4
PUSH_TYPE_NAME  = 5
PUSH_TYPE_REAL  = 6
class push_val_t_union(Union):
    _fields_ = [("bool", push_bool_t),
                ("code", push_code_P),
                ("int", push_int_t),
                ("instr", push_instr_P),
                ("name", push_name_t),
                ("real", push_real_t)]
class push_val_t(Structure):
    _fields_ = [("type", c_int),
                ("v", push_val_t_union)]
push_val_P = POINTER(push_val_t)

# code iteration function type
push_code_iter_func_t = CFUNCTYPE(c_void, push_val_P, c_void_p)



# load library
__libpush__ = CDLL("./libpush.so")


# load function prototypes
def __init_prototypes__(l):
    prototypes = [
        # Interpreter
        [l.push_version, c_int],
        [l.push_new, push_P],
        [l.push_destroy, c_void, push_P],
        [l.push_intern_name, push_name_t, push_P, c_char_p],
        [l.push_instr_reg, c_void, push_P, c_char_p, push_instr_func_t, c_void_p],
        [l.push_instr_lookup, push_instr_P, push_P, c_char_p],
        [l.push_define, c_void, push_P, push_name_t, push_val_P],
        [l.push_undef, c_void, push_t, push_name_t],
        [l.push_lookup, push_val_P, push_P, push_name_t],
        [l.push_steps, push_int_t, push_P, push_int_t],
        [l.push_done, push_bool_t, push_P],
        [l.push_dump_state, c_void_p, push_P],
        [l.push_free, c_void, c_void_p],
        [l.push_load_state, c_void, push_P, c_char_p],
        [l.push_add_dis, c_void, push_P],
        # Stack
        [l.push_stack_push, c_void, push_stack_P, push_val_P],
        [l.push_stack_push_nth, c_void, push_stack_P, c_int, push_val_P],
        [l.push_stack_pop, push_val_P, push_stack_P],
        [l.push_stack_pop_nth, push_val_P, push_stack_P, c_int],
        [l.push_stack_peek, push_val_P, push_stack_P],
        [l.push_stack_peek_nth, push_val_P, push_stack_P, c_int],
        [l.push_stack_length, c_int, push_stack_P],
        [l.push_stack_flush, c_void, push_stack_P],
        # Value
        [l.push_val_new, push_val_P, push_P, c_int],
        [l.push_val_destroy, c_void, push_val_P],
        [l.push_val_equal, push_bool_t, push_val_P, push_val_P],
        [l.push_val_make_code, push_val_P, push_P, push_val_P],
        # Code
        [l.push_code_new, push_code_P],
        [l.push_code_destroy, c_void, push_code_P],
        [l.push_code_append, c_void, push_code_P, push_val_P],
        [l.push_code_prepend, c_void, push_code_P, push_val_P],
        [l.push_code_insert, c_void, push_code_P, c_int, push_val_P],
        [l.push_code_pop, push_val_P, push_code_P],
        [l.push_code_pop_nth, push_val_P, push_code_P, c_int],
        [l.push_code_peek, push_val_P, push_code_P],
        [l.push_code_peek_nth, push_val_P, push_code_P, c_int],
        [l.push_code_flush, c_void, push_code_P],
        [l.push_code_dup, push_code_P, push_code_P],
        [l.push_code_equal, push_bool_t, push_code_P, push_code_P],
        [l.push_code_concat, push_code_P, push_code_P, push_code_P],
        [l.push_code_container, push_code_P, push_P, push_code_P, push_val_P],
        [l.push_code_discrepancy, push_int_t, push_code_P, push_code_P],
        [l.push_code_extract, push_val_P, push_code_P, push_int_t],
        [l.push_code_index, push_int_t, push_code_P, push_val_P],
        [l.push_code_size, push_int_t, push_code_P],
        [l.push_code_foreach, c_void, push_code_P, c_void_p],
        # Rand
        [l.push_rand_set_seed, c_void, push_P, c_int],
        [l.push_rand_val, push_val_P, push_P, c_int, POINTER(push_int_t), push_bool_t],
        # Config
        [l.push_config_set, c_void, push_P, c_char_p, push_val_P],
        [l.push_config_get, push_val_P, push_P, c_char_p]
        ]

    for p in prototypes:
        p[0].restype = p[1]
        p[0].argtypes = p[2:]


__init_prototypes__(__libpush__)

# check version
if (__libpush__.push_version() != PUSH_VERSION):
    raise ImportError("""Invalid versions:
Python wrapper version: %d
Push library version:   %d"""%(PYTHON_VERSION, __libpush__.push_version()))


# Instruction python type
class instr(str):
    def __repr__(self):
        return "instr('%s')"%self


class Interpreter:
    """ Class wrapping C library functions of __libpush__ """

    # C pointer to PUSH interpreter
    _push = None

    def __init__(self, xml_data = None, dis = True, seed = None):
        self._push = __libpush__.push_new()

        if (dis):
            __libpush__.push_add_dis(self._push)

        self.loads(xml_data)

        if (seed != None):
            self.set_seed(seed)

        # remember function pointers
        self.registered_functions = []

    def __del__(self):
        __libpush__.push_destroy(self._push)

    def loads(self, xml_data):
        if (xml_data != None):
            __libpush__.push_load_state(self._push, xml_data)

    def dumps(self):
        ptr = __libpush__.push_dump_state(self._push)
        xml_data = cast(ptr, c_char_p).value
        __libpush__.push_free(ptr)
        return xml_data

    def set_seed(self, seed):
        __libpush__.push_rand_set_seed(self._push, seed)

    def step(self, n = 1):
        return __libpush__.push_steps(self._push, n)

    def done(self):
        return bool(__libpush__.push_done(self._push))

    def set_seed(self, seed):
        __libpush__.push_rand_set_seed(self._push, seed)

    def register_instruction(self, name, func, args = (), kwargs = {}):
        def callback(push_ptr, userdata):
            func(*args, **kwargs)

        # NOTE: remember function pointer or python will throw it away and
        #       PUSH will crash.
        fp = push_instr_func_t(callback)
        self.registered_functions.append(fp)
        __libpush__.push_instr_reg(self._push, name, fp, None)

    def to_code(self, c): # TODO: test
        code = __libpush__.push_code_new()
        for v in c:
            p = self.to_val(v)
            __libpush__.push_code_append(code, p)
        return code

    def to_val(self, v, t = None):
        def guess_type(v):
            T = {bool: PUSH_TYPE_BOOL,
                 list: PUSH_TYPE_CODE,
                 int: PUSH_TYPE_INT,
                 instr: PUSH_TYPE_INSTR,
                 str: PUSH_TYPE_NAME,
                 float: PUSH_TYPE_REAL}
            t = type(v)
            if (t in T):
                return T[t]
            else:
                raise TypeError("Invalid type: %s (%s)"%(repr(v), t))

        val = __libpush__.push_val_new(self._push, PUSH_TYPE_NONE)
        if (t == None):
            t = guess_type(v)
        val[0].type = t
            
        if (t == PUSH_TYPE_BOOL):
            val[0].v.bool = push_bool_t(v)
        elif (t == PUSH_TYPE_CODE):
            val[0].v.code = self.to_code(v)
        elif (t == PUSH_TYPE_INT):
            val[0].v.int = push_int_t(v)
        elif (t == PUSH_TYPE_INSTR):
            i = __libpush__.push_instr_lookup(self._push, v)
            if (i):
                val[0].v.instr = i
            else:
                __libpush__.push_val_destroy(val)
                raise ValueError("Unknown instruction: %s"%repr(v))
        elif (t == PUSH_TYPE_NAME):
            val[0].v.name = __libpush__.push_intern_name(self._push, v)
        elif (t == PUSH_TYPE_REAL):
            val[0].v.real = push_real_t(v)
        else:
            raise TypeError("Invalid PUSH type: %s (%s)"%(repr(v), str(type(v))))
        return val

    def from_code(self, code):
        if (code):
            c = []
            f = push_code_iter_func_t(lambda val, u: c.append(self.from_val(val)))
            __libpush__.push_code_foreach(code, f)
            return c
        else:
            return None

    def from_val(self, val):
        if (val):
            t = val[0].type
            if (t == PUSH_TYPE_BOOL):
                return bool(val[0].v.bool)
            elif (t == PUSH_TYPE_CODE):
                return self.from_code(val[0].v.code)
            elif (t == PUSH_TYPE_INT):
                return val[0].v.int
            elif (t == PUSH_TYPE_INSTR):
                return instr(cast(val[0].v.instr[0].name, c_char_p).value.decode())
            elif (t == PUSH_TYPE_NAME):
                return cast(val[0].v.name, c_char_p).value.decode()
            elif (t == PUSH_TYPE_REAL):
                return val[0].v.real
        return None

    def pop_stack(self, stack, n = None, peek = False):
        if (n == None):
            f = __libpush__.push_stack_peek if peek else __libpush__.push_stack_pop
            val = f(stack)
        else:
            f = __libpush__.push_stack_peek_nth if peek else __libpush__.push_stack_pop_nth
            val = f(stack, n)
        return self.from_val(val)

    def pop_bool(self, n = None, peek = False):
        return self.pop_stack(self._push[0].bool, n, peek)
    def pop_code(self, n = None, peek = False):
        return self.pop_stack(self._push[0].code, n, peek)
    def pop_exec(self, n = None, peek = False):
        return self.pop_stack(self._push[0].exec, n, peek)
    def pop_int(self, n = None, peek = False):
        return self.pop_stack(self._push[0].int, n, peek)
    def pop_name(self, n = None, peek = False):
        return self.pop_stack(self._push[0].name, n, peek)
    def pop_real(self, n = None, peek = False):
        return self.pop_stack(self._push[0].real, n, peek)

    def push_stack(self, stack, v, n = None, t = None):
        val = self.to_val(v, t)
        if (n == None):
            __libpush__.push_stack_push(stack, val)
        else:
            __libpush__.push_stack_push_nth(stack, n, val)

    def push_bool(self, v, n = None):
        self.push_stack(self._push[0].bool, v, n, PUSH_TYPE_BOOL)
    def push_code(self, v, n = None):
        self.push_stack(self._push[0].code, v, n)
    def push_exec(self, v, n = None):
        self.push_stack(self._push[0].exec, v, n)
    def push_int(self, v, n = None):
        self.push_stack(self._push[0].int, v, n, PUSH_TYPE_INT)
    def push_name(self, v, n = None):
        self.push_stack(self._push[0].name, v, n, PUSH_TYPE_NAME)
    def push_real(self, v, n = None):
        self.push_stack(self._push[0].real, v, n, PUSH_TYPE_REAL)

    def set_config(self, name, val):
        __libpush__.push_config_set(self._push, name, self.to_val(val))

    def get_config(self, name):
        return self.from_val(__libpush__.push_config_get(self._push, name))

    def interrupt(self, i = 1):
        self._push[0].interrupt = i

    def rand_code(self, size):
        _size = push_int_t(size)
        code = __libpush__.push_rand_val(self._push, PUSH_TYPE_CODE, byref(_size), True)
        __libpush__.push_stack_push(self._push[0].code, code)
        __libpush__.push_stack_push(self._push[0].exec, code)


class Process(Interpreter):
    def __init__(self, id, xml_data, syscalls = {}):
        Interpreter.__init__(self)
        self.id = id

        for name, func in syscalls.items():
            self.register_instruction("SYS.%s"%name, func, (self,))

        self.loads(xml_data)


class VM:
    STEPS_PER_CYCLE = 128

    def __init__(self, process_class = Process):
        self.process_class = process_class

        self.processes = {}
        self.syscalls = {}

        # search instance for syscalls
        for k in dir(self):
            c = self.__getattribute__(k)
            if (hasattr(c, "__call__")):
                try:
                    name = c.__push_syscall__
                    self.syscalls[name] = c
                except AttributeError:
                    pass

    def new(self, id, xml_data):
        proc = self.process_class(id, xml_data, self.syscalls)
        self.processes[id] = proc
        return proc
        
    def kill(self, id):
        del self.processes[id]

    def get_state(self, id):
        # NOTE: I don't think we have to lock something. Since python has the
        #       GIL and therefore calls to C code (push_step) are atomic.
        proc = self.processes[id]
        return proc.dumps()

    def step(self):
        for proc in list(self.processes.values()):
            proc.step(self.STEPS_PER_CYCLE)
        self.remove_dead_processes()

    def run(self):
        while (True):
            self.step()

    def remove_dead_processes(self):
        for id, proc in list(self.processes.items()):
            if (proc.done()):
                self.kill(id)

# syscall decorator
def syscall(name = None):
    def decorator(method):
        method.__push_syscall__ = func.__name__.upper() if (name == None) else name
        return method
    return decorator


if (__name__ == "__main__"):
    i = Interpreter()
    print(i.dumps().decode())

