#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
struct TreeNode{
  int val;
  struct TreeNode* left;
  struct TreeNode* right;
};
static struct TreeNode ** trees = NULL;
static int MAX_NODE_LEN = 48;
void preOrder(struct TreeNode * t, struct TreeNode ** arr, int * ord){
  if(!t)return;
  arr[*ord] = t;
  *ord = *ord + 1;
  preOrder(t->left, arr, ord);
  preOrder(t->right, arr, ord);
}
int main(){
  struct TreeNode *t1 = (struct TreeNode*) malloc(sizeof(struct TreeNode));
  t1->val = 1;
  struct TreeNode *t2 = (struct TreeNode*) malloc(sizeof(struct TreeNode));
  t2->val = 2;
  struct TreeNode *t3 = (struct TreeNode*) malloc(sizeof(struct TreeNode));
  t3->val = 3;
  struct TreeNode *t4 = (struct TreeNode*) malloc(sizeof(struct TreeNode));
  t4->val = 4;
  struct TreeNode *t5 = (struct TreeNode*) malloc(sizeof(struct TreeNode));
  t5->val = 5;
  t1->left = t2;
  t1->right = t3;
  t2->left = NULL;
  t2->right = NULL;
  t3->left = t4;
  t3->right = t5;
  t4->left = NULL;
  t4->right = NULL;
  t5->left = NULL;
  t5->right = NULL;
  trees = (struct TreeNode**) malloc(sizeof(struct TreeNode *) * MAX_NODE_LEN);;
  int ord = 0;
  preOrder(t1,trees, &ord);
  for(int i = 0;trees[i];i++){
    printf("arr[%d]:%d\n",ord,trees[i]->val);
  }
  int j;
  for(j = 0; trees[j];j++); //find tree size of trees
  struct TreeNode * tmp = trees[j-1];
  free(trees);
  printf("arr[%d]:%d\n",j-1,tmp->val);

}