/*
 * This file defines the interface for using a huffman
 * tree.
 *
 * Author: Yash Gupta <ygupta@ucsc.edu>
 * Copyright: Yash Gupta, UC Santa Cruz
 */
#include <ctype.h>
#include "huffman_tree.h"

/**
 * This function is used to parse a huffman tree and build
 * opcodes for each leaf node using a depth first algorthm.
 * Whenever we go into the left child, an opcode of 0 is appended
 * and whenever we go right, an opcode of 1 is appended. This function
 * is not presented as an interface function.
 *
 * @param helement_node The node of the huffman tree
 * @param opcode_table The table which holds the opcodes
 * @param opcode_string The string representing out working opcode.
 * @param count The count of all the nodes in the tree.
 */
void
_parse(helement_t *helement_node, char **opcode_table, char *opcode_string, uint64_t *count)
{
    /*
     * The working of this function is quite simple. We check
     * if we have arrived at a leaf node. In case we have, then
     * we copy the opcode_string into the correct table entry
     * and we return.
     *
     * If we are not a leaf node, then we need to traverse through
     * our children. When traversing through the left child, we append
     * an opcode of 0 to the opcode string. When traversing through
     * right child, we append an opcode of 1 to the opcode string.
     *
     * Every time after returning from the children, we rollback
     * by making the last character of the opcode_string a NULL again.
     */
    const char *LEFT_OPCODE = "0";
    const char *RIGHT_OPCODE = "1";

    *count += 1;
    if (helement_node->leaf_node) {
        opcode_table[helement_node->element] = strdup(opcode_string);
    } else {
        if (helement_node->left_child) {
            strcat(opcode_string, LEFT_OPCODE);
            _parse(helement_node->left_child, opcode_table, opcode_string, count);
            opcode_string[strlen(opcode_string) - 1] = '\0';
        }

        if (helement_node->right_child) {
            strcat(opcode_string, RIGHT_OPCODE);
            _parse(helement_node->right_child, opcode_table, opcode_string, count);
            opcode_string[strlen(opcode_string) - 1] = '\0';
        }
    }
}

/**
 * This function uses depth first recursive algorithm to assign
 * each huffman element in the huffman tree an index and then
 * print the elements using an in-order traversal. This function
 * is not presented as an interface function.
 *
 * @param The current node of the huffman tree.
 * @param index The index of this node.
 * @return The total indexes which have been traversed.
 */
uint64_t
_print(helement_t *helement_node, uint64_t index)
{
    uint64_t received_index;

    if (helement_node->left_child) {
        received_index = _print(helement_node->left_child, index + 1);
    }

    printf("{%4llu} ", index);
    helement_print_tree(helement_node);

    if (helement_node->right_child) {
        /*
         * In case we have a right child, we need to return however
         * much it incremented itself upto.
         */
        index = _print(helement_node->right_child, received_index + 1);
    }

    return index;
}

/**
 * This function recursively writes out the contents of the
 * huffman tree elements to a file descriptor. It returns the offset
 * after which the next thing should begin writing. This function
 * is not presented as an interface function.
 *
 * @param helement_node The node to write
 * @param fd The file to write to
 * @param Success or Error
 */
ssize_t
_output(helement_t *helement_node, int fd, uint64_t offset)
{
    ssize_t ret;
    ssize_t size_to_write;
    ssize_t bytes_written;

    /* Write the element value */
    size_to_write = sizeof(helement_node->element);
    bytes_written = pwrite(fd, &(helement_node->element), size_to_write, offset);
    if (bytes_written < size_to_write) {
        return -1;
    } else {
        offset += bytes_written;
    }

    /* Write the leaf flag value */
    size_to_write = sizeof(helement_node->leaf_node);
    bytes_written = pwrite(fd, &(helement_node->leaf_node), size_to_write, offset);
    if (bytes_written < size_to_write) {
        return -1;
    } else {
        offset += bytes_written;
    }

    /* Leaf node simply returns the current offset */
    ret = offset;

    /* Write out the left sub-tree */
    if (helement_node->left_child) {
        ret = _output(helement_node->left_child, fd, offset);
        if (ret == -1) {
            return -1;
        }
    }

    /* Write out the right sub-tree */
    if (helement_node->right_child) {
        ret = _output(helement_node->right_child, fd, ret);
        if (ret == -1) {
            return -1;
        }
    }

    return ret;
}

/**
 * Read in a single object from a binary file to construct a
 * huffman element. This function is not presented as an
 * interface function.
 *
 * @param fd The file to read from.
 * @param index The index of the element
 * @return A huffman element
 */
