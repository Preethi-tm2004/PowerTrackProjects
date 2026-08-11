#include "rbt.h"

Node *right_rotate(Node *root, Node *y)
{
    Node *x = y->left;

    y->left = x->right;

    if(x->right != NULL)
    {
        x->right->parent = y;
    }

    x->parent = y->parent;

    if(y->parent == NULL)
    {
        root = x;
    }
    else if(y == y->parent->left)
    {
        y->parent->left = x;
    }
    else
    {
        y->parent->right = x;
    }

    x->right = y;
    y->parent = x;

    return root;

}