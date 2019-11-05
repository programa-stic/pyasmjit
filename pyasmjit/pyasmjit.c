// Copyright (c) 2014, Fundacion Dr. Manuel Sadosky
// All rights reserved.

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:

// 1. Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.

// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <Python.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

/* TODO: Memory: See where to locate this information. */
void * arm_mem_pool = 0;
unsigned int arm_mem_pool_size = 0;

typedef struct {
    unsigned long eax;      // 0
    unsigned long ebx;      // 1
    unsigned long ecx;      // 2
    unsigned long edx;      // 3
    unsigned long edi;      // 4
    unsigned long esi;      // 5
    unsigned long ebp;      // 6
    unsigned long esp;      // 7
    unsigned long eip;      // 8
    unsigned long eflags;   // 9
} x86_context_t;

typedef struct {
    unsigned long rax;      // 0
    unsigned long rbx;      // 1
    unsigned long rcx;      // 2
    unsigned long rdx;      // 3
    unsigned long rdi;      // 4
    unsigned long rsi;      // 5
    unsigned long rbp;      // 6
    unsigned long rsp;      // 7
    unsigned long rip;      // 8
    unsigned long r8;       // 9
    unsigned long r9;       // 10
    unsigned long r10;      // 11
    unsigned long r11;      // 12
    unsigned long r12;      // 13
    unsigned long r13;      // 14
    unsigned long r14;      // 15
    unsigned long r15;      // 16
    unsigned long rflags;   // 17
} x86_64_context_t;

typedef struct {
    unsigned long r0;       // 0
    unsigned long r1;       // 1
    unsigned long r2;       // 2
    unsigned long r3;       // 3
    unsigned long r4;       // 4
    unsigned long r5;       // 5
    unsigned long r6;       // 6
    unsigned long r7;       // 7
    unsigned long r8;       // 8
    unsigned long r9;       // 9
    unsigned long r10;      // 10
    unsigned long r11;      // 11
    unsigned long r12;      // 12
    unsigned long r13;      // 13
    unsigned long r14;      // 14
    unsigned long r15;      // 15
    unsigned long apsr;     // 16
} arm_context_t;

void
x86_print_context(x86_context_t *ctx)
{
    printf("ctx @ %p\n", ctx);

    printf("   eax : 0x%016lx @ %p\n",    ctx->eax,    &ctx->eax);
    printf("   ebx : 0x%016lx @ %p\n",    ctx->ebx,    &ctx->ebx);
    printf("   ecx : 0x%016lx @ %p\n",    ctx->ecx,    &ctx->ecx);
    printf("   edx : 0x%016lx @ %p\n",    ctx->edx,    &ctx->edx);
    printf("   edi : 0x%016lx @ %p\n",    ctx->edi,    &ctx->edi);
    printf("   esi : 0x%016lx @ %p\n",    ctx->esi,    &ctx->esi);
    printf("   ebp : 0x%016lx @ %p\n",    ctx->ebp,    &ctx->ebp);
    printf("   esp : 0x%016lx @ %p\n",    ctx->esp,    &ctx->esp);
    printf("   eip : 0x%016lx @ %p\n",    ctx->eip,    &ctx->eip);
    printf("eflags : 0x%016lx @ %p\n", ctx->eflags, &ctx->eflags);
}

