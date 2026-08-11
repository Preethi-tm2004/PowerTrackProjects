#include "rbt.h"

Status insert(Node **root, data_t data)
{
    /* Normal BST insertion logic with fixup */
    Node *new = create_node(data);

    if(new == NULL)
    {
        return FAILURE;
    }

    if(*root == NULL)
    {
        *root = new;
        new->color = BLACK;
        return SUCCESS;
    }

    Node *temp = *root;
    Node *parent = NULL;

    while(temp != NULL)
    {
        parent = temp;
        if(data < temp->data)
        {
            temp = temp->left;
        }
        else if(data > temp->data)
        {
            temp = temp->right;
        }
        else
        {
            free(new);
            return DUPLICATE;
        }
    }

    if(data < parent->data)
    {
        parent->left = new;
    }
    else if(data > parent->data)
    {
        parent->right = new;
    }

    new->parent = parent;

    insert_fixup(root, new);

    return SUCCESS;

}