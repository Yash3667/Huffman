/*
 * This file decalres the interface for using a huffman
 * tree.
 *
 * Author: Yash Gupta <ygupta@ucsc.edu>
 * Copyright: Yash Gupta, UC Santa Cruz
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "huffman_element.h"

#ifndef HUFFMAN_TREE_H
#define HUFFMAN_TREE_H

/* Defaults */
#define DEFAULT_TREE_COUNT      (0U)
#define DEFAULT_PARSE_BIT       (0U)
#define DEFAULT_TREE_START      (NULL)

/* Bad Count */
#define TREE_BAD_COUNT          (0xFFFFFFFFFFFFFFFFU)

/* Max Table Size */
#define TREE_MAX_TABLE_SIZE     (256)

/* These are the macros for an element in a huffman tree binary file */
#define TREE_INPUT_ROOT_ELEMENT_INDEX           (0U)
#define TREE_INPUT_COUNT_OFFSET                 (0U)
#define TREE_INPUT_COUNT_SIZE                   (sizeof(uint64_t))
#define TREE_INPUT_ELEMENT_SIZE                 (sizeof(uint8_t))
#define TREE_INPUT_LEAFNODE_SIZE                (sizeof(uint8_t))
#define TREE_INPUT_OBJECT_SIZE                  (TREE_INPUT_ELEMENT_SIZE + TREE_INPUT_LEAFNODE_SIZE)
#define TREE_INPUT_OBJECT_OFFSET(element)       (TREE_INPUT_COUNT_OFFSET + TREE_INPUT_COUNT_SIZE + (element * TREE_INPUT_OBJECT_SIZE))
#define TREE_INPUT_ELEMENT_OFFSET(element)      (TREE_INPUT_OBJECT_OFFSET(element))
#define TREE_INPUT_LEAFNODE_OFFSET(element)     (TREE_INPUT_OBJECT_OFFSET(element) + TREE_INPUT_LEAFNODE_SIZE)

typedef struct huffman_tree {
    /* This is the beginning of the tree */
    helement_t *root;

    /* This is the number of elements in the tree */
    uint64_t count;

    /*
     * This is a bit which tells us whether this tree has been
     * parsed or not. Helps us maintain consistency in the code
     */
    uint8_t _parsed : 1;
} htree_t;

/**
 * This function is used to build a new tree of
 * huffman elements. It takes no arguments and
 * returns a type htree_t with all values set
 * to defaults.
 */
htree_t* htree_create();

/**
 * This function is used to free a huffman
 * tree.
 *
 * NOTE: This function only free's fields which are based
 * on a tree. List fields are not free'd.
 */
void htree_free(htree_t*);

/**
 * This function is used to add an element to the huffman tree.
 * An element in the tree is always added using recursive in-order
 * traversal of the tree.
 */
helement_t* htree_add_element(htree_t*, helement_t*);

/**
 * This function is used to return the count of the elements in
 * the tree.
 */
uint64_t htree_count(htree_t*);

/**
 * This function is used to connect a element to its
 * children so that we can build a node element without
 * the constructor.
 */
int htree_connect(helement_t*, helement_t*, helement_t*);

/**
 * This function is used to parse the entire huffman tree and
 * build opcodes for each  of the leaf nodes.
 */
char** htree_parse(htree_t*);

/**
 * This function is used to output the tree to a binary file
 * and it does so using a depth first algorithm. It returns the offset
 * after which the next thing should begin writing.
 */
ssize_t htree_output(htree_t*, int, uint64_t);

/**
 * This function is used to take input for a huffman tree
 * from a binary file and it does so using a pre-order fetch
 * algorithm.
 */
htree_t* htree_input(int);

/**
 * This function is used to state step the huffman tree
 * depending on the opcode which is given. An opcode of
 * 0 moves the tree towards the left child and opcode
 * of 1 moves the tree towards the right child.
 */
helement_t* htree_state_step(htree_t*, helement_t*, int*, int);

/**
 * This function is used to print a huffman tree in its
 * entirety. This is used to debug the working of the tree
 * and can be very helpful.
 */
void htree_print(htree_t*);

#endif
