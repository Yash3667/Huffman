/*
 * This file implements huffman coding with the use of several
 * huffman based data structures. It supports multiple flags and
 * is able to compress and decompress files which have en encoded
 * using a binary and an ascii representation of the encoding.
 *
 * Auhtor: Yash Gupta <ygupta@ucsc.edu>
 * Copyright: Yash Gupta
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "huffman_element.h"
#include "huffman_list.h"
#include "huffman_tree.h"
#include "bit_vector.h"

/* Debug Macro */
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#define ERROR_DEBUG(format, ...)                               \
    do {                                                       \
        printf(format " [%d: %s]\n",##__VA_ARGS__,             \
        errno,                                                 \
        strerror(errno));                                      \
        return errno;                                          \
    } while(0)

/*
 * This enumeration is used to maintain all the flags which are
 * supported for this program. The last flag is simply used to hold
 * the length which the bit vector is made from.
 */
enum huffman_flags {
    FLAG_ENCODE = 0,
    FLAG_DECODE,
    FLAG_ASCII,
    FLAG_PRINT,
    FLAG_HELP,
    FLAG_INPUT,
    FLAG_OUTPUT,
    FLAG_LENGTH
};
bvector_t *flags;

/* Pointers to hold file names */
char *input_filename = NULL;
char *output_filename = NULL;

/**
 * This function is used to print the usage of this
 * program including all the supported flags and correct
 * method of input.
 *
 * @param The value to return.
 */
void
print_usage(int retval)
{
    printf("Usage: huffman [opt] -i [input_file] -o [output_file]\n");
    printf("    -i: Input File Name\n");
    printf("    -o: Output File Name\n");
    printf("    -e: Encode The Input File\n");
    printf("    -d: Decode The Input File\n");
    printf("    -a: Perform Compression in ASCII\n");
    printf("    -p: Print The Encode String\n");
    printf("    -h: Print This Help Message\n");

    exit(retval);
}

/**
 * This function is used to use the getopt library and set all
 * the flags. We use the global variable flags which is a bit
 * vector.
 *
 * @param count The number of arguments
 * @param arg_value The array representing the values.
 * @return 0 on Sucess or error code
 */
int
set_flags(int count, char **arg_val)
{
    int opt_option;

    /* Suppress getopt warnings */
    opterr = 0;

    /* Acquire the flags */
    while ((opt_option = getopt(count, arg_val, "i:o:aphed")) != -1) {
        switch(opt_option) {
            case 'i':
                /* Confirm that flag has not been set before */
                if (bvector_check_bit(flags, FLAG_INPUT) == VECTOR_BIT_SET) {
                    printf("[FLAGS] Input Flag Already Set {-i %s}\n\n", input_filename);
                    return -1;
                } else {
                    bvector_set_bit(flags, FLAG_INPUT);
                    input_filename = optarg;
                }
                break;
            case 'o':
                /* Confirm that bit has not been set before */
                if (bvector_check_bit(flags, FLAG_OUTPUT) == VECTOR_BIT_SET) {
                    printf("[FLAGS] Output Flag Already Set {-o %s}\n\n", output_filename);
                    return -2;
                } else {
                    bvector_set_bit(flags, FLAG_OUTPUT);
                    output_filename = optarg;
                }
                break;
            case 'e':
                /* Confirm decode bit is not set */
                if (bvector_check_bit(flags, FLAG_ENCODE) == VECTOR_BIT_SET) {
                    printf("[FLAGS] Decode Flag Already Set {-d}\n\n");
                    return -3;
                } else {
                    bvector_set_bit(flags, FLAG_ENCODE);
                }
                break;
            case 'd':
                /* Confirm encode bit is not set */
                if (bvector_check_bit(flags, FLAG_DECODE) == VECTOR_BIT_SET) {
                    printf("[FLAGS] Encode Flag Already Set {-e}\n\n");
                    return -4;
                } else {
                    bvector_set_bit(flags, FLAG_DECODE);
                }
                break;
            case 'a':
                bvector_set_bit(flags, FLAG_ASCII);
                break;
            case 'p':
                bvector_set_bit(flags, FLAG_PRINT);
                break;
            case 'h':
                print_usage(0);
                break;
            case '?':
                /*
                 * In case an unknown flag is given or a value for a flag is not
                 * given.
                 */
                if (optopt == 'i') {
                    printf("[FLAGS] Need To Specify Input Filename {-i}\n\n");
                    return -1;
                } else if (optopt == 'o') {
                    printf("[FLAGS] Need To Specify Output Filename {-o}\n\n");
                    return -2;
                } else {
                    printf("[FLAGS] Unknown Flag Given {-%c}\n", optopt);
                    return -5;
                }
                break;
            default:
                printf("[FLAGS] Something Unknown And Terrible Happened!\n\n");
                return -6;
                break;
        }
    }

    /* Confirm atleast one of encode or decode is set */
    if (bvector_check_bit(flags, FLAG_ENCODE) == VECTOR_BIT_OFF) {
        if (bvector_check_bit(flags, FLAG_DECODE) == VECTOR_BIT_OFF) {
            printf("[FLAG] Neither Encode Nor Decode Flag Set {Use Flags: -e | -d}\n\n");
            return -7;
        }
    }

    /* We need to confirm that both input and out filenames are set */
    if (input_filename == NULL || output_filename == NULL) {
        printf("[FLAGS] Either Input or Output Filename Not Set {Use Flags: -i | -o}\n\n");
        return -8;
    }

    return 0;
}

