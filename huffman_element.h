/*
 * This file declares the fields and attributes of our main
 * structure which is used to hold all the information for
 * a Huffman Element.
 *
 * Author: Yash Gupta <ygupta@ucsc.edu>
 * Copyright: Yash Gupta, UC Santa Cruz
 */
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>

#ifndef HUFFMAN_ELEMENT_H
#define HUFFMAN_ELEMENT_H

/* Defaults */
#define DEFAULT_ELEMENT_FREQUENCY       (1U)
#define DEFAULT_ELEMENT_LEFT_CHILD      (NULL)
#define DEFAULT_ELEMENT_RIGHT_CHILD     (NULL)
#define DEFAULT_ELEMENT_NEXT            (NULL)
#define DEFAULT_ELEMENT_PREVIOUS        (NULL)

/* Specials */
#define SPECIAL_ELEMENT_FREQUENCY       (0U)

/*
 * This structure is the main part of this header. It holds
 * information to make a tree and list out of the structure.
 */
typedef struct huffman_element {
    /*
     * This is the element which is contained inside the
     * huffman element. All other information in the structure
     * is metadata for this element.
     */
    uint8_t element;

    /* TREE
     * This field lets us know whether this element is a
     * leaf node or not. This is technically redundant as
     * we can infer the same by checking the children,
     * however, I feel this makes for cleaner code. Field has
     * been limited to a single bit.
     */
    uint8_t leaf_node;

    /* TREE
     * This is the left child of the huffman element and
     * is a semantic for the tree.
     */
    struct huffman_element *left_child;

    /* TREE
     * This is the right child of the huffman element and
     * is a semantic for the tree.
     */
    struct huffman_element *right_child;

    /* LIST
     * This is the field which holds the frequency of the
     * element and how many times it appears in our
     * buffer.
     */
    uint64_t frequency;

    /* LIST
     * This is the next huffman element of the list
     * and is a semantic for a list.
     */
    struct huffman_element *next;

    /* LIST
     * This is the previous huffman element of the list
     * and is a semantic for a list.
     */
    struct huffman_element *previous;
} helement_t;

/**
 * This function is used to build a huffman element
 * and initialize it to default values.
 */
helement_t* helement_create(uint8_t, uint8_t, uint64_t);

/**
 * This function is used to destroy a huffman element
 * after it is no longer needed.
 *
 * NOTE: This function does not recusively free
 * all element of the tree or list. Only this specific
 * element.
 */
void helement_free(helement_t*);

/**
 * This function is used to swap two huffman elements
 * cleanly for a list.
 */
void helement_swap_list(helement_t*, helement_t*);

/**
 * This function is used to print a single huffman element
 * for a list with all details.
 */
void helement_print_list(helement_t*);

/**
 * This function is used to print a single huffman element
 * for a tree with all details.
 */
void helement_print_tree(helement_t*);

#endif
