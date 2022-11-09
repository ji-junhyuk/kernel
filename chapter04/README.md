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

### 커널 내부에서 프로세스를 생성하는 흐름
- kthreadd() 함수를 호출해 커널 프로세스 생성을 전담하는 kthreadd 스레드에게 프로세스 생성을 요청
- kthreadd_create() 함수에서 _do_fork() 함수를 호출
- 유저 프로세스와 커널 프로ㅔ스를 생성할 때 공통으로 _do_fork() 함수를 호출

### _do_fork() 함수 실행 흐름
- 1단계
	 - copy_process() 함수를 호출해서 프로세스를 생성
	 - copy_process() 함수는 부모 프로세스의 리소스를 자식 프로세스에게 복제
- 2단계
	- wake_up_new_task()함수를 호출해서 프로세스를 깨움
	- 프로세스를 깨운다는 의미는 스케줄러에게 프로세스의 실행 요청을 하는 것
```c
...
/* 부모 프로세스의 메모리 및 시스템 정보를 자식 프로세스에게 복사*/
p = copy_process(clone_flags, stack_start, stack_size,
			 child_tidptr, NULL, trace, tls, NUMA_NO_NODE);
	add_latent_entropy();

	if (IS_ERR(p))
		return PTR_ERR(p);
	/* sched_process_fork 이벤트에 대한 메시지 출력: 생성되는 프로세스의 정보 */
	trace_sched_process_fork(current, p);
	
	pid = get_task_pid(p, PIDTYPE_PID);
	nr = pid_vnr(pid);
	
	wake_up_new_task(p); /* 생성한 프로세스를 깨움 */
	put_pid(pid);
	return nr; /* 프로세스의 PID를 담고 있는 정수형 타입의 nr 지역변수를 반환 */
```
### copy_process 코드
```c
int retval;
struct task_struct *p;

/* 생성할 프로세스의 테스크 디스크립터인 task_struct 구조체와 프로세스가 실행될 스택 공간을 할당. dup_task_struct() 함수를 호출해 테스크 디스크립터를 p에 저장 */
retval = -ENOMEM;
p = dup_task_struct(current, node);
	if (!p)
		goto fork_out;

/* 테스크 디스크립터를 나타내는 task_struct 구조체에서 스케줄링 관련 정보를 초기화 */
retval = sched_fork(clone_flags, p);
	if (retval)
		goto bad_fork_cleanup_policy;

/* 프로세스의 파일 디스크립터 관련 내용(파일 디스크립터, 파일 디스크립터 테이블)을 초기화 */
retval = copy_files(clone_flags, p);
	if (retval)
		goto bad_fork_cleanup_semundo;
	retval = copy_fs(clone_flags, p);

/* 프로세스가 등록한 시그널 핸들러 정보인 sighand_struct 구조체를 생성해서 복사*/
retval = copy_sighand(clone_flags, p);
	if (retval)
		goto bad_fork_cleanup_fs;
```
### wake_up_new_task()
- 주요 동작
	- 프로세스의 상태를 TASK_RUNNING으로 변경
	- 현재 실행 중인 CPU 번호를 thread_info 구조체의 cpu 필드에 저장
	- 런큐에 프로세스를 큐잉

```c
struct rq_flags rf;
struct rq *rq;
	
raw_spin_lock_irqsave(&p->pi_lock, rf.flags);
/* 프로세스의 상태를 TASK_RUNNING으로 바꿈 */
p->state = TASK_RUNNING;

#ifdef CONFIG_SMP
/* 프로세스와 thread_info 구조체의 cpu 필드에 현재 실행 중인 CPU 번호를 저장 */
	p->recent_used_cpu = task_cpu(p);
	__set_task_cpu(p, select_task_rq(p, task_cpu(p), SD_BALANCE_FORK, 0));
#endif
/* 런큐 주소를 읽음 */
	rq = __task_rq_lock(p, &rf);
	update_rq_clock(rq);
	post_init_entity_util_avg(&p->se);

/* activate_task() 함수를 호출해 런큐에 새롭게 생성한 프로세스를 삽입 */
	activate_task(rq, p, ENQUEUE_NOCLOCK);
```

