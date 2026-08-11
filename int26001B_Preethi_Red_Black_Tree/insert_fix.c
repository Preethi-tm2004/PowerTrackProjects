#include "rbt.h"

void insert_fixup(Node **root, Node *new)
{
    while(new->parent != NULL && new->parent->color == RED)
    {
        Node *parent = new->parent;
        Node *grandparent = parent->parent;

        /* Parent is the left child of grand parent */
        if(parent == grandparent->left)
        {
            Node *uncle = grandparent->right;

            /* Uncle is RED */
            if(uncle != NULL && uncle->color == RED)
            {
                parent->color = BLACK;
                uncle->color = BLACK;
                grandparent->color = RED;

                new = grandparent;
            }
            else
            {
                /* Uncle is BLACK */

                /* LR case : Left rotation on parent, then right rotation on grandparent */
                if(new == parent->right)
                {
                    *root = left_rotate(*root, parent);

                    parent = new;
                    grandparent = parent->parent;
                }
                /* LL case : Right rotate grandparent */
                parent->color = BLACK;
                grandparent->color = RED;

                *root = right_rotate(*root, grandparent);
            }
        }
        else
        {
            /* Parent is the right child of grand parent */
            Node *uncle = grandparent->left;

            /* Uncle is RED */
            if(uncle != NULL && uncle->color == RED)
            {
                parent->color = BLACK;
                uncle->color = BLACK;
                grandparent->color = RED;

                new = grandparent;
            }
            else
            {
                /* uncle is black */

                if(new == parent->left)
                {
                    /* RL case : Right rotate parent, then left rotate grandparent  */
                    *root = right_rotate(*root, parent);

                    parent = new;
                    grandparent = parent->parent;
                }

                /* RR case : left rotate grandparent  */
                parent->color = BLACK;
                grandparent->color = RED;

                *root = left_rotate(*root, grandparent);
            }
        }
    }

    /* Root should be BLACK always */
    (*root)->color  = BLACK;
}