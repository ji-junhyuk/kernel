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