void
x86_64_print_context(x86_64_context_t *ctx)
{
    printf("ctx @ %p\n", ctx);

    printf("   rax : 0x%016lx @ %p\n",    ctx->rax,    &ctx->rax);
    printf("   rbx : 0x%016lx @ %p\n",    ctx->rbx,    &ctx->rbx);
    printf("   rcx : 0x%016lx @ %p\n",    ctx->rcx,    &ctx->rcx);
    printf("   rdx : 0x%016lx @ %p\n",    ctx->rdx,    &ctx->rdx);
    printf("   rdi : 0x%016lx @ %p\n",    ctx->rdi,    &ctx->rdi);
    printf("   rsi : 0x%016lx @ %p\n",    ctx->rsi,    &ctx->rsi);
    printf("   rbp : 0x%016lx @ %p\n",    ctx->rbp,    &ctx->rbp);
    printf("   rsp : 0x%016lx @ %p\n",    ctx->rsp,    &ctx->rsp);
    printf("   rip : 0x%016lx @ %p\n",    ctx->rip,    &ctx->rip);
    printf("    r8 : 0x%016lx @ %p\n",     ctx->r8,     &ctx->r8);
    printf("    r9 : 0x%016lx @ %p\n",     ctx->r9,     &ctx->r9);
    printf("   r10 : 0x%016lx @ %p\n",    ctx->r10,    &ctx->r10);
    printf("   r11 : 0x%016lx @ %p\n",    ctx->r11,    &ctx->r11);
    printf("   r12 : 0x%016lx @ %p\n",    ctx->r12,    &ctx->r12);
    printf("   r13 : 0x%016lx @ %p\n",    ctx->r13,    &ctx->r13);
    printf("   r14 : 0x%016lx @ %p\n",    ctx->r14,    &ctx->r14);
    printf("   r15 : 0x%016lx @ %p\n",    ctx->r15,    &ctx->r15);
    printf("rflags : 0x%016lx @ %p\n", ctx->rflags, &ctx->rflags);
}

void
arm_print_context(arm_context_t *ctx)
{
    printf("ctx @ %p\n", ctx);

    printf("  r0 : 0x%08lx @ %p\n",   ctx->r0,   &ctx->r0);
    printf("  r1 : 0x%08lx @ %p\n",   ctx->r1,   &ctx->r1);
    printf("  r2 : 0x%08lx @ %p\n",   ctx->r2,   &ctx->r2);
    printf("  r3 : 0x%08lx @ %p\n",   ctx->r3,   &ctx->r3);
    printf("  r4 : 0x%08lx @ %p\n",   ctx->r4,   &ctx->r4);
    printf("  r5 : 0x%08lx @ %p\n",   ctx->r5,   &ctx->r5);
    printf("  r6 : 0x%08lx @ %p\n",   ctx->r6,   &ctx->r6);
    printf("  r7 : 0x%08lx @ %p\n",   ctx->r7,   &ctx->r7);
    printf("  r8 : 0x%08lx @ %p\n",   ctx->r8,   &ctx->r8);
    printf("  r9 : 0x%08lx @ %p\n",   ctx->r9,   &ctx->r9);
    printf(" r10 : 0x%08lx @ %p\n",  ctx->r10,  &ctx->r10);
    printf(" r11 : 0x%08lx @ %p\n",  ctx->r11,  &ctx->r11);
    printf(" r12 : 0x%08lx @ %p\n",  ctx->r12,  &ctx->r12);
    printf(" r13 : 0x%08lx @ %p\n",  ctx->r13,  &ctx->r13);
    printf(" r14 : 0x%08lx @ %p\n",  ctx->r14,  &ctx->r14);
    printf(" r15 : 0x%08lx @ %p\n",  ctx->r15,  &ctx->r15);
    printf("apsr : 0x%08lx @ %p\n", ctx->apsr, &ctx->apsr);
}

unsigned long
load_register_from_dict(PyObject *dict, const char *reg, unsigned long _default)
{
    unsigned long rv = _default;

    if (PyDict_Contains(dict, Py_BuildValue("s", reg)) == 1) {
        rv = PyLong_AsUnsignedLong(PyDict_GetItemString(dict, reg));
    }

    return rv;
}

void
x86_load_context_from_dict(PyObject *dict, x86_context_t *ctx)
{
    ctx->eax    = load_register_from_dict(dict,    "eax",     0);
    ctx->ebx    = load_register_from_dict(dict,    "ebx",     0);
    ctx->ecx    = load_register_from_dict(dict,    "ecx",     0);
    ctx->edx    = load_register_from_dict(dict,    "edx",     0);
    ctx->edi    = load_register_from_dict(dict,    "edi",     0);
    ctx->esi    = load_register_from_dict(dict,    "esi",     0);
    ctx->ebp    = load_register_from_dict(dict,    "ebp",     0);
    ctx->esp    = load_register_from_dict(dict,    "esp",     0);
    ctx->eip    = load_register_from_dict(dict,    "eip",     0);
    /* TODO: Check the default value of eflags. */
    ctx->eflags = load_register_from_dict(dict, "eflags", 0x202);
}

