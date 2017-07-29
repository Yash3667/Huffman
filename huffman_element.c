/*
 * This file defines the fields and attributes of our main
 * structure which is used to hold all the information for
 * a Huffman Element.
 *
 * Author: Yash Gupta <ygupta@ucsc.edu>
 * Copyright: Yash Gupta, UC Santa Cruz
 */
#include "huffman_element.h"

/**
 * This function is used to build a huffman element
 * and initialize it to default values.
 *
 * @param element This is the element for the helement.
 * @param leaf_node_flag Flag to specify is this is a leaf ndoe or not.
 * @param frequency The frequency of this element.
 * @return A default helement or NULL
 */
helement_t*
helement_create(uint8_t element, uint8_t leaf_node_flag, uint64_t frequency)
{
    helement_t *temp;

    /* Allocate space for a helement */
    temp = malloc(sizeof(helement_t));
    if (!temp) {
        return NULL;
    }

    /* Set element as passed */
    temp->element = element;

    /* Set Leaf Node */
    temp->leaf_node = leaf_node_flag;

    /* Set frequency */
    if (frequency == SPECIAL_ELEMENT_FREQUENCY) {
        temp->frequency = DEFAULT_ELEMENT_FREQUENCY;
    } else {
        temp->frequency = frequency;
    }

    /* Set to all default values */
    temp->left_child = DEFAULT_ELEMENT_LEFT_CHILD;
    temp->right_child = DEFAULT_ELEMENT_RIGHT_CHILD;
    temp->next = DEFAULT_ELEMENT_NEXT;
    temp->previous = DEFAULT_ELEMENT_PREVIOUS;

    /* Return newly created helement */
    return temp;
}

/**
 * This function is used to destroy a huffman element
 * after it is no longer needed.
 *
 * NOTE: This function does not recusively free
 * all element of the tree or list. Only this specific
 * element.
 *
 * @param this_element The element to be free'd
 */
void
helement_free(helement_t *this_element)
{
    free(this_element);
}

/**
 * This function is used to swap two huffman elements
 * cleanly for a list.
 *
 * @param first The first huffman element.
 * @param second The second huffman element.
 */
void
helement_swap_list(helement_t *first, helement_t *second)
{
    first->next = second->next;
    second->previous = first->previous;
    second->next = first;
    first->previous = second;

    /*
     * We need to make sure that the element which was originally
     * before first, now points to second. But that element should now
     * be the previous of second as first and second got swapped.
     */
    if (second->previous) {
        helement_t *temp = second->previous;
        temp->next = second;
    }

    /*
     * We need to make sure that the element which was original
     * after second has its previous set to first. Since first and
     * second got swapped, that element is now after first.
     */
    if (first->next) {
        helement_t *temp = first->next;
        temp->previous = first;
    }
}

/**
 * This function is used to print a single huffman element
 * for a list with all details.
 *
 * @param this_element The huffman element we want to print
 */
void
helement_print_list(helement_t *this_element)
{
    if (this_element == NULL) {
        printf("NULL Element\n");
    } else if (isprint(this_element->element)) {
        printf("ELEMENT: %3u => \'%c\' | FREQUENCY: %6llu | LEAF: %u\n",
            this_element->element,
            (char)(this_element->element),
            this_element->frequency,
            this_element->leaf_node);
    } else {
        printf("ELEMENT: %3u => [-] | FREQUENCY: %6llu | LEAF: %u\n",
            this_element->element,
            this_element->frequency,
            this_element->leaf_node);
    }
}

/**
 * This function is used to print a single huffman element
 * for a tree with all details.
 *
 * @param this_element The huffman element we want to print
 */
void
helement_print_tree(helement_t *this_element)
{
    if (this_element == NULL) {
        printf("NULL Element\n");
    } else if (isprint(this_element->element)) {
        printf("(%3u) \'%c\' | FREQUENCY: %6llu | LEAF: %u\n",
            this_element->element,
            (char)(this_element->element),
            this_element->frequency,
            this_element->leaf_node);
    } else {
        printf("(%3u) [-] | FREQUENCY: %6llu | LEAF: %u\n",
            this_element->element,
            this_element->frequency,
            this_element->leaf_node);
    }
}
