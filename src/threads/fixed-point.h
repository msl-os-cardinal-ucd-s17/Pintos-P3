#ifndef THREADS_FIXED_H
#define THREADS_FIXED_H

#include <stdint.h>

/*

This file implements fixed point arithmetic for the pintos 
operating system.

 */

// Using the 17.14 p.q format
#define UPPER_BITS 17
#define FRACTIONAL_BITS 14

// Define the conversion factor (left shift is equivalent to 2^14)
#define FACTOR 2<<FRACTIONAL_BITS


// Convert integer to fixed point
inline int int_to_fixed(int integer)
{
    return integer * FACTOR;
}

// Convert fixed point to integer (round toward 0)
inline int fixed_to_int_round0(int fixed)
{
    return fixed / FACTOR;
} 

// Convert fixed point to integer (round to nearest integer)
inline int fixed_to_int_roundInt(int fixed)
{
    if (fixed >= 0) {
        return fixed + (FACTOR/2);
    }
    else {
        return fixed - (FACTOR/2);
    }
}

// Add two fixed point numbers
inline int add_fixed(int fixed1, int fixed2)
{
    return fixed1 + fixed2;
}

// Subtract two fixed point numbers
inline int sub_fixed(int fixed1, int fixed2)
{
    return fixed1 - fixed2;
}

// Add an integer to a fixed point number
inline int fixed_plus_int(int fixed, int integer)
{
    return fixed + integer * FACTOR;
}

// Subtract integer from fixed point number
inline int fixed_minus_int(int fixed, int integer)
{
    return fixed - (integer * FACTOR);
}

// Multiply two fixed integers
inline int mult_fixed(int fixed1, int fixed2)
{
    return ((int64_t)fixed1) * fixed2/FACTOR;
}

// multiply fixed and int
inline int fixed_mult_int(int fixed, int integer)
{
    return fixed * integer;
}

// divide two fixed point numbers
inline int div_fixed(int fixed1, int fixed2)
{
    return ((int64_t)fixed1) * FACTOR/fixed2;
}

// divide fixed by int
inline int fixed_div_int(int fixed, int integer)
{
    return fixed/integer;
}


#endif /* threads/fixed-point.h */