void
x86_64_load_context_from_dict(PyObject *dict, x86_64_context_t *ctx)
{
    ctx->rax    = load_register_from_dict(dict,    "rax",     0);
    ctx->rbx    = load_register_from_dict(dict,    "rbx",     0);
    ctx->rcx    = load_register_from_dict(dict,    "rcx",     0);
    ctx->rdx    = load_register_from_dict(dict,    "rdx",     0);
    ctx->rdi    = load_register_from_dict(dict,    "rdi",     0);
    ctx->rsi    = load_register_from_dict(dict,    "rsi",     0);
    ctx->rbp    = load_register_from_dict(dict,    "rbp",     0);
    ctx->rsp    = load_register_from_dict(dict,    "rsp",     0);
    ctx->rip    = load_register_from_dict(dict,    "rip",     0);
    ctx->r8     = load_register_from_dict(dict,     "r8",     0);
    ctx->r9     = load_register_from_dict(dict,     "r9",     0);
    ctx->r10    = load_register_from_dict(dict,    "r10",     0);
    ctx->r11    = load_register_from_dict(dict,    "r11",     0);
    ctx->r12    = load_register_from_dict(dict,    "r12",     0);
    ctx->r13    = load_register_from_dict(dict,    "r13",     0);
    ctx->r14    = load_register_from_dict(dict,    "r14",     0);
    ctx->r15    = load_register_from_dict(dict,    "r15",     0);
    ctx->rflags = load_register_from_dict(dict, "rflags", 0x202);
}

void
arm_load_context_from_dict(PyObject *dict, arm_context_t *ctx)
{
    ctx->r0   = load_register_from_dict(dict,   "r0", 0);
    ctx->r1   = load_register_from_dict(dict,   "r1", 0);
    ctx->r2   = load_register_from_dict(dict,   "r2", 0);
    ctx->r3   = load_register_from_dict(dict,   "r3", 0);
    ctx->r4   = load_register_from_dict(dict,   "r4", 0);
    ctx->r5   = load_register_from_dict(dict,   "r5", 0);
    ctx->r6   = load_register_from_dict(dict,   "r6", 0);
    ctx->r7   = load_register_from_dict(dict,   "r7", 0);
    ctx->r8   = load_register_from_dict(dict,   "r8", 0);
    ctx->r9   = load_register_from_dict(dict,   "r9", 0);
    ctx->r10  = load_register_from_dict(dict,  "r10", 0);
    ctx->r11  = load_register_from_dict(dict,  "r11", 0);
    ctx->r12  = load_register_from_dict(dict,  "r12", 0);
    ctx->r13  = load_register_from_dict(dict,  "r13", 0);
    ctx->r14  = load_register_from_dict(dict,  "r14", 0);
    ctx->r15  = load_register_from_dict(dict,  "r15", 0);
    ctx->apsr = load_register_from_dict(dict, "apsr", 0);
}

void
x86_save_context_to_dict(PyObject *dict, x86_context_t *ctx)
{
    PyDict_SetItemString(dict,    "eax", Py_BuildValue("I",    ctx->eax));
    PyDict_SetItemString(dict,    "ebx", Py_BuildValue("I",    ctx->ebx));
    PyDict_SetItemString(dict,    "ecx", Py_BuildValue("I",    ctx->ecx));
    PyDict_SetItemString(dict,    "edx", Py_BuildValue("I",    ctx->edx));
    PyDict_SetItemString(dict,    "edi", Py_BuildValue("I",    ctx->edi));
    PyDict_SetItemString(dict,    "esi", Py_BuildValue("I",    ctx->esi));
    PyDict_SetItemString(dict,    "ebp", Py_BuildValue("I",    ctx->ebp));
    PyDict_SetItemString(dict,    "esp", Py_BuildValue("I",    ctx->esp));
    PyDict_SetItemString(dict,    "eip", Py_BuildValue("I",    ctx->eip));
    PyDict_SetItemString(dict, "eflags", Py_BuildValue("I", ctx->eflags));
}

