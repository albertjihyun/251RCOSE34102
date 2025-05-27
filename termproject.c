#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

//  Process 개수를 전역으로 설정 -> main에서 시작할 때 값 지정정
static int p_cnt = 0;
//  Process
typedef struct{
    //  process 기본 변수
    int pid;
    int arrival_time;
    int cpu_burst_time;
    //int io_burst_time;
    //int io_request_time;
    int priority;

    //  기타 변수 - cpu scheduling simulator 구현 및 평가에 있어 추가적으로 필요한 변수
    int remaining_time;     // 프로세스가 running queue에 있는 순간마다 ++;
    int turnaround_time;    // remaining time이 0이 되는 순간 기록. Average turnaroung time 계산시 사용
    int waiting_time;       // 프로세스가 waiting queue에 있는 순간마다 ++; Average waiting time 계산시 사용

} Process;

//  evaluation()을 위한 구조체
typedef struct{
    char algorithm[32];
    float avg_waiting_time;
    float avg_turnaround_time;
} Result;

//  레디 큐
typedef struct{
    Process *p_arr;
    int cnt;
    int capacity;
} Ready_queue;

//  함수 원형
void create_processes(Process *p);
Ready_queue *config(int capacity);
void enqueue(Ready_queue *r_q, Process p);
void schedule(Process *p, Result *r);
int fcfs_compare(const void *a, const void *b);
int sjf_compare(const void *a, const void *b);
int priority_compare(const void *a, const void *b);
void fcfs(Process *p, Result *r);
void sjf_np(Process *p, Result *r);
void sjf_p(Process *p,  Result *r);
void priority_np(Process *p, Result *r);
void priority_p(Process *p,  Result *r);
void rr(Process *p, Result *r);
void evaulation();

//  함수 정의
/*  create_processes(): 프로세스 6개 생성 & 초기 변수값 설정 - random
    Process ID, Arrival time, CPU burst time, I/O burst time, I/0 request time, Priority
    pid는 온 순서대로 정수값 설정, Arrival time & CPU burst time & I/O burst, request time & Priority -> random 값
    CPU burst time은 1에서 20 범위 설정 -> 지나치게 늘어지는 것을 막기 위함
    I/O의 경우, 발생 횟수를 2~5회 범위 내 random 값 지정 -> 시간 소요 줄이기 위함
    Priority의 경우, process의의 총 갯수 n 받아 0~(n-1) 중 random하게 배정
*/
void create_processes(Process *p){
    for(int i=0;i<p_cnt;i++){
        p[i].pid = i;
        p[i].arrival_time = rand() % 10;
        p[i].cpu_burst_time = (rand() % 10)+1;
        //p[i].io_burst_time = rand() % 3;
        //p[i].io_request_time = rand() % 4;
        p[i].priority = rand() % 5;

        p[i].remaining_time = p[i].cpu_burst_time;
        p[i].turnaround_time = 0;
        p[i].waiting_time = 0;
    }
}

/*  config(): 시스템 환경 설정
    Ready queue, Waiting Queue
*/
Ready_queue *config(int capacity){
    Ready_queue *r_q = (Ready_queue*)malloc(sizeof(Ready_queue));
    if(!r_q) printf("r_q error");

    r_q->p_arr = (Process*)malloc(sizeof(Process) * capacity);
    r_q->cnt = 0;
    r_q->capacity = capacity;

    return r_q;
}

//  큐에 프로세스 삽입
void enqueue(Ready_queue *r_q, Process p){
    if(r_q->cnt>=r_q->capacity);

    r_q->p_arr[r_q->cnt] = p;
    r_q->cnt++;
}

//  fcfs,sjf,priority에서 쓰일 qsort를 위한 compare
int fcfs_compare(const void *a, const void *b){
    Process *p1 = (Process*)a;
    Process *p2 = (Process*)b;
    return p1->arrival_time - p2->arrival_time;
}
int sjf_compare(const void *a, const void *b){
    Process *p1 = (Process*)a;
    Process *p2 = (Process*)b;
    return p1->cpu_burst_time - p2->cpu_burst_time;
}
int priority_compare(const void *a, const void *b){
    Process *p1 = (Process*)a;
    Process *p2 = (Process*)b;
    return p1->priority - p2->priority;
}

