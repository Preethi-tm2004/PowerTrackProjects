#include "rbt.h"

Status delete_node(Node **root, data_t data)
{
    Node *z = *root;
    Node *y;
    Node *x;
    Node *x_parent;
    int x_is_left;
    Color original_color;

    /* Search for the node */
    while(z != NULL)
    {
        if(data < z->data)
        {
            z = z->left;
        }
        else if(data > z->data)
        {
            z = z->right;
        }
        else 
        {
            break;
        }
    }

    /* Node  not found */
    if(z == NULL)
    {
        return FAILURE;
    }

    /* Y is the node that will actually be removed */
    y = z;

    /* Sve the color of the node that will be removed */
    original_color = y->color;

    /* Case 1: z has no left child */
    if(z->left == NULL)
    {
        x = z->right;
        x_parent = z->parent;

        if(x_parent != NULL)
        {
            x_is_left = (z == x_parent->left);
        }
        else
        {
            x_is_left = 0;
        }

        transplant(root, z, z->right);
    }

    /* Case 2: z has no right child */
    else if(z->right == NULL)
    {
        x = z->left;
        x_parent = z->parent;

        if(x_parent != NULL)
        {
            x_is_left = (z == x_parent->left);
        }
        else
        {
            x_is_left = 0;
        }

        transplant(root, z, z->left);
    }

    /* Case 3: z has two children */
    else
    {
        /* Find inorder successor */
        y = z->right;

        while(y->left != NULL)
        {
            y = y->left;
        }

        /* Save successor's color */
        original_color = y->color;

        /* Xx is successor's right child */
        x = y->right;

        /* Successor is directly under z */
        if(y->parent == z)
        {
            x_parent = y;

            x_is_left = 0;

            if(x != NULL)
            {
                x->parent = y;
            }
        }
        else
        {
            /* save x's parent before transplant, becuase y->parent will change later */
            x_parent = y->parent;

            x_is_left = (y == y->parent->left);

            /* Remove y from its original position */
            transplant(root, y, y->right);

            /* Put z's right subtree under y */
            y->right = z->right;
            y->right->parent = y;
        }

        /* Replace z with y */
        transplant(root, z, y);

        /* put z's left subtree under y */
        y->left = z->left;
        y->left->parent = y;

        /* y takes z's color */
        y->color = z->color;
    }

    /* z/y has been physically removed */
    if(y != z)
    {
        free(z);
    }
    else
    {
        free(y);
    }

    /* if the removed node is BLACK the RBT properties will be voilated, we need to handle them */
    if(original_color == BLACK)
    {
        delete_fixup(root, x, x_parent, x_is_left);
    }

    return SUCCESS;
}