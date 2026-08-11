#include "rbt.h"

Status delete_max(Node **root)
{
    Node *temp;

    if(*root == NULL)
    {
        return FAILURE;
    }

    /* Accessing the leftmost node */
    temp = *root;

    while(temp->right != NULL)
    {
        temp = temp->right;
    }

    /* Deleting the maximum node */
    return delete_node(root, temp->data);
}