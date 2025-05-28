#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

//  Process 개수를 전역으로 설정 -> main에서 시작할 때 값 지정정
#define time_quantum 2
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
} Ready_queue;

//  함수 원형
void create_processes(Process *p);
Ready_queue *config(int capacity);
void enqueue(Ready_queue *r_q, Process p);
void schedule(Process *p, Result *r);
int wait_compare(const void *a, const void *b);
int turnaround_compare(const void *a, const void *b);
int fcfs_compare(const void *a, const void *b);
void fcfs(Process *p, Result *r);
void sjf_np(Process *p, Result *r);
void sjf_p(Process *p,  Result *r);
void priority_np(Process *p, Result *r);
void priority_p(Process *p,  Result *r);
void rr(Process *p, Result *r);
void evaulation(Result *r);

//  함수 정의
/*  create_processes(): 프로세스 4~6개 랜덤 생성 & 초기 변수값 설정 - random
    Process ID, Arrival time, CPU burst time, I/O burst time, I/0 request time, Priority
    pid는 온 순서대로 정수값 설정, Arrival time & CPU burst time & I/O burst, request time & Priority -> random 값
    CPU burst time은 1에서 10 범위 설정 -> 지나치게 늘어지는 것을 막기 위함
    I/O의 경우, 발생 횟수를 2~5회 범위 내 random 값 지정 -> 시간 소요 줄이기 위함
    Priority의 경우, process의의 총 갯수 n 받아 0~(n-1) 중 random하게 배정
*/
void create_processes(Process *p){
    printf("process\tarrival\tcpu\tpriority\n");
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

        printf("%d\t%d\t%d\t%d\n",p[i].pid,p[i].arrival_time,p[i].cpu_burst_time,p[i].priority);
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

    return r_q;
}

//  큐에 프로세스 삽입
void enqueue(Ready_queue *r_q, Process p){
    r_q->p_arr[r_q->cnt] = p;
    r_q->cnt++;
}

// evaluation에서 쓰일 qsort를 위한 compare
int wait_compare(const void *a, const void *b){
    Result *r1 = (Result*)a;
    Result *r2 = (Result*)b;
    return r1->avg_waiting_time - r2->avg_waiting_time;
}
int turnaround_compare(const void *a, const void *b){
    Result *r1 = (Result*)a;
    Result *r2 = (Result*)b;
    return r1->avg_turnaround_time - r2->avg_turnaround_time;
}
//  fcfs,priority에서 쓰일 qsort를 위한 compare
int fcfs_compare(const void *a, const void *b){
    Process *p1 = (Process*)a;
    Process *p2 = (Process*)b;
    return p1->arrival_time - p2->arrival_time;
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

    for(int i=0;i<p_cnt;i++){
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
        printf("| P%d ",f_p->pid); //  Gantt-Chart 프로세스 흐름
    }
    printf("|\n");

    //  Gantt-Chart 시간 흐름
    printf("0");
    if(p_start[0]!=0) printf("  %d", p_start[0]);   //  P0가 0에 시작하지 않을 때 Gantt-Chart 시간 흐름
    for(int i=0;i<p_cnt;i++) printf("   %d",p_end[i]);
    printf("\n\n\n");

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
    Ready_queue *r_q = config(p_cnt+1); //  ready queue 생성 - config()
    for(int i=0;i<p_cnt;i++) enqueue(r_q, p[i]);    //  enqueue

    int completed=0, current=0; //  완료된 프로세스 갯수, 현재 시각
    float total_wait=0, total_turnaround=0; //  result 위한 변수
    int p_start[p_cnt], p_end[p_cnt];  //  Gantt-Chart 위한 변수수

    printf("\nGantt-Chart : SJF(Non-preemptive)\n");

    while(completed<p_cnt){
        int flag=-1;
        int min = 100;
        
        for(int i=0;i<p_cnt;i++){
            Process *f_p = &r_q->p_arr[i];
            if(f_p->remaining_time!=0 && f_p->arrival_time<=current){   //  끝나지 않았고, 도착했다면
                if(f_p->cpu_burst_time<min){    //  제일 작다면
                    min = f_p->cpu_burst_time;
                    flag = i;
                }
            }
        }

        if(flag==-1){   //  실행 불가 -> +1초초
            current++;
            continue;
        }

        Process *r_p = &r_q->p_arr[flag];
        p_start[completed] = current;   //  시작점 저장장
        if(completed==0 && p_start[0]!=0) printf(" | ");    //  0에 시작하지 않을 때 Gantt-Chart 프로세스 흐름

        r_p->waiting_time = current - r_p->arrival_time;
        while(r_p->remaining_time>0){   //  cpu burst 끝날 때까지 시간 증가가
            r_p->remaining_time--;
            current++;
        }

        r_p->turnaround_time = current;
        p_end[completed] = current; //  종료지점 저장장

        total_wait += r_p->waiting_time;
        total_turnaround += r_p->turnaround_time;
        completed++;
        printf("| P%d", r_p->pid);
    }
    printf("|\n");

    //  Gantt-Chart 시간 흐름
    printf("0");
    if(p_start[0]!=0) printf("  %d", p_start[0]);   //  P0가 0에 시작하지 않을 때 Gantt-Chart 시간 흐름
    for(int i=0;i<p_cnt;i++) printf("   %d",p_end[i]);
    printf("\n\n\n");

    //  result_arr[1]에 sjf_np 결과값 저장장
    strcpy(r->algorithm,"SJF(Non-preemptive)");
    r->avg_waiting_time = total_wait/p_cnt;
    r->avg_turnaround_time = total_turnaround/p_cnt;

    //  할당된 메모리 해제제
    free(r_q->p_arr);
    free(r_q);
}

