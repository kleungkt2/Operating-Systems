#include "kernel/types.h"
#include "user/setjmp.h"
#include "user/threads.h"
#include "user/user.h"
#define NULL 0

static struct thread* current_thread = NULL;
static struct thread* root_thread = NULL;
static struct thread* previous_thread = NULL;
static int id = 1;
static jmp_buf env_st;
int MAX_NODE_LEN = 33;
struct thread** main_thread_arr = NULL;
static int main_tree_size = 0;

int call1 = 0;
int call2 = 0;
int call3 = 0;
int call4 = 0;
int call5 = 0;
int call6 = 0;
int call7 = 0;
int call8 = 0;
int call9 = 0;
int call10 = 0;
int call11 = 0;
int call12 = 0;
int call13 = 0;
int call14 = 0;
int call15 = 0;
int call16 = 0;
int call17 = 0;
int call18 = 0;
int call19 = 0;
int call20 = 0;
int call21 = 0;
int print = 0; //1 for print, 0 for not print
int * order = NULL;
void printStack(int* pt, int index){
    if(print){
    *pt = *pt + 1;
    printf("======== Start printing stack for position%d - thread%d======\n",index, current_thread->ID);
    printf("[%dpos - %dth]Current thread%d sp: %d\n",index, *pt,current_thread->ID, current_thread->env->sp);
    printf("[%dpos - %dth]Current env_st sp: %d\n", index, *pt,env_st->sp);
    printf("[%dpos - %dth]Current thread%d->stack: %d\n", index, *pt,current_thread->ID,current_thread->stack);
    printf("[%dpos - %dth]Current thread%d->stack_p: %d\n", index, *pt,current_thread->ID,current_thread->stack_p);
    printf("======== End printing stack for position%d - thread%d======\n\n",index,current_thread->ID);
        
    return;
    }
    
}
struct thread *thread_create(void (*f)(void *), void *arg){
    struct thread *t = (struct thread*) malloc(sizeof(struct thread));
    //unsigned long stack_p = 0;
    unsigned long new_stack_p;
    unsigned long new_stack;
    new_stack = (unsigned long) malloc(sizeof(unsigned long)*0x100);
    new_stack_p = new_stack +0x100*8-0x2*8;
    t->fp = f;
    t->arg = arg;
    t->ID  = id;
    t->buf_set = 0;
    t->stack = (void*) new_stack;
    t->stack_p = (void*) new_stack_p;
    t->left = NULL;
    t->right = NULL;
    t->parent = NULL;
    id++;
    if(print)printf("DEBUG: thread %d is created\n", t->ID);
    return t;
}
int recal_tree_size(struct thread ** arr){
    int i = 0;
    while(arr[i]){
        i++;
    }
    return i;
}
void update_binary_tree(struct thread * p, struct thread ** arr, int * ord){
    if(print)printf("DEBUG:5\n");
    if(p == NULL) {if(print)printf("DEBUG:return from update binary tree\n");return;}
    if(print)printf("DEBUG:6\n");
    if(print)printf("DEBUG:ord:%d\n",*ord);
    if(print)printf("DEBUG:p->ID:%d\n",p->ID);
    if(print)printf("DEBUG: in update binary tree: ord: %d, p->ID:%d\n", *ord, p->ID);
    if(print)printf("DEBUG:7\n");
    arr[*ord] = p;
    if(print)printf("DEBUG:8\n");
    *ord = *ord + 1;
    update_binary_tree(p->left, arr, ord);
    update_binary_tree(p->right, arr, ord);
}
struct thread ** create_binary_tree(struct thread * t){
    if(print)printf("DEBUG: creating binary tree rooted at: %d\n",t->ID);
    
