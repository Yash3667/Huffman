/*
 * This file defines the interface for using a huffman
 * list.
 *
 * Author: Yash Gupta <ygupta@ucsc.edu>
 * Copyright: Yash Gupta, UC Santa Cruz
 */
#include <ctype.h>
#include "huffman_list.h"

/**
 * This function is used to search for the presence of an
 * element in the huffman list. This interface is NOT
 * available to the user and is used as a helper
 * function.
 *
 * @param list Search this list
 * @param element Search for this element in the list
 * @return The node for the element or NULL.
 */
static helement_t*
_search(hlist_t *hlist, uint8_t element)
{
    helement_t *helement_node = hlist->list;

    while (helement_node) {
        if (helement_node->element == element) {
            return helement_node;
        }
        helement_node = helement_node->next;
    }

    return NULL;
}

/**
 * This function is used to create a new huffman element
 * and add it to the huffman list. It also increments
 * the count of the elements in the huffman list. This function
 * is not presented as an interface function.
 *
 * @param hlist The huffman list to add to.
 * @param element The element to be added.
 * @param leaf_node_flag Flag specifies if this element is a leaf node
 * @param frequency The initial frequency of this element
 * @return 0 on success and -1 on error
 */
static int
_add(hlist_t *hlist, uint8_t element, uint8_t leaf_node_flag, uint64_t frequency)
{
    helement_t *helement_node;

    helement_node = helement_create(element, leaf_node_flag, frequency);
    if (!helement_node) {
        return -1;
    }

    if (!(hlist->list)) {
        hlist->list = helement_node;
    } else {
        /*
         * We save the current list. Then we change the current
         * heads previous to be that of the new node we create.
         * Then, we make the head actually point to new node and
         * then finally, we change the next of the new node to
         * point to the previous head.
         */
        helement_t *temp = hlist->list;
        temp->previous = helement_node;

        hlist->list = helement_node;
        helement_node->next = temp;
    }

    hlist->count += 1;
    return 0;
}

/**
 * This is a recursive function and it is used to fix
 * the order of the frequency of elements after an element
 * increases its frequency by being added again. This function
 * is not presented as an interface function.
 *
 * @param hlist The huffman list we have.
 * @param helement_node The huffman element to fix order at.
 */
static void
_fix_order(hlist_t *hlist, helement_t *helement_node)
{
    helement_t *next_node = helement_node->next;

    if (next_node == NULL) {
        return;
    } else if (helement_node->frequency <= next_node->frequency) {
        return;
    } else {
        helement_swap_list(helement_node, next_node);

        /*
         * In case the element being swapped forward is the first
         * element in the list, we need to change where the list head
         * inside the huffman list points to as well. In our case,
         * we swap helement_node with new_node and so we need to make
         * that that we point hlist->list to new_node.
         */
        if (hlist->list == helement_node) {
            hlist->list = next_node;
        }
    }

    _fix_order(hlist, helement_node);
}

/**
 * This function is used to remove the first two elements from
 * a huffman list. As the list is always sorted, this simply
 * removes the two smallest elements. This function is not
 * presented as an interface.
 *
 * @param hlist The huffman list to remove elements from.
 */
static void
_remove(hlist_t *hlist)
{
    helement_t *first_temp;
    helement_t *second_temp;
    helement_t *third_temp;

    if (!hlist) {
        return;
    } else if (hlist_count(hlist) < 2) {
        return;
    } else {
        first_temp = hlist->list;
        second_temp = first_temp->next;
        third_temp = second_temp->next;

        /*
         * The concept here is quite simple. We simply take each of the
         * first three elements and reset them.
         *
         * -> For the first element, we set its next pointer to NULL. As
         * it is the first element, it should have no previous.
         *
         * -> For the second element, we set its next and its previous
         * pointer to be NULL.
         *
         * -> We check whether or not the third element exists. If it is not
         * NULL, we set its previous pointer to be NULL and then we set it
         * to become the first element in the list.
         */

        /* Reset First */
        first_temp->next = NULL;

        /* Reset Second */
        second_temp->previous = NULL;
        second_temp->next = NULL;

        /* Reset Third */
        if (third_temp) {
            third_temp->previous = NULL;
        }

        /* Set Third as First and reduce count */
        hlist->list = third_temp;
        hlist->count -= 2;
    }
}