//  sjf_p(): preemptive sjf algorithm 구현
void sjf_p(Process *p, Result *r){
    Ready_queue *r_q = config(p_cnt+1); //  ready queue 생성 - config()
    for(int i=0;i<p_cnt;i++) enqueue(r_q, p[i]);    //  enqueue

    int completed=0, current=0, finish_time=0; //  완료된 프로세스 갯수, 현재 시각
    float total_wait=0, total_turnaround=0; //  result 위한 변수
    int gantt_chart[200];  //  Gantt-Chart 위한 변수수

    printf("\nGantt-Chart : SJF(preemptive)\n");
    
    while(completed<p_cnt){
        int flag=-1;
        int min = 100;
        for(int i=0;i<p_cnt;i++){
            Process *f_p = &r_q->p_arr[i];
            if(f_p->remaining_time!=0 && f_p->arrival_time<=current){   //  끝나지 않았고, 도착했다면
                if(f_p->remaining_time<min){    //  제일 작다면
                    min = f_p->remaining_time;
                    flag = i;
                }
            }
        }

        if(flag==-1){   //  실행 불가 -> +1초초
            gantt_chart[current] = -1;
            current++;
            continue;
        }

        Process *r_p = &r_q->p_arr[flag];   //  현재 실행 중인 프로세스

        gantt_chart[current] = r_p->pid;    //  ganttchart에 p0 등 저장장

        r_p->remaining_time--;  //  매초 preemption 일어날 수 있으므로 1초만
        current++;

        if(r_p->remaining_time==0){ //  만약 끝나버렸다면
            r_p->turnaround_time = current;
            r_p->waiting_time = r_p->turnaround_time - r_p->arrival_time - r_p->cpu_burst_time;

            total_turnaround += r_p->turnaround_time;
            total_wait += r_p->waiting_time;

            completed++;
            if(current>finish_time) finish_time = current;  //  ganttchart 그릴 때 조금만 그리기 위해해
        }
    }

    int prev = -100;    //  변화가 있을 때만 그리기기
    for(int i=0;i<finish_time;i++){
        if(gantt_chart[i]!=prev && gantt_chart[i]>=0){
            if(i!=0 && prev==-100) printf("| | P%d ",gantt_chart[i]);   //  0부터 시작하지 않을 때때
            else printf("| P%d ",gantt_chart[i]);

            prev = gantt_chart[i];
        }
    }
    printf("|\n");

    prev = -100;
    printf("0");
    for(int i=0;i<finish_time;i++){
        if(gantt_chart[i]!=prev && gantt_chart[i]>=0){
            if(i!=0) printf("   %d ",i);    //  0부터 시작하지 않을 때때
            prev = gantt_chart[i];
        }
    }
    printf("   %d\n\n\n",finish_time);

    //  result_arr[2]에 sjf_np 결과값 저장장
    strcpy(r->algorithm,"SJF(Preemptive)");
    r->avg_waiting_time = total_wait/p_cnt;
    r->avg_turnaround_time = total_turnaround/p_cnt;

    //  할당된 메모리 해제제
    free(r_q->p_arr);
    free(r_q);
}

