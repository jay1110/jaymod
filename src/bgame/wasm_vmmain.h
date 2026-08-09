#ifndef BGAME_WASM_VMMAIN_H
#define BGAME_WASM_VMMAIN_H

///////////////////////////////////////////////////////////////////////////////

/* WebAssembly module entry point ABI.
 *
 * Native builds are called through a C variadic/cdecl call, so the engine and
 * the module only have to agree on the arguments actually used. WebAssembly
 * has no such tolerance: an indirect call whose function type does not match
 * the callee's type exactly traps with "indirect call signature mismatch".
 *
 * The engine calls vmMain with a fixed signature of one command plus sixteen
 * intptr_t arguments (seventeen intptr_t parameters in total), so every module
 * must export exactly that signature and forward to its own handler.
 *
 * Each module also exports vmWasmAbi1() as a marker so the engine can tell
 * that the module was built against this ABI.
 */

#if !defined( JAYMOD_WASM )
#    error "wasm_vmmain.h is only valid on WebAssembly builds."
#endif

#include <stdint.h>

///////////////////////////////////////////////////////////////////////////////

#define JAYMOD_WASM_VMMAIN_ABI_ARGS                                           \
    intptr_t command, intptr_t arg0, intptr_t arg1, intptr_t arg2,            \
    intptr_t arg3, intptr_t arg4, intptr_t arg5, intptr_t arg6,               \
    intptr_t arg7, intptr_t arg8, intptr_t arg9, intptr_t arg10,              \
    intptr_t arg11, intptr_t arg12, intptr_t arg13, intptr_t arg14,           \
    intptr_t arg15

// Unused trailing arguments still have to be named and accepted; silence them.
#define JAYMOD_WASM_VMMAIN_UNUSED                                             \
    (void)arg12; (void)arg13; (void)arg14; (void)arg15;

// ABI marker: presence of this export tells the engine the module speaks the
// fixed-signature vmMain / array-based dllEntry ABI described above.
#define JAYMOD_WASM_ABI_MARKER                                                \
    extern "C" LF_PUBLIC int                                                  \
    vmWasmAbi1( void ) {                                                      \
        return 1;                                                             \
    }

///////////////////////////////////////////////////////////////////////////////

#endif // BGAME_WASM_VMMAIN_H
