/*
 * This file defines the interface for using a bit vector which allows
 * us to store multiple bit values conveniently in a bit vector.
 *
 * Author: Yash Gupta <ygupta@ucsc.edu>
 * Copyright: Yash Gupta
 */
#include "bit_vector.h"

/**
 * This function is used to create a new bit vector and
 * set it to default values.
 *
 * @param length The length of the bit vector we need.
 * @return A bit vector or NULL.
 */
bvector_t*
bvector_create(uint64_t length)
{
    bvector_t *temp;

    if (length == 0) {
        return NULL;
    }

    temp = malloc(sizeof(bvector_t));
    if (!temp) {
        return NULL;
    }

    temp->vector_length = length;
    temp->vector = calloc((length / VECTOR_BYTE_SIZE) + 1, sizeof(uint8_t));
    if (!(temp->vector)) {
        free(temp);
        return NULL;
    }

    /* Set Defaults */
    temp->working_index = DEFAULT_VECTOR_INDEX;

    return temp;
}

/**
 * This function is used to free a bit vector so
 * that we do not have any wasted memory space.
 *
 * @param bvector The bit vector to free
 */
void
bvector_free(bvector_t *bvector)
{
    free(bvector->vector);
    free(bvector);
}

/**
 * This function is used to set a specific bit in the bit
 * vector. It is important to note that the bit vector
 * begins count from 0.
 *
 * @param bvector The bvector to set bit for
 * @param index The index of the bit to set.
 * @return The bit vector passed or NULL.
 */
bvector_t*
bvector_set_bit(bvector_t *bvector, uint64_t index)
{
    if (!bvector) {
        return NULL;
    } else if (index >= bvector->vector_length) {
        return NULL;
    } else {
        bvector->vector[VECTOR_BYTE_INDEX(index)] |= VECTOR_SET_STRING(index);
        return bvector;
    }
}

/**
 * This function is used to clear a specific bit in the
 * bit vector. It is important to node that the bit vector
 * begins count from 0.
 *
 * @param bvector The bvector to clear bit for
 * @param index The index of the bit to clear.
 * @return The bit vector passed or NULL.
 */
bvector_t*
bvector_clear_bit(bvector_t *bvector, uint64_t index)
{
    if (!bvector) {
        return NULL;
    } else if (index >= bvector->vector_length) {
        return NULL;
    } else {
        bvector->vector[VECTOR_BYTE_INDEX(index)] &= VECTOR_CLEAR_STRING(index);
        return bvector;
    }
}

/**
 * This function is used to check a specific bit in the
 * bit vector. It is important to node that the bit vector
 * begins count from 0.
 *
 * @param bvector The bvector to check bit for
 * @param index The index of the bit to check.
 * @return Bit value or error code
 */
int8_t
bvector_check_bit(bvector_t *bvector, uint64_t index)
{
    if (!bvector) {
        return -1;
    } else if (index >= bvector->vector_length) {
        return -2;
    } else {
        return !!(bvector->vector[VECTOR_BYTE_INDEX(index)] & VECTOR_CHECK_STRING(index));
    }
}

/**
 * This function is used to provide the size of a bit
 * vector depending on the flag which has been passed.
 *
 * @param bvector The vector whose size we want
 * @param flag The type of size to return.
 * @return Vector size or 0
 */
uint64_t
bvector_get_size(bvector_t *bvector, uint8_t flag)
{
    if (!bvector) {
        return 0;
    } else if (flag == VECTOR_FLAG_FULL) {
        return bvector->vector_length;
    } else if (flag == VECTOR_FLAG_STREAM) {
        return bvector->working_index;
    }

    return 0;
}

/**
 * This function is used to resize the bit vector so that
 * now it can hold the specified amount of bits in it.
 *
 * @param bvector The bit vector to resize.
 * @param length The new length of the bit vector.
 */
bvector_t*
bvector_resize(bvector_t *bvector, uint64_t length)
{
    uint8_t *temp_vector;

    /* Confirm valid vector */
    if (!bvector) {
        return NULL;
    }

    /* Resize the length and call realloc */
    bvector->vector_length = length;
    temp_vector = realloc(bvector->vector, (length / VECTOR_BYTE_SIZE) + 1);
    if (!temp_vector) {
        bvector_free(bvector);
        return NULL;
    } else {
        bvector->vector = temp_vector;
    }

    return bvector;
}

/**
 * This function is used to append a bit to the bit vector.
 * In most case this function is used when working with a stream
 * of bits in which case it cab be useful to have a function
 * which treats this vector as a stream of bits.
 *
 * @param bvector The vector to which we want to append a bit.
 * @param bit The bit to append.
 * @return The bit vector or NULL.
 */
bvector_t*
bvector_append_bit(bvector_t* bvector, uint8_t bit)
{
    if (!bvector) {
        return NULL;
    } else if (bit > 1) {
        return NULL;
    }

    if (bvector->working_index == bvector->vector_length) {
        bvector_resize(bvector, bvector->vector_length * 2);
    }

    if (bit == 0) {
        bvector_clear_bit(bvector, bvector->working_index);
    } else {
        bvector_set_bit(bvector, bvector->working_index);
    }
    bvector->working_index += 1;

    return bvector;
}