//  Priority(nonpreemptive)
void priority_np(Process *p, Result *r){
    Ready_queue *r_q = config(p_cnt+1); //  ready queue 생성 - config()
    for(int i=0;i<p_cnt;i++) enqueue(r_q, p[i]);    //  enqueue

    int completed=0, current=0; //  완료된 프로세스 갯수, 현재 시각
    float total_wait=0, total_turnaround=0; //  result 위한 변수
    int p_start[p_cnt], p_end[p_cnt];  //  Gantt-Chart 위한 변수수

    printf("\nGantt-Chart : Priority(Non-preemptive)\n");

    while(completed<p_cnt){
        int flag=-1;
        int min = 100;

        for(int i=0;i<p_cnt;i++){
            Process *f_p = &r_q->p_arr[i];
            if(f_p->remaining_time!=0 && f_p->arrival_time<=current){
                if(f_p->priority<min){
                    min = f_p->priority;
                    flag=i;
                }
            }
        }

        if(flag==-1){
            current++;
            continue;
        }

        Process *r_p = &r_q->p_arr[flag];
        p_start[completed] = current;
        if(completed==0 && p_start[0]!=0) printf(" | ");    //  0에 시작하지 않을 때 Gantt-Chart 프로세스 흐름

        r_p->waiting_time = current - r_p->arrival_time;
        while(r_p->remaining_time>0){
            r_p->remaining_time--;
            current++;
        }

        r_p->turnaround_time = current;
        total_wait += r_p->waiting_time;
        total_turnaround += r_p->turnaround_time;

        p_end[completed] = current;
        completed++;

        printf("| P%d ",r_p->pid);
    }
    printf("|\n");

    printf("0");
    if(p_start[0]!=0) printf("  %d", p_start[0]);   //  P0가 0에 시작하지 않을 때 Gantt-Chart 시간 흐름
    for(int i=0;i<p_cnt;i++) printf("   %d",p_end[i]);
    printf("\n\n\n");

    //  result_arr[3]에 sjf_np 결과값 저장장
    strcpy(r->algorithm,"Priority(Preemptive)");
    r->avg_waiting_time = total_wait/p_cnt;
    r->avg_turnaround_time = total_turnaround/p_cnt;

    //  할당된 메모리 해제제
    free(r_q->p_arr);
    free(r_q);
}