helement_t*
_input_object(int fd, uint64_t index)
{
    uint64_t element_offset, leaf_flag_offset;
    uint8_t element, leaf_flag;
    ssize_t size_to_read;
    ssize_t bytes_read;
    helement_t *this_element;

    /* Get offsets */
    element_offset = TREE_INPUT_ELEMENT_OFFSET(index);
    leaf_flag_offset = TREE_INPUT_LEAFNODE_OFFSET(index);

    /* Read in the element */
    size_to_read = TREE_INPUT_ELEMENT_SIZE;
    bytes_read = pread(fd, &element, size_to_read, element_offset);
    if (bytes_read < size_to_read) {
        return NULL;
    }

    /* Read in the leaf flag */
    size_to_read = TREE_INPUT_LEAFNODE_SIZE;
    bytes_read = pread(fd, &leaf_flag, size_to_read, leaf_flag_offset);
    if (bytes_read < size_to_read) {
        return NULL;
    }

    /* Create a huffman element out of these values */
    this_element = helement_create(element, leaf_flag, SPECIAL_ELEMENT_FREQUENCY);
    if (!this_element) {
        return NULL;
    }

    return this_element;
}

/**
 * This function uses a pre-order fetch algorithm to read
 * in a huffman tree from a file recursively. This function
 * is not presented as an interface function.
 *
 * @param fd The file we read from
 * @param htree The huffman tree we are constructing
 * @param helement_node The tree node we are currently at
 * @param index The index of the current element
 * @param leaf_flag Flag specifying if this is a leaf node
 * @return A huffman tree
 */
uint64_t
_input(int fd, htree_t *htree, helement_t *helement_node, uint64_t index, uint8_t leaf_flag)
{
    uint8_t child_leaf_flag;
    uint64_t leftc_index, rightc_index;
    helement_t *left_child;
    helement_t *right_child;

    /*
     * The working of this function is rather complex and dependent
     * on the design of how I output the huffman tree onto a file.
     *
     * Finding the index of the left child is really simple. It is
     * simply your own index + 1. This is because of how pre-order
     * traversal works.
     *
     * Finding the index of the right child is a lot more complex.
     * Something to realize is that there is never a case where
     * the right child is a leaf node and the left child is not.
     * That in consideration, there are two cases using which
     * we calculate the index of the right child.
     *
     * 1. When the left child was a leaf node
     * If this is the case, the index of the right child is simply
     * the index of the left child + 1. This is again because of how
     * pre-order traversal works.
     *
     * 2. When the left child is not a leaf node
     * In this case, the index of the right child is the index
     * of your left childs right child + 1. I'm not sure if this is
     * because of pre-order traversal or not, but this property holds
     * true in the tree I emit to a file.
     * right_index = left_child->right_child->index + 1;
     *
     * Once we read in the children, we simply set them to their
     * parents accordingly.
     */

    if (leaf_flag) {
        return index;
    } else {
        /*
         * Calculate left childs index and then read it in. Set its
         * parent as us and then set a flag which tells us if this is
         * a leaf node or not.
         */
        leftc_index = index + 1;
        left_child = _input_object(fd, leftc_index);
        if (!left_child) {
            return TREE_BAD_COUNT;
        } else {
            child_leaf_flag = left_child->leaf_node;
            helement_node->left_child = left_child;
        }

        /* This value should give us the index for the right child because
         * if the left child is a leaf node then we simply get its value and
         * increment one.
         *
         * If not a leaf node, then we get the value of the left childs right
         * child because of what we return down below.
         */
        rightc_index = _input(fd, htree, left_child, leftc_index, child_leaf_flag);
        if (rightc_index == TREE_BAD_COUNT) {
            return TREE_BAD_COUNT;
        } else {
            rightc_index += 1;
        }

        right_child = _input_object(fd, rightc_index);
        if (!right_child) {
            return TREE_BAD_COUNT;
        } else {
            child_leaf_flag = right_child->leaf_node;
            helement_node->right_child = right_child;
        }

        /* This will end up returning the index of the right child */
        return _input(fd, htree, right_child, rightc_index, child_leaf_flag);
    }
}

/**
 * This function is used to build a new tree of huffman elements.
 * It takes no arguments and returns a type htree_t with all values set
 * to defaults.
 */
htree_t*
htree_create()
{
    htree_t *temp;

    temp = malloc(sizeof(htree_t));
    if (!temp) {
        return NULL;
    }

    /* Set to all default values */
    temp->root = DEFAULT_TREE_START;
    temp->count = DEFAULT_TREE_COUNT;
    temp->_parsed = DEFAULT_PARSE_BIT;

    /* Return newly created htree */
    return temp;
}

