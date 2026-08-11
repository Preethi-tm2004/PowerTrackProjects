#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>

typedef int data_t;

typedef enum
{
    RED,
    BLACK
}Color;

typedef struct node
{
    data_t data;
    Color color;

    struct node *left;
    struct node *right;
    struct node *parent;

}Node;

typedef enum
{
    SUCCESS,
    FAILURE,
    DUPLICATE,
    NOELEMENT
}Status;

Node *create_node(data_t data);

Status insert(Node **root, data_t data);

Status search(Node *root, data_t data);

Status find_min(Node *root, data_t *min);

Status find_max(Node *root, data_t *max);

void inorder(Node *root);

int black_height(Node *root);

Status delete_node(Node **root, data_t data);

Status delete_min(Node **root);

Status delete_max(Node **root);

Status destroy_tree(Node **root);

Node *left_rotate(Node *root, Node *x);

Node *right_rotate(Node *root, Node *y);

void insert_fixup(Node **root, Node *new);

void transplant(Node **root, Node *u, Node *v);

void delete_fixup(Node **root, Node *x, Node *parent, int x_is_left);

Status validate_rbt(Node *root);

#endif