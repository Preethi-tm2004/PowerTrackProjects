#include "rbt.h"

int main()
{
    int choice;
    data_t data;
    int result;

    Node *root = NULL;
    while(1)
    {
        printf("\n-------------------------------------------\n");
        printf("\033[034m              RED BLACK TREE               \033[0m");
        printf("\n-------------------------------------------\n");

        printf("1. Insert Node\n2. Delete Node\n3. Search Node\n4. Find Minimum\n5. Delete Minimum\n6. Find Maximum\n7. Delete Maximum\n8. Display Tree\n9. Destroy tree\n10. Check BLACK height\n11. Validate RBT\n12. Exit\n");
        printf("-------------------------------------------\n");
        printf("Enter Choice : ");

        if(scanf("%d", &choice) != 1)
        {
            while(getchar() != '\n');

            printf("\033[31mError : Invalid input\033[0m\n");

            continue;
        }

        if(choice < 1 || choice > 12)
        {
            printf("\033[31mInvalid Choice\033[0m\n");
            continue;
        }

        switch(choice)
        {
            case 1:
            {
                printf("Enter data to be inserted : ");
                scanf("%d", &data);

                if(insert(&root, data) == SUCCESS)
                {
                    printf("\033[32mData Inserted successfully\033[0m\n");
                }
                else if(insert(&root, data) == DUPLICATE)
                {
                    printf("\033[31mDuplicate data\033[0m\n");
                }
                else
                {
                    printf("\033[31mInsertion failed\033[0m\n");
                }

                break;
            }

            case 2:
            {
                printf("Enter data to be deleted : ");
                scanf("%d", &data);

                if(delete_node(&root, data) == SUCCESS)
                {
                    printf("\033[32mData deleted successfully\033[0m\n");
                }
                else
                {
                    printf("\033[31mData not found\033[0m\n");
                }

                break;
            }

            case 3:
            {
                printf("Enter the data to be searched : ");
                scanf("%d", &data);

                result = search(root, data);

                if(result == NOELEMENT)
                {
                    printf("\033[31mData not found\033[0m\n");
                }
                else if(result == FAILURE)
                {
                    printf("\033[31mTree is empty\033[0m\n");
                }
                else
                {
                    printf("\033[32mData found\033[0m\n");
                }
                break;
            }

            case 4:
            {
                data_t min;

                if(find_min(root, &min) == SUCCESS)
                {
                    printf("\033[32mMinimum value of the tree is %d\033[0m\n", min);
                }
                else
                {
                    printf("\033[31mTree is empty\033[0m\n");
                }

                break;
            }

            case 5:
            {
                if(delete_min(&root) == SUCCESS)
                {
                    printf("\033[32mMinimum node deleted successfully\033[0m\n");
                }
                else
                {
                    printf("\033[31mTree is empty\033[0m\n");
                }

                break;
            }

            case 6:
            {
                data_t max;

                if(find_max(root, &max) == SUCCESS)
                {
                    printf("\033[32mMaximum value of the tree is %d\033[0m\n", max);
                }
                else
                {
                    printf("\033[31mTree is empty\033[0m\n");
                }

                break;
            }

            case 7:
            {
                if(delete_max(&root) == SUCCESS)
                {
                    printf("\033[32mMaximum node deleted successfully\033[0m\n");
                }
                else
                {
                    printf("\033[31mTree is empty\033[0m\n");
                }

                break;
            }

            case 8:
            {
                if(root == NULL)
                {
                    printf("\033[31mTree is Empty\033[0m\n");
                }
                else
                {
                    printf("\033[32mInorder Traversal : \033[0m");
                    inorder(root);
                    printf("\n");
                }
                break;
            }

            case 9:
            {
                if(destroy_tree(&root) == SUCCESS)
                {
                    printf("\033[32mTree destroyed successfully\033[0m\n");
                }
                else
                {
                    printf("\033[31mTree is already empty\033[31m\n");
                }

                break;
            }

            case 10:
            {
                int height;

                height = black_height(root);

                if(height == -1)
                {
                    printf("\033[31mBLACK height violation\033[0m\n");
                }
                else
                {
                    printf("\033[32mBLACK Height is valid\nBLACK height = %d\033[0m\n", height);
                }

                break;
            }

            case 11: 
            {
                if(validate_rbt(root) == SUCCESS)
                {
                    printf("\033[32mRED BLACK TREE is valid\033[0m\n");
                }
                else
                {
                    printf("\033[31mRED BLACK TREE is invalid\033[0m\n");
                }

                break;
            }

            case 12:
            {
                printf("\033[31mExiting......\033[0m\n");
                return SUCCESS;
                break;
            }

            default:
                printf("\03[31mInvalid Choice\033[0m\n");
                break;

            return SUCCESS;
        }
    }

    return SUCCESS;
    
}