void
x86_64_save_context_to_dict(PyObject *dict, x86_64_context_t *ctx)
{
    PyDict_SetItemString(dict,    "rax", Py_BuildValue("k",    ctx->rax));
    PyDict_SetItemString(dict,    "rbx", Py_BuildValue("k",    ctx->rbx));
    PyDict_SetItemString(dict,    "rcx", Py_BuildValue("k",    ctx->rcx));
    PyDict_SetItemString(dict,    "rdx", Py_BuildValue("k",    ctx->rdx));
    PyDict_SetItemString(dict,    "rdi", Py_BuildValue("k",    ctx->rdi));
    PyDict_SetItemString(dict,    "rsi", Py_BuildValue("k",    ctx->rsi));
    PyDict_SetItemString(dict,    "rbp", Py_BuildValue("k",    ctx->rbp));
    PyDict_SetItemString(dict,    "rsp", Py_BuildValue("k",    ctx->rsp));
    PyDict_SetItemString(dict,    "rip", Py_BuildValue("k",    ctx->rip));
    PyDict_SetItemString(dict,     "r8", Py_BuildValue("k",     ctx->r8));
    PyDict_SetItemString(dict,     "r9", Py_BuildValue("k",     ctx->r9));
    PyDict_SetItemString(dict,    "r10", Py_BuildValue("k",    ctx->r10));
    PyDict_SetItemString(dict,    "r11", Py_BuildValue("k",    ctx->r11));
    PyDict_SetItemString(dict,    "r12", Py_BuildValue("k",    ctx->r12));
    PyDict_SetItemString(dict,    "r13", Py_BuildValue("k",    ctx->r13));
    PyDict_SetItemString(dict,    "r14", Py_BuildValue("k",    ctx->r14));
    PyDict_SetItemString(dict,    "r15", Py_BuildValue("k",    ctx->r15));
    PyDict_SetItemString(dict, "rflags", Py_BuildValue("k", ctx->rflags));
}

void
arm_save_context_to_dict(PyObject *dict, arm_context_t *ctx)
{
    PyDict_SetItemString(dict,   "r0", Py_BuildValue("I",  ctx->r0));
    PyDict_SetItemString(dict,   "r1", Py_BuildValue("I",  ctx->r1));
    PyDict_SetItemString(dict,   "r2", Py_BuildValue("I",  ctx->r2));
    PyDict_SetItemString(dict,   "r3", Py_BuildValue("I",  ctx->r3));
    PyDict_SetItemString(dict,   "r4", Py_BuildValue("I",  ctx->r4));
    PyDict_SetItemString(dict,   "r5", Py_BuildValue("I",  ctx->r5));
    PyDict_SetItemString(dict,   "r6", Py_BuildValue("I",  ctx->r6));
    PyDict_SetItemString(dict,   "r7", Py_BuildValue("I",  ctx->r7));
    PyDict_SetItemString(dict,   "r8", Py_BuildValue("I",  ctx->r8));
    PyDict_SetItemString(dict,   "r9", Py_BuildValue("I",  ctx->r9));
    PyDict_SetItemString(dict,  "r10", Py_BuildValue("I", ctx->r10));
    PyDict_SetItemString(dict,  "r11", Py_BuildValue("I", ctx->r11));
    PyDict_SetItemString(dict,  "r12", Py_BuildValue("I", ctx->r12));
    PyDict_SetItemString(dict,  "r13", Py_BuildValue("I", ctx->r13));
    PyDict_SetItemString(dict,  "r14", Py_BuildValue("I", ctx->r14));
    PyDict_SetItemString(dict,  "r15", Py_BuildValue("I", ctx->r15));
    PyDict_SetItemString(dict, "apsr", Py_BuildValue("I", ctx->apsr));
}

