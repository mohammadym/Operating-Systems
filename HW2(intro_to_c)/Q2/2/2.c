#include <stdio.h>
#include <stdlib.h>

struct tree
{
    int data;
    struct tree *right_child;
    struct tree *left_child;
};

struct tree* btree_search(struct tree *root, int value)
{
    if(root==NULL || root->data==value)
        return root;
    else if(value>root->data)
        return btree_search(root->right_child, value);
    else
        return btree_search(root->left_child,value);
}

struct tree* find_minimum(struct tree *root)
{
    if(root == NULL)
        return NULL;
    else if(root->left_child != NULL)
        return find_minimum(root->left_child);
    return root;
}

struct tree* btree_new_node(int x)
{
    struct tree *p;
    p = malloc(sizeof(struct tree));
    p->data = x;
    p->left_child = NULL;
    p->right_child = NULL;
    return p;
}

struct tree* btree_add_node(struct tree *root, int x)
{
    if(root==NULL)
        return btree_new_node(x);
    else if(x>root->data)
        root->right_child = btree_add_node(root->right_child, x);
    else
        root->left_child = btree_add_node(root->left_child,x);
    return root;
}

struct tree* btree_remove_node(struct tree *root, int value)
{
    if(root==NULL)
        return NULL;
    if (value>root->data)
        root->right_child = btree_remove_node(root->right_child, value);
    else if(value<root->data)
        root->left_child = btree_remove_node(root->left_child, value);
    else
    {
        if(root->left_child==NULL && root->right_child==NULL)
        {
            free(root);
            return NULL;
        }


        else if(root->left_child==NULL || root->right_child==NULL)
        {
            struct tree *temp;
            if(root->left_child==NULL)
                temp = root->right_child;
            else
                temp = root->left_child;
            free(root);
            return temp;
        }


        else
        {
            struct tree *temp = find_minimum(root->right_child);
            root->data = temp->data;
            root->right_child = btree_remove_node(root->right_child, temp->data);
        }
    }
    return root;
}

void inorder(struct tree *root)
{
    if(root!=NULL)
    {
        inorder(root->left_child);
        printf(" %d ", root->data);
        inorder(root->right_child);
    }
}

int main()
{
    struct tree *root;
    root = btree_new_node(20);
    btree_add_node(root,5);
    btree_add_node(root,1);
    btree_add_node(root,15);
    btree_add_node(root,9);
    btree_add_node(root,7);
    btree_add_node(root,12);
    btree_add_node(root,30);
    btree_add_node(root,25);
    btree_add_node(root,40);
    btree_add_node(root, 45);
    btree_add_node(root, 42);
    inorder(root);
    printf("\n");
    root = btree_remove_node(root, 1);
    inorder(root);
    printf("\n");
    root = btree_add_node(root, 24);
    inorder(root);
    printf("\n");
    root = btree_remove_node(root, 45);
    inorder(root);
    printf("\n");
    root = btree_search(root, 9);
    inorder(root);
    printf("\n");
    return 0;
}
