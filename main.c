#include <stdlib.h>
#include <assert.h>
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

typedef struct {
    Node *root;
} Tree;

// check whether a key would belong to the right or left of a node
Direction check_node_direction(Node *node, uint64_t key) {
    if (key > node->val)
        return RIGHT;
    else if (key < node->val)
        return LEFT;
    else return THIS;
}

#define FLIP_DIR(dir) (assert(dir != THIS), (dir == RIGHT) ? LEFT : RIGHT)

/* rotates from a specific node and returns the new root node which takes the place of
 * the previous *node in the tree.
 * I tried to do it myself but I was looking at the wikipedia samples as I was
 * implementing this so it is probably quite similar, therefore here is some credit for ya:
 *
 * https://en.wikipedia.org/wiki/Red-black_tree */
Node *rbtree_rotate(Tree *tree, Node *node, Direction dir) {
    assert(dir != THIS && "invalid direction for rbtree_rotate");

    Node *parent    = node->parent;
    Node *new_root  = node->children[FLIP_DIR(dir)];
    Node *new_child = new_root->children[dir];

    node->children[FLIP_DIR(dir)] = new_child;
    
    if (new_child) new_child->parent = node;
    
    new_root->children[dir] = node;
    new_root->parent = parent;
    node->parent = new_root;

    if (parent) {
        Direction new_dir = (node == parent->children[RIGHT]) ? RIGHT : LEFT;
        parent->children[new_dir] = new_root;
    } else tree->root = new_root;

    return new_root;
}

// TODO, maybe make this not be recursive. if it doesnt exist then it should
// return the parent of what the node would be.
Node *rbtree_search(Node *root, uint64_t key) {
    if (!root) {
        fprintf(stderr, "got nullptr for tree in rbtree_search\n");
        return NULL;
    }
    Direction direction = check_node_direction(root, key);
    switch (direction) {
    case RIGHT:
    case LEFT:
        if (root->children[direction] == NULL)
            return root; // if it doesnt exist return parent
        return rbtree_search(root->children[direction], key);
    case THIS:
        printf("Found node of key %zu\n", key);
        return root;
    }
    fprintf(stderr, "unreachable\n");
    return NULL;
}

// like rbtree_search, but if it doesn't exist then it will return NULL instead of the parent
// of what it wouldve been. also instead of a node to start searching from it takes a Tree*
Node *rbtree_search_err(Tree *tree, uint64_t key) {
    Node *ret = rbtree_search(tree->root, key);
    
    Direction dir = check_node_direction(ret, key);
    if (dir != THIS) {
        printf("couldn't find key %zu\n", key);
        return NULL;
    }

    return ret;
}

static inline const char *strdir(Direction dir) {
    const char *stringified[] = {
        [LEFT ] = "LEFT",
        [RIGHT] = "RIGHT",
        [THIS ] = "THIS",
    };
    return stringified[dir];
}

int rbtree_insert_first_node(Tree *tree, uint64_t key, Node **inserted_node_buf) {
    Node *node = (Node*) malloc(sizeof(Node));
    node->parent = NULL;
    node->val    = key;
    node->colour = BLACK;
    memset(node->children, 0, sizeof(node->children));

    if (inserted_node_buf) *inserted_node_buf = node;

    tree->root = node;
    return 0;
}

// return 0 on success, -1 on error. will not rebalance the tree.
// both *_buf args can be NULL if you don't care about them, otherwise they
// will point to the parent of the inserted node and the inserted node.
int rbtree_insert(Tree *tree, uint64_t key, Node **parent_buf, Node **inserted_node_buf) {
    printf("Try insert key %zu...\n", key);
    if (!tree->root) {
        *parent_buf = NULL;
        return rbtree_insert_first_node(tree, key, inserted_node_buf);
    }

    Node *parent = rbtree_search(tree->root, key);

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

    if (parent_buf) *parent_buf = parent;
    if (inserted_node_buf) *inserted_node_buf = node;

    parent->children[direction] = node;
    return 0;
}

// this is a chonky macro name but idc its descriptive. 
#define DIR_OF_CHILD_IN_PARENT(node) (((node)->parent->children[RIGHT] == (node)) ? RIGHT : LEFT)
// assumes node was just inserted and is still RED as it has not been modified.
// this is also pretty damn based on the wikipedia one so credit is as given in
// the link above.
void rbtree_rebalance(Tree *tree, Node *parent, Node *node) {
    do {
        // no need to rebalance, as it is black->red, not red->red
        if (parent->colour == BLACK) return;

        Node *grandparent = parent->parent;
        if (!grandparent) {
            /* if the parent is the root node, we can really easily fix the
             * red->red issue by just making the parent black, then we can be
             * sure it won't cause issues further up the tree. */
            parent->colour = BLACK;
            return;
        }

        Direction dir = DIR_OF_CHILD_IN_PARENT(parent);
        assert(parent->parent->children[dir] == parent);
        assert(dir != THIS);
        assert(node->parent->children[RIGHT] != node->parent->children[LEFT]);

        Node *uncle = grandparent->children[FLIP_DIR(dir)];
        if (!uncle || uncle->colour == BLACK) {
            if (node == parent->children[FLIP_DIR(dir)]) {
                // parent is red but the uncle, its sibling, is black. we want
                // to rotate so that the parent becomes the grandparent
                rbtree_rotate(tree, parent, dir);
                node = parent;
                parent = grandparent->children[dir];
            }

            // the node is now an outer node (left->left or right->right), rotate
            // so that the parent replaces the grandparent, parent becomes the parent
            // of both the node and the grandparent.
            rbtree_rotate(tree, grandparent, FLIP_DIR(dir));
            parent->colour = BLACK;
            grandparent->colour = RED;
            return;
        }

        // parent and uncle are both red, they can become black while the
        // grandparent (their parent) becomes red, to ensure that the number of
        // black nodes from any node to its leaf nodes are the same.
        assert(uncle->parent == grandparent && parent->parent == grandparent);
        parent->colour = uncle->colour = BLACK;
        grandparent->colour = RED;
        node = grandparent;
    } while((parent = node->parent));
}

// ret 0 on success, -1 on error. inserts then ensures the tree is balanced.
int rbtree_insert_balanced(Tree *tree, uint64_t key) {
    Node *parent, *node;
    if (rbtree_insert(tree, key, &parent, &node) < 0) {
        fprintf(stderr, "Failed insertion of key %zu\n", key);
        return -1;
    }
   
    /* we don't wanna rebalance if it was the root node (aka the first node)
     * we just inserted */
    if (parent)
        rbtree_rebalance(tree, parent, node);
    return 0;
}

#define DO_BALANCE true
#define INSERT(rbtree, key) (DO_BALANCE ? rbtree_insert_balanced(rbtree, key) : rbtree_insert(rbtree, key, NULL, NULL))
int main(void) {
    Tree rbtree = {0};

    printf(" === Insertions test ===\n");
    if (INSERT(&rbtree, 69) < 0) return -1;
    if (INSERT(&rbtree, 12) < 0) return -1;
    if (INSERT(&rbtree, 14) < 0) return -1;
    if (INSERT(&rbtree, 72) < 0) return -1;
    if (INSERT(&rbtree, 18) < 0) return -1;

    printf("\n === Searches test ===\n");
    rbtree_search_err(&rbtree, 69);
    rbtree_search_err(&rbtree, 14);
    rbtree_search_err(&rbtree, 72);
    rbtree_search_err(&rbtree, 18);
    rbtree_search_err(&rbtree, 12);
    rbtree_search_err(&rbtree, 95); // doesnt exist

    return 0;
}
