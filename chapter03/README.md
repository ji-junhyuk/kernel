* [kernel debugging and code learning](#kernel-debugging-and-code-learning)
	* [디버깅](#디버깅)
		* [임베디드 리눅스 BSP 디버깅 툴](#임베디드-리눅스-bsp-디버깅-툴)
		* [커널 로그 분석](#커널-로그-분석)
		* ['cat /proc/interrupts' 디버깅](#cat-procinterrupts-디버깅)
		* [디버깅 순서](#디버깅-순서)
		* [ftrace 로그에서 알 수 있는 정보](#ftrace-로그에서-알-수-있는-정보)
		* [단순히 코드 분석이 아닌 디버깅을 해야 하는 이유](#단순히-코드-분석이-아닌-디버깅을-해야-하는-이유)
	* [printk()](#printk)
		* [printk를 써서 함수의 심벌 정보를 출력하는 실습](#printk를-써서-함수의-심벌-정보를-출력하는-실습)
		* [printk를 쓸 때 주의사항](#printk를-쓸-때-주의사항)
	* [dump_stack()](#dump_stack)
		* [실습](#실습)
		* [dump_stack()를 쓸 때 주의사항](#dump_stack를-쓸-때-주의사항)
	* [ftrace](#ftrace)
		* [커널 컨피그](#커널-컨피그)
		* [ftrace 관련 디렉토리](#ftrace-관련-디렉토리)
		* [ftrace 설정](#ftrace-설정)
		* [1. tracing_on: 트레이서 활성화/비활성화하기](#1-tracing_on-트레이서-활성화비활성화하기)
		* [2. current_tracer](#2-current_tracer)
		* [3. event](#3-event)
		* [4. filter](#4-filter)
		* [5. func_stack_trace](#5-func_stack_trace)
		* [6. sym-offset](#6-sym-offset)
		* [7. 추가 설정 파일](#7-추가-설정-파일)
# kernel debugging and code learning
## 디버깅
### 임베디드 리눅스 BSP 디버깅 툴
- 로그기반
	- UART 로그
	- 커널 로그 -dmesg
	- ftrace
- 메모리 덤프 
	- coredump(유저 영역) GDB를 debugging
	- vmcore(크래시 유틸리티) + ramdump
	- JTAG Hardward Debugger: TRACE32

### 커널 로그 분석
1. 오류 메시지를 커널 어디에서 출력했는지 확인
2. `디버그 패치 작성` 후 테스트
		
### 'cat /proc/interrupts' 디버깅
- 알고 싶은 정보
	- 인터럽트 디스크립터인 irq_desc 구조체의 action 필드에 저장된 인터럽트 속성 정보를 점검
	- 'cat /proc/interrupts' 명령어를 입력하면 show_interrupts() 함수가 호출되는지 확인
	- show_interrupts() 함수를 호출할 때 프로세스 정보
```c
action = desc->action;

if (action)
	rpi_get_interrupt_info(action);
	// main logic 전에 action필드가 설정되었으면 구조체 정보 출력
if (action)
	main logic
```
### 디버깅 순서
1. 패치 코드 작성 후 리눅스 커널 빌드
2. 커널 이미지 빌드
3. 라즈베리파이 재부팅
4. ftrace 설정 셀 스크립트 실행
5. 'cat /proc/interrupts' 명령어 입력
6. ./get_ftrace.sh // 로그를 받아온다.
7. ftrace 로그를 분석한다.

### ftrace 로그에서 알 수 있는 정보
- 4114번째 줄, cat-1106 : pid가 1106인 cat process가 `rpi_get_interrupt_info` 호출
- 4124 번째 줄(`ret_fast_syscall`)에서 4116번째 줄(`rpi_get_interrupt_info`)까지 함수가 호출되었다.

### 단순히 코드 분석이 아닌 디버깅을 해야 하는 이유
```
'cat /proc/interrupts'가 실행되었을 때,
show_interrupts()함수가 호출되겠구나! 
```
- `패치코드`를 통해 프로세스 이름, 인터럽트 번호, 인터럽트 이름, 인터럽트 핸들러 함수 이름을 알 수 있다.
- `ftrace`를 통해 show_interrupts()함수가 호출되기 전에 1.ret_fast_syscall 2. sys_read 3.ksys_read 4.vfs_read 5.proc_reg_read 6.seq_read 함수가 먼저 호출되는 것을 알 수 있다!
- 디바이스 코드를 작성 후 testing 과정을 거치면, 다양한 오류를 만나게 된다. 내가 작성한 디바이스 드라이버 코드에서 오류가 발생하면 디버깅이 쉽지만, 대부분 `리눅스 커널 내부에 있는 함수에서 오류 정보가 출력`된다.

## printk()
```bash
$ cat /var/log/kern.log
```
- 자료형에 따른 포멧

###  printk를 써서 함수의 심벌 정보를 출력하는 실습
- 커널에 어느 함수에서나 적용가능한 패치 코드
```c
static void insert_wq_barrier(struct pool_workqueue *pwq, struct wq_barrier *barrier *barr, struct work_struct *target, struct worker *worker)
{
	struct list_head *head;
	unsigned int linked = 0;
	
++	printk("[+] process: %s \n", current->comm);
++	printk("[+] [debug] message [F: %s, L:%d]: caller:(%pS)\n",
++		__func__, __LINE__, (void *)__builtin_return_address(0));
}
```
	- current->comm : 현재 프로세스의 태스크 디스크립터 주소를 가리킨다.
	- %pS : 아규먼트로 지정한 주소를 심벌로 변환해 출력
	- \_\_func\_\_ : 현재 실행 중인 함수의 이름
	- \_\_\LINE\_\_ : 현재 실행 중인 코드 라인
	- __builtin_return_address: 현재 실행 중인 함수를 호출한 함수의 주소
- log (/var/log/kern.log)
```
[+] process :kworker/3:1
[+] [debug] message [F: insert_wq_barrier, L:2542]
caller[:flush_work+0x1c/0x20)
```
	- printk 로그를 실행한 프로세스 이름은 kworker/3:1이다.
	- printk가 추가된 곳은 insert_wq_barrier함수의 2542번 째 줄이다.
	- insert_wq_barrier() 함수는 flush_work() 함수가 호출했다.

### printk를 쓸 때 주의사항
- printk가 추가된 함수를 실행해야 커널 로그가 출력됨
- printk의 호출 빈도를 반드시 체크
	- 리눅스 커널에서 1초에 수백 번 이상 호출되는 함수에 printk를 사용하면 시스템이 락업(Lockup)되거나 커널 패닉으로 오동작
	- 자주 호출되더라도 확인하고 싶다면?
		- `ftrace 기능`을 사용한다.

## dump_stack()
- 호출하면 출력되는 정보
	- 콜 스택
	- 프로세스 이름
	- 프로세서의 PID
```c
#include <linux/kernel.h>

asmlinkage __visible void dump_stack(void);
```
### 실습
```c
++ static int debug_kernel_thread = 1;
long _do_fork(unsigned long clone flags, ...)
	..
	add_latent_entropy();

++	if (debug_kernel_thread)
++	{
++		printk("[+][%s] process \n", current->comm);
++		dump_stack();
++	}
```
- 사용 방법
	- `#include <linux/kernel.h>` 헤더파일을 C코드에 추가
	- 콜 스택을 보고 싶은 코드에 `dump_stack()함수` 추가
		- 콜스택 방향은 아래에서 위로
- log (/var/log/kern.log)

```
CPU: 1 PID: 149 Comm: systemd-udevd Not tainted: G	C	4.19.127-v7l+ #5
Hardware name: BCM 2835
(unwind_backtrace)
(show_stack)
(dump_stack)
(_do_fork)
(sys_clone)
```
	- 콜 스택을 시행한 프로세스의 정보, PID가 149인 systemd-udeved 프로세스가 CPU 1에서 실행 중이다.
	- 함수 호출은 아래(sys_clone)에서 위 방향으로 이루어진다.

### dump_stack()를 쓸 때 주의사항
- 1초에 100번 이상 호출되는 함수에 dump_stack() 함수를 추가하면 시스템 응답속도가 매우 느려질 수 있음
- 내부 동작
	- 현재 실행 중인 프로세스 스택 주소를 읽어서 스택에 푸시된 프레임 포인터(Frame Pointer)레지스터를 읽음(printk보다 훨씬 많은 일)
	- 함수 호출 내역을 추적
- 다음과 같은 불편함
	1. 함수 실행 빈도를 몰라 dump_stack()을 쓰기 두렵다.
	2. 커널 코드에 dump_stack()함수를 추가하고 커널 빌드를 해야 한다.

## ftrace
- ftrace는 리눅스 커널에서 제공하는 가장 강력한 트레이서
	- [Debugging the kernel using Ftrace](https://lwn.net/Articles/365835/)
	- [part2](https://lwn.net/Articles/366796/)
- 특징
	- 인터럽트, 스케줄링, 커널 타이머 등의 커널 동작을 상세히 추적
	- 함수 필터를 지정하면 지정한 함수를 호출한 함수와 전체 콜 스택까지 출력(코드 수정할 필요 없음
	- 함수를 어느 프로세스가 실행하는지 확인
	- 함수가 실행된 시각 정보 확인
	- ftrace 로그를 활성화해도 시스템 동작에 부하를 거의 주지 않음
	- 컨텍스트 정보와 CPU 번호 확인 

### 커널 컨피그
- 커널 설정 컨피그(configuration)를 활성
- 라즈비안에서는 ftrace의 기본 기능이 모두 활성화돼 있음

### ftrace 관련 디렉토리
```bash
$ cd /sys/kernel/debug/tracing
```

### ftrace 설정
### 1. tracing_on: 트레이서 활성화/비활성화하기
```bash
echo 0 > /sys/kernel/debug/tracing/tracing_on
echo 1 > /sys/kernel/debug/tracing/tracing_on 
```
	- tracing_on은 부팅 후 기본적으로 0으로 설정
### 2. current_tracer
```bash
$ echo function > /sys/kernel/debug/tracing/current_tracer
```
	- ftrace는 nop, function, function_graph 트레이서를 제공
	- nop: 기본 트레이서로써 ftrace 이벤트만 출력
	- function: 함수 트레이서로 set_ftrace_filter로 지정한 함수를 누가 호출하는지 출력
	- function_graph: 함수 실행 시간과 세부 호출 정보를 그래프 포맷으로 출력 
### 3. event
- 이벤트란 ftrace에서는 커널 서브시스템과 기능별로 세부 동작을 출력하는 기능을 지원
- ftrace 이벤트를 모두 비활성화하는 코드
```bash
echo 0 > /sys/kernel/debug/tracing/events/enable
```
- sched_wakeup, sched_switch, irq_handler_entry, irq_handler_exit 이벤트 설정
```bash
echo 1 > /sys/kernel/debug/tracing/events/sched/sched_wakeup/enable
echo 1 > /sys/kernel/debug/tracing/events/sched/sched_switch/enable
echo 1 > /sys/kernel/debug/tracing/events/irq/irq_handler_entry/enable
echo 1 > /sys/kernel/debug/tracing/events/irq/irq_handler_exit/enable
```

### 4. filter
- set_ftrace_filterㄹ 파일에 트레이싱하고 싶은 함수 이름을 지정
- set_ftrace_filter는 현재 트레이서를 function_graph과 function로 설정할 경우 작동하는 파일
- 리눅스 커널에 존재하는 모든 함수를 필터로 지정할 수는 없고 available_filter_functions 파일에 포함된 함수만 지정
- 라즈베리 파이에서 available_filter_functions 파일에 없는 함수를 set_ftrace_filter 파일에 지정하면 시스템은 락업
- set_ftrace_filter 파일에 필터로 함수를 지정하지 않으면 모든 커널 함수를 트레이싱
```bash
echo secondary_start_kernel > /sys/kernel/debug/tracing/set_ftrace_filter
echo schedule ttwu_do_wakeup > /sys/kernel/debug/tracing/set_ftrace_filter
```

### 5. func_stack_trace
- option/func_stack_trace 파일을 1로 설정하면 ftrace를 통해 콜스택 확인

### 6. sym-offset
- option/sym-offset 파일을 1로 설정하면 ftrace를 통해 심벌 오프셋 확인
- log
	- do_sys_poll+0x3d8/0x500
	- sys_poll+0x74/0x114
		- 0x114: 이 함수의 전체 크기
		- 0x74: sys_poll함수 시작 주소에서 0x74바이트 떨어진 곳에 do_sys_poll함수가 호출 됌.

### 7. 추가 설정 파일
- buffer_size_kb: ftrace 로그의 버퍼 크기이며, 킬로바이트 단위. ftrace 로그를 더 많이 저장하고 싶을 때 설정한다.
- available_filter_functions: 트레이싱할 수 있는 함수 목록을 저장한 파일. 리눅스 드라이버나 커널에 새로운 함수를 새로 구현했으면 이 파일에 새롭게 작성한 함수의 이름을 볼 수 있음. 
- event
	- ftrace에서 제공하는 이벤트의 종류를 알 수 있는 디렉토리
- sched
	- 프로세스 스케줄링 동작과 스케줄링 프로파일링을 트레이싱하는 이벤트
		- sched_switch: 컨텍스트 스위치 동작
		- sched_wakeup: 프로세스를 깨우는 동작
- irq
	- 인터럽트와 소프트웨어 인터럽트를 트레이싱하는 이벤트
		- irq_handler_entry: 인터럽트가 발생한 시각과 인터럽트 번호 및 이름을 출력
		- irq_handler_exit: 인터럽트 핸들링이 완료
		- softirq_raise: Soft IRQ 서비스 실행 요청
		- softirq_entry: Soft IRQ 서비스 실행 시작
		- softirq_exit : Soft IRQ 서비스 실행 완료
