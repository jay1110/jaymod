#ifndef BASE_CONFIG_H
#define BASE_CONFIG_H

///////////////////////////////////////////////////////////////////////////////

#if defined( JAYMOD_LINUX ) || defined( JAYMOD_LINUX64 ) || defined( JAYMOD_LINUX_AARCH64 )
#    include <base/linux/public.h>
#elif defined( JAYMOD_MINGW ) || defined( JAYMOD_MINGW64 )
#    include <base/mingw/public.h>
#elif defined( JAYMOD_OSX ) || defined( JAYMOD_OSX64 ) || defined( JAYMOD_OSX_ARM64 )
#    include <base/osx/public.h>
#elif defined( JAYMOD_ANDROID_ARM64 ) || defined( JAYMOD_ANDROID_ARMV7A ) || defined( JAYMOD_ANDROID_X86 ) || defined( JAYMOD_ANDROID_X86_64 )
#    include <base/linux/public.h>
#elif defined( JAYMOD_WASM )
#    include <base/linux/public.h>
#elif defined( JAYMOD_WINDOWS )
#    include <base/windows/public.h>
#else
#    error "JAYMOD platform is not defined."
#endif

///////////////////////////////////////////////////////////////////////////////

#if defined( __i386__ )
#    define JAYMOD_LITTLE_ENDIAN
#elif  defined( __ppc__ )
#    define JAYMOD_BIG_ENDIAN
#else
#    define JAYMOD_LITTLE_ENDIAN
#endif

///////////////////////////////////////////////////////////////////////////////

#if defined( __GNUC__ ) || defined( __clang__ )
#    define JAYMOD_FUNCTION __PRETTY_FUNCTION__
#elif defined( _MSC_VER )
#    define JAYMOD_FUNCTION __FUNCTION__
#else
#    define JAYMOD_FUNCTION __FUNCTION__
#endif

///////////////////////////////////////////////////////////////////////////////

// Platforms whose size_t is a type distinct from both uint32 and uint64, and
// which therefore need their own dedicated size_t overloads. On the 32-bit
// targets size_t is the same type as uint32 and a separate overload would be a
// redefinition.
#if defined( JAYMOD_OSX ) || defined( JAYMOD_OSX64 ) || defined( JAYMOD_OSX_ARM64 ) \
 || defined( JAYMOD_LINUX64 ) || defined( JAYMOD_LINUX_AARCH64 ) \
 || defined( JAYMOD_ANDROID_ARM64 ) || defined( JAYMOD_ANDROID_X86_64 ) \
 || defined( JAYMOD_WASM )
#    define JAYMOD_DISTINCT_SIZE_T
#endif

///////////////////////////////////////////////////////////////////////////////

#define USE_MDXFILE

///////////////////////////////////////////////////////////////////////////////

#endif // BASE_CONFIG_H
