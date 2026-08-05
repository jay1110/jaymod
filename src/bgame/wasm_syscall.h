#ifndef BGAME_WASM_SYSCALL_H
#define BGAME_WASM_SYSCALL_H

///////////////////////////////////////////////////////////////////////////////

/* WebAssembly system call adapter.
 *
 * The native builds receive a variadic syscall pointer through dllEntry().
 * WebAssembly has no compatible calling convention for that, so an Emscripten
 * SIDE_MODULE is handed an array based syscall pointer instead:
 *
 *     intptr_t (*syscall)( intptr_t* args )
 *
 * with args[0] holding the syscall number and args[1..] the arguments. To keep
 * the ~450 existing trap_* call sites unchanged, dllEntry() stores the engine
 * pointer here and hands the module a variadic forwarder which marshals its
 * arguments into such an array.
 *
 * This header must only be included by a module's *_syscalls.cpp; each module
 * keeps its own copy of the engine pointer.
 */

#if !defined( JAYMOD_WASM )
#    error "wasm_syscall.h is only valid on WebAssembly builds."
#endif

#include <stdarg.h>
#include <stdint.h>

///////////////////////////////////////////////////////////////////////////////

// Largest number of arguments any ET syscall takes, excluding the syscall
// number itself. BOTLIB_AAS_PREDICT_CLIENT_MOVEMENT, the widest one, uses 13.
#define WASM_SYSCALL_MAXARGS 16

typedef intptr_t (*WasmSyscallPtr)( intptr_t* );

static WasmSyscallPtr wasmSyscallEngine = 0;

static int QDECL
wasmSyscallForward( int cmd, ... )
{
    intptr_t args[WASM_SYSCALL_MAXARGS + 1];
    args[0] = cmd;

    va_list ap;
    va_start( ap, cmd );
    for (int i = 1; i <= WASM_SYSCALL_MAXARGS; i++)
        args[i] = va_arg( ap, intptr_t );
    va_end( ap );

    return (int)wasmSyscallEngine( args );
}

///////////////////////////////////////////////////////////////////////////////

#endif // BGAME_WASM_SYSCALL_H