### 프로세스의 종료 흐름 파악
- 보통 유저 프로세스가 정해진 시나리오에 따라 종료해야 할 때 exit() 함수를 호출
	- exit() 시스템 콜
		- sys_exit_group
		- do_group_exit
		- do_exit
	- kill 시그널을 받아 프로세스가 소멸
		- slow_work_pending
		- do_work_pending
		- do_signal
		- do_group_exit
		- do_exit

### do_exit()
```c
void __noreturn do_exit(long code);
```
- code 인자: 프로세스 종료 코드
- 선언 키워드: _noreturn 키워드 지시자로 실행 후 자신을 호출한 함수로 되돌아가지 않음

- 주요 특징
	- do_exit() 함수에서 do_task_dead() 함수를 호출해서 schedule() 함수를 실행함으로써 함수 흐름을 마무리
	- 프로세스는 자신의 프로세스 스택 메모리 공간을 해제할 수 없음
	- schedule() 함수를 호출해 스케줄링한 후 '다음에 실행되는 프로세스'가 종료되는 프로세스의 스택 메모리 공간을 해제

### do_exit() 실행 단계
1. init 프로세스가 종료하면 강제 커널 패닉 유발: 보통 부팅 과정에서 발생
2. 이미 프로세스가 do_exit() 함수의 실행으로 프로세스가 종료되는 도중 다시 do_exit() 함수가 호출됐는지 점검
3. 프로세스 리소스(파일 디스크립터, 가상 메모리, 시그널) 등을 해제
4. 부모 프로세스에게 자신이 종료되고 있다고 알림
5. 프로세스의 실행 상태를 task_struct 구조체의 state 필드에 TASK_DEAD로 설정
6. do_task_dead() 함수를 호출해 스케줄링을 실행
```c
struct task_struct *tsk = current;
int group_dead;

/* task_struct 구조체의 flags에 PF_EXITING 플래그가 설정됐을 때 아래 실행. 프로세스가 do_Exit() 함수를 실행하는 도중에 do_exit() 함수가 호출됐을 때 예외를 처리하는 코드 */
if (unlikely(tsk->flags & PF_EXITING)) {
	pr_alert("Fixing recursive fault but reboot is needed!\n");
	tsk->flags |= PF_EXITPIDONE;
	set_current_state(TASK_UNINTERRUPTIBLE);
	schedule();
}

/* 프로세스의 task_sturct 구조체의 flags 필드를 PF_EXITING으로 바꿈 */
exit_signals(tsk); 

/* 프로세스의 메모리 디스크립터인 mm_sturct 구조체의 리소스를 해제하고 메모리 디스크립터 사용 카운트를 1만큼 감소시킴 */
exit_mm();

/* 프로세스가 사용하고 있는 파일 디스크립터 정보를 해제 */
exit_files(tsk);
exit_fs(tsk);

/* 부모 프로세스에게 현재 프로세스가 ㄷ종료 중이라는 사실을 통지 */
exit_notify(tsk, group_dead);

do_task_dead();
```

### do_task_dead()
- do_task_dead() 함수 실행 후 호출되는 함수 목록
	- __schedule() 함수
	- context_switch() 함수
	- finish_task_switch() 함수

- 위 함수들이 실행되면서 다음과 같은 과정으로 프로세스가 소멸
	- 종료할 프로세스는 do_exit() 함수에서 대부분 자신의 리소스를 커널에게 반납하고 자신의 상태를 TASK_DEAD로 바꿈
	- 컨텍스트 스위칭 수행
	- 컨텍스트 스위칭으로 다음에 실행하는 프로세스는 finish_task_switch() 함수에서 이전에 실행했던 프로세스 상태(종료할 프로세스)가 TASK_DEAD이면 프로세스 스택 공간을 해제

### __schdule()
```c
if (likely(prev != next)) {
/* context_switch()통해 컨텍스트 스위치 */
	rq = context_switch(rq, prev, next, &rf);
} else {
	rq->clock_update_flags &= ~(RQCF_ACT_SKIP|RQCF_REQ_SKIP);
	rq_unlock_irq(rq, &rf);
}
```

### context_switch()
```c
switch_to(prev, next, prev); // assembly로 구현
barrier();

return finish_task_switch(prev);
```

