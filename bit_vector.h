/*
 * This file declares the interface for using a bit vector which allows
 * us to store multiple bit values conveniently in a bit vector
 *
 * Author: Yash Gupta <ygupta@ucsc.edu>
 * Copyright: Yash Gupta
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#ifndef BIT_VECTOR_H
#define BIT_VECTOR_H

/* Macros */
#define VECTOR_BYTE_SIZE                  (8)
#define VECTOR_BYTE_INDEX(index)          (index / VECTOR_BYTE_SIZE)
#define VECTOR_BIT_INDEX(index)           (index & 0x7)
#define VECTOR_SET_STRING(index)          (0x1 << VECTOR_BIT_INDEX(index))
#define VECTOR_CLEAR_STRING(index)        (~VECTOR_SET_STRING(index))
#define VECTOR_CHECK_STRING(index)        (VECTOR_SET_STRING(index))

/* Default Values */
#define DEFAULT_VECTOR_INDEX              (0U)

/* Flag Macros */
#define VECTOR_FLAG_STREAM                (0)
#define VECTOR_FLAG_FULL                  (1)

/* Bit Macros */
#define VECTOR_BIT_SET                    (1)
#define VECTOR_BIT_OFF                    (0)

/*
 * This structure presents a nice method of holding bits. We
 * have an unsigned 8 bit integer which can hold up to 8
 * bits worth of information.
 */
typedef struct bit_vector {
    /*
     * The vector field is an array of unsigned 8 bit
     * integer and this is the memory which holds the actual
     * bits.
     */
    uint8_t *vector;

    /*
     * This field holds the total length of the bit vector and
     * signifies how much information in terms of bit can this
     * vector currently holds.
     */
    uint64_t vector_length;

    /*
     * The working index can also be used to maintain the
     * count of how many bits have been used in this bit
     * vector. This is useful for when we might need to write
     * out this bit vector to a file. This field is primarily
     * used when we treat the bit vector like a bit stream.
     */
    uint64_t working_index;
} bvector_t ;

/**
 * This function is used to create a new bit vector and
 * set it to default values.
 */
bvector_t* bvector_create(uint64_t);

/**
 * This function is used to free a bit vector so
 * that we do not have any wasted memory space.
 */
void bvector_free(bvector_t*);

/**
 * This function is used to set a specific bit in the bit
 * vector. It is important to note that the bit vector
 * begins count from 0.
 */
bvector_t* bvector_set_bit(bvector_t*, uint64_t);

/**
 * This function is used to clear a specific bit in the
 * bit vector. It is important to node that the bit vector
 * begins count from 0.
 */
bvector_t* bvector_clear_bit(bvector_t*, uint64_t);

/**
 * This function is used to check a specific bit in the
 * bit vector. It is important to node that the bit vector
 * begins count from 0.
 */
int8_t bvector_check_bit(bvector_t*, uint64_t);

/**
 * This function is used to provide the size of a bit
 * vector depending on the flag which has been passed.
 */
uint64_t bvector_get_size(bvector_t*, uint8_t);

/**
 * This function is used to resize the bit vector so that
 * now it can hold the specified amount of bits in it.
 */
bvector_t* bvector_resize(bvector_t*, uint64_t);

/**
 * This function is used to append a bit to the bit vector.
 * In most case this function is used when working with a stream
 * of bits in which case it cab be useful to have a function
 * which treats this vector as a stream of bits.
 */
bvector_t* bvector_append_bit(bvector_t*, uint8_t);

/**
 * This function is used to append one vector to another vector
 * and is really helpful when working with multiple bit vectors.
 */
bvector_t* bvector_append_vector(bvector_t*, bvector_t*, uint8_t);

/**
 * This function is used to print the bit vector depending
 * on the flag which is passed.
 */
void bvector_print(bvector_t*, uint8_t);

/**
 * This function is used to output a bit vector onto a file
 * after a specified offset. This is helpful when you want to
 * save a bit vector in a binary format.
 */
ssize_t bvector_output(bvector_t*, int, uint64_t, uint8_t);

/**
 * This function is used to input a bit vector from a file.
 * This function expects the file to be formatted the way
 * bvector_output emits. This is helpful for saving and restoring
 * state of the bit vector.
 */
bvector_t* bvector_input(int, uint64_t);

/**
 * This function is used to convert a bit string which is
 * represented as a C-style string into a bit vector string.
 */
bvector_t* bvector_convert(char*);

#endif