/**
 * This function is used to free a huffman
 * tree.
 *
 * NOTE: This function only free's fields which are based
 * on a tree. List fields are not free'd.
 */
void htree_free(htree_t *htree)
{
    htree = NULL;
}

/**
 * This function is used to add an element to the huffman tree.
 * An element in the tree is always added using recursive in-order
 * traversal of the tree.
 *
 * @param htree The tree to which the element needs to be added.
 * @param helement_node The huffman element which needs to be added.
 * @return The element which was added or NULL.
 */
helement_t*
htree_add_element(htree_t *htree, helement_t *helement_node)
{
    if (!htree) {
        return NULL;
    }

    /*
     * If we reach this part of the code, then it means that a new
     * element will definitely be added. Hence, we reset the parsed bit
     * in the structure.
     */
    htree->_parsed = 0;

    /* Check if tree is empty */
    if (!(htree->root)) {
        htree->root = helement_node;
        htree->count += 1;
    } else {
        // Recursive procedure.
    }

    return helement_node;
}

/**
 * This function is used to return the count of the elements in
 * the tree.
 */
uint64_t
htree_count(htree_t *htree)
{
    htree = NULL;
    return 0;
}

/**
 * This function is used to connect a element to its
 * children so that we can build a node element without
 * the constructor.
 *
 * @param parent The parent whose children we want to change
 * @param left_child The left child we want to set.
 * @param right_child The right child we want to set.
 * @return 0 for success or negative code for failure.
 */
int htree_connect(helement_t *parent, helement_t *left_child, helement_t *right_child)
{
    /*
     * We confirm that parent is indeed a valid element and
     * then we confirm that parent is not meant to be a leaf element
     * because leaf elements cannot have any children.
     */

    if (!parent) {
        return -1;
    } else if (parent->leaf_node) {
        return -2;
    } else {
        /*
         * We always want that if it is a special element, it
         * ends up being on the left child. This does not change
         * any of the working but constructs a tree which is more
         * conventional when we think of a huffman tree.
         */

        if (right_child->leaf_node) {
            parent->left_child = left_child;
            parent->right_child = right_child;
        } else {
            parent->left_child = right_child;
            parent->right_child = left_child;
        }
    }
    return 0;
}

/**
 * This function is used to parse the entire huffman tree and
 * build opcodes for each  of the leaf nodes.
 *
 * @param The tree we want to parse.
 * @return The opcode table or NULL
 */
char**
htree_parse(htree_t *htree)
{
    /* An opcode should never be larger than 256 bytes */
    const int MAX_OPCODE = 256;

    int i;
    char *opcode_string;
    char **opcode_table;

    /*
     * Confirm that the huffman tree is a valid tree with allocated
     * resources.
     */
    if (!htree) {
        return NULL;
    } else if (!(htree->root)) {
        return NULL;
    }

    /*
     * Initialize the table and make space to hold 256 pointers to
     * characters. This is again because we potentially need to
     * be able to hold opcodes for every possible ASCII character.
     */
    opcode_table = malloc(sizeof(char *) * TREE_MAX_TABLE_SIZE);
    if (!opcode_table) {
        return NULL;
    }

    /*
     * We initialize every pointer in the table to be NULL. During
     * the parse phase, if we come across a leaf node, then a strdup
     * is used to set the opcode for the specific element.
     */
    for (i = 0; i < TREE_MAX_TABLE_SIZE; i++) {
        opcode_table[i] = NULL;
    }

    /*
     * The default length of the opcode_length is zero and so
     * we set the first character in the string to be 0.
     */
    opcode_string = malloc(sizeof(char) * MAX_OPCODE);
    if (!opcode_string) {
        free(opcode_table);
        return NULL;
    } else {
        opcode_string[0] = '\0';
    }

    /*
     * We set the bit in the huffman tree signifying that this
     * version of the tree has been parsed and writing this tree
     * to a file will be valid.
     *
     * We can use this function to our advantage here in that
     * we can use the _parse function to calculate the count of
     * the elements in the tree.
     */
    htree->_parsed = 1;
    htree->count = 0;
    _parse(htree->root, opcode_table, opcode_string, &(htree->count));
    return opcode_table;
}

/**
 * This function is used to output the tree to a binary file
 * and it does so using a depth first algorithm. It returns the offset
 * after which the next thing should begin writing.
 *
 * @param htree The huffman tree we output.
 * @param fd The file we output to.
 * @param The offset to begin writing at.
 * @return 0 on success or error code
 */