    if(print)printf("DEBUG:1\n");
    struct thread ** thread_arr = (struct thread**) malloc(sizeof(struct thread *) * MAX_NODE_LEN);
    for(int i = 0; i< MAX_NODE_LEN; i++)
        thread_arr[i] = NULL;
    order = (int *)malloc(sizeof(int));
    memset(order,0,sizeof(int));
    update_binary_tree(t, thread_arr, order);
    free(order);
    if(print)printf("DEBUG:4\n");
    return thread_arr;
}
void print_tree(struct thread * root){
    if(print){
    if(!root)return;
    printf("DEBUG: Priting inorder tree:[ID] %d\n",root->ID);
    print_tree(root->left);
    print_tree(root->right);    
    }
    
}
void print_tree_wrap(struct thread * root){
    if(print){
    printf("===== DEBUG START: Printing inorder tree:\n");
    print_tree(root);
    printf("===== DEBUG END: Printing inorder tree:\n\n");
    }
   
}
void print_arr(struct thread ** arr){
    if(print){
    printf("===== DEBUG START: Printing tree array:\n");
    printf("Main Tree Size: %d\n", main_tree_size);
    int i = 0;
    while(arr && arr[i]){
        printf("[order:%d] - [ID:%d]\n",i,arr[i]->ID);
        i++;
    }
    printf("===== DEBUG END: Printing tree array:\n\n");
    return;
    }
    

}
int search_order(struct thread * p, struct thread ** arr){
    int i = 0;
    if(print)printf("DEBUG: in search order p->ID: %d\n",p->ID);
    while(arr[i] != NULL){
        if(print)printf("DEBUG: in search_order arr[%d] = %d\n",i,arr[i]->ID);
        if(arr[i]->ID == p->ID){
            return i;
        }
        i++;
    }
    return -1; //not found, error
}
void thread_add_runqueue(struct thread *t){
    /*
    if(t->left != NULL && t->right != NULL){
        free(t);
        free(t->stack);
        return;
    }*/
    if(root_thread == NULL){
        current_thread = t;
        root_thread = t;
        t->parent = NULL;
        if(print)printf("DEBUG: Added thread %d to runqueue as root.\n", t->ID);
    }
    else{
        if(current_thread->left == NULL){
            current_thread->left = t;
            t->parent = current_thread;
            if(print)printf("DEBUG: Added thread %d to runqueue as %d's left child.\n", t->ID, current_thread->ID);
        }
        else if(current_thread->right == NULL){
            current_thread->right = t;
            t->parent = current_thread;
            if(print)printf("DEBUG: Added thread %d to runqueue as %d's right child.\n", t->ID, current_thread->ID);
        }
        else{
            if(print)printf("DEBUG:Discarded thread%d\n",t->ID);
            free(t->stack);
            free(t);
            return;
        }
    }
    if(print)printf("DEBUG: after adding runqueue, print_arr:\n");
    print_arr(main_thread_arr);
    if(main_thread_arr)
        free(main_thread_arr);
    main_thread_arr = create_binary_tree(root_thread);
    main_tree_size = recal_tree_size(main_thread_arr);
    if(print)printf("DEBUG: after recreate binary tree, print_arr:\n");
    print_arr(main_thread_arr);
    return;
}
void thread_yield(void){
    //this function is called outside in user program
    //hands off the right of control to another thread
    //save context, call schedule() and dispatch()
    printStack(&call10, 10);    
    int ret = setjmp(current_thread->env);
    if(ret == 3)
        return;
    printStack(&call11, 11);    
    schedule();
	dispatch();
}