### finish_task_switch()
```c
/* 이전에 실행했던 프로세스 상태가 TASK_DEAD일 때 아래를 실행하는 조건문 */
if (unlikely(prev_state == TASK_DEAD)) {
		if (prev->sched_class->task_dead)
			prev->sched_class->task_dead(prev);

		kprobe_flush_task(prev);
		
/* put_task_stack() 함수를 호출해서 프로세스의 스택 메모리 공간을 해제하고 커널 메모리 공간에 반환 */
		put_task_stack(prev);
/* put_task_struct() 함수를 실행해 프로세스를 표현하는 자료구조인 task_struct가 위치한 메모리를 해제 */
		put_task_struct(prev);
	}
```

### 정리
- 프로세스가 종료할 때는 do_exit함수 호출
- do_exit에선 주요 리소스 해제
- 프로세스가 자신의 리소스를 해제한 다음 do_task_dead() 호출해서 프로세스의 태스크 디크립터와 스택 공간을 다음에 실행될 프로세스에 의해서 해제한다. 

### 태스크 디스크립터
- 프로세스의 속성 정보를 표현하는 가장 중요한 자료구조
- 프로세스의 이름과 PID와 같은 프로세스 정보 저장
- 프로세스의 관계를 알 수 있는 데이터 저장
- 프로세스가 생성되면 커널이 태스크 디스크립터를 프로세스에 부여
```c
task_struct 구조체 필드가 어떤 함수에서 변경되었는지 아는 것이 중요!
```
### 프로세스를 식별하는 필드
- char comm[TASK_COMM_LEN];
	- comm은 TASK_COMM_LEN 크기의 배열이며 프로세스 이름을 저장
- pid_t pid;
	- pid는 process ID의 약자로 프로세스마다 부여하는 정수형 값
- pid_t tgid;
	- pid와 같은 타입의 필드로 스레드 그룹 아이디를 표현하는 정수형 값
- volatile long state
	- 프로세스의 상태를 저장하며 다음 매크로 중 하나를 저장
```c
#define TASK_RUNNING		0x0000
#define TASK_INTERRUPTIBLE	0x0001
#define TASK_UNITERRUPTIBLE	0x0002
```
- unsigned int flags;
	- 프로세스의 종류와 프로세스 세부 실행 상태를 저장하는 필드
	- flags 필드는 PF_*로 시작하는 매크로 필드를 OR 연산한 결과 저장
```c
#define PF_IDLE				0x000000002
#define PF_EXITING			0x000000004
#define PF_EXITPIDONE		0x000000008
#define PF_WQ_WORKER		0x000000020
#define PF_KTHREAD			0x002000000
```
- int exit_code;
	- 프로세스의 종료 코드를 저장하는 코드
	- do_exit() 함수의 3번째 줄에서 종료 코드를 저장
```c
void __noreturn do_Exit(long code)
{
...
	tsk->exit_code = code;
}
```

### 태스크 디스크립터: 프로세스 간 관계
- sturct task_struct *real_parent;
	- 자신을 생성한 부모 프로세스의 태스크 디스크립터 주소를 저장
- struct task_struct *parent;
	- 부모 프로세스의 태스크 디스크립터 주소를 저장
- 흐름
	- sys_exit_group
	- do_group_exit
	- do_exit
	- exit_notify
	- forget_original_parent
	- find_new_reaper
- forget_original_parent() 함수와 find_new_reaper() 함수에서 새로운 부모 프로세스 지정
- sturct list_head children
	- 부모 프로세스가 자식 프로세스를 생성할 때 children 연결 리스트에 자식 프로세스를 등록
- sturct list_head sibling
	- 같은 부모 프로세스로 생성된 프로세스의 연결리스트 주소를 저장하며 형제 관계의 프로세스들이 등록된 연결리스
<사진1>
- "kthreadd" 프로세스 태스크 디스크립터의 children 필드는 연결리스트이며 연결리스트 헤드에 등록된 자식 프로세스의 task_struct 구조체의 sibling 필드 주소를 저장
- "kthreadd" 프로세스의 자식 프로세스인 "kworker/0:0H" 입장에서 ""mm_percpu_wq"와 "ksoftirqd/0" 프로세스는 자신의 sibling 연결 리스트로 이어져 있음.
- struct list_head tasks;
	- 커널에서 구동 중인 모든 프로세스는 tasks 연결 리스트에 등록됨
	- 프로세스는 처음 생성될 때 init_task 전역변수 필드인 tasks 연결 리스트에 등록