unsigned long
x86_run(unsigned char *data, unsigned int size, x86_context_t *ctx) {
    /* Allocate executable memory */
    void *mem = mmap(
        NULL,
        size,
        PROT_WRITE | PROT_EXEC,
#if defined (__x86_64__)
        MAP_ANONYMOUS | MAP_PRIVATE | MAP_32BIT,
#else
        MAP_ANONYMOUS | MAP_PRIVATE,
#endif
        -1,
        0
    );

    /* Return on error */
    if (mem == MAP_FAILED) {
        return -1;
    }

    /* Copy binary code into allocated memory */
    memcpy(mem, data, size);

    /* Typecast allocated memory to a function pointer */
    void (*func) (x86_context_t *) = mem;

    /* Run code */
    func(ctx);

    /* Free up allocated memory */
    munmap(mem, size);

    return 0;
}

unsigned long
x86_64_run(unsigned char *data, unsigned int size, x86_64_context_t *ctx) {
    /* Allocate executable memory */
    void *mem = mmap(
        NULL,
        size,
        PROT_WRITE | PROT_EXEC,
#if defined (__x86_64__)
        MAP_ANONYMOUS | MAP_PRIVATE | MAP_32BIT,
#else
        MAP_ANONYMOUS | MAP_PRIVATE,
#endif
        -1,
        0
    );

    /* Return on error */
    if (mem == MAP_FAILED) {
        return -1;
    }

    /* Copy binary code into allocated memory */
    memcpy(mem, data, size);

    /* Typecast allocated memory to a function pointer */
    void (*func) (x86_64_context_t *) = mem;

    /* Run code */
    func(ctx);

    /* Free up allocated memory */
    munmap(mem, size);

    return 0;
}

unsigned long
arm_run(unsigned char *data, unsigned int size, arm_context_t *ctx) {
    /* Allocate executable memory */
    void *mem = mmap(
        NULL,
        size,
        PROT_WRITE | PROT_EXEC,
        MAP_ANONYMOUS | MAP_PRIVATE,
        -1,
        0
    );

    /* Return on error */
    if (mem == MAP_FAILED) {
        return -1;
    }

    /* Copy binary code into allocated memory */
    memcpy(mem, data, size);

    /* Typecast allocated memory to a function pointer */
    void (*func) (arm_context_t *) = mem;

    /* Run code */
    func(ctx);

    /* Free up allocated memory */
    munmap(mem, size);

    return 0;
}

/*
 * Function to be called from Python
 */
static PyObject *
pyasmjit_x86_jit(PyObject * self, PyObject * args)
{
    unsigned char    *data;
    unsigned int      size;
    unsigned int      rv;
    PyObject         *dict_in;
    PyObject         *dict_out = PyDict_New();
    x86_context_t  ctx;

    /* Check newly created dict is not null */
    if (dict_out == NULL)
        return Py_BuildValue("I{}", -1);

    /* Parse input arguments */
    PyArg_ParseTuple(args, "s#O!", &data, &size, &PyDict_Type, &dict_in);

    /* Load context from input dictionary */
    x86_load_context_from_dict(dict_in, &ctx);

    /* Run input code */
    rv = x86_run(data, size, &ctx);

    /* Save context to output dictionary */
    x86_save_context_to_dict(dict_out, &ctx);

    /* Build return value and return */
    return Py_BuildValue("IO", rv, dict_out);
}

/*
 * Function to be called from Python
 */
static PyObject *
pyasmjit_x86_64_jit(PyObject * self, PyObject * args)
{
    unsigned char    *data;
    unsigned int      size;
    unsigned int      rv;
    PyObject         *dict_in;
    PyObject         *dict_out = PyDict_New();
    x86_64_context_t  ctx;

    /* Check newly created dict is not null */
    if (dict_out == NULL)
        return Py_BuildValue("I{}", -1);

    /* Parse input arguments */
    PyArg_ParseTuple(args, "s#O!", &data, &size, &PyDict_Type, &dict_in);

    /* Load context from input dictionary */
    x86_64_load_context_from_dict(dict_in, &ctx);

    /* Run input code */
    rv = x86_64_run(data, size, &ctx);

    /* Save context to output dictionary */
    x86_64_save_context_to_dict(dict_out, &ctx);

    /* Build return value and return */
    return Py_BuildValue("IO", rv, dict_out);
}

/*
 * Function to be called from Python
 */
