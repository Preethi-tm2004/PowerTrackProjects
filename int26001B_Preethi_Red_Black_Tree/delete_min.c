#include "rbt.h"

Status delete_min(Node **root)
{
    Node *temp;

    /* Tree is empty */
    if(*root == NULL)
    {
        return FAILURE;
    }

    /* Accessing the leftmost node */
    temp = *root;

    while(temp->left != NULL)
    {
        temp = temp->left;
    }

    /* Deleting the minimum node */
    return delete_node(root, temp->data);
}