/**
 * This function is used to append one vector to another vector
 * and is really helpful when working with multiple bit vectors.
 *
 * @param bvector The vector to which we append.
 * @param copy_vector The vector which we copy.
 * @param flag The flag of how to copy.
 * @return bvector or NULL.
 */
bvector_t*
bvector_append_vector(bvector_t *bvector, bvector_t *copy_vector, uint8_t flag)
{
    uint8_t bit;
    uint64_t i, append_length;

    if (!bvector || !copy_vector) {
        return NULL;
    }

    /* Set length according to flag */
    append_length = bvector_get_size(copy_vector, flag);

    /* Append all bits of the copy_vector */
    for (i = 0; i < append_length; i++) {
        bit = bvector_check_bit(copy_vector, i);
        bvector_append_bit(bvector, bit);
    }

    return bvector;
}

/**
 * This function is used to print the bit vector depending
 * on the flag which is passed.
 *
 * @param bvector The vector to print
 * @param flag Style of printing
 */
void
bvector_print(bvector_t *bvector, uint8_t flag)
{
    uint8_t bit;
    uint64_t i, print_length;

    if (!bvector) {
        return;
    }

    /* Print vector either as a stream or as the full vector */
    print_length = bvector_get_size(bvector, flag);

    for (i = 0; i < print_length; i++) {
        if (i % (VECTOR_BYTE_SIZE / 2) == 0 && i > 0) {
            printf(" ");
        }
        bit = bvector_check_bit(bvector, i);
        printf("%u", bit);
    }
    printf("\n");
}

/**
 * This function is used to output a bit vector onto a file
 * after a specified offset. This is helpful when you want to
 * save a bit vector in a binary format.
 *
 * @param bvector The bit vector you want to output
 * @param fd The file you want to output to
 * @param offset The offset in file to output to
 * @param flags Style of output.
 * @return 0 on success or error code
 */
ssize_t
bvector_output(bvector_t *bvector, int fd, uint64_t offset, uint8_t flag)
{
    uint64_t length;
    ssize_t size_to_write;
    ssize_t bytes_written;

    if (!bvector) {
        return -1;
    }

    /* Get which length to write depending on flag */
    length = bvector_get_size(bvector, flag);

    /* Get length depending on flag and write it out */
    size_to_write = sizeof(uint64_t);
    bytes_written = pwrite(fd, &length, size_to_write, offset);
    if (bytes_written < size_to_write) {
        return -2;
    } else {
        offset += bytes_written;
    }

    /* Write out the entire vector */
    length = (length / VECTOR_BYTE_SIZE) + 1;
    size_to_write = length;
    bytes_written = pwrite(fd, bvector->vector, size_to_write, offset);
    if (bytes_written < size_to_write) {
        return -4;
    } else {
        offset += bytes_written;
    }

    return offset;
}

/**
 * This function is used to input a bit vector from a file.
 * This function expects the file to be formatted the way
 * bvector_output emits. This is helpful for saving and restoring
 * state of the bit vector.
 *
 * @param fd The file you want to output to.
 * @param offset The offset in file to output to.
 * @return A bit vector or null
 */
bvector_t*
bvector_input(int fd, uint64_t offset)
{
    bvector_t *bvector;
    uint64_t length;
    ssize_t size_to_read;
    ssize_t bytes_read;

    /* Read the length */
    size_to_read = sizeof(uint64_t);
    bytes_read = pread(fd, &length, size_to_read, offset);
    if (bytes_read < size_to_read) {
        return NULL;
    } else {
        offset += bytes_read;
    }

    /* Make the vector */
    bvector = bvector_create(length);
    if (!bvector) {
        return NULL;
    } else {
        bvector->working_index = length;
    }

    /* Read in the entire vector */
    length = (length / VECTOR_BYTE_SIZE) + 1;
    size_to_read = length;
    bytes_read = pread(fd, bvector->vector, size_to_read, offset);
    if (bytes_read < size_to_read) {
        return NULL;
    }

    return bvector;
}


/**
 * This function is used to convert a bit string which is
 * represented as a C-style string into a bit vector string.
 *
 * @param bit_string The bit string to convert
 * @return A bit vector or NULL
 */
bvector_t*
bvector_convert(char *bit_string)
{
    char bit_code;
    uint64_t i, length;
    bvector_t *temp;

    length = strlen(bit_string);
    temp = bvector_create(length);
    if (!temp) {
        return NULL;
    }

    /*
     * The following loop is powerful in that it can create
     * a bit vector even if the bit_string contains elements
     * which are not 0 or 1. If a string which has extra characters
     * is passed in, then those extra characters are ignored.
     */

    for (i = 0; i < length; i++) {
        bit_code = bit_string[i];
        if (bit_code == '0') {
            temp = bvector_append_bit(temp, 0);
        } else if (bit_code == '1') {
            temp = bvector_append_bit(temp, 1);
        }

        if (!temp) {
            return NULL;
        }
    }

    /*
     * We resize this bit vector in case extra invalid characters were
     * passed in the string so that our vector does not consume extra
     * memory. We simply return whatever we got, be it NULL or a valid
     * pointer.
     */
    temp = bvector_resize(temp, temp->working_index);
    return temp;
}
