// Challenge #3 in chapter 1 of "Crafting Interpreters" by Robert Nystrom:
//
//    "To get some practice with pointers, define a doubly linked list of heap-
//    allocated strings. Write functions to insert, find, and delete items from
//    it. Test them."
//
// I tried to emulate the interface a "real" library the best I could; this
// source file contains the both the library itself and a modest test suite.
//

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// =========== List definition and API start =========== //

typedef struct Node Node;
struct Node {
    Node *prev;
    Node *next;
    char *data;
};

typedef struct {
    Node *head;
    Node *last;
} List;

// Crude error reporting system; these definitions and functions would be
// 'static' if the library was in its own file.
typedef enum {
    ALLOC_FAIL,
    LEN_INVALID,
    NULL_PTR
} Error;
const char *ERROR[] = {
    "Dynamic memory allocation failed.",
    "Tried to initialize something with negative length.",
    "Function received null pointer argument."
};
void print_error(Error e)
{
    fprintf(stderr, "list_lib error: %s\n", ERROR[e]);
}

// Inserts 'new_node' into 'list' after 'node'.
// Returns 0 if successful.
// Returns an error enum if unsuccessful.
int insert_after(List *list, Node *node, Node *new_node)
{
    if (list == NULL || node == NULL || new_node == NULL) {
        print_error(NULL_PTR);
        return NULL_PTR;
    }

    new_node->prev = node;
    if (node->next == NULL) {
        new_node->next = NULL;
        list->last = new_node;
    } else {
        new_node->next = node->next;
        node->next->prev = new_node;
    }
    node->next = new_node;

    return 0;
}

// Inserts 'new_node' into 'list' before 'node'.
// Returns 0 if successful.
// Returns an error enum if unsuccessful.
int insert_before(List *list, Node *node, Node *new_node)
{
    if (list == NULL || node == NULL || new_node == NULL) {
        print_error(NULL_PTR);
        return NULL_PTR;
    }

    new_node->next = node;
    if (node->prev == NULL) {
        new_node->prev = NULL;
        list->head = new_node;
    } else {
        new_node->prev = node->prev;
        node->prev->next = new_node;
    }
    node->prev = new_node;

    return 0;
}

// Inserts 'new_node' at the front of the list.
// Returns 0 if successful.
// Returns an error enum if unsuccessful.
int insert_front(List *list, Node *new_node)
{
    if (list == NULL || new_node == NULL) {
        print_error(NULL_PTR);
        return NULL_PTR;
    }

    if (list->head == NULL) {
        list->head = new_node;
        list->last = new_node;
        new_node->prev = NULL;
        new_node->next = NULL;
    } else {
        insert_before(list, list->head, new_node);
    }

    return 0;
}

// Inserts 'new_node' at the end of the list.
// Returns 0 if successful.
// Returns an error enum if unsuccessful.
int insert_end(List *list, Node *new_node)
{
    if (list == NULL || new_node == NULL) {
        print_error(NULL_PTR);
        return NULL_PTR;
    }

    if (list->last == NULL) {
        insert_front(list, new_node);
    } else {
        insert_after(list, list->last, new_node);
    }

    return 0;
}

// Removes 'node' from 'list'
// Returns 0 if successful.
// Returns an error enum if unsuccessful.
int remove_node(List *list, Node *node)
{
    if (list == NULL || node == NULL) {
        print_error(NULL_PTR);
        return NULL_PTR;
    }

    // Case: 'node' is list head
    if (node->prev == NULL) {
        list->head = node->next;
    } else {
        node->prev->next = node->next;
    }
    // Case: 'node' is list last
    if (node->next == NULL) {
        list->last = node->prev;
    } else {
        node->next->prev = node->prev;
    }

    free(node->data);
    free(node);
    return 0;
}

// Find a node in 'list' with contents 'data'.
// Return pointer to node if found.
// Return NULL if not found or if given arguments are NULL.
Node *find(const List *list, const char *data)
{
    if (list == NULL || data == NULL) {
        print_error(NULL_PTR);
        return NULL;
    }

    Node *i = list->head;
    while (i != NULL) {
        if (strcmp(i->data, data) == 0)
            return i;
        i = i->next;
    }
    return NULL;
}

// Allocate a new node with 'data' payload
// Returns pointer to new node if successful.
// Return NULL if unsuccessful.
Node *make_node(const char *data)
{
    Node *new_node = malloc(sizeof(*new_node));
    if (new_node == NULL) {
        print_error(ALLOC_FAIL);
        return NULL;
    }
    size_t bytes = strlen(data) + 1;
    new_node->data = malloc(sizeof(*new_node->data) * bytes);
    if (new_node->data == NULL) {
        print_error(ALLOC_FAIL);
        free(new_node);
        return NULL;
    }
    new_node->next = NULL;
    new_node->prev = NULL;
    strcpy(new_node->data, data);
    return new_node;
}

