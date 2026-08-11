#include "rbt.h"

void inorder(Node *root)
{
    if(root == NULL)
    {
        return;
    }

    inorder(root->left);

    printf("%d[", root->data);

    if(root->color == BLACK)
    {
        printf("\033[90mB\033[0m");
    }
    else
    {
        printf("\033[31mR\033[0m");
    }

    printf("]");

    if(root->parent != NULL)
    {
        printf("(P:%d) ", root->parent->data);
    }
    else
    {
        /* if the node is root node */
        printf("(P:NULL) ");
    }

    inorder(root->right);
}