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

/*
    Wrap the fixed point value into a struct to protect from 
    conversion/interpretation errors
 */ 
struct fixed_point 
{
    int value;
};


// Convert integer to fixed point
inline struct fixed_point int_to_fixed(int integer)
{
    struct fixed_point number;
    number.value = integer * FACTOR;
    return number;
}

// Convert fixed point to integer (round toward 0)
inline int fixed_to_int_round0(struct fixed_point num)
{
    return num.value/FACTOR;
} 

// Convert fixed point to integer (round to nearest integer)
inline int fixed_to_int_roundInt(struct fixed_point num)
{

    if (num.value >= 0) {
        return num.value + (FACTOR/2);
    }
    else {
        return num.value - (FACTOR/2);
    }
}

// Add two fixed point numbers
inline struct fixed_point add_fixed(struct fixed_point fixed1, struct fixed_point fixed2)
{
    struct fixed_point number;
    number.value = fixed1.value + fixed2.value;
    return number;
}

// Subtract two fixed point numbers
inline struct fixed_point sub_fixed(struct fixed_point fixed1, struct fixed_point fixed2)
{
    struct fixed_point number;
    number.value = fixed1.value - fixed2.value;
    return number;
}

// Add an integer to a fixed point number
inline struct fixed_point fixed_plus_int(struct fixed_point fixed, int integer)
{
    struct fixed_point number;
    number.value = fixed.value + (integer * FACTOR);
    return number;
}

// Subtract integer from fixed point number
inline struct fixed_point fixed_minus_int(struct fixed_point fixed, int integer)
{
    struct fixed_point number;
    number.value = fixed.value - (integer * FACTOR);
    return number;
}

// Multiply two fixed integers
inline struct fixed_point mult_fixed(struct fixed_point fixed1, struct fixed_point fixed2)
{
    struct fixed_point number;
    number.value = ((int64_t)fixed1.value) * fixed2.value/FACTOR;
    return number;
}

// multiply fixed and int
inline struct fixed_point fixed_mult_int(struct fixed_point fixed, int integer)
{
    struct fixed_point number;
    number.value = fixed.value * integer;
    return number;
}

// divide two fixed point numbers
inline struct fixed_point div_fixed(struct fixed_point fixed1, struct fixed_point fixed2)
{
    struct fixed_point number;
    number.value = ((int64_t)fixed1.value) * FACTOR/fixed2.value;
    return number;
}

// divide fixed by int
inline struct fixed_point fixed_div_int(struct fixed_point fixed, int integer)
{
    struct fixed_point number;
    number.value = fixed.value/integer;
    return number;
}


#endif /* threads/fixed-point.h */