// Deallocate all dynamic memory associated with 'list'
void dealloc_list(List *list)
{
    if (list == NULL) {
        return;
    }
    Node *i = list->head;
    while (i != NULL) {
        Node *tmp = i->next;
        free(i->data);
        free(i);
        i = tmp;
    }
}

// Attempts to dynamically allocate 'data_len' Node objects for 'list'.
// You must provide a static array of string data that will be copied into the
// newly allocated data structure.
// Returns 0 if successful.
// Returns an error enum if unsuccessful.
int init_list(List *list, const char *data[], int data_len)
{
    if (data == NULL) {
        print_error(NULL_PTR);
        return NULL_PTR;
    }
    if (data_len < 0) {
        print_error(LEN_INVALID);
        return LEN_INVALID;
    }

    list->head = NULL;
    list->last = NULL;

    // Allocate each node
    int i;
    for (i = 0; i < data_len; ++i) {
        Node *new_node = make_node(data[i]);
        if (new_node == NULL) {
            dealloc_list(list);
            return ALLOC_FAIL;
        }
        insert_end(list, new_node);
    }

    return 0;
}

// =========== List definition and API end =========== //


// =========== List test functions start =========== //

const char *TEST_DATA[] = {
    "ABC0",
    "ABC1",
    "ABC2",
    "ABC3",
    "ABC4",
    "ABC5",
    "ABC6",
    "ABC7",
    "ABC8",
    "ABC9"
};

const char **TEST_DATA_NULL = NULL;

const int DATA_LEN = sizeof(TEST_DATA) / sizeof(TEST_DATA[0]);
const int DATA_LEN_NEGATIVE = -DATA_LEN;

void test_print_list(List *list)
{
    Node *i = list->head;
    puts("==== List start ====");
    while (i != NULL) {
        puts(i->data);
        i = i->next;
    }
    puts("==== List end ====");
}

// For comparing integer values
void assert_int_equal(int expected, int actual, const char *test_name)
{
    printf("%s: ", test_name);
    if (actual != expected)
        printf("FAIL: expected %d, got %d\n", expected, actual);
    else
        printf("PASS\n");
}

// For comparing Node pointer values
void assert_node_ptr_equal(Node *expected, Node *actual, const char *test_name)
{
    printf("%s: ", test_name);
    if (actual != expected)
        printf("FAIL: expected %p, got %p\n", expected, actual);
    else
        printf("PASS\n");
}

// Call init_list with NULL 'data' argument
void test_init_list_null_data()
{
    List list;
    int init_ret = init_list(&list, TEST_DATA_NULL, DATA_LEN);
    if (init_ret == 0)
        dealloc_list(&list);
    assert_int_equal(NULL_PTR, init_ret, "test_init_list_null_data");
}

// Call init_list with negative 'data_len' argument
void test_init_list_negative_data_len()
{
    List list;
    int init_ret = init_list(&list, TEST_DATA, DATA_LEN_NEGATIVE);
    if (init_ret == 0)
        dealloc_list(&list);
    assert_int_equal(LEN_INVALID, init_ret, "test_init_list_negative_data_len");
}

// Call init_list with NULL 'data' argument
void test_init_list_success()
{
    List list;
    int init_ret = init_list(&list, TEST_DATA, DATA_LEN);
    if (init_ret == 0)
        dealloc_list(&list);
    assert_int_equal(0, init_ret, "test_init_list_success");
}

// Insert a node after list head
void test_insert_after_head()
{
    List list;
    int init_ret = init_list(&list, TEST_DATA, DATA_LEN);
    int ins_ret = insert_after(&list, list.head, make_node("zzzz"));
    test_print_list(&list);
    if (init_ret == 0)
        dealloc_list(&list);
    assert_int_equal(0, ins_ret, "test_insert_after_head");
}

// Insert a node after a node somewhere in the middle of a list
void test_insert_after_middle()
{
    List list;
    int init_ret = init_list(&list, TEST_DATA, DATA_LEN);
    Node *mid = list.head->next->next->next->next; // ABC4
    int ins_ret = insert_after(&list, mid, make_node("zzzz"));
    test_print_list(&list);
    if (init_ret == 0)
        dealloc_list(&list);
    assert_int_equal(0, ins_ret, "test_insert_after_middle");
}

// Insert a node after list last
void test_insert_after_last()
{
    List list;
    int init_ret = init_list(&list, TEST_DATA, DATA_LEN);
    int ins_ret = insert_after(&list, list.last, make_node("zzzz"));
    test_print_list(&list);
    if (init_ret == 0)
        dealloc_list(&list);
    assert_int_equal(0, ins_ret, "test_insert_after_last");
}

// Insert a node before list head
void test_insert_before_head()
{
    List list;
    int init_ret = init_list(&list, TEST_DATA, DATA_LEN);
    int ins_ret = insert_before(&list, list.head, make_node("zzzz"));
    test_print_list(&list);
    if (init_ret == 0)
        dealloc_list(&list);
    assert_int_equal(0, ins_ret, "test_insert_before_head");
}

