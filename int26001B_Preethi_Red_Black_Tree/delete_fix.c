#include "rbt.h"

void delete_fixup(Node **root, Node *x, Node *parent, int x_is_left)
{
    Node *sibling;

    while(x != *root && (x == NULL || x->color == BLACK))
    {
        if(parent == NULL)
        {
            break;
        }
       /* if x exists, determine its position from the actual parent pointer */
       if(x != NULL)
       {
            x_is_left = (x == parent->left);
       }
       if(x_is_left)
       {
            sibling = parent->right;
            /*
             * CASE 1:
             * Sibling is RED
             */
            if(sibling != NULL && sibling->color == RED)
            {
                sibling->color = BLACK;
                parent->color = RED;

                *root = left_rotate(*root, parent);

                sibling = parent->right;
            }

            /*
             * CASE 2:
             * Sibling is BLACK
             * and both sibling children are BLACK
             */
            if(sibling == NULL ||
               ((sibling->left == NULL ||
                 sibling->left->color == BLACK) &&
                (sibling->right == NULL ||
                 sibling->right->color == BLACK)))
            {
                if(sibling != NULL)
                {
                    sibling->color = RED;
                }

                x = parent;
                parent = x->parent;
            }
            else
            {
                /*
                 * CASE 3:
                 * Sibling is BLACK
                 * Near child is RED
                 * Far child is BLACK
                 */
                if(sibling->right == NULL ||
                   sibling->right->color == BLACK)
                {
                    if(sibling->left != NULL)
                    {
                        sibling->left->color = BLACK;
                    }

                    sibling->color = RED;

                    *root = right_rotate(*root, sibling);

                    sibling = parent->right;
                }

                /*
                 * CASE 4:
                 * Sibling is BLACK
                 * Far child is RED
                 */
                sibling->color = parent->color;
                parent->color = BLACK;

                if(sibling->right != NULL)
                {
                    sibling->right->color = BLACK;
                }

                *root = left_rotate(*root, parent);

                x = *root;
                parent = NULL;
            }
        }

        /*
         * x is the RIGHT child
         */
        else
        {
            sibling = parent->left;

            /*
             * CASE 1:
             * Sibling is RED
             */
            if(sibling != NULL && sibling->color == RED)
            {
                sibling->color = BLACK;
                parent->color = RED;

                *root = right_rotate(*root, parent);

                sibling = parent->left;
            }

            /*
             * CASE 2:
             * Sibling is BLACK
             * and both sibling children are BLACK
             */
            if(sibling == NULL ||
               ((sibling->left == NULL ||
                 sibling->left->color == BLACK) &&
                (sibling->right == NULL ||
                 sibling->right->color == BLACK)))
            {
                if(sibling != NULL)
                {
                    sibling->color = RED;
                }

                x = parent;
                parent = x->parent;
            }
            else
            {
                /*
                 * CASE 3:
                 * Sibling is BLACK
                 * Near child is RED
                 * Far child is BLACK
                 */
                if(sibling->left == NULL ||
                   sibling->left->color == BLACK)
                {
                    if(sibling->right != NULL)
                    {
                        sibling->right->color = BLACK;
                    }

                    sibling->color = RED;

                    *root = left_rotate(*root, sibling);

                    sibling = parent->left;
                }

                /*
                 * CASE 4:
                 * Sibling is BLACK
                 * Far child is RED
                 */
                sibling->color = parent->color;
                parent->color = BLACK;

                if(sibling->left != NULL)
                {
                    sibling->left->color = BLACK;
                }

                *root = right_rotate(*root, parent);

                x = *root;
                parent = NULL;
            }
        }
    }

    if(x != NULL)
    {
        x->color = BLACK;
    }
}