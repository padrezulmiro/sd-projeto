#include <stdlib.h>

#include "entry.h"
#include "list-private.h"
#include "list.h"

struct list_t *list_create() {
    struct list_t *list = (struct list_t *) calloc(1, sizeof(struct list_t));
    return list;
}

int list_add(struct list_t *list, struct entry_t *entry) {
    // TODO extract new_node creation to a separate function
    // TODO refactor this function

    if (!list->head) {
        struct node_t* new_node =
            (struct node_t*) calloc(1, sizeof(struct node_t));
        new_node->entry = entry;
        new_node->next = NULL;
        list->head = new_node;
        return 0;
    }

    struct node_t* node = list->head;
    int cmp_ret = entry_compare(entry, node->entry);

    if (cmp_ret == 0) {
        if (entry_replace(node->entry, entry->key, entry->value) == -1) {
            return -1;
        } else {
            return 1;
        }
    } else if (cmp_ret == -1) {
        struct node_t *new_node =
            (struct node_t *)calloc(1, sizeof(struct node_t));
        new_node->entry = entry;
        new_node->next = node;
        list->head = new_node;
        return 0;
    }

    struct node_t *previous_node;
    while (node->next != 0) {
        previous_node = node;
        node = node->next;
        cmp_ret = entry_compare(entry, node->entry);

        if (cmp_ret == 0) {
            if (entry_replace(node->entry, entry->key, entry->value) == -1) {
              return -1;
            } else {
              return 1;
            }
        } else if (cmp_ret == -1) {
            struct node_t *new_node =
                (struct node_t *) calloc(1, sizeof(struct node_t));
            new_node->entry = entry;
            new_node->next = node;
            previous_node->next = new_node;
            return 0;
        }
    }

    struct node_t *new_node = (struct node_t *) calloc(1, sizeof(struct node_t));
    new_node->entry = entry;
    new_node->next = node;
    previous_node->next = new_node;
    return 0;
}