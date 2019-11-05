# Copyright (c) 2014, Fundacion Dr. Manuel Sadosky
# All rights reserved.

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:

# 1. Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.

# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.

# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

from __future__ import absolute_import

import os
import subprocess
import tempfile

from builtins import bytes

from . import pyasmjit


x86_template_assembly = """\
;; Make sure to compile in 32 bits
BITS 32

;; Build stack frame
push ebp
mov ebp, esp

;; Save registers
pushad

;; Store the pointer to context in EAX
mov eax, [ebp+8]

;; Load context (registers)
mov ebx, [eax+ 1*4]
mov ecx, [eax+ 2*4]
mov edx, [eax+ 3*4]
mov edi, [eax+ 4*4]
mov esi, [eax+ 5*4]

;; TODO: Set ebp, esp and eip registers

; mov ebp, [eax+ 6*4]
; mov esp, [eax+ 7*4]
; mov eip, [eax+ 8*4]

;; Load context (flags)
push dword [eax+9*4]
popfd

;; Load eax value
mov eax, [eax+ 0*4]

;; Run code
{code}

;; Save current eax value and restore ptr to context
push eax
mov eax, [ebp+8]

;; Save context
mov [eax+ 1*4], ebx
mov [eax+ 2*4], ecx
mov [eax+ 3*4], edx
mov [eax+ 4*4], edi
mov [eax+ 5*4], esi

; mov [eax+ 6*4], ebp
; mov [eax+ 7*4], esp
; mov [eax+ 8*4], eip

;; Save context (flags)
pushfd
pop dword [eax+9*4]

;; Copy eax to ebx
mov ebx, eax

;; Restore current eax value
pop eax

;; Save eax value
mov [ebx+ 0*4], eax

;; Restore registers
popad

pop ebp
ret
"""

x86_64_template_assembly = """\
;; Make sure to compile in 64 bits
BITS 64

;; Build stack frame
push rbp
mov rbp, rsp

;; Allocate stack memory
sub rsp, 8

;; Save registers
push rax
push rbx
push rcx
push rdx
push rdi
push rsi
push r8
push r9
push r10
push r11
push r12
push r13
push r14
push r15

;; Save ptr to context
mov [rbp-0x08], rdi

;; Load context (flags)
push qword [rdi+6*8]
popfq

;; Load context (registers)
mov rax, [rdi+ 0*8]
mov rbx, [rdi+ 1*8]
mov rcx, [rdi+ 2*8]
mov rdx, [rdi+ 3*8]
mov rsi, [rdi+ 5*8]

;; TODO: Set rbp, rsp and rip registers

; mov rbp, [rdi+ 6*8]
; mov rsp, [rdi+ 7*8]
; mov rip, [rdi+ 8*8]

mov r8,  [rdi+ 9*8]
mov r9,  [rdi+10*8]
mov r10, [rdi+11*8]
mov r11, [rdi+12*8]
mov r12, [rdi+13*8]
mov r13, [rdi+14*8]
mov r14, [rdi+15*8]
mov r15, [rdi+16*8]

;; Load context (flags)
push qword [rdi+17*8]
popfq

;; Load rdi value
mov rdi, [rdi+ 4*8]

;; Run code
{code}

;; Save current rdi value and restore ptr to context
push rdi
mov rdi, [rbp-0x08]

;; Save context
mov [rdi+ 0*8], rax
mov [rdi+ 1*8], rbx
mov [rdi+ 2*8], rcx
mov [rdi+ 3*8], rdx
mov [rdi+ 5*8], rsi

; mov [rdi+ 6*8], rbp
; mov [rdi+ 7*8], rsp
; mov [rdi+ 8*8], rip

mov [rdi+ 9*8], r8
mov [rdi+10*8], r9
mov [rdi+11*8], r10
mov [rdi+12*8], r11
mov [rdi+13*8], r12
mov [rdi+14*8], r13
mov [rdi+15*8], r14
mov [rdi+16*8], r15

;; Save context (flags)
pushfq
pop qword [rdi+17*8]

;; Copy rdi to rsi
mov rsi, rdi

;; Restore current rdi value
pop rdi

;; Save rdi value
mov [rsi+ 4*8], rdi

;; Restore registers
pop r15
pop r14
pop r13
pop r12
pop r11
pop r10
pop r9
pop r8
pop rsi
pop rdi
pop rdx
pop rcx
pop rbx
pop rax

;; Free up stack memory
add rsp, 8

pop rbp
ret
"""

