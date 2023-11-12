#pragma once

#include <stddef.h>
#include <stdint.h>

#if __STDC_VERSION__ < 202300L
#include <assert.h> // For static_assert.
#include <stdbool.h> // For bool/true/false, nothing to rename here but it's also a primitive type.
#endif

// Signed integers
typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef int_fast8_t  i8fast;
typedef int_fast16_t i16fast;
typedef int_fast32_t i32fast;
typedef int_fast64_t i64fast;

typedef int_least8_t  i8least;
typedef int_least16_t i16least;
typedef int_least32_t i32least;
typedef int_least64_t i64least;

// Unsigned integers
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef uint_fast8_t  u8fast;
typedef uint_fast16_t u16fast;
typedef uint_fast32_t u32fast;
typedef uint_fast64_t u64fast;

typedef uint_least8_t  u8least;
typedef uint_least16_t u16least;
typedef uint_least32_t u32least;
typedef uint_least64_t u64least;

typedef size_t usize;

// Bits/bytes
typedef char byte;

// Floats
typedef float  f32;
typedef double f64;


// Unicode & Strings
// Note: I like using these to make it clearer for me which encoding I should expect when manipulating strings/bytes.
typedef char ascii;
typedef byte utf8;
typedef i16  utf16;
typedef i32  utf32;


#pragma region TESTS SECTION


#define VALIDATE_BYTES_SIZE( _type, _operator, _expected_bsize )								\
	static_assert( sizeof( _type ) _operator _expected_bsize, "Unexpected number of bytes for the type " #_type )

VALIDATE_BYTES_SIZE( i8,  ==, 1 /* byte */ );
VALIDATE_BYTES_SIZE( i16, ==, 2 /* bytes */ );
VALIDATE_BYTES_SIZE( i32, ==, 4 /* bytes */ );
VALIDATE_BYTES_SIZE( i64, ==, 8 /* bytes */ );

VALIDATE_BYTES_SIZE( i8fast,  >=, 1 /* byte */ );
VALIDATE_BYTES_SIZE( i16fast, >=, 2 /* bytes */ );
VALIDATE_BYTES_SIZE( i32fast, >=, 4 /* bytes */ );
VALIDATE_BYTES_SIZE( i64fast, >=, 8 /* bytes */ );

VALIDATE_BYTES_SIZE( i8least,  >=, 1 /* byte */ );
VALIDATE_BYTES_SIZE( i16least, >=, 2 /* bytes */ );
VALIDATE_BYTES_SIZE( i32least, >=, 4 /* bytes */ );
VALIDATE_BYTES_SIZE( i64least, >=, 8 /* bytes */ );


VALIDATE_BYTES_SIZE( u8,  ==, 1 /* byte */ );
VALIDATE_BYTES_SIZE( u16, ==, 2 /* bytes */ );
VALIDATE_BYTES_SIZE( u32, ==, 4 /* bytes */ );
VALIDATE_BYTES_SIZE( u64, ==, 8 /* bytes */ );

VALIDATE_BYTES_SIZE( u8fast,  >=, 1 /* byte */ );
VALIDATE_BYTES_SIZE( u16fast, >=, 2 /* bytes */ );
VALIDATE_BYTES_SIZE( u32fast, >=, 4 /* bytes */ );
VALIDATE_BYTES_SIZE( u64fast, >=, 8 /* bytes */ );

VALIDATE_BYTES_SIZE( u8least,  >=, 1 /* byte */ );
VALIDATE_BYTES_SIZE( u16least, >=, 2 /* bytes */ );
VALIDATE_BYTES_SIZE( u32least, >=, 4 /* bytes */ );
VALIDATE_BYTES_SIZE( u64least, >=, 8 /* bytes */ );


VALIDATE_BYTES_SIZE( byte, ==, 1 /* byte */ );

VALIDATE_BYTES_SIZE( f32, ==, 4 /* bytes */ );
VALIDATE_BYTES_SIZE( f64, ==, 8 /* bytes */ );

VALIDATE_BYTES_SIZE( ascii, ==, 1 /* byte */ );
VALIDATE_BYTES_SIZE( utf8,  ==, 1 /* byte */ );
VALIDATE_BYTES_SIZE( utf16, ==, 2 /* bytes */ );
VALIDATE_BYTES_SIZE( utf32, ==, 4 /* bytes */ );
	
#undef VALIDATE_BYTES_SIZE
// End of the file. Making sure that there is a \n after my #undef in this gist.

#pragma endregion TESTS SECTION