/**
 * This function is used to build a new list of huffman
 * elements. It takes no arguments and returns a type
 * hlist_t with all values set to defaults.
 *
 * @return A huffman list or NULL
 */
hlist_t*
hlist_create()
{
    hlist_t *temp;

    temp = malloc(sizeof(hlist_t));
    if (!temp) {
        return NULL;
    }

    /* Set to all default values */
    temp->list = DEFAULT_LIST_START;
    temp->count = DEFAULT_LIST_COUNT;

    /* Return newly created hlist */
    return temp;
}

/**
 * This function is used to free a huffman list.
 *
 * NOTE: This function only free's fields which are based
 * on a list. Tree fields are not free'd
 */
void
hlist_free(hlist_t *list)
{
    // TODO: Free the huffman element list
    free(list);
}

/*
 * This function is used to add or increment the frequency of
 * an element to the huffman list. An element in the list
 * is always added to the beginning of the list.
 *
 * @param hlist The huffman list where we want to add or increment element
 * @param element The element to be added or incremented.
 * @param frequency The frequency of this element.
 * @return The newly created or modified element.
 */
helement_t*
hlist_add_increment_element(hlist_t *hlist, uint8_t element, uint64_t frequency)
{
    int err;
    int special_element;
    uint8_t leaf_node_flag;
    helement_t *helement_node;

    /*
     * The control flow of this function is simple but might look confusing.
     * If an element is found in the list and it is not a special element,
     * its frequency is simply incremented and then its order in the list is fixed.
     *
     * If it is found but it is a special element, it is none the less added to
     * the list again. This is because a special element is the congregation of
     * two leaf elements.
     *
     * If a new element is being added and it is not a special element, then we
     * set the leaf node flag to be 1 and we pass the frequency of 0 to the
     * _add function.
     */

    if (element == LIST_SPECIAL_ELEMENT && frequency != SPECIAL_ELEMENT_FREQUENCY) {
        special_element = 1;
        leaf_node_flag = 0;
    } else {
        special_element = 0;
        leaf_node_flag = 1;
    }

    helement_node = _search(hlist, element);
    if (!helement_node || special_element) {
        err = _add(hlist, element, leaf_node_flag, frequency);
        if (err) {
            return NULL;
        }
        helement_node = hlist->list;
    } else {
        helement_node->frequency += 1;
    }

    /*
     * -> If after incrementing the frequency of the element
     * the order of the list gets messed up, we need to fix that order.
     *
     * -> If there is an element which has a big frequency but wasn't initially
     * part of the list, it gets added to the front. We need to fix the order of
     * the list in that case as well.
     */
    _fix_order(hlist, helement_node);
    return helement_node;
}

/**
 * This function is used to return the count of the elements in
 * the list.
 *
 * @param hlist The huffman list for the count.
 * @return Count or LIST_BAD_COUNT
 */
uint64_t hlist_count(hlist_t *hlist)
{
    if (hlist) {
        return hlist->count;
    } else {
        return LIST_BAD_COUNT;
    }
}

/**
 * This function is used to acquire the two smallest elements in the
 * huffman list.
 *
 * NOTE: This function removes the elements froms the list.
 *
 * @param hlist The list we want to get elements from.
 * @param first The address to store first element at.
 * @param second The address to store second element at.
 * @return 0 on success and error code on failure
 */
int
hlist_get_two_min(hlist_t *hlist, helement_t **first, helement_t **second)
{
    uint64_t count;

    if (!hlist) {
        return -1;
    }

    count = hlist_count(hlist);
    if (count >= 2 && count != LIST_BAD_COUNT) {
        *first = hlist->list;
        *second = (*first)->next;
    } else {
        return -2;
    }

    _remove(hlist);
    return 0;
}

/**
 * This function is used to print a huffman list in its
 * entirety. This is used to debug the working of the list
 * and can be very helpful.
 *
 * @param hlist The huffman list we want to print
 */
void
hlist_print(hlist_t *hlist)
{
    helement_t *helement_list = hlist->list;

    // NOTE: Pretty print only until frequency < 7 digits
    printf("<================== PRINT LIST ==================>\n");
    printf("<------------------ COUNT: %3llu ------------------>\n", hlist->count);

    while (helement_list) {
        helement_print_list(helement_list);
        helement_list = helement_list->next;
    }
    printf("<================== PRINT STOP ==================>\n");
}
