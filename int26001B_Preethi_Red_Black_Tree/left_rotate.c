#include "rbt.h"

Node *left_rotate(Node *root, Node *x)
{
    Node *y = x->right;

    x->right = y->left;

    if(y->left != NULL)
    {
        y->left->parent = x;
    }

    y->parent = x->parent;

    if(x->parent == NULL)
    {
        root = y;
    }
    else if(x == x->parent->left)
    {
        x->parent->left = y;
    }
    else
    {
        x->parent->right = y;
    }

    y->left = x;
    x->parent = y;

    return root;
}