```c
static __latent_entropy struct task_struct *copy_process(
					unsigned long clone_flags,
					unsigned long stack_start,
					unsigned long stack_size,
					int __user *child_tidptr,
					struct pid *pid,
					int trace,
					unsigned long tls,
					int node)
{
	struct task_struct *p;
	p = dup_task_struct(current, node);

	/* init_task.tasks 연결 리스트의 마지막 노트에 현재 프로세스의 task_struct 구조체의 tasks 주소를 등록 */
	list_add_tail_rcu(&p->tasks, &init_task.task);
}
```
<사진2>

### 프로세스 실행 시각 정보
- u64 utime
	- 유저모드에서 프로세스가 실행한 시각
	- 이 필드는 account_user_time() 함수의 6번째 줄에서 변경
```c
void account_user_time(sturct task_struct *p, u64 cputime)
{
	int index;
	
	p->utime += cputime;
}
```
- u64 stime
	- 커널모드에서 프로세스가 실행한 시각을 저장
	- 이 필드는 account_system_index_time() 함수에서 변경
```c
void account_system_index_time(struct task_sturct *p, u64 cputime, enum cpu_usage_stat index)
{
	p->stime += cputime;
}
```
- sched_info.last_arrival 필드
	- 프로세스가 마지막에 CPU에서 실행된 시간 저장
	- 이 필드는 sched_info_arrive() 함수에서 변경
```c
static void sched_info_arrive(struct rq *rq, struct task_struct *t)
{
	unsigned long long now = rq_clock(rq), delta = 0;
	
	if (sched_info.last_queued)
		delta = new - t->sched_info.last_queued;
	sched_info_reset_dequeued(t);
	t->sched_info.run_delay += delta;
	t->sched_info.last_arrival = now;
}
```

### thread_info
- thread_info 구조체란
	- thread_info 구조체는 다음과 같은 프로세스의 핷미 실행 정보 저장
		- 선점 스케줄링 실행 여부
		- 시그널 전달 여부
		- 인터럽트 컨텍스트와 Soft IRQ 컨텍스트 상태
		- 휴먼 상태로 진입하기 직전 레지스터 세트를 로딩 및 백업
cf. CPU 아키텍처마다 조금씩 다르다 (ARM PROCESSOR 관점)
	- thread_info 구조체는 어디에 있을까?
		- 프로세스 스택의 최상단 주소에 위치
		- 프로세스마다 자신만의 스택이 있으니 프로세스마다 1개의 thread_info 구조체가 있음
- thread_info 구조체는 CPU아키텍처마다 다름
	- x86
```c
struct thread_info (
	unsigned long	flags;
	u32				status;
);
```
	- ARMv7 아키텍처의 thread_info 구조체 선언부
```c
struct thread_info {
	unsigned long flags;
	int				preempt_count;
	mm_segment_t	addr_limit;
	struct task struct *task;
	__u32				cpu;
}
```
- 태스크 디스크립터인 task_struct 구조체와 thread_info 구조체 차이점
	- 태스크 디스크립터인 task_struct 구조체는 CPU 아키텍처에 독립적인 프로세스 관리용 속성을 저장
	- 커널 버전이 같으면 x86이나 ARMv7 아키텍처에서 task_struct 구조체의 기본 필드는 같음
	- thread_info 구조체는 CPU 아키텍처에 종속적인 프로세스의 세부 속성을 저장하므로 서로 다름
- thread_info 구조체에서 관리하는 커널의 세부 동작
	- 현재 실행 중인 코드가 인터럽트 컨텍스트인지 여부
	- 현재 프로세스가 선점 가능한 조건인지 점검
	- 프로세스가 시그널을 받았는지 여부
	- 컨텍스트 스케줄링 전후로 실행했던 레지스터 세트를 저장하거나 로딩
(프로세스의 구체적인 실행정보: thread_info)