/**
 * This function is used to append a string onto another string.
 * If the destination string runs out of memory, then more memory is
 * malloc'ed for it.
 *
 * @param dest The destination array
 * @param dest_length The destination array size
 * @param src The source array which will be appended.
 * @param src_length The length of the source.
 * @return The destination array
 */
uint8_t*
array_append(uint8_t *dest, uint64_t *dest_length, uint8_t *src, uint64_t src_length)
{
    uint64_t i, start_index;

    /* Build a new string if needed */
    if (dest == NULL) {
        *dest_length = src_length;
        dest = calloc(*dest_length, sizeof(uint8_t));
        start_index = 0;
    } else {
        start_index = *dest_length;
        *dest_length += src_length;
        dest = realloc(dest, *dest_length);
    }

    for (i = 0; i < src_length; i++) {
        dest[i + start_index] = src[i];
    }

    return dest;
}

/**
 * This function is used to perform huffman coding onto a file to compress
 * it. It can only be decompressed using this program and nothing else.
 * It uses structures from the huffman_tree and huffman_list files.
 *
 * @param in_fd The input file
 * @param out_fd The output file
 * @return 0 on success or error code
 */
int
huffman_encode(int in_fd, int out_fd)
{
    int another_input_fd;
    char **ascii_opcode_table;
    uint8_t *ascii_opcodes;
    uint64_t ascii_opcodes_size;
    bvector_t *vector_opcodes;
    uint8_t element;
    bvector_t **vector_opcode_table;
    ssize_t ret, offset;
    ssize_t bytes_read;
    ssize_t bytes_written;
    hlist_t *distribution_list;
    htree_t *distribution_tree;
    helement_t *temp_ptr;

    /*
     * We need to create a distribution list which maintains the frequency
     * of all the elements in the file. This list is then traversed through,
     * finding the minimums and making them into a tree node. This node is
     * then again inserted into the list and then repeated until only a single
     * element is left in the list.
     */
    int8_t ascii_set = bvector_check_bit(flags, FLAG_ASCII);
    int8_t print_set = bvector_check_bit(flags, FLAG_PRINT);

    distribution_list = hlist_create();
    if (!distribution_list) {
        ERROR_DEBUG("Error On Create {distribution_list}");
    }

    /* We build the distribution by reading in byte by byte */
    while ((bytes_read = read(in_fd, &element, sizeof(uint8_t))) > 0) {
        temp_ptr = hlist_add_increment_element(distribution_list, element, SPECIAL_ELEMENT_FREQUENCY);
        if (!temp_ptr) {
            ERROR_DEBUG("Error On Add/Increment {distribution_list: %u}", element);
        }
    }
    close(in_fd);

    /*
     * We traverse the list and acquire the two minimum elements in the
     * list and then we create a node with the combined frquency of the
     * two and add it back to the list. We then connect this new node with
     * its children.
     */
    helement_t *parent, *min_first, *min_second;
    while ((ret = hlist_get_two_min(distribution_list, &min_first, &min_second)) >= 0) {
        uint64_t combined_frequency = min_first->frequency + min_second->frequency;

        /* Add node to the list */
        parent = hlist_add_increment_element(distribution_list, LIST_SPECIAL_ELEMENT, combined_frequency);
        if (!parent) {
            ERROR_DEBUG("Error On Add/Increment {distribution_list, frequency: %llu}", combined_frequency);
        }

        /* Connect element with its children */
        ret = htree_connect(parent, min_first, min_second);
        if (ret) {
            ERROR_DEBUG("Error On Connection {parent, error: %ld}", ret);
        }
    }

    /*
     * We now have a list with a single element and this element
     * will be the root node of the tree we will create next. We
     * will them parse this tree to acquire encoding opcodes for each
     * possible element.
     */
    distribution_tree = htree_create();
    if (!distribution_tree) {
        ERROR_DEBUG("Error On Create {distribution_tree}");
    }

    /* Add the root element */
    temp_ptr = htree_add_element(distribution_tree, parent);
    if (!temp_ptr) {
        ERROR_DEBUG("Error On Add {distribution_tree: parent}");
    }

    /*
     * Parse the tree for the opcodes and then copy them over to
     * form vector opcodes as well
     */
    ascii_opcodes = NULL;
    ascii_opcodes_size = 0;
    ascii_opcode_table = htree_parse(distribution_tree);
    if (!ascii_opcode_table) {
        ERROR_DEBUG("Error On Parse {distribution_tree: ascii_opcode_table}");
    }

    /* Form vector opcodes if ASCII mode is not set */
    if (ascii_set == VECTOR_BIT_OFF) {
        /* This is used to concatenate all opcodes later */
        vector_opcodes = bvector_create(1);
        if (!vector_opcodes) {
            ERROR_DEBUG("Error On Create {vector_opcodes: %llu}", (uint64_t)1);
        }

        vector_opcode_table = malloc(sizeof(bvector_t *) * TREE_MAX_TABLE_SIZE);
        if (!vector_opcode_table) {
            ERROR_DEBUG("Error On Malloc {vector_opcode_table}");
        }

        int i;
        for (i = 0; i < TREE_MAX_TABLE_SIZE; i++) {
            /*
             * Set each vector opcode to NULL and if we find a valid opcode
             * in the ascii table, then convert the ascii opcode into vector
             * opcode.
             */
            vector_opcode_table[i] = NULL;
            if (ascii_opcode_table[i]) {
                vector_opcode_table[i] = bvector_convert(ascii_opcode_table[i]);
                if (!vector_opcode_table[i]) {
                    ERROR_DEBUG("Error On Convert {vector_opcode_table: %d}", i);
                }
            }
        }
    }

    /*
     * We output the tree onto the output file since this tree will be
     * needed during decompression. We get back an offset at which
     * we can begin writing the opcodes.
     */
    offset = htree_output(distribution_tree, out_fd, 0);
    if (offset < 0) {
        ERROR_DEBUG("Error On Output {distribution_tree: %ld}", offset);
    }

    /*
     * Now, we need to read in the input file again and this time for each element we
     * come across, we write out its opcode out to the output file.
     */
    another_input_fd = open(input_filename, O_RDONLY);
    if (another_input_fd < 0) {
        ERROR_DEBUG("Error On Open {another_input_fd: %d}", another_input_fd);
    }

    /* Begin Reading */
    while ((bytes_read = read(another_input_fd, &element, sizeof(uint8_t))) > 0) {
        if (ascii_set == VECTOR_BIT_SET) {
            /*
             * Append the ASCII based opcode onto the ASCII opcode string
             * and keep doing until the entire file has been read.
             */
            uint64_t length = strlen(ascii_opcode_table[element]);
            ascii_opcodes = array_append(ascii_opcodes, &ascii_opcodes_size,
                                         (uint8_t*)ascii_opcode_table[element], length);
        } else {
            /**
             * We keep appending each of the vector based opcodes onto
             * the main vector until the entire file has been read.
             */
            bvector_append_vector(vector_opcodes, vector_opcode_table[element], VECTOR_FLAG_FULL);
        }
    }

    /*
     * Write out the opcodes to the file depending on the flag which
     * was passed in by the user.
     */
    if (ascii_set == VECTOR_BIT_OFF) {
        /* Write the opcode vector onto the file */
        bvector_output(vector_opcodes, out_fd, offset, VECTOR_FLAG_STREAM);

        if (print_set == VECTOR_BIT_SET) {
            /* Print opcode vector onto stdout if flag set */
            printf("Character Encoding\n");
            bvector_print(vector_opcodes, VECTOR_FLAG_STREAM);
        }

        free(vector_opcodes);
    } else {
        /* Write the entire opcode buffer onto the file */
        bytes_written = pwrite(out_fd, ascii_opcodes, ascii_opcodes_size, offset);
        if (bytes_written < (ssize_t)ascii_opcodes_size) {
            ERROR_DEBUG("Error On Write {out_fd: %d}", out_fd);
        }

        /* Print opcode buffer onto stdout if flag set */
        if (print_set == VECTOR_BIT_SET) {
            printf("Character Encoding\n");
            printf("%s\n", (char *)ascii_opcodes);
        }

        free(ascii_opcodes);
    }

    // TODO: Garbage Collection

    return 0;
}

