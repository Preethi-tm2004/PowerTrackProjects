#include "rbt.h"

void transplant(Node **root, Node *u, Node *v)
{
    if(u->parent == NULL)
    {
        /* u is root */
        *root = v;
    }
    else if(u == u->parent->left)
    {
        u->parent->left = v;
    }
    else
    {
        u->parent->right = v;
    }

    if(v != NULL)
    {
        v->parent = u->parent;
    }
}