static PyObject *
pyasmjit_arm_jit(PyObject * self, PyObject * args)
{
    unsigned char   *data;
    unsigned int     size;
    unsigned int     rv;
    PyObject        *dict_in;
    PyObject        *dict_out = PyDict_New();
    arm_context_t    ctx;

    /* Check newly created dict is not null */
    if (dict_out == NULL)
        return Py_BuildValue("I{}", -1);

    /* Parse input arguments */
    PyArg_ParseTuple(args, "s#O!", &data, &size, &PyDict_Type, &dict_in);

    /* Load context from input dictionary */
    arm_load_context_from_dict(dict_in, &ctx);

    /* Run input code */
    rv = arm_run(data, size, &ctx);

    /* Save context to output dictionary */
    arm_save_context_to_dict(dict_out, &ctx);

    /* TODO: Check that if pool is null it doesn't break the function. */
#if (PY_MAJOR_VERSION == 3)
    PyObject *py_buffer = Py_BuildValue("y#", arm_mem_pool, arm_mem_pool_size);
#else
    PyObject *py_buffer = Py_BuildValue("s#", arm_mem_pool, arm_mem_pool_size);
#endif

    /* Build return value and return */
    return Py_BuildValue("IOO", rv, dict_out, py_buffer);
}

/*
 * Function to be called from Python
 */
static PyObject *
pyasmjit_arm_alloc(PyObject * self, PyObject * args)
{
    unsigned int size;

    /* Parse input arguments */
    PyArg_ParseTuple(args, "I", &size);

    arm_mem_pool_size = size;

    /* Allocate executable memory */
    arm_mem_pool = mmap(
        NULL,
        size,
        PROT_WRITE,
        MAP_ANONYMOUS | MAP_PRIVATE,
        -1,
        0
    );

    memset(arm_mem_pool, 0, arm_mem_pool_size);

    /* Build return value and return */
    return Py_BuildValue("I", arm_mem_pool);
}

/*
 * Function to be called from Python
 */
static PyObject *
pyasmjit_arm_free(PyObject * self, PyObject * args)
{
    /* TODO: Allow multiple memory pools. */
    if (arm_mem_pool) {
        munmap(arm_mem_pool, arm_mem_pool_size);
        arm_mem_pool = 0;
    }

    return Py_BuildValue("I", 0);
}

/*
 * Bind Python function names to our C functions
 */
static PyMethodDef pyasmjit_methods[] = {
    {"x86_jit", pyasmjit_x86_jit, METH_VARARGS, "JIT execute x86 code"},
    {"x86_64_jit", pyasmjit_x86_64_jit, METH_VARARGS, "JIT execute x86_64 code"},
    {"arm_jit", pyasmjit_arm_jit, METH_VARARGS, "JIT execute ARMv7 code"},
    {"arm_alloc", pyasmjit_arm_alloc, METH_VARARGS, "Map ARM memory"},
    {"arm_free", pyasmjit_arm_free, METH_VARARGS, "Unmap ARM memory"},
    {NULL, NULL, 0, NULL}
};

#if (PY_MAJOR_VERSION == 3)
/*
 * Module definition
 * The arguments of this structure tell Python what to call your extension,
 * what it's methods are and where to look for it's method definitions
 */
static struct PyModuleDef pyasmjit_definition = {
    PyModuleDef_HEAD_INIT,
    "pyasmjit",
    "JIT module for pyasmjit",
    -1,
    pyasmjit_methods
};

/*
 * Module initialization
 * Python calls this function when importing your extension. It is important
 * that this function is named PyInit_[[your_module_name]] exactly, and matches
 * the name keyword argument in setup.py's setup() call.
 */
PyMODINIT_FUNC PyInit_pyasmjit(void) {
    Py_Initialize();
    return PyModule_Create(&pyasmjit_definition);
}

#else /* Python 2.x */

static const char *pyasmjit_docstring = "JIT module for pyasmjit";

/*
 * Python calls this to let us initialize our module
 */
PyMODINIT_FUNC
initpyasmjit(void)
{
    (void) Py_InitModule3("pyasmjit", pyasmjit_methods, pyasmjit_docstring);
}

#endif