/**
 * This function is used to perform huffman coding onto a file to decompress
 * it. It can only be compressed using this program and nothing else.
 * It uses structures from the huffman_tree and huffman_list files.
 *
 * @param in_fd The input file
 * @param out_fd The output file
 * @return 0 on success or error code
 */
int
huffman_decode(int in_fd, int out_fd)
{
    uint8_t *ascii_opcodes;
    uint64_t ascii_opcodes_size;

    uint8_t *decoded_string;
    uint64_t decoded_string_size;

    int decoded_element;
    int opcode;
    uint64_t i, opcode_loop_size;
    bvector_t *vector_opcodes;
    ssize_t bytes_read;
    ssize_t bytes_written;
    ssize_t offset;
    htree_t *constructed_tree;
    helement_t *temp_ptr;

    /*
     * The concept of the decode function is quite simple. We need
     * to reconstruct our huffman tree from the input file, read in all
     * the opcodes and then simply state step through the binary tree
     * and output the element once we reach a leaf node. Our data
     * structures really help us do all of this.
     */
    ascii_opcodes = NULL;
    int8_t ascii_set = bvector_check_bit(flags, FLAG_ASCII);
    int8_t print_set = bvector_check_bit(flags, FLAG_PRINT);

    /* Construct the huffman tree from the input file */
    constructed_tree = htree_input(in_fd);
    if (!constructed_tree) {
        ERROR_DEBUG("Error On Input {constructed_tree}");
    }

    /*
     * Compute offset for readinf in the opcodes and then read in
     * all the opcodes for the entire file. Decoding is done in
     * a separate loop.
     */
    offset = TREE_INPUT_OBJECT_OFFSET(constructed_tree->count);

    if (ascii_set == VECTOR_BIT_OFF) {
        vector_opcodes = bvector_input(in_fd, offset);
        if (!vector_opcodes) {
            ERROR_DEBUG("Error On Input {vector_opcodes}");
        }

        if (print_set == VECTOR_BIT_SET) {
            printf("Character Encoding\n");
            bvector_print(vector_opcodes, VECTOR_FLAG_STREAM);
        }
    } else {
        uint8_t element;
        ascii_opcodes_size = 0;

        /* Begin reading */
        while ((bytes_read = pread(in_fd, &element, sizeof(char), offset)) > 0) {
            ascii_opcodes = array_append(ascii_opcodes, &ascii_opcodes_size, &element, 1);
            offset += sizeof(char);
        }

        if (print_set == VECTOR_BIT_SET) {
            printf("Character Encoding\n");
            printf("%s\n", (char *)ascii_opcodes);
        }
    }

    /* Begin state stepping through all the opcodes */
    if (ascii_set == VECTOR_BIT_SET) {
        opcode_loop_size = ascii_opcodes_size;
    } else {
        opcode_loop_size = bvector_get_size(vector_opcodes, VECTOR_FLAG_STREAM);
    }

    decoded_string = NULL;
    decoded_string_size = 0;
    temp_ptr = constructed_tree->root;
    decoded_element = -1;
    for (i = 0; i < opcode_loop_size; i++) {
        if (ascii_set) {
            opcode = ascii_opcodes[i] - 48;
        } else {
            opcode = bvector_check_bit(vector_opcodes, i);
        }

        temp_ptr = htree_state_step(constructed_tree, temp_ptr, &decoded_element, opcode);
        if (decoded_element >= 0) {
            uint8_t element = decoded_element;
            decoded_string = array_append(decoded_string, &decoded_string_size, &element, 1);
        }
    }

    /* Write the decoded string onto the output file */
    bytes_written = write(out_fd, decoded_string, decoded_string_size);
    if (bytes_written < (ssize_t)decoded_string_size) {
        ERROR_DEBUG("Error On Write {out_fd: %d}", out_fd);
    }

    // TODO: Garbage Collection

    return 0;
}

/**
 * The main function reads in all arguments from the command line
 * and performs huffman based encoding or decoding depending on the
 * flags given.
 *
 * @param argc The number of arguments passed.
 * @param argv The values of the arguments.
 * @return 0 on success or error code
 */
int
main(int argc, char *argv[])
{
    ssize_t ret;
    int input_fd, output_fd;

    /* Create the flags bit vector */
    flags = bvector_create(FLAG_LENGTH);
    if (!flags) {
        ERROR_DEBUG("Error On bvector_create {flags}");
    }

    ret = set_flags(argc, argv);
    if (ret < 0) {
        print_usage(-1);
    }

    /* Open input and output file as required */
    input_fd = open(input_filename, O_RDONLY);
    if (input_fd < 0) {
        ERROR_DEBUG("Error On Open {input_fd: %d}", input_fd);
    }

    output_fd = open(output_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (output_fd < 0) {
        ERROR_DEBUG("Error On Open {output_fd: %d}", output_fd);
    }

    /* Run Encoding or Decoding */
    if (bvector_check_bit(flags, FLAG_ENCODE) == VECTOR_BIT_SET) {
        return huffman_encode(input_fd, output_fd);
    } else {
        return huffman_decode(input_fd, output_fd);
    }
}
