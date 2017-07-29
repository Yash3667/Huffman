/*
 * This file decalres the interface for using a huffman
 * list.
 *
 * Author: Yash Gupta <ygupta@ucsc.edu>
 * Copyright: Yash Gupta, UC Santa Cruz
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "huffman_element.h"

#ifndef HUFFMAN_LIST_H
#define HUFFMAN_LIST_H

/* Defaults */
#define DEFAULT_LIST_COUNT      (0U)
#define DEFAULT_LIST_START      (NULL)

/*
 * This is the element which will be formed when we add two
 * leaf elements together. We define is element code for
 * convenience.
 */
#define LIST_SPECIAL_ELEMENT    (0xFFU)

/* Bad Count */
#define LIST_BAD_COUNT          (0xFFFFFFFFFFFFFFFFU)

typedef struct huffman_list {
    /* This is the beginning of the list */
    helement_t *list;

    /* This is the number of elements in the list */
    uint64_t count;
} hlist_t;

/**
 * This function is used to build a new list of
 * huffman elements. It takes no arguments and
 * returns a type hlist_t with all values set
 * to defaults.
 */
hlist_t* hlist_create();

/**
 * This function is used to build a free a huffman
 * list.
 *
 * NOTE: This function only free's fields which are based
 * on a list. Tree fields are not free'd
 */
void hlist_free(hlist_t*);

/**
 * This function is used to add or increment the frequency of
 * an element to the huffman list. An element in the list
 * is always added to the beginning of the list.
 */
helement_t* hlist_add_increment_element(hlist_t*, uint8_t, uint64_t);

/**
 * This function is used to return the count of the elements in
 * the list.
 */
uint64_t hlist_count(hlist_t*);

/**
 * This function is used to acquire the two smallest elements in the
 * huffman list.
 *
 * NOTE: This function removes the two elements which we acquire.
 */
int hlist_get_two_min(hlist_t*, helement_t**, helement_t**);

/**
 * This function is used to print a huffman list in its
 * entirety. This is used to debug the working of the list
 * and can be very helpful.
 */
void hlist_print(hlist_t*);

#endif
