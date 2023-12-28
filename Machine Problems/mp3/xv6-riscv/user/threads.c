#include "kernel/types.h"
#include "user/setjmp.h"
#include "user/threads.h"
#include "user/user.h"
#define NULL 0
#define TIME_QUANTUM_SIZE 5

static struct thread* current_thread = NULL;
static int id = 1;
static int is_thread_start = 0;
static jmp_buf env_st;
static int threading_system_time = 0;
static struct adding_thread_node* adding_queue = NULL; 
// static jmp_buf env_tmp;

struct thread *thread_create(void (*f)(void *), void *arg, int execution_time_quantum){
    struct thread *t = (struct thread*) malloc(sizeof(struct thread));
    //unsigned long stack_p = 0;
    unsigned long new_stack_p;
    unsigned long new_stack;
    new_stack = (unsigned long) malloc(sizeof(unsigned long)*0x200);
    new_stack_p = new_stack +0x200*8-0x2*8;
    t->fp = f;
    t->arg = arg;
    t->ID  = -1;
    t->buf_set = 0;
    t->stack = (void*) new_stack;
    t->stack_p = (void*) new_stack_p;
    t->next = NULL;
    t->previous = NULL;
    t->remain_execution_time = execution_time_quantum * TIME_QUANTUM_SIZE;

    t->is_yield = 0;
    t->is_exited = 0;
    t->tq = 0;
    return t;
}

void thread_add_at( struct thread *t, int adding_time ){
    struct adding_thread_node *tmp_node = (struct adding_thread_node*) malloc(sizeof(struct adding_thread_node));
    tmp_node->thrd = t;
    tmp_node->adding_time = adding_time;
    tmp_node->previous = NULL;
    tmp_node->next = NULL;

    if(adding_queue == NULL){
        adding_queue = tmp_node;
        adding_queue->previous = tmp_node;
        adding_queue->next = tmp_node;
        return;
    }
    else{
        if(adding_queue->previous == adding_queue){
            //Single node in queue
            adding_queue->previous = tmp_node;
            adding_queue->next = tmp_node;
            tmp_node->previous = adding_queue;
            tmp_node->next = adding_queue;
        }
        else{
            //Two or more nodes in queue
            adding_queue->previous->next = tmp_node;
            tmp_node->previous = adding_queue->previous;
            tmp_node->next = adding_queue;
            adding_queue->previous = tmp_node;
        }
    }
}

void add_from_adding_queue(){
    struct adding_thread_node *tmp_node = adding_queue ;
    while( tmp_node != NULL ){
        struct adding_thread_node *to_remove = tmp_node;
        tmp_node = tmp_node->next;
        if( threading_system_time >= adding_queue->adding_time ){
            // printf("adding\n");
            thread_add_runqueue( to_remove->thrd ) ;
            if( to_remove->next != to_remove ){
                //Connect the remaining queue
                struct adding_thread_node *to_remove_next = to_remove->next;
                to_remove_next->previous = to_remove->previous;
                to_remove->previous->next = to_remove_next;
                free( to_remove ) ;
            }
            else{
                //No more node in adding_queue
                free( to_remove ) ;
                adding_queue = NULL;
                break;
            }
        }

        if( tmp_node == adding_queue )
            break;
    }
}

void thread_add_runqueue(struct thread *t){
    t->start_time = threading_system_time;
    t->ID  = id;
    id ++;
    if(current_thread == NULL){
        current_thread = t;
        current_thread->previous = t;
        current_thread->next = t;
        return;
    }
    else{
        if(current_thread->previous->ID == current_thread->ID){
            //Single thread in queue
            current_thread->previous = t;
            current_thread->next = t;
            t->previous = current_thread;
            t->next = current_thread;
        }
        else{
            //Two or more threads in queue
            current_thread->previous->next = t;
            t->previous = current_thread->previous;
            t->next = current_thread;
            current_thread->previous = t;
        }
    }
}

void my_thrdstop_handler(void *arg){
    uint64 myarg = (uint64) arg;
    // printf("thrd%d execute %p\n", current_thread->ID, myarg);
    current_thread->remain_execution_time -= myarg ;
    // printf("my_thrdstop_handler thrd%d myarg=%d remain=%d\n", current_thread->ID, myarg, current_thread->remain_execution_time);
    if( current_thread->remain_execution_time <= 0 )
    {
        thread_exit();
    }
    else
    {
        threading_system_time += myarg ;
        // printf("my_thrdstop_handler thrd%d myarg=%d remain=%d threading_system_time=%d\n", current_thread->ID, myarg, current_thread->remain_execution_time, threading_system_time);

        add_from_adding_queue() ;
        schedule();
        dispatch();
    }
}

void thread_yield(void){
    int consume_ticks = cancelthrdstop( current_thread->thrdstop_context_id, 0 ); // cancel previous thrdstop and save the current thread context
    // printf("consume_ticks=%d\n", consume_ticks);
    if( current_thread->is_yield == 0 )
    {
        current_thread->remain_execution_time -= consume_ticks ;

        // printf("thread_yield thrd%d consume_ticks=%d remain=%d\n", current_thread->ID, consume_ticks, current_thread->remain_execution_time);

        current_thread->is_yield = 1;
        // printf("thread_yield threading_system_time %d consume_ticks=%d\n", threading_system_time + consume_ticks, consume_ticks);

        if( current_thread->remain_execution_time <= 0 )
        {
            thread_exit();
        }
        else
        {
            threading_system_time += consume_ticks ;
            // printf("thread_yield thrd%d consume_ticks=%d remain=%d threading_system_time=%d\n", current_thread->ID, consume_ticks, current_thread->remain_execution_time, threading_system_time);
            add_from_adding_queue() ;
            schedule();
            dispatch();
        }
    }
    else{
        current_thread->is_yield = 0;
    }
}

void dispatch(void){
    if(current_thread->buf_set)
    {
        uint64 next_time = (TIME_QUANTUM_SIZE >= current_thread->remain_execution_time )? current_thread->remain_execution_time: TIME_QUANTUM_SIZE;

        // printf("dispatch1: thread%d\n", current_thread->ID);
        thrdstop( next_time, current_thread->thrdstop_context_id, my_thrdstop_handler, (void *)next_time ); 
        thrdresume(current_thread->thrdstop_context_id);
    }
    else // init
    {

        current_thread->buf_set = 1;
        unsigned long new_stack_p;
        new_stack_p = (unsigned long) current_thread->stack_p;      
        // printf("dispatch2: thread%d\n", current_thread->ID);
        current_thread->thrdstop_context_id = thrdstop( TIME_QUANTUM_SIZE, -1, my_thrdstop_handler, (void *)TIME_QUANTUM_SIZE);
        if( current_thread->thrdstop_context_id < 0 )
        {
            printf("error: number of threads may exceed\n");
            exit(1);
        }
        
        // set sp to stack pointer of current thread.
        asm volatile("mv sp, %0" : : "r" (new_stack_p));
        current_thread->fp(current_thread->arg);
       
    }
    thread_exit();
}
void schedule(void){
    #ifdef THREAD_SCHEDULER_DEFAULT

    if( is_thread_start == 0 )
    {
        // execute the first thread in wait_queue at time==0
        return ;
    }
    else
    {
        current_thread = current_thread->next;
    }

    #endif

    #ifdef THREAD_SCHEDULER_RR
    // ... implement RR here.
    current_thread->tq++;
    // printf("[%d] rr thrd%d tq=%d remain=%d yield=%d\n", uptime(), current_thread->ID, current_thread->tq, current_thread->remain_execution_time, current_thread->is_yield);
    if( is_thread_start == 0 )
    {
        // execute the first thread in wait_queue at time==0
        // printf("rr this is the first start");
        return ;
    }
    else if (current_thread->tq == 4 || current_thread->remain_execution_time == 0 || current_thread->is_yield)
    {
        current_thread->tq = 0;
        current_thread = current_thread->next;
        current_thread->tq = 0;
        // printf("[%d] rr switch to thrd%d\n", uptime(), current_thread->ID);
    }
    else
    {
        // printf("rr keep thrd%d \n", current_thread->ID);
    }
    #endif

    #ifdef THREAD_SCHEDULER_FCFS
    // ... implement FCFS here
    if (is_thread_start == 0 || current_thread->remain_execution_time > 0)
    {
        return;
    }
    else
    {
        current_thread = current_thread->next;
    }
    
    #endif


    #ifdef THREAD_SCHEDULER_SJF
    // ... implement SJF here.
    // find the shortest job
    
    struct thread* ptr = current_thread;
    int shortest_remain_time = -1;
    struct thread* selected_thread = NULL;
    if (is_thread_start && current_thread->remain_execution_time > 0)
    {
        // keep current thread
        return;
    }
    
    do
    {
        if (shortest_remain_time == -1 || ptr->remain_execution_time < shortest_remain_time)
        {
            if (ptr->remain_execution_time > 0)
            {
                selected_thread = ptr;
                shortest_remain_time = ptr->remain_execution_time;
            }
            
        }
        else if (ptr->remain_execution_time == shortest_remain_time && selected_thread->start_time > ptr->start_time)
        {
            if (ptr->remain_execution_time > 0)
            {
                selected_thread = ptr;
                shortest_remain_time = ptr->remain_execution_time;
            }
        }
        
        ptr = ptr->next;
    } while (ptr != current_thread);
    
    current_thread = selected_thread;

    // printf("sjf select=%d remain=%d\n", current_thread->ID, shortest_remain_time);

    #endif

    #ifdef THREAD_SCHEDULER_PSJF
    // ... implement PSJF here.
    struct thread* ptr = current_thread;
    int shortest_remain_time = -1;
    struct thread* selected_thread = NULL;
    do
    {
        if (shortest_remain_time == -1 || ptr->remain_execution_time < shortest_remain_time)
        {
            if (ptr->remain_execution_time > 0)
            {
                selected_thread = ptr;
                shortest_remain_time = ptr->remain_execution_time;
            }
            
        }
        else if (ptr->remain_execution_time == shortest_remain_time && selected_thread->start_time > ptr->start_time)
        {
            if (ptr->remain_execution_time > 0)
            {
                selected_thread = ptr;
                shortest_remain_time = ptr->remain_execution_time;
            }
        }
        
        ptr = ptr->next;
    } while (ptr != current_thread);
    
    current_thread = selected_thread;

    // printf("psjf select=%d remain=%d\n", current_thread->ID, shortest_remain_time);
    #endif

}
void thread_exit(void){
    // remove the thread immediately, and cancel previous thrdstop.
    int consume_ticks = cancelthrdstop(current_thread->thrdstop_context_id, 1);
    // printf("consume_ticks %d\n", consume_ticks);
    threading_system_time += consume_ticks ;
    // printf("thrd%d exit threading_system_time %d consume_ticks=%d\n",current_thread->ID, threading_system_time, consume_ticks);

    add_from_adding_queue() ;

    struct thread* to_remove = current_thread;
    // int nowTime = uptime();
    printf("thrd%d exec %d ticks\n", to_remove->ID, threading_system_time - to_remove->start_time);

    to_remove->is_exited = 1;

    if(to_remove->next != to_remove){
        //Still more thread to execute
        schedule() ;
        //Connect the remaining threads
        struct thread* to_remove_next = to_remove->next;
        to_remove_next->previous = to_remove->previous;
        to_remove->previous->next = to_remove_next;


        //free pointers
        free(to_remove->stack);
        free(to_remove);
        dispatch();
    }
    else{
        //No more thread to execute
        free(to_remove->stack);
        free(to_remove);
        current_thread = NULL;
        longjmp(env_st, -1);
    }
}
void thread_start_threading(){
    int r;
    r = setjmp(env_st);
    
    if(current_thread != NULL && r==0){
        schedule() ;
        is_thread_start = 1;
        dispatch();
    }
}