ssize_t
htree_output(htree_t *htree, int fd, uint64_t offset)
{
    ssize_t ret;
    ssize_t size_to_write;
    ssize_t bytes_written;

    /*
     * Confirm that we have a valid tree.
     *
     * 1. If tree is NULL, then it is not a valid tree.
     * 2. In case the version of the tree has not been parsed.
     * 3. In case the tree is empty.
     *
     * NOTE: It is crucial that check for 3 comes after check for 2 as
     * the count may be unreliable unless the tree has been parsed.
     */

    if (!htree) {
        return -1;
    } else if (!(htree->_parsed)) {
        return -2;
    } else if (htree->count < 1) {
        return -3;
    }

    /*
     * Write out the count of the number of elements in the
     * huffman tree and then recursively write out all the elements
     * in the tree.
     */
    size_to_write = sizeof(htree->count);
    bytes_written = write(fd, &(htree->count), size_to_write);
    if (bytes_written < size_to_write) {
        return -4;
    } else {
        offset += size_to_write;
    }

    ret = _output(htree->root, fd, offset);
    if (ret == -1) {
        return -5;
    }

    return ret;
}

/**
 * This function is used to take input for a huffman tree
 * from a binary file and it does so using a pre-order fetch
 * algorithm.
 *
 * @param fd The file we want to read from
 * @return A fully constructed huffman tree or error
 */
htree_t*
htree_input(int fd)
{
    /*
     * This is for the way we read in the tree. Essentially, we pass in
     * this flag so that the helper function knows that the root is not
     * a leaf node.
     */
    const int ROOT_LEAF_FLAG = 0;

    ssize_t size_to_read;
    ssize_t bytes_read;
    htree_t *this_tree;
    helement_t *helement_node;

    /* Create the structure for a huffman tree */
    this_tree = htree_create();
    if (!this_tree) {
        return NULL;
    }

    /*
     * We need to read in the count and the root element for the tree
     * and then call the recursive pre-order fetch function to read
     * in the rest of the tree.
     */
    size_to_read = TREE_INPUT_COUNT_SIZE;
    bytes_read = read(fd, &(this_tree->count), size_to_read);
    if (bytes_read < size_to_read) {
        free(this_tree);
        return NULL;
    }

    /* Read in root */
    helement_node = _input_object(fd, TREE_INPUT_ROOT_ELEMENT_INDEX);
    if (!helement_node) {
        free(this_tree);
        return NULL;
    } else {
        this_tree->root = helement_node;
    }

    _input(fd, this_tree, helement_node, TREE_INPUT_ROOT_ELEMENT_INDEX, ROOT_LEAF_FLAG);
    return this_tree;
}

/**
 * This function is used to state step the huffman tree
 * depending on the opcode which is given. An opcode of
 * 0 moves the tree towards the left child and opcode
 * of 1 moves the tree towards the right child.
 *
 * @param htree The huffman tree we want to step through
 * @param helement_node The node of the tree we are at
 * @param element A pointer to the element we return
 * @param opcode The opcode we state step on
 * @return The node we are currently at
 */
helement_t*
htree_state_step(htree_t *htree, helement_t *helement_node, int *element, int opcode)
{
    /*
     * Working is again quite simple. We confirm the validity of
     * the arguments passed to us and then use the provided opcode
     * to step through the nodes of the tree.
     */
    const int LEFT_OPCODE = 0;
    const int RIGHT_OPCODE = 1;

    if (!htree) {
        return NULL;
    } else if (!helement_node) {
        return NULL;
    } else if (opcode > RIGHT_OPCODE || opcode < LEFT_OPCODE) {
        return NULL;
    }

    /* Move depending on the opcode */
    if (opcode == LEFT_OPCODE) {
        helement_node = helement_node->left_child;
    } else {
        helement_node = helement_node->right_child;
    }

    /*
     * If we are a leaf node, then we set the element to
     * be an element and we reset the node we pass to the root
     * of the tree so that next time we call this function we begin
     * again from the root.
     */
    if (helement_node->leaf_node) {
        *element = helement_node->element;
        helement_node = htree->root;
    } else {
        *element = -1;
    }

    return helement_node;
}

/**
 * This function is used to print a huffman tree in its
 * entirety. This is used to debug the working of the tree
 * and can be very helpful.
 *
 * @param htree The huffman tree we need to print.
 */
void htree_print(htree_t *htree)
{
    helement_t *helement_tree = htree->root;
    uint64_t print_count;

    // NOTE: Pretty print only until frequency < 7 digits
    printf("<================= PRINT TREE ================>\n");
    printf("<----------------- COUNT: %3llu ---------------->\n", htree->count);

    print_count = _print(helement_tree, 1);

    printf("<----------------- COUNT: %3llu ---------------->\n", print_count);
    printf("<================= PRINT STOP ================>\n");
}
