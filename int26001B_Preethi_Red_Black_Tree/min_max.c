#include "rbt.h"

Status find_min(Node *root, data_t *min)
{
    if(root == NULL)
    {
        return FAILURE;
    }

    while(root->left != NULL)
    {
        root = root->left;
    }

    *min = root->data;

    return SUCCESS;
}

Status find_max(Node *root, data_t *max)
{
    if(root == NULL)
    {
        return FAILURE;
    }

    while(root->right != NULL)
    {
        root = root->right;
    }

    *max = root->data;

    return SUCCESS;
}