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
	- 함수 호출 내역을 추
- 다음과 같은 불편함
	1. 함수 실행 빈도를 몰라 dump_stack()을 쓰기 두렵다.
	2. 커널 코드에 dump_stack()함수를 추가하고 커널 빌드를 해야 한다.
