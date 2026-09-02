#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

typedef enum {
    LEFT  = 0,
    RIGHT = 1,
    THIS  = 2,
} Direction;

typedef enum {
    BLACK, RED,
} Colour;

typedef struct Node Node;
struct Node {
    Colour colour; /* this should later probably be stored within parent or
                    * val in some aligned bits */
    Node *parent;
    uint64_t val;
    Node *children[2];
};

void rbtree_init(Node *tree, uint64_t initial_val) {
    *tree = (Node) {0};
    tree->val = initial_val;
    tree->colour = BLACK;
}

// check whether a key would belong to the right or left of a node
Direction check_node_direction(Node *node, uint64_t key) {
    if (key > node->val)
        return RIGHT;
    else if (key < node->val)
        return LEFT;
    else return THIS;
}

// TODO, maybe make this not be recursive. if it doesnt exist then it should
// return the parent of what the node would be.
Node *rbtree_search(Node *tree, uint64_t key) {
    if (!tree) {
        fprintf(stderr, "got nullptr for tree in rbtree_search\n");
        return NULL;
    }
    Direction direction = check_node_direction(tree, key);
    switch (direction) {
    case RIGHT:
    case LEFT:
        if (tree->children[direction] == NULL) {
            printf("Child didn't exist, return intended parent of key %zu: %zu\n", key, tree->val);
            return tree; // if it doesnt exist return parent
        }
        return rbtree_search(tree->children[direction], key);
    case THIS:
        printf("Found node of key %zu\n", key);
        return tree;
    }
}

// return 0 on success, -1 on error
int rbtree_insert(Node *tree, uint64_t key) {
    Node *parent = rbtree_search(tree, key);

    Direction direction = check_node_direction(parent, key);
    if (direction == THIS) {
        fprintf(stderr, "A node with key %zu already exists in the red/black tree\n", key);
        return -1;
    }

    Node *node = (Node*) malloc(sizeof(Node));
    node->parent = parent;
    node->val    = key;
    node->colour = RED;
    memset(node->children, 0, sizeof(node->children));

    parent->children[direction] = node;
    return 0;
}

int main(void) {
    Node rbtree;
    rbtree_init(&rbtree, 69);

    if (rbtree_insert(&rbtree, 12) < 0) return -1;
    if (rbtree_insert(&rbtree, 14) < 0) return -1;
    if (rbtree_insert(&rbtree, 72) < 0) return -1;
    if (rbtree_insert(&rbtree, 18) < 0) return -1;

    rbtree_search(&rbtree, 69);
    rbtree_search(&rbtree, 14);
    rbtree_search(&rbtree, 72);
    rbtree_search(&rbtree, 18);
    rbtree_search(&rbtree, 95);

    return 0;
}
