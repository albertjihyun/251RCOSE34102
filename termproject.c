#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//  변수
//  Process
typedef struct{
    //  process 기본 변수
    int pid;
    int arrival_time;
    int cpu_burst_time;
    int io_burst_time;
    int io_request_time;
    int priority;

    //  기타 변수 - cpu scheduling simulator 구현 및 평가에 있어 추가적으로 필요한 변수
    int remaining_time;     // 프로세스가 running queue에 있는 순간마다 ++;
    int turnaround_time;    // remaining time이 0이 되는 순간 기록. Average turnaroung time 계산시 사용
    int waiting_time;       // 프로세스가 waiting queue에 있는 순간마다 ++; Average waiting time 계산시 사용

} Process;

typedef struct{
    
} Queue;

//  함수 원형
void create_processes();
void config();
void schedule();
void fcfs();
void sjf_np();
void sjf_p();
void priority_np();
void priority_p();
void rr();
void evaulation();

//  함수 정의
/*  create_processes(): 프로세스 생성 & 초기 변수값 설정 - random
    Process ID, Arrival time, CPU burst time, I/O burst time, I/0 request time, Priority
    pid는 온 순서대로 정수값 설정, Arrival time & CPU burst time & I/O burst, request time & Priority -> random 값
    CPU burst time은 1에서 20 범위 설정 -> 지나치게 늘어지는 것을 막기 위함
    I/O의 경우, 발생 횟수를 2~5회 범위 내 random 값 지정 -> 시간 소요 줄이기 위함
    Priority의 경우, process의의 총 갯수 n 받아 0~(n-1) 중 random하게 배정
*/
void create_processes(){

}

/*  config(): 시스템 환경 설정
    Ready queue, Waiting Queue
*/
void config(){

}

/*  schedule(): CPU 스케줄링 알고리즘 구현
    FCFS, SJF(nonpreemptive & preemptive), Priority(nonpreemptive & preemptive), RR
*/
void schedule(){
    fcfs();
    sjf_np();
    sjf_p();
    priority_np();
    priority_p();
    rr();
}

//  fcfs(): FCFS algorithm 구현
void fcfs(){

}

//  sjf_np(): non-preemptive sjf algorithm 구현
void sjf_np(){

}

//  sjf_p(): preemptive sjf algorithm 구현
void sjf_p(){

}

//  Priority(nonpreemptive)
void priority_np(){

}

//  Priority(preemptive)
void priority_p(){

}

//  RR
void rr(){

}

//  각 스케줄링 알고리즘들간 비교 평가
//  Average waiting time & turnaround time
void evaulation(){

}

// 메인 함수 작동
int main(){
    //  프로세스 배열
    Process *p;
    //  evaluation()을 위한 스케줄링 함수 결과 배열
    Result *r;

    
    create_processes(p);
    config();
    schedule(p);
    evaulation();


    return 0;
}