#include "rbt.h"

int black_height(Node *root)
{
    int left_height;
    int right_height;

    if(root == NULL)
    {
        return 0;
    }

    left_height = black_height(root->left);

    if(left_height == -1)
    {
        return -1;
    }

    right_height = black_height(root->right);

    if(right_height == -1)
    {
        return -1;
    }

    /* BLACK height voilation */
    if(left_height != right_height)
    {
        return -1;
    }

    /* counting current node if it is BLACK */
    if(root->color == BLACK)
    {
        return left_height + 1;
    }

    return left_height;

}