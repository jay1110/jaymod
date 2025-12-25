#ifndef BASE_CONFIG_H
#define BASE_CONFIG_H

///////////////////////////////////////////////////////////////////////////////

#if defined( JAYMOD_LINUX ) || defined( JAYMOD_LINUX64 ) || defined( JAYMOD_LINUX_AARCH64 )
#    include <base/linux/public.h>
#elif defined( JAYMOD_MINGW ) || defined( JAYMOD_MINGW64 )
#    include <base/mingw/public.h>
#elif defined( JAYMOD_OSX ) || defined( JAYMOD_OSX64 )
#    include <base/osx/public.h>
#elif defined( JAYMOD_WINDOWS ) || defined( JAYMOD_WINDOWS64 )
#    include <base/windows/public.h>
#elif defined( JAYMOD_ANDROID_ARM64 ) || defined( JAYMOD_ANDROID_X86_64 ) || defined( JAYMOD_ANDROID_X86 )
#    include <base/linux/public.h>
#else
#    error "JAYMOD platform is not defined."
#endif

///////////////////////////////////////////////////////////////////////////////

#if defined( __i386__ )
#    define JAYMOD_LITTLE_ENDIAN
#elif defined( __x86_64__ ) || defined( _M_X64 )
#    define JAYMOD_LITTLE_ENDIAN
#elif defined( __aarch64__ ) || defined( __arm64__ )
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

#define USE_MDXFILE

///////////////////////////////////////////////////////////////////////////////

#endif // BASE_CONFIG_H