void dispatch(void){
    //if 
    int ret;
    /*
    if(previous_thread != NULL) {
        printf("DEBUG: Previous thread is not empty, execute: thread %d",current_thread->ID);
        ret = setjmp(previous_thread->env);
        if (ret) {
            return;
        }
    }
    */
    //if thread has never run before:
    if(current_thread->buf_set == 0){
        if(print)printf("DEBUG: thread %d has never run before.\n", current_thread->ID);
        printStack(&call4, 4);
        /*
        __asm__ __volatile__("mov %%rax,%%rsp"
									:
									:"a"(current_thread->stack_p));
		__asm__ __volatile__("mov %%rax,%%rbp"
									:
									:"a"(current_thread->stack_p));
        */
        ret = setjmp(current_thread->env); //current_thread->env is initialized to save the stack when thread is first called from main
        //current_thread->env->sp should be the stack pointer now [16288]
        //current_thread->stack_p is the target location [81712]
        //ret will be 0 for the first setjmp
        printStack(&call5, 5);
        if(ret == 1){
        printStack(&call6, 6);
        if(print)printf("DEBUG: thread %d execute function now.\n", current_thread->ID);
        (*(current_thread->fp))(current_thread->arg); //[f1,]
        thread_exit();
        }
        printStack(&call7, 7);
        current_thread->env->sp = (unsigned long)current_thread->stack_p; //current_thread->stack_p is pointing to the stack end of current thread
        //set the saved SP to be our desired location
        printStack(&call8, 8);
        current_thread->buf_set = 1;
        longjmp(current_thread->env,1);
        //after longjmp, it restores SP,RA,PC
        printStack(&call9, 9);
        return;
    }
    else if(current_thread->buf_set == 1) {
        if(print)printf("DEBUG: thread %d has ran before.\n", current_thread->ID);
        longjmp(current_thread->env, 3);
    } 
    if(print)printf("DEBUG: thread %d execute function now.\n", current_thread->ID);
    
    
}
void schedule(void){
    call12 = call12 + 1;
    if(print)printf("Entered Schedule for %d time - thread%d\n",call12,current_thread->ID);
    printStack(&call13, 13);   
    previous_thread = current_thread;
    int order = search_order(current_thread, main_thread_arr);
    print_arr(main_thread_arr);
    if(order != -1 && order != main_tree_size-1){
        current_thread = main_thread_arr[order + 1];
    }
    else{
        if(order == -1) {
            if(print){
                printf("ERROR: search order return -1\n");
            }
        }
        if(order == main_tree_size-1) {
            if(print){
                printf("Reach the end\n");
            }
        }

        current_thread = root_thread;
    }
    printStack(&call14, 14);   
}
void thread_exit(void){
    printStack(&call15, 15);   
    struct thread * toReplace = NULL;
    struct thread * toRemove = NULL;
    struct thread * next_curr = NULL;
    struct thread ** subtree_arr = NULL;
    if(main_thread_arr)
        free(main_thread_arr);
    main_thread_arr = create_binary_tree(root_thread);
    main_tree_size = recal_tree_size(main_thread_arr);
    if(current_thread->ID == root_thread->ID && main_tree_size == 1){
        if(print)printf("DEBUG: root thread has executed, should return to main\n");
        longjmp(env_st, 2);
        return;
    }
    if(current_thread->left == NULL && current_thread->right == NULL){
        if(print)printf("DEBUG: leaf node [id:%d] wants to exit, simply remove it\n", current_thread->ID);
        //if it is non-root leaf node, simply remove
        //remove here means setting current_thread to parent,remove link, then free, update tree
        toReplace = current_thread->parent; // if 3 wanna leave, 1 is toReplace
        toRemove = current_thread;
        if(toReplace->left == current_thread){
            toReplace->left = NULL;
        }
        else if(toReplace->right == current_thread){
            toReplace->right = NULL;
        }
        free(toRemove);
        free(toRemove->stack);
        current_thread = toReplace;
        if(main_thread_arr)
            free(main_thread_arr);
        main_thread_arr = create_binary_tree(root_thread);
        main_tree_size = recal_tree_size(main_thread_arr);
        
    }
    else{
        //Another situation: non-root node with sub-tree
        //otherwise, replace the node with the last node in preorder 
        //traversal of the subtree rooted at it.
        if(print)printf("DEBUG: current node [id:%d] with subtree wants to exit, search for replacement\n", current_thread->ID);
        //First, find subtree
        subtree_arr = create_binary_tree(current_thread);
        print_arr(subtree_arr);
        int i;
        for(i = 0; subtree_arr[i] ;i++); //find tree size of subtree
        toReplace = subtree_arr[i-1];
        toRemove = current_thread;
        //Second, get last node
        if(print)printf("DEBUG: Found toReplace node, ID: %d\n",toReplace->ID);
        //Third, replace
        //Situation 1:
        //nonroot goes to root
            //1. copy parent
            if(print)printf("DEBUG-IF:toReplace && toReplace->parent\n");
            
            if(toReplace && toReplace->parent){
                if(print)printf("DEBUG-EXEC:toReplace && toReplace->parent\n");
                if(toReplace->parent->left == toReplace)
                    toReplace->parent->left=NULL;
                if(toReplace->parent->right == toReplace)
                    toReplace->parent->right=NULL;
                toReplace->parent = toRemove->parent;
            }
                
            //2. copy children
            if(print)printf("DEBUG-IF:toRemove->left && toRemove->left->parent != toReplace\n");
            if(toRemove->left && toRemove->left->parent != toReplace){
                if(print)printf("DEBUG-EXEC:toRemove->left && toRemove->left->parent != toReplace\n");
                toRemove->left->parent = toReplace;
            }
            if(print)printf("DEBUG-IF:toRemove->right && toRemove->right->parent != toReplace\n");
            if(toRemove->right && toRemove->right->parent != toReplace){
                if(print)printf("DEBUG-EXEC:toRemove->right && toRemove->right->parent != toReplace\n");
                toRemove->right->parent = toReplace;
            }
            if(print)printf("DEBUG-IF:toRemove->left  && toRemove->left != toReplace\n");
            if(toRemove->left  && toRemove->left != toReplace){
                if(print)printf("DEBUG-EXEC:toRemove->left  && toRemove->left != toReplace\n");
                 toReplace->left = toRemove->left;
            }
            if(print)printf("DEBUG-IF:NOT (toRemove->left  && toRemove->left != toReplace)\n");
            else{
                if(print)printf("DEBUG-EXEC:NOT (toRemove->left  && toRemove->left != toReplace)\n");
                toReplace->left = NULL;
            }
            if(print)printf("DEBUG-IF:toRemove->right && toRemove->right != toReplace\n");
            if(toRemove->right && toRemove->right != toReplace){
                if(print)printf("DEBUG-EXEC:toRemove->right && toRemove->right != toReplace\n");
                toReplace->right = toRemove->right;
            }
                
            else{
                if(print)printf("DEBUG-EXEC:NOT (toRemove->right && toRemove->right != toReplace)\n");
                toReplace->right = NULL;
            }
            if(print)printf("DEBUG-IF:toRemove->parent && toRemove->parent->left == toRemove && toRemove->parent->left != toReplace\n");
            if(toRemove->parent && toRemove->parent->left == toRemove && toRemove->parent->left != toReplace){
                if(print)printf("DEBUG-EXEC:toRemove->parent && toRemove->parent->left == toRemove && toRemove->parent->left != toReplace\n");
                toRemove->parent->left = toReplace;
            }
            if(print)printf("DEBUG-IF:toRemove->parent && toRemove->parent->right == toRemove && toRemove->parent->right != toReplace\n");
            if(toRemove->parent && print)printf("DEBUG: toRemove has valid parent\n");
            if(toRemove->parent && toRemove->parent->right == toRemove && toRemove->parent->right != toReplace){
                if(print)printf("DEBUG-EXEC:toRemove->parent && toRemove->parent->right == toRemove && toRemove->parent->right != toReplace\n");
                toRemove->parent->right = toReplace;
            }
            if(root_thread == toRemove){
                root_thread = toReplace;
            }
            //free(toRemove->stack);
            //free(toRemove);
            //free(subtree_arr);
        //after relinking all pointer, we can do reorder again
        //if(main_thread_arr)
        //    free(main_thread_arr);
        main_thread_arr = create_binary_tree(root_thread);
        main_tree_size = recal_tree_size(main_thread_arr);
        if(print)printf("DEBUG: after relinking all pointer: print arr:\n");
        print_arr(main_thread_arr);
        int toReplaceOrder = -1;
        for(int i = 0; i < main_tree_size; i++){
            if(main_thread_arr[i]->ID == toReplace->ID){
                toReplaceOrder = i;
                break;break;
            }
                
        }
        if(toReplaceOrder == -1)
            toReplaceOrder = 0;
        //if(print)printf("DEBUG: toReplace ID in search_order:%d\n", toReplace->ID);
        //int toReplaceOrder = search_order(toReplace, main_thread_arr);
        //if(print)printf("DEBUG: toReplaceOrder:%d\n", toReplaceOrder);
        if(toReplaceOrder == main_tree_size - 1){
            next_curr = main_thread_arr[0];
        }
        else{
            next_curr = main_thread_arr[toReplaceOrder+1];
        }
        if(print)printf("DEBUG:next_curr: %d\n",next_curr->ID);
        current_thread = next_curr;
            //current_thread = toReplace;
            //note, we need to set current_thread to be the one next to preorder traversal
            

    }
    //replace code:

    //situation 1: leaf exit
    //situation 2: non leaf exit 

    printStack(&call16, 16);   
    dispatch();
}

void thread_start_threading(void){
    if(print)printf("DEBUG: thread %d start threading.\n", current_thread->ID); //call dispatch for current thread
    printStack(&call1, 1);   
    if(current_thread == NULL){ //nearly impossible edge case
    if(print)printf("DEBUG: current_thread is NULL when starting threading\n");
    return;
    }
    if(setjmp(env_st) == 2){ //if return from thread_exit, meaning all threads are completed
    if(print)printf("DEBUG: return from thread_exit as 2\n");
    printStack(&call2, 2);
        return;
    } else {
    printStack(&call3, 3);
    dispatch();
    }
}

