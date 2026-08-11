#include "rbt.h"

Status validate_rbt(Node *root)
{
    int height;

    if(root == NULL)
    {
        return SUCCESS;
    }

    /* Root must be BLACK */
    if(root->parent == NULL && root->color != BLACK)
    {
        return FAILURE;
    }

    /* RED node cannot have RED child */
    if(root->color == RED)
    {
        if((root->left != NULL && root->left->color == RED) ||
           (root->right != NULL && root->right->color == RED))
        {
            return FAILURE;
        }
    }

    /* Validate left subtree */
    if(validate_rbt(root->left) == FAILURE)
    {
        return FAILURE;
    }

    /* Validate right subtree */
    if(validate_rbt(root->right) == FAILURE)
    {
        return FAILURE;
    }

    /* Validate black height */
    height = black_height(root);

    if(height == -1)
    {
        return FAILURE;
    }

    return SUCCESS;
}