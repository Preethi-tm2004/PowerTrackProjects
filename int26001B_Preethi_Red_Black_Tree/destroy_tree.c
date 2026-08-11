#include "rbt.h"

Status destroy_tree(Node **root)
{
    if(*root == NULL)
    {
        return FAILURE;
    }

    /* recursively doing destruction*/
    if((*root)->left != NULL)
    {
        destroy_tree(&((*root)->left));
    }

    if((*root)->right != NULL)
    {
        destroy_tree(&((*root)->right));
    }

    free(*root);
    *root = NULL;

    return SUCCESS;
}