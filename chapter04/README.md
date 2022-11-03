* [프로세스](#프로세스)
		* [해당 단원 학습 팁](#해당-단원-학습-팁)
		* [프로세스를 배우는 지름길](#프로세스를-배우는-지름길)
		* [프로세스](#프로세스-1)
		* [태스크](#태스크)
		* [스레드](#스레드)
		* [ps명령어로 프로세스 목록 확인하기](#ps명령어로-프로세스-목록-확인하기)
		* [PID](#pid)
		* [프로세의 종류 별로 프로세스가 생성되는 방식](#프로세의-종류-별로-프로세스가-생성되는-방식)
		* [프로세스는 누가 생성할까?](#프로세스는-누가-생성할까)
		* [프로세스 생성:_do_fork() 함수](#프로세스-생성_do_fork-함수)
		* [유저레벨에서 프로세스를 생성할 때 흐름](#유저레벨에서-프로세스를-생성할-때-흐름)
		* [커널 프로세스의 생성 과정](#커널-프로세스의-생성-과정)
		* [정리](#정리)
# 프로세스
### 해당 단원 학습 팁
1. 너무 정독하지 말기(전체를 보고 나면 더 확실히 이해)
2. 여러번 자주 읽기
3. 실습 꼭 하기
4. 실습을 프로젝트에 활용하기

### 프로세스를 배우는 지름길
1. 직접 간단한 커널 스레드 작성해보기
2. 커널 스레드 함수 개선해보기
3. `ftrace를 활용해 프로세스 디버깅을 자주 해보기`

### 프로세스
1. 프로세스간 무엇인가
- 프로세스란 `리눅스 시스템 메모리에서 실행 중인 프로그램`
```
프로세스가 실행을 대기한다면 실행할 때 어떤 과정을 거칠까?
프로세스는 어떤 구조체로 식별할까?
```
- 프로세스를 관리하는 자료구조이자 객체를 `태스크 디스크립터(task descriptor)`라고 부르고, `task_struct` 구조체로 표현된다.
	- 이 구조체에 프로세스가 쓰는 `메모리 리소스, 프로세스 이름, 실행 시각, 프로세스 아이디(pid), 프로세스 스택의 최상단 주소와 같은 속성 정보`가 저장돼 있다.
```
프로세스의 실행 흐름은 어느 구조체에 저장할 수 있을까?
```
- 프로세스의 실행 흐름을 표현하는 또 한가지 중요한 공간은 `프로세스 스택` 공간이다. 이 `프로세스 스택의 최상단 주소에thread_info 구조체`가 있다.

```
-000:__schedule()
-001:schedule_timeout()
-002:do_sigtimedwait()
-003:sys_rt_sigtimewait()
-004:ret_fast_syscall()
```
	- 함수 호출 방향은 004번째 줄에서 000번째 줄 방향
	- 유저 공간에서 sigtimewait() 함수를 호출하면 이에 대응하는 시스템 콜 핸들러 함수인 sys_rt_sigtimewait()
	- 000번째 줄의 __schedule()함수가 호출 돼 스케줄링된다.
- 프로세스는 함수를 호출하면서 함수를 실행한다. 함수를 호출하고 실행할 때 어떤 리소스를 쓸까? 바로, 프로세스 스택의 메모리 공간이다. `모든 프로세스들은 커널 공간에서 실행될 때 각자 스택 공간을 할당받으며 스택 공간 내에서 함수를 실행한다.`

### 태스크
- 기존 임베디드 RTOS 기반 시스템에서 특정 코드나 프로그램 실행을 일괄 처리했는데 이런 `실행 및 작업 단위를 테스크`라고 부름. (기존 임베디드개발자들이 리눅스를 사용하는 임베디드 프로젝트에 유입)
- 다음 함수는 모두 프로세스를 관리 및 제어하는 역할을 수행하며, 함수 이름에 보이는 `task는 process라고 바꿔도 무방`
	- dump_task_regs(), get_task_mm(), get_task_pid(), idle_task()

### 스레드
- 스레드는 `유저 레벨에서 생성된 가벼운 프로세스`
- 스레드는 자신이 속한 프로세스 내의 다른 스레드와 `파일 디스크립터. 파일 및 시그널 정보에 대한 주소 공간을 공유`
- 커널 입장에서는 스레드를 다른 프로세스와 동등하게 관리. 대신 각 프로세스 식별자인 태스크 디스크립터(task_struct)에서 스레드 그룹 여부를 점검

```TRACE32
v.v %all (struct task_struct*)0xf161800000

```
- task_struct에서 stack필드는 프로세스의 스택 최상단 주소를 의미한다.
```Trace32
v.v %all (struct thread_info*)0xF1604000
r											: 레지스터 정보 보기
v.f											: 콜 스택 보기
```
- `thread_info는 아키텍쳐에 의존적인 프로세스의 동작`이다.
- `프로세스의 실행 흐름을 나타내는 주요 레지스터`
	- 스택 포인터 레지스터와(sp)과 프로그램 카운터 레지스터(pc)
	- 두 개의 레지스터(sp, pc)를 암코어 레지스터에 로딩하기
```Trace32
r.s	sp 0xF1605E40
r.s pc 0XC0FF4658
```
- log
	- freezable_schedule ...
	- sys_epoll ...
	- ref_fast_syscall
- `thread_info 구조체 내`에 `cpu_context_field는 레지스터 셋트 정보`를 담고 있다. `레지스터 셋트는 프로그램 실행 정보를 저장하는 중요한 자료구조`이다.

### ps명령어로 프로세스 목록 확인하기
```
ps 명령을 입력하면 리눅스 커널 내부의 어떤 자료구조에 접근해서 정보를 출력할까?
```
- init_task 전역변수를 통해 전체 프로세스 목록을 출력한다.
	- 모든 프로세스(유저 레벨, 커널 스레드)는 init 프로세스를 표현하는 자료구조인 init_task 전역변수의 task 필드에 연결리스트로 등록되어 있다. 이 연결리스트를 순회하면서 프로세스 정보인 task_struct 구조체의 주소를 계산해 프로세스 정보를 출력한다. 
- 프로세스 목록 확인
```bash
ps -ely
e: see every process
l: long format
y: do not show flags
```
	- pid가 2인 "kthreadd"는 커널 공간에서만 실행하는 커널 프로세스를 생성하는 임무를 수행한다.
- 프로세스 목록은 부모 자식 프로세스 관계를 토대로 프로세를 출력
```bash
ps -ejH
```
	- pid가 1인 "systemed"는 모든 프로세스들을 관리하는 init 시스템 및 프로세스를 말한다.
	- 조부모, 부모, 자식프로세스가 있다고 가정할 때, 예외 상황으로 부모 프로세스가 종료되면 자식 프로세스 입장에서는 부모 프로세스가 사라진다. 이때 조부모가 부모 프로세스가 되며, 대부분 `init porocess`가 조부모 역할(새로운 부모 프로세스)를 수행한다.

### PID
- 리눅스 커널은 프로세스가 생성될 때 intgud ID인 PID를 프로세스에게 부여
- PID를 증가시키면서 프로세스에게 부여
- 리눅스에서 공통으로 커널이 생성하는 프로세스가 있는데 각각 다음과 같은 PID를 부여
	- swapper process: 0
	- init process: 1
	- kthreadd: 2
- 리눅스 시스템 프로그래밍을 할 때 getpid()함수를 호출하면 프로세스의 PID를 읽어올 수 있음
	- 유저 공간에서 getpid() 함수를 호출하면 이에 대응하는 시스템 콜 핸들러인 sys_getpid() 함수가 호출
```c
SYSCALL_DEFINE0(getpid)
{
	return task_tgid_vnr(current);
}
```

### 프로세스의 종류 별로 프로세스가 생성되는 방식
- 유저 프로세스
	- 유저 공간에서 프로세스를 생성하는 라이브러리의 도움을 받아 커널에게 프로세스 생성 요청
	- 유저 모드에서 fork()함수나 pthread_create() 함수를 호출
	- glibc 리눅스 라이브러리 파일의 도움으로 커널에 서비스를 요청해 생성
- 커널 프로세스
	- 커널 내부의 kthreadd_create() 함수를 직접 호출해서 커널 프로세스를 생성
	- 커널 프로세스를 생성하면 프로세스의 세부 동작을 스레드 핸들러에 구현해야 함.
- 프로세스 종류에 따라 프로세스를 생성하는 흐름은 다르나 공통으로 _do_fork() 함수가 호출됨.

### 프로세스는 누가 생성할까?
- init process: 주로 부팅 과정에서 유저 프로세스를 생성
- ktrheadd process: 커널 레벨 프로세스(커널 스레드)를 생성

### 프로세스 생성:_do_fork() 함수
- _do_fork()
```c
long _do_fork(unsigned long clone_flags,
			unsigned long stack_start,
			unsigned long stack_size,
			int __user *parent_tidptr,
			int __user *child_tidptr,
			unsigned long tls)
```
- 반환
	- 반환값 타입은 long인데 process의 pid를 반환
	- 프로세스 생성 시 에러가 발생하면 pid대신 PTR_ERR() 매크로로 지정된 에러 값을 반환
- 인자
	- unsigned long clone_flags: 프로세스를 생성할 때 지정하는 옵션 정보를 저장하며 다음 매크로를 OR 연산한 결과를 저장
```c
#define CSIGNAL		0x000000ff
#define CLONE_VM	0x00001000
...
#define CLONE_THREAD 0x00010000
```
	- unsigned long stack_start: 보통 유저 영역에서 스레드를 생성할 때 복사하려는 스택의 주소
	- unsigned long stack_size: 유저 영역에서 실행 중인 스택 크기
	- int __user *parent_tidptr/int __user *child_tidptr: 부모와 자식 스레드 그룹을 관리하는 핸들러 정보

### 유저레벨에서 프로세스를 생성할 때 흐름
![1](https://user-images.githubusercontent.com/67992469/197530385-3c5d2d76-1bff-418d-89e9-75b823574856.png)
- fork()
	- sys_clone(커널 영역)
	- _do_fork() (커널 영역)
	- copy_process
- 리눅스 커널 계층에서는 fork() 함수에 대응하는 시스템 콜 핸들러인 sys_clone() 함수를 호출
- sys_clone()
```c
#ifdef __ARCH_WAANT_SYS_CLONE
#ifdef CONFIG_CLONE_BACKWARDS
SYSCALL_DEFINE5(clone, unsigned long, clone_flags, unsigned long, newsp, 
		int__user *, parent_tidptr,
		unsigned long, tls,
		int__user *, child_tidptr,)
...
#endif
{
	return _do_fork(clone_flags, newsp, 0, parent_tidptr, child_tidptr, tls);
}
#endif
```

### 커널 프로세스의 생성 과정
- 1단계: kthreadd 프로세스에게 커널 프로세스 생성을 요청
- 2단계: ktrheadd 프로세스는 깨어나 프로세스를 생성해달라는 요청이 있으면 프로세스를 생성
![2](https://user-images.githubusercontent.com/67992469/197530423-57c2033b-9b14-4161-99d7-21e88332b5e8.png)
```c
long vhost_dev_set_owner(struct vhost_dev *dev)
{
	struct task_struct *worker;
	int err;
	
	dev->mm = get_task_mm(current);
	worker = kthread_create(vhost_worker, dev, "vhost-%d", current->pid);
}
```
- kthread_create() 함수를 호출해 커널 스레드를 생성하는 kthreadd 프로세스에게 커널 스레드 생성 요청
- kthreadd 스레드는 _do_fork() 함수를 실행해서 프로세스를 생성

### 정리
- Q0 : 커널 프로세스는 언제 실행될까?
	- kthread_create() 함수를 호출해야 하며, 함수의 인자로 '커널 스레드 핸들러 함수', '매개변수', '커널 스레드 이름'을 지정.
- Q1 : 커널 프로세스는 누가 실행을 시킬까?
	- 커널 프로세스의 세부 동작 방식은 커널 스레드 핸들러에 구현됨
	- 커널 프로세스 자신이 스스로 휴면 상태에 빠지거나 다시 깨어나 실행함
	- 주로 배경 작업으로 주기적으로 실행됨
	- 특정 커널 기능(서브 시스템)에서는 기능의 시나리오에 따라 커널 프로세스가 일을 시작(ex: 커널에서 메모리가 부족하면 kwapd 프로세스가 페이지를 확보하는 동작 수행)
- 커널 스레드
	- 시스템 부팅 과정에서 대부분의 커널 스레드를 생성한다. 커널 스레드는 생성된 후 바로 일을 시작하며, 이후 배경 작업으로 주기적으로 실행.(ex: 워커스레드)
	- 리눅스 드라이버에서 많은 워크를 워크큐에 큐잉하면 커널은 커널 스레드의 종류인 워커 스레드를 더 생성한다.
	- 커널에서 메모리가 부족하면 페이지를 확보하는 일을 하는 kswapd 스레드를 깨워 실행한다.

###  fork() 함수 유저 프로세스
- 유저 공간에서 리눅스 시스템 저수준 함수로 fork() 함수를 호출하면 fork 시스템 콜이 발생해 커널 모드로 실행 흐름이 변경
- 커널 모드에서 시스템 콜 번호에 해당하는 시스템 콜 핸들러인 sys_clone() 함수가 호출
	- sched_wakeup
	- sched_switch
		- raspbian_proc 프로세스가 실행했다가 스케줄링으로 휴먼 상태로 진입
		- 실행 시간을 보면 3초 간격으로 휴면 상태로 진입했다 실행 반복(sleep(3)을 호출했기 때문)
- kill 명령어는 프로세스를 종료시키는 시그널을 전달하는 명령어
	- 프로세스는 자신이 종료할 것이라는 사실을 부모프로세스에게 시그널로 통지
	- 17은 SIGCHLD


#### 프로세스 종료 로그
- do_exit()
- do_group_exit
- _get_signal
- do_signal
- do_work_pending
- slow_work_pending

####  프로세스 생성 단계의 함수 흐름
- copy_process
- _do_fork
- sys_clone
- ret_fast_syscall
	- 유저 공간에서 fork()함수를 호출하면, system call에 대응대는 handler(sys_clone)가 호출이 된다. ret_fast_syscall 레이블은 handler가 복귀할 곳이다.

####  프로세스 종료 단계의 함수 흐름
- do_exit
- do_group_exit
- get_signal
- do_signal
- do_work_pending
- slow_work_pending

### exit() 함수로 프로세스가 종료: 로그 분석
- 종료
	- do_exit
	- do_group_exit
	- _wake_up_parent+0x0/0x30 // sys_exit_group()가 호출됌. (wake_up_parent 심볼에 오프셋을 빼면 계산, 실제 코드에서 호출되는 주소보다 오프셋을 출력하는 형식이여서 그렇다.)
	- ret_fast_syscall
- rpi_proc_exit
	- rpi_proc_exit프로세는 종료 과정에서 pid가 2181인 부모 bash 프로세스에게 SIGCHLD 시그널을 전달.
- signal_deliver : 시그널을 전달받음
- sig=17:전달 받은 시그널
- sa_handler=55a6c: 시그너 핸들러 함수 주소(유저 공간)
 
### 정리
- 프로세스가 스스로 exit POSIX 시스템 콜을 호출하면 스스로 종료될 수 있음
- exit POSIX 시스템 콜에 대한 시스템 콜 핸들러는 sys_exit_group()함수
- 프로세스는 소멸되는 과정에서 부모 프로세스에게 SIGCHLD 시그널을 전달해 자신이 종료될 것이라고 통지

## 커널 스레드란
### 커널 스레드의 정의
- 커널 프로세스는 커널 공간에서만 실행되는 프로세스
- 백그라운드 작업으로 실행되면서 시스템 메모리나 전원을 제어하는 동작을 수행

### 커널 스레드의 특징
- 커널 스레드는 커널 공간에서만 실행되며, 유저 공간과 상호작용 하지 않음
- 커널 스레드는 실행, 휴먼 등 모든 동작을 커널에서 직접 제어 관리
- 대부분의 커널 스레드는 시스템이 부팅할 때 생성되고 시스템이 종료할 때까지 백그라운드로 실행

### 커널 스레드의 종류
- kthreadd 프로세스 (커널 스레드 데몬 프로세스)
	- kthreadd  프로세스는 모든 커널 스레드의 부모 프로세스
	- 스레드 핸들러 함수는 kthreadd()이며, 커널 스레드를 생성하는 역할을 수행.
- 워커 스레드
	- 워크큐에 큐잉된 워크(work)를 실행하는 프로세스
	- 스레드 핸들러 함수는 worker_thread()이며, process_one_work() 함수를 호출해 워크를 실행
- ksoftirqd 프로세스
	- soft IRQ를 위해 실행되는 프로세스
	- ksoftirqd 스레드의 핸들러는 run_ksoftirqd() 함수로서 Soft IRQ 서비스를 실행
	- soft IRQ 서비스를 처리하는 _do_softirq() 함수에서 ksoftirqd를 깨움
- IRQ 스레드(threaded IRQ)
	- 인터럽트 후반부 처리를 위해 쓰이는 프로세스

### 커널 스레드는 어떻게 생성될까?
- 1단계: kthreadd 프로세스에게 커널 스레드 생성을 요청
	- kthread_create() 함수
	- kthread_cretae_on_node() 함수
	- __kthread_create_on_node() 함수
- 2단계: kthreadd 프로세스가 커널 스레드를 생성
	- kthreadd() 함수
	- create_ktrhead() 함수

### kthread_create()
```c
#define kthread_create(kthreadfn, data, namefmt, arg...) \
	kthread_create_on_node(threadfn, data, NUMA_NO_NODE, namefmt, ## arg)
	
struct task_struct *kthread_create_on_node(int (*threadfn)(void *data), void *data, int node, const char namefmt[], ...)
```
- 함수인자
	- int (*threadfn)(void *data): 스레드 핸들러 함수 주소를 저장하는 필드
	- void *data: 스레드 핸들러 함수로 전달하는 매개변수
	- int node: 노드 정보
	- const char namefmt[]: 커널 스레드 이름을 저장
- 코드 분석
	 - 커널 컴파일 과정에서 전처리기는 kthread_create() 함수를 ktrhead_create_on_node() 함수로 교체
	 - 커널이나 드라이버 코드에서 kthread_Create_on_node() 함수를 호출하면 실제로 동작하는 코드는 kthread_create_on_node() 함수
	 - kthread_create_on_node() 함수 내부에서 __kthread_create_on_node() 함수 호출
- kthread_create_on_node()
```c
struct task_struct *kthread_create_on_node(int (*threadfn)(void *data), void *data, int node, const char namefmt[], ...)
{
 	struct task_struct *task;
	va_list args;
	
	va_start(args, namefmt);
	task = __kthread_create_on_node(threadfn, data, node, namefmt, args);
	va_end(args);
	
	return task;
}
```
- __kthread_cretae_on_node()
```c
struct task_struct *__kthread_create_on_node(int (*threadfn)(void *data), void *data, int node, const char namefmt[], va_list args)
{
	DECLARE_COMPLITION_ONSTACK(done);
	struct task_struct *task;
	sturct kthread_create_info *create = kmalloc(sizeof(*create), GFP_KERNEL);
	
	if (!create)
	return ERR_PTR(-ENOMEM);
	create->threadfn = threadfn;
	create->data = data;
	create->node = node;
	create->done = &done;
	
	spin_lock(&kthread_create_lock);
	list_add_tail(&create->list, &kthread_create_list);
	spin_unlock(&kthread_create_lock);
	
	wake_up_process(kthreadd_task);
}

- 커널 스레드 생성하는 예제 코드
```c
long vhost_dev_set_owner(struct vhost_dev *dev)
{
	struct task struct *worker;
	int err;
	
	dev->mm - get_task_mm(current);
	worker - kthread_create(vhost_worker, dev, "vhost-%d", current->pid);
}
```
- kthread_create() 함수의 첫 번째 인자로 vhost_worker로 스레드 핸들러 함수의 이름을 지정
- vhost_dev 구조체의 주소를 2번째 인자로 전달
- 3번째 인자로 "vhost-%d"를 전달하며 커널 스레드의 이름을 나타냄.

### 커널 스레드 생성:2단계
- 1단계에서 커널 스레드를 생성해 달라고 kthreadd 프로세스에게 요청한 후 kthreadd 프로세스를 깨움
- kthreadd 프로세스를 깨우면 kthreadd 프로세스의 스레드 핸들러인 kthreadd()함수가 호출되어 요청한 프로세스를 생

### kthreadd()함수의 핵심 기능
- kthread_create_info 연결 리스트를 확인해 프로세스 생성 요청을 확인
- create_kthread() 함수를 호출해 프로세스를 생성
- (커널 스레드 핸들러 함수, 커널 스레드 데몬 프로세스의 실제 세부 동작 구현)
```c
for (;;) {
	set_current_sate(TASK_INTERRUPTIBLE);
	if (list_empty(&kthreadd_create_list))
		schedule(); /* schedule() 함수를 호출해 스스로 휴면 상태로 진입 */
	__set_current_state(TASK_RUNNING);
	
	spin_lock(&kthread_create_lock);
	/* kthread_create_list 연결 리스트가 비어있지 않으면 21~32번째 줄을 실행해 커널 스레드를 생성 */
	while (!list_empty(&kthreadd_create_list)) {
		sturct kthread_create_info *create;
	/* kthread_create_list.next 필트를 통해 kthread_create_info 구조체의 주소를 읽음 */
	create = list_entry(kthreadd_create_list.next, struct kthread_create_info, list);
	list_del_init(*create->list);
	spin_unlock(&kthread_create_lock);
	/* create_kthread() 함수를 호출해서 커널 스레드를 생성 */
	create_kthread(create);
	}
	spin_unlock(&kthread_create_lock);
}
```

### kthreadd() 함수에서 커널 스레드를 생성할 때 자료구조
- kthread_create_list 연결 리스트와 kthread_create_info 구조체의 관계
	- kthread_create_list 전역변수의 next 필드가 kthread_create_info 구조체의 list 필드 주소를 가리킴
	- kthread_create_list라는 전역 변수의 next 필드는 화살표로 kthread_create_info 구조체의 list 필드가 위치한 주소를 가리킴

### create_kthread()
```c
static void create_kthread(struct kthread_create_info *create)
{
	int pid;
	
#ifndef CONFIC_NUMA
	current->pref_node_fork = create->node;
#endif
	pid = kernel_thread(kthread, create, CLONE_FS | CLONE_FILES | STGCHLD);
}
```

### kernel_thread()
```c
pid_t kernel_thread(int (*fn)(void *), void *arg, unsigned long flags)
{
	return _do_fork(flags|CLONE_VM | CLONE_UNTRACED, (unsigned long)fn, (unsigned long)arg, NULL, NULL, 0);
}
```

### 커널 스레드를 생성하는 과정 정리
- Q. 커널 스레드를 생성하려면 어떤 함수를 호출해야 할까?
	- A: ktrhead_create() 함수를 호출해야 하며 함수의 인자로 '스레드 핸들러 함수', '매개변수', '커널 스레드 이름'을 지정
- Q. 커널 스레드를 생성하는 단계는 무엇일까?
	- A: kthreadd 프로세스에게 커널 스레드 생성을 요청한 후 kthreadd 프로세스를 깨움. kthreadd 프로세스는 깨어나 자신에게 커널 스레드 생성 요청이 있었는지 확인한 후 만약 생성 요청이 있다면 커널 스레드를 생성.


