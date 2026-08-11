#include "rbt.h"

Status search(Node *root, data_t data)
{
    if(root == NULL)
    {
        return FAILURE;
    }

    Node *temp = root;

    while(temp != NULL)
    {
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
            return SUCCESS;
        }
    }

    return NOELEMENT;
}