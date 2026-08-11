#include "rbt.h"

Node *create_node(data_t data)
{
    //Allocate memory
    Node *new = malloc(sizeof(*new));

    //check if memory alloacation failed
    if(new == NULL)
    {
        return NULL;
    }

    new->data = data;
    new->color = RED;
    new->left = NULL;
    new->right = NULL;
    new->parent = NULL;

    return new;
}