//  Priority(preemptive)
void priority_p(Process *p, Result *r){
    Ready_queue *r_q = config(p_cnt+1); //  ready queue 생성 - config()
    for(int i=0;i<p_cnt;i++) enqueue(r_q, p[i]);    //  enqueue

    int completed=0, current=0, finish_time=0; //  완료된 프로세스 갯수, 현재 시각
    float total_wait=0, total_turnaround=0; //  result 위한 변수
    int gantt_chart[200];  //  Gantt-Chart 위한 변수수

    printf("\nGantt-Chart : Priority(preemptive)\n");
    
    while(completed<p_cnt){
        int flag=-1;
        int min = 100;

        for(int i=0;i<p_cnt;i++){
            Process *f_p = &r_q->p_arr[i];
            if(f_p->remaining_time!=0 && f_p->arrival_time<=current){
                if(f_p->priority<min){
                    min=f_p->priority;
                    flag=i;
                }
            }
        }

        if(flag==-1){
            gantt_chart[current]=-1;
            current++;
            continue;
        }

        Process *r_p = &r_q->p_arr[flag];

        gantt_chart[current] = r_p->pid;    //  ganttchart에 p0 등 저장장

        r_p->remaining_time--;  //  매초 preemption 일어날 수 있으므로 1초만
        current++;

        if(r_p->remaining_time==0){ //  만약 끝나버렸다면
            r_p->turnaround_time = current;
            r_p->waiting_time = r_p->turnaround_time - r_p->arrival_time - r_p->cpu_burst_time;

            total_turnaround += r_p->turnaround_time;
            total_wait += r_p->waiting_time;

            completed++;
            if(current>finish_time) finish_time = current;  //  ganttchart 그릴 때 조금만 그리기 위해해
        }
    }

    int prev = -100;    //  변화가 있을 때만 그리기기
    for(int i=0;i<finish_time;i++){
        if(gantt_chart[i]!=prev && gantt_chart[i]>=0){
            if(i!=0 && prev==-100) printf("| | P%d ",gantt_chart[i]);   //  0부터 시작하지 않을 때때
            else printf("| P%d ",gantt_chart[i]);

            prev = gantt_chart[i];
        }
    }
    printf("|\n");

    prev = -100;
    printf("0");
    for(int i=0;i<finish_time;i++){
        if(gantt_chart[i]!=prev && gantt_chart[i]>=0){
            if(i!=0) printf("   %d ",i);    //  0부터 시작하지 않을 때때
            prev = gantt_chart[i];
        }
    }
    printf("   %d\n\n\n",finish_time);

    //  result_arr[4]에 sjf_np 결과값 저장장
    strcpy(r->algorithm,"Priority(Preemptive)");
    r->avg_waiting_time = total_wait/p_cnt;
    r->avg_turnaround_time = total_turnaround/p_cnt;

    //  할당된 메모리 해제제
    free(r_q->p_arr);
    free(r_q);
}

