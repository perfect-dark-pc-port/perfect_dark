#ifndef _IN_PLATFORM_H
#define _IN_PLATFORM_H

// detect OS
#if defined(__EMSCRIPTEN__)
	#define PLATFORM_WEB 1
#elif defined(_WIN32)
	#define PLATFORM_WIN32 1
#elif defined(__SWITCH__)
	#define PLATFORM_POSIX 1
	#define PLATFORM_NSWITCH 1
#elif defined(__linux__)
	#define PLATFORM_POSIX 1
	#define PLATFORM_LINUX 1
#elif defined(__APPLE__)
	#define PLATFORM_POSIX 1
	#define PLATFORM_OSX 1
#else
	// assume POSIX-compatible
	#define PLATFORM_POSIX 1
	#warning "Unknown OS. Please add it to platform.h."
#endif

// detect arch
#if defined(__wasm32__)
	#define PLATFORM_WASM32 1
#elif defined(__x86_64__) || defined(_M_X64)
	#define PLATFORM_X86_64 1
	#define PLATFORM_64BIT 1
#elif defined(__i386__) || defined(_X86_) || defined(_M_IX86)
	#define PLATFORM_X86 1
#elif defined(__aarch64__) || defined(_M_ARM64)
	#define PLATFORM_ARM 8
	#define PLATFORM_64BIT 1
#elif defined(__arm__) || defined(_M_ARM)
	// assume armv7
	#define PLATFORM_ARM 7
#else
	#error "Unknown CPU arch."
#endif

// detect endianness
#if defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__)
	#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
		#define PLATFORM_BIG_ENDIAN 1
	#else
		#define PLATFORM_LITTLE_ENDIAN 1
	#endif
#elif __BIG_ENDIAN__
	#define PLATFORM_BIG_ENDIAN 1
#else
	#error "Could not determine endianness."
#endif

// byteswap macros

#if defined(__GNUC__) || defined(__clang__)
	#define PD_BSWAP16(x) __builtin_bswap16(x)
	#define PD_BSWAP32(x) __builtin_bswap32(x)
	#define PD_BSWAP64(x) __builtin_bswap64(x)
#else
	#error "Implement PD_BSWAP macros for your compiler."
#endif

#ifdef PLATFORM_BIG_ENDIAN
	#define PD_BE16(x) (x)
	#define PD_BE32(x) (x)
	#define PD_BE64(x) (x)
	#define PD_LE16(x) PD_BSWAP16(x)
	#define PD_LE32(x) PD_BSWAP32(x)
	#define PD_LE64(x) PD_BSWAP64(x)
#else
	#define PD_BE16(x) PD_BSWAP16(x)
	#define PD_BE32(x) PD_BSWAP32(x)
	#define PD_BE64(x) PD_BSWAP64(x)
	#define PD_LE16(x) (x)
	#define PD_LE32(x) (x)
	#define PD_LE64(x) (x)
#endif

// TODO: use uintptr_t or something
#ifdef PLATFORM_64BIT
	#define PD_BEPTR(x) PD_BE64(x)
	#define PD_LEPTR(x) PD_LE64(x)
#else
	#define PD_BEPTR(x) PD_BE32(x)
	#define PD_LEPTR(x) PD_LE32(x)
#endif

// module constructor function attribute
#if defined(__GNUC__) || defined(__clang__)
	#define PD_CONSTRUCTOR __attribute__((constructor))
#else
	#error "Implement PD_CONSTRUCTOR macro for your compiler."
#endif

#endif