arm_template_assembly = """\
/* Save registers */
push {{r0 - r12, lr}}

/* Save flags (user mode) */
mrs r1, apsr
push {{r1}}

/* Save context pointer (redundant, it was saved before, but done for code clarity) */
push {{r0}}

/* Load context */
ldr r1, [r0, #(16 * 4)]
msr apsr_nzcvq, r1
ldm r0, {{r0 - r12}}

/* Run code */
{code}

/* TODO: lr is used as scratch register when saving the context so its value is not saved correctly */
/* Restore context pointer */
pop {{lr}}

/* Save context */
stm lr, {{r0 - r12}}
mrs r1, apsr
str r1, [lr, #(16 * 4)]

/* Restore flags */
pop {{r1}}
msr apsr_nzcvq, r1

/* Restore registers */
pop {{r0 - r12, lr}}

/* Return */
blx lr
"""


def x86_execute(assembly, context):
    # Initialize return values
    rc  = 0
    ctx = {}

    # Instantiate assembly template.
    assembly = x86_template_assembly.format(code=assembly)

    # Create temporary files for compilation.
    f_asm = tempfile.NamedTemporaryFile(delete=False)
    f_obj = tempfile.NamedTemporaryFile(delete=False)

    # Write assembly to a file.
    f_asm.write(bytes(assembly, 'ascii'))
    f_asm.close()

    # Run nasm.
    cmd_fmt = "nasm -f bin -o {0} {1}"
    cmd = cmd_fmt.format(f_obj.name, f_asm.name)
    return_code = subprocess.call(cmd, shell=True)

    # Check for assembler errors.
    if return_code == 0:
        # Read binary code.
        binary = f_obj.read()
        f_obj.close()

        # Run binary code.
        rc, ctx = pyasmjit.x86_jit(binary, context)
    else:
        rc = return_code

    # Remove temporary files.
    os.remove(f_asm.name)
    os.remove(f_obj.name)

    return rc, ctx


def x86_64_execute(assembly, context):
    # Initialize return values
    rc  = 0
    ctx = {}

    # Instantiate assembly template.
    assembly = x86_64_template_assembly.format(code=assembly)

    # Create temporary files for compilation.
    f_asm = tempfile.NamedTemporaryFile(delete=False)
    f_obj = tempfile.NamedTemporaryFile(delete=False)

    # Write assembly to a file.
    f_asm.write(bytes(assembly, 'ascii'))
    f_asm.close()

    # Run nasm.
    cmd_fmt = "nasm -f bin -o {0} {1}"
    cmd = cmd_fmt.format(f_obj.name, f_asm.name)
    return_code = subprocess.call(cmd, shell=True)

    # Check for assembler errors.
    if return_code == 0:
        # Read binary code.
        binary = f_obj.read()
        f_obj.close()

        # Run binary code.
        rc, ctx = pyasmjit.x86_64_jit(binary, context)
    else:
        rc = return_code

    # Remove temporary files.
    os.remove(f_asm.name)
    os.remove(f_obj.name)

    return rc, ctx


def arm_execute(assembly, context):
    # Initialize return values
    rc  = 0
    ctx = {}

    # Instantiate assembly template.
    assembly = arm_template_assembly.format(code=assembly)

    # Create temporary files for compilation.
    f_asm = tempfile.NamedTemporaryFile(delete=False)
    f_obj = tempfile.NamedTemporaryFile(delete=False)
    f_bin = tempfile.NamedTemporaryFile(delete=False)

    # Write assembly to a file.
    f_asm.write(bytes(assembly, 'ascii'))
    f_asm.close()

    # Run nasm.
    cmd_fmt = "gcc -c -x assembler {asm} -o {obj}; objcopy -O binary {obj} {bin};"
    cmd = cmd_fmt.format(asm=f_asm.name, obj=f_obj.name, bin=f_bin.name)
    return_code = subprocess.call(cmd, shell=True)

    # Check for assembler errors.
    if return_code == 0:
        # Read binary code.
        binary = f_bin.read()

        # Run binary code.
        rc, ctx, mem = pyasmjit.arm_jit(binary, context)
    else:
        rc = return_code

    # Remove temporary files.
    os.remove(f_asm.name)
    os.remove(f_obj.name)
    os.remove(f_bin.name)

    return rc, ctx, mem


def arm_alloc(size):
    return pyasmjit.arm_alloc(size)


def arm_free():
    return pyasmjit.arm_free()