// Insert a node before a node somewhere in the middle of a list
void test_insert_before_middle()
{
    List list;
    int init_ret = init_list(&list, TEST_DATA, DATA_LEN);
    Node *mid = list.head->next->next->next->next; // ABC4
    int ins_ret = insert_before(&list, mid, make_node("zzzz"));
    test_print_list(&list);
    if (init_ret == 0)
        dealloc_list(&list);
    assert_int_equal(0, ins_ret, "test_insert_before_middle");
}

// Insert a node before list last
void test_insert_before_last()
{
    List list;
    int init_ret = init_list(&list, TEST_DATA, DATA_LEN);
    int ins_ret = insert_before(&list, list.last, make_node("zzzz"));
    test_print_list(&list);
    if (init_ret == 0)
        dealloc_list(&list);
    assert_int_equal(0, ins_ret, "test_insert_before_last");
}

// Test call to insert_front
void test_insert_front_success()
{
    List list;
    int init_ret = init_list(&list, TEST_DATA, DATA_LEN);
    int ins_ret = insert_front(&list, make_node("zzzz"));
    test_print_list(&list);
    if (init_ret == 0)
        dealloc_list(&list);
    assert_int_equal(0, ins_ret, "test_insert_front_success");
}

// Test call to insert_end
void test_insert_end_success()
{
    List list;
    int init_ret = init_list(&list, TEST_DATA, DATA_LEN);
    int ins_ret = insert_end(&list, make_node("zzzz"));
    test_print_list(&list);
    if (init_ret == 0)
        dealloc_list(&list);
    assert_int_equal(0, ins_ret, "test_insert_end_success");
}

// Remove the list head
void test_remove_node_head()
{
    List list;
    int init_ret = init_list(&list, TEST_DATA, DATA_LEN);
    int rem_ret = remove_node(&list, list.head);
    test_print_list(&list);
    if (init_ret == 0)
        dealloc_list(&list);
    assert_int_equal(0, rem_ret, "test_remove_node_head");
}

// Remove a node in the middle
void test_remove_node_middle()
{
    List list;
    int init_ret = init_list(&list, TEST_DATA, DATA_LEN);
    Node *mid = list.head->next->next->next->next; // ABC4
    int rem_ret = remove_node(&list, mid);
    test_print_list(&list);
    if (init_ret == 0)
        dealloc_list(&list);
    assert_int_equal(0, rem_ret, "test_remove_node_middle");
}

// Remove the list last
void test_remove_node_last()
{
    List list;
    int init_ret = init_list(&list, TEST_DATA, DATA_LEN);
    int rem_ret = remove_node(&list, list.last);
    test_print_list(&list);
    if (init_ret == 0)
        dealloc_list(&list);
    assert_int_equal(0, rem_ret, "test_remove_node_last");
}

// Test find function when node is list head
void test_find_head()
{
    List list;
    int init_ret = init_list(&list, TEST_DATA, DATA_LEN);
    Node *t = find(&list, "ABC0");

    assert_node_ptr_equal(NULL, t->prev, "test_find_head1");
    assert_node_ptr_equal(list.head->next, t->next, "test_find_head2");
    assert_int_equal(0, strcmp("ABC0", t->data), "test_find_head3");

    if (init_ret == 0)
        dealloc_list(&list);
}

// Test find function when node is somewhere in the middle
void test_find_middle()
{
    List list;
    int init_ret = init_list(&list, TEST_DATA, DATA_LEN);
    Node *t = find(&list, "ABC4");

    Node *mid = list.head->next->next->next->next; // ABC4
    assert_node_ptr_equal(mid->prev, t->prev, "test_find_middle1");
    assert_node_ptr_equal(mid->next, t->next, "test_find_middle2");
    assert_int_equal(0, strcmp("ABC4", t->data), "test_find_middle3");

    if (init_ret == 0)
        dealloc_list(&list);
}

// Test find function when node is list last
void test_find_last()
{
    List list;
    int init_ret = init_list(&list, TEST_DATA, DATA_LEN);
    Node *t = find(&list, "ABC9");

    assert_node_ptr_equal(list.last->prev, t->prev, "test_find_last1");
    assert_node_ptr_equal(NULL, t->next, "test_find_last2");
    assert_int_equal(0, strcmp("ABC9", t->data), "test_find_last3");

    if (init_ret == 0)
        dealloc_list(&list);
}

// =========== List test functions end =========== //

int main()
{
    if (isatty(STDERR_FILENO)) {
        printf("==== Running list_lib test suite... suggest redirecting stderr"
               " to /dev/null during tests ====\n\n");
    }

    test_init_list_null_data();
    test_init_list_negative_data_len();
    test_init_list_success();
    test_insert_after_head();
    test_insert_after_middle();
    test_insert_after_last();
    test_insert_before_head();
    test_insert_before_middle();
    test_insert_before_last();
    test_insert_front_success();
    test_insert_end_success();
    test_remove_node_head();
    test_remove_node_middle();
    test_remove_node_last();
    test_find_head();
    test_find_middle();
    test_find_last();
    return 0;
}