//  RR
void rr(Process *p, Result *r){
    Ready_queue *r_q = config(p_cnt+1); //  ready queue 생성 - config()
    for(int i=0;i<p_cnt;i++) enqueue(r_q, p[i]);    //  enqueue
    
    qsort(r_q->p_arr, r_q->cnt, sizeof(Process), fcfs_compare); //  도착시간 기준 정렬

    int current=0, completed=0, finish_time=0, head=0, tail=0, next=0;
    float total_wait=0, total_turnaround=0;
    int gantt_chart[200], rr_queue[1000], check[6];
    memset(check,0,sizeof(check));

    printf("\nGantt-Chart : Round-robin\n");

    while(p_cnt>next && r_q->p_arr[next].arrival_time <= current){
        rr_queue[tail]=next;
        tail++;
        check[next]=1;
        next++;
    }

    while(completed<p_cnt){
        if(head==tail){
            gantt_chart[current]=-1;
            current++;
            while(p_cnt>next && r_q->p_arr[next].arrival_time <= current){
                rr_queue[tail]=next;
                tail++;
                check[next]=1;
                next++;
            }
            continue;
        }

        int flag = rr_queue[head];  //  실행할 프로세스, 머리에서 뽑음
        head++;
        Process *r_p = &r_q->p_arr[flag];

        int exec;
        if(r_p->remaining_time < time_quantum) exec = r_p->remaining_time;  //  남은 시간에 따라 작업 시간 상이
        else exec = time_quantum;

        for(int i=0;i<exec;i++){
            r_p->remaining_time--;
            gantt_chart[current]=r_p->pid;  //  exec만큼 실행, 간트 차트에도 저장
            current++;

            while(p_cnt>next && r_q->p_arr[next].arrival_time <= current){  //  프로세스 실시간 삽입
                rr_queue[tail]=next;
                tail++;
                check[next]=1;
                next++;
            }
        }

        if(r_p->remaining_time==0){
            r_p->turnaround_time=current;
            r_p->waiting_time = r_p->turnaround_time-r_p->cpu_burst_time-r_p->arrival_time;
            total_wait+=r_p->waiting_time;
            total_turnaround+=r_p->turnaround_time;
            completed++;
            if(current>finish_time) finish_time = current;
        }
        else{
            rr_queue[tail]=flag;    //  아직 남았다면 맨 뒤로
            tail++;
        }
    }

    int prev = -100;    //  변화가 있을 때만 그리기기
    for(int i=0;i<finish_time;i++){
        if(gantt_chart[i]!=prev && gantt_chart[i]>=0){
            if(i!=0 && prev==-100) printf("| | P%d ",gantt_chart[i]);   //  0부터 시작하지 않을 때때
            else printf("| P%d ",gantt_chart[i]);

            prev = gantt_chart[i];
        }
    }
    printf("|\n");

    prev = -100;
    printf("0");
    for(int i=0;i<finish_time;i++){
        if(gantt_chart[i]!=prev && gantt_chart[i]>=0){
            if(i!=0) printf("   %d ",i);    //  0부터 시작하지 않을 때때
            prev = gantt_chart[i];
        }
    }
    printf("   %d\n\n\n",finish_time);

    //  result_arr[5]에 sjf_np 결과값 저장장
    strcpy(r->algorithm, "Round-robin");
    r->avg_waiting_time = total_wait/p_cnt;
    r->avg_turnaround_time = total_turnaround/p_cnt;

    //  할당된 메모리 해제제
    free(r_q->p_arr);
    free(r_q);
}

//  각 스케줄링 알고리즘들간 비교 평가
//  Average waiting time & turnaround time
void evaulation(Result *r){
    Result *wait_res = (Result*)malloc(sizeof(Result)*6);
    Result *turnaround_res = (Result*)malloc(sizeof(Result)*6);
    memcpy(wait_res, r, sizeof(Result)*6);
    memcpy(turnaround_res, r, sizeof(Result)*6);
    qsort(wait_res, 6, sizeof(Result),wait_compare);
    qsort(turnaround_res, 6, sizeof(Result),turnaround_compare);

    printf("\nEvaluation of cpu scheduling algorithms - avg waiting time\n\n");
    for(int i=0;i<6;i++){
        printf("%d위: %s\t",i+1, wait_res[i].algorithm);
    }
    printf("\nEvaluation of cpu scheduling algorithms - avg turnaround time\n\n");
    for(int i=0;i<6;i++){
        printf("%d위: %s\t",i+1, turnaround_res[i].algorithm);
    }

    free(wait_res);
    free(turnaround_res);
}

// 메인 함수 작동
int main(){
    srand(time(NULL));
    p_cnt = (rand()%3)+4;   //  프로세스 개수 랜덤 설정(4~6 사이) -> 적절히 끝나게 하기 위함함

    Process *process_arr = (Process*)malloc(sizeof(Process) * p_cnt);   //  프로세스 배열
    Result *result_arr = (Result*)malloc(sizeof(Result) * 6);   //  evaluation()을 위한 결과 배열, 알고리즘 6개
    
    create_processes(process_arr);    //  랜덤 갯수 넘겨 create_processes()
    schedule(process_arr, result_arr); //  schedule() -> 알고리즘 구현
    evaulation(result_arr);

    free(process_arr);  //  메모리 할당 해제
    free(result_arr);
    return 0;
}