/*  schedule(): CPU 스케줄링 알고리즘 구현
    FCFS, SJF(nonpreemptive & preemptive), Priority(nonpreemptive & preemptive), RR
*/
void schedule(Process *p, Result *r){
    fcfs(p,&r[0]);
    sjf_np(p,&r[1]);
    sjf_p(p,&r[2]);
    priority_np(p,&r[3]);
    priority_p(p,&r[4]);
    rr(p,&r[5]);
}

//  fcfs(): FCFS algorithm 구현
void fcfs(Process *p, Result *r){
    Ready_queue *r_q = config(p_cnt+1); //  ready queue 생성 - config()
    for(int i=0;i<p_cnt;i++) enqueue(r_q, p[i]);    //  enqueue
    
    qsort(r_q->p_arr, r_q->cnt, sizeof(Process), fcfs_compare); //  도착시간 기준 정렬

    int current=0;  //  현재 시각
    float total_wait=0, total_turnaround=0; //  result 위한 변수
    int p_start[p_cnt], p_end[p_cnt];   //  Gantt-Chart 위한 변수수

    printf("\nGantt-Chart : FCFS\n");

    for(int i=0;i<r_q->cnt;i++){
        Process *f_p = &r_q->p_arr[i];
        while(current<f_p->arrival_time) current++;

        p_start[i] = current;
        if(i==0 && p_start[0]!=0) printf(" | ");    //  P0가 0에 시작하지 않을 때 Gantt-Chart 프로세스 흐름

        f_p->waiting_time = current - f_p->arrival_time;    //  대기 시간 계산
        while(f_p->remaining_time){ //  cpu-burst time이 다할 때까지 시간 흐름
            f_p->remaining_time--;
            f_p->turnaround_time++;
            current++;
        }
        f_p->turnaround_time = current; //  종료시간 계산
        p_end[i] = current; //  Gantt-Chart 위해 각각 종료 시점 기억해두기

        total_wait += f_p->waiting_time;
        total_turnaround += f_p->turnaround_time;
        printf("| P%d ",i); //  Gantt-Chart 프로세스 흐름
    }
    printf("|\n");

    //  Gantt-Chart 시간 흐름
    printf("0");
    if(p_start[0]!=0) printf("  %d", p_start[0]);   //  P0가 0에 시작하지 않을 때 Gantt-Chart 시간 흐름
    for(int i=0;i<p_cnt;i++) printf("   %d",p_end[i]);
    printf("\n\n");

    //  result_arr[0]에 fcfs 결과값 저장장
    strcpy(r->algorithm,"FCFS");
    r->avg_waiting_time = total_wait/p_cnt;
    r->avg_turnaround_time = total_turnaround/p_cnt;

    //  할당된 메모리 해제제
    free(r_q->p_arr);
    free(r_q);
}

//  sjf_np(): non-preemptive sjf algorithm 구현
void sjf_np(Process *p, Result *r){
    
}

//  sjf_p(): preemptive sjf algorithm 구현
void sjf_p(Process *p, Result *r){

}

//  Priority(nonpreemptive)
void priority_np(Process *p, Result *r){

}

//  Priority(preemptive)
void priority_p(Process *p, Result *r){

}

//  RR
void rr(Process *p, Result *r){

}

//  각 스케줄링 알고리즘들간 비교 평가
//  Average waiting time & turnaround time
void evaulation(Result *r){
    
}

// 메인 함수 작동
int main(){
    srand(time(NULL));
    p_cnt = (rand()%3)+4;   //  프로세스 개수 랜덤 설정(4~6 사이) -> 적절히 끝나게 하기 위함함

    Process *process_arr = (Process*)malloc(sizeof(Process) * p_cnt);   //  프로세스 배열
    Result *result_arr = (Result*)malloc(sizeof(Result) * 6);   //  evaluation()을 위한 결과 배열, 알고리즘 6개
    
    create_processes(process_arr);    //  랜덤 갯수 넘겨 create_processes()
    schedule(process_arr, result_arr); //  schedule() -> 알고리즘 구현
    //evaulation(result_arr);

    free(process_arr);  //  메모리 할당 해제
    free(result_arr);
    return 0;
}