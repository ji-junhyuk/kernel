# 6장. 인터럽트 후반부 처리
- 인터럽트 후반부 기법은 왜 적용하고 무엇이 있는가?

### 인터럽트 후반부 기법을 적용하는 이유
- 디바이스 드라이버에서는 다음과 같은 요구 사항이 생김
	- 인터럽트가 발생한 후 처리해야 할 동작이 많을 때가 있다.
	- 인터럽트 핸들러는 빨리 실행돼야 한다.
- 대안으로 인터럽트 발생했을 때 인터럽트를 처리할 코드를 2단계로 분리
	- 빨리 실행할 코드: 인터럽트 핸들러 및 인터럽트 컨텍스트에서 실행
	- 실시간으로 빨리 실행되지 않아도 되는 코드: 인터럽트 후반부 기법 적용

### 인터럽트 후반부 처리 기법
- IRQ 스레드
	- 인터럽트 처리를 전용 IRQ 스레드에서 후속 처리 수행.
- Soft IRQ
	- 인터럽트 핸들러 실행이 끝나면 바로 일을 시작.
	- 후반부 처리를 Soft IRQ 컨텍스트에서 실행
	- Soft IRQ 서비스 핸들러 실행 도중 시간이 오래 걸리면 ksoftirqd 프로세스를 깨우고 Soft IRQ 서비스를 종료한다. ksoftirqd라는 프로세스에서 나머지 인터럽트 후반부를 처리하는 구조이다.
- 태스크릿
	- Soft IRQ 서비스를 동적으로 쓸 수 있는 인터페이스이자 자료구조이다.
- 워크 큐
	- 인터럽트 핸들러가 실행될 때 워크를 워크 큐에 큐잉하고 프로세스 레벨의 워커 스레드에서 인터럽트 후반부 처리를 하는 방식이다.

### 인터럽트 컨텍스트에서 많은 일을 하면 어떻게 될까?
- 인터럽트 핸들러에서 실행시간이 길다면
	- 오동작
	- 커널 크래시
- 실행 시간이 오래 걸리는 코드
	- I/O시작
	- 과도한 while 반복문
	- 유저 공간으로 uevent를 전달해서 인터럽트 발생 알림
	- 스케줄링 지원하는 커널 함수 호출
	
###  Top Half / Bottom Halfo
- Top Half
	- 인터럽트가 발생한 후 빨리 처리해야 하는 일
	- 인터럽트 핸들러에서 최소한의 연산 수행
- Bottom Half
	- 인터럽트 처리를 프로세스 레벨에서 수행하는 방식
	- 인터럽트 후반부 기법으로 처리
	- Top Half 부분의 인터럽트 핸들러는 최소한의 일을 수행하고 약간 지연되어 처리되어도 상관 없는 부분은 Bottom Half에서 처리
- 리눅스 커널에서 지원하는 인터럽트 후반부(Bottom Half)기법
	- IRQ 스레드, Soft IRQ, 태스크릿, 워크 큐

### 어떤 인터럽트 후반부 처리 기법을 적용해야 할까?
- 1. 인터럽트가 1초에 수십 번 발생하는 디바이스의 경우 어떤 인터럽트 후반부 기법을 적용해야 할까?
	- 인터럽트가 자주 발생할 때 인터럽트 후반부 처리 기법으로 Soft IRQ나 태스크릿을 적용하는 것이 바람직
	- IRQ 스레드 방식과 워크큐 방식은 그리 적합하지 않음.
- 2. 현재 개발 중인 시스템은 인터럽트 개수가 200개 정도. 어떤 방식을 적용하면 좋을까요?
	- 1초에 인터럽트가 수백 번 이상 발생하는 경우를 제외하곤 인터럽트 후반부 기법으로 IRQ 스레드 방식을 적용
	- 인터럽트 개수만큼 IRQ 스레드를 생성하면 IRQ 스레드를 구동하기 위해 필요한 메모리 공간(스택/태스크 디스크립터)을 사용한다는 점을 고려해야 함.

### IRQ와 IRQ 스레드(threaded IRQ)란?
- IRQ
	- Interrupt Request의 약자로 하드웨어에서 발생한 인터럽트를 처리한다는 의미
	- 인터럽트가 발생한 후 인터럽트 핸들러까지 처리되는 흐름을 의미
 
- IRQ 스레드란
	- 인터럽트 핸들러에서는 바로 처리하지 않아도 되는 일을 수행하는 프로세스
	- 인터럽트 후반부 처리를 위한 인터럽트 처리 전용 프로세스
	- IRQ 스레드 기법은 인터럽트 후반부 처리를 IRQ 스레드에서 수행하는 방식
	- 리눅스 커널 커뮤니티에서는 irq_thread 혹은 threaded IRQ 방식으로 부름

1. 함수 흐름 파악
2. 관련된 서브 시스템을 제어하는 핵심 자료 구조 파악
3. 관련된 ftrace이벤트가 있는지, 없으면 적절한 ftrace를 직접 생성하거나 trace, printk함수 사용해서 동작 출력하는 식으로 파악

### IRQ 스레드 확인
- 라즈베리 파이에서 IRQ 확인
	- ps -ely
	- irq/86-mmc1 프로세스는 "mmc1"이라는 이름의 86번 인터럽트를 처리하는 IRQ 스레드로 해석 가능


### IRQ 스레드는 언제 생성할까?
- 디바이스 드라이버의 초기화 코드(부팅 과정)에서 request_threaded_irq() 함수를 호출해 IRQ 스레드를 생성
- IRQ 스레드는 생성된 후 시스템 전원이 공급돼 동작하는 동안 해당 인터럽트 후반부를 처리하는 기능을 수행
- IRQ 스레드 처리 함수
	- 인터럽트 별로 지정한 IRQ 스레드별로 후반부를 처리하는 함수를 의미
- IRQ 스레드 핸들러 함수
	- irq_thread() 함수를 뜻하며 인터럽트별로 지정된 IRQ 스레드 처리 함수를 호출하는 역할을 수행
	- 각 인터럽트별로 지정한 IRQ 스레드가 깨어나면 공통으로 IRQ 스레드 핸들러 함수인 irq_thread()가 호출됨
	- 3개의 IRQ 스레드가 있으면 3개의 IRQ 스레드의 핸들러 함수는 모두 irq_thread()임

- request_threaded_irq() 함수의 역할
	- 전달한 IRQ 스레드 정보를 해당 인터럽트 디스크립터에 설정
	- ktrehad_create() 함수를 호출해서 IRQ tㅡ레드를 생성
- 매개변수
	- irq, unsigned int, 인터럽트 번호
	- handler, irq_handler_t, 인터럽트 핸들러의 주소
	- thread_fn, irq_hander_t, IRQ 스레드 처리 함수의 주소
	- flags, unsigned long, 인터럽트 핸들링 플래그
	- name, const char, 인터럽트 이름
- __setup_irq() 함수
	- IRQ 스레드 처리 함수가 등록됐는지 점검
	- 만약 IRQ 스레드가 등록됐으면 setup_irq_thread() 함수를 호출해 IRQ 스레드를 생성
- setup_irq_thread() 함수 10번째 줄
	- ktrhead_create()함수 호출하여 커널 스레드 생성
	- kthread_create()함수 호출 시 지정하는 인자
		- irq_thread: IRQ 스레드 핸들러 함수
		- new: IRQ 스레드 핸들러인 irq_thread() 함수로 전달되는 매개변수(irqaction)
		- irq: 인터럽트 번호
		- new->name: 인터럽트 이름
- IRQ 스레드 처리 함수
	- 인터럽트별로 지정한 IRQ 스레드별로 후반부를 처리하는 함수를 의미.
- IRQ 스레드 핸들러 함수
	- irq_thread() 함수를 뜻하며 인터럽트별로 지정된 IRQ 스레드 처리 함수를 호출하는 역할을 수행
	- 각 인터럽트별로 지정한 IRQ 스레드가 깨어나면 공통으로 IRQ 스레드 핸들러 함수인 irq_thread()가 호출됨
	- 3개의 IRQ 스레드가 있으면 3개의 IRQ 스레드의 핸들러 함수는 모두 irq_thread()임
	
### irq_thread()에 전달되는 매개변수
- irq 스레드가 실행될 때 irq_thread() 함수가 실행되는데 함수의 인자로 void 타입의 data 포인터를 전달
- irq_thread() 함수의 4번째 줄을 보면, 이 포인터를 sturct irqaction * 타입으로 캐스팅

### IRQ 스레드 생성 실습

### IRQ 스레드의 전체 실행 흐름 파악
- 1. 인터럽트 핸들링
	- 인터럽트가 발생한 후 인터럽트 벡터인 __irq_svc 레이블로부터 커널 내부 함수의 실행 흐름
- 2. 인터럽트 핸들러
	- 인터럽트 핸들러에서 IRQ_WAKE_THREAD를 반환
	- __irq_wake_thread: IRQ 스레드를 깨우는 동작
- 3. IRQ 스레드 
	- IRQ 스레드의 스레드 핸들러인 irq_thread() 함수가 호출되고, 이어서 IRQ 스레드 처리 함수가 호출
- IRQ 스레드 실행의 출발점은?
	- 인터럽트 핸들러에서 IRQ_WAKE_THREAD를 반환하는 시점
- IRQ 스레드는 어느 시점에 깨울까?
	- 인터럽트 핸들러에서 IRQ_WAKE_THREAD를 반환하면 해당 IRQ 스레드를 깨움.
- 정확히 IRQ 스레드는 언제 실행을 시작할까?
	- IRQ 스레드를 깨우면 스케줄러는 우선순위를 고려한 후 IRQ 스레드를 실행
	- 이후 IRQ 스레드의 핸들러인 irq_thread() 함수가 호출됨

### IRQ 스레드 핸들러 관련 함수
- irq_thread() 함수
- irq_thread_fn()
	- irqaction 구조체의 thread_fn 필드로 IRQ 스레드 처리 함수를 호출
	- thread_fn 필드는 함수 포인터와 비슷한 동작을 수행

### Soft IRQ
- Soft IRQ는 리눅스 커널을 이루는 핵심 기능 중 하나
- Soft IRQ 서비스의 형태로 커널의 타이머, 스케줄링은 물론 네트워크 시스템이 동작
- Soft IRQ는 인터럽트 후반부 기법으로 사용됨

### SOft IRQ 서비스
- 리눅스 커널에서는 10가지 Soft_IRQ 서비스를 지원
- Soft IRQ 서비스는 부팅할 때 open_softirq()라는 함수를 써서 등록
```c
const char *const softirq_to_name[NR_SOFTIRQS] = {
"HI", "TIMER" ...
```
- 라이프 사이클
	- 1단계: 부팅 과정
		- 부팅 과정에서 open_softirq() 함수를 호출해 Soft IRQ 서비스 등록
	- 2단계: 인터럽트 처리
		- 인터럽트 핸들러(인터럽트 컨텍스트)나 인터럽트 핸드럴 내에서 호출한 서브 함수에서 raise_softirq() 함수를 호출해 Soft IRQ 서비스를 요청
	- 3단계: Soft IRQ 컨텍스트
		- _do_softirq() 함수에서 이미 요청한 Soft IRQ 서비스를 실행
- Soft IRQ 서비스 핸들러
	- Soft IRQ 서비스 핸들러는 Soft IRQ 서비스를 실행할 때 호출되는 함수
	- 부팅 과정에서 open_softirq() 함수를 호출해 softirq_vec이라는 전역변수에 등록

### Soft IRQ 전체 실행 흐름
- 1단계
	- 인터럽트가 발생하면 해당 인터럽트 핸들러에서 Soft IRQ 서비스 요청
	- raise_softirq_irqoff()함수를 호출
- 2단계
	- 인터럽트 서비스 루틴 동작이 끝나면 irq_exit()함수를 호출
	- Soft IRQ 서비스 요청 여부를 점검해 요청한 Soft IRQ 서비스가 있으면 __do_softirq() 함수를 호출해서 해당 Soft IRQ 서비스 핸들러를 실행
	- Soft IRQ 서비스 요청이 없으면 irq_exit() 함수는 바로 종료
- 3단계
	- __do_softirq() 함수에서 Soft IRQ 서비스 핸들러를 호출
	- Soft IRQ 서비스 핸들러 처리 시간이 2ms이상이거나 10번 이상 Soft IRQ 서비스 핸들러를 처리했다면 다음 동작을 수행
- 4단계
	- ksoftirqd 프로세스가 깨어나 3단계에서 마무리하지 못한 Soft IRQ 서비스 핸들러를 실행
		- wakeup_softirqd() 함수를 호출해서 ksoftirqd 프로세스를 깨움
		- __do_softirq() 함수 종료
	
### 후반부 기법으로 Soft IRQ를 언제 쓸까?
- Soft IRQ 기법은 인터럽트 발생 빈도가 높거나 인터럽트 후반부를 빨리 처리해야 할 때 사용
- 인터럽트 핸들러 호출 이후 바로 Soft IRQ 서비스를 실행하기 때문
- 인터럽트 발생 빈도가 높거나 인터럽트 후반부를 빨리 처리해야 하는 디바이스 드라이버에서는 Soft IRQ 인터페이스인 태스크릿을 사용하면 됨

### Soft IRQ 서비스
- softirq_to_name 전역변수
- 열거형으로 정의

### Soft IRQ 서비스 타입

### Soft IRQ 서비스 핸들러는 언제 등록할까?
- 1단계: Soft IRQ 서비스 등록
	- 부팅 과정에서 open_softirq() 함수를 호출해 Soft IRQ서비스를 등록
- 2단계: Soft IRQ 서비스 요청
	- 인터럽트 컨텍스트에서 Soft IRQ 서비스를 요청
- 3단계: Soft IRQ 서비스 실행
	- 요청한 Soft IRQ 서비스를 실행

### Soft IRQ 서비스는 어떻게 등록할까?
- 다음 규칙에 따라 ofen_softirq() 함수 호출
	- open_softirq(Soft IRQ 서비스의 아이디, Soft IRQ 서비스의 핸들러);
- Soft IRQ 서비스가 실행하면 해당 Soft IRQ 서비스 핸들러를 실행됨

- TIMER_SOFTIRQ Soft IRQ 서비스 핸들러를 등록하는 코드
```c
void __init init_timers(void)
{
	init_timer_cpus();
	open_softirq(TIMER_SOFTIRQ, run_tiemr_softirq);
}
```
- Soft IRQ의 TIMER_SOFTIRQ 서비스를 실행하면 run_tiemr_softirq() 함수를 호출해달라는 의미로 해석
- open_softirq() 함수
```c
void open_softirq(int nr, void (*action)(struct softirq_action *))
{
	softirq_vec[nr].action = action;
}
```
- 첫번째 정수형 타입인 nr 인자에 해당하는 softirq_vec 배열 원소의 action 필드에 두 번째 인자를 저장
- softirq_vec 배열의 nr 인덱스에 해당하는 원소의 action 필드에 Soft IRQ 서비스 핸들러를 할당

### Soft IRQ 서비스 핸들러 관련 핵심 자료구조
- softirq_vec
	- softirq_vec 배열은 NR_SOFTIRQS 크기의 배열로 Soft IRQ 서비스의 종류별로 Soft IRQ 서비스 핸들러 함수의 주소를 저장
	- struct softirq_action 타입

### Soft IRQ 서비스 핸들러의 등록과정 실습

### Soft IRQ 서비스 요청을 점검하는 전체흐름
- 전체 흐름도
- 인터럽트 컨텍스트 종료 상태 저장
- Soft IRQ 컨텍스트 실행 상태 저장 

- 1. 인터럽트 처리
	- 인터럽트 핸들러나 인터럽트 핸들러 서브루틴에서 호출하는 함수의 동작을 의미
	- _raise_softirq_irqoff()/or_softirq_pending() 함수가 실행돼 Soft IRQ 서비스를 요청
- 2. Soft IRQ 서비스 실행
	- irq_exit()함수를 호출해 Soft IRQ 서비스 요청이 있었는지 확인
	- irq_stat[cpu].__softirq_pending 자료구조를 점검
- 3. ksoftirqd/[cpu] 프로세스
	- ksoftirqd/cpu 커널 스레드도 irq_stat[cpu].__softirq_pending 자료구조에 접근해 Soft IRQ 서비스를 요청했는지 체크

### Soft IRQ 서비스 요청 관련 함수
- Soft IRQ 서비스를 요청할 때 실행되는 함수 목록
	- raise_softirq()
	- __rqiase_softirq_irqoff()
- Soft IRQ 서비스를 요청한다는 표현
	- 일부 리눅스 커널 자료에서는 raise_softirq() 함수의 동작을 'Soft IRQ를 올린다'라고 표현
	- Soft IRQ의 전체 실행 흐름상 'Soft IRQ를 올린다'라는 표현 대신 'Soft IRQ 서비스를 요청한다'라고 명시함.

```c
void raise_softirq(unsigned int nr)
{
	unsigned long flags;
	
	local_irq_save(flags);
	raise_softirq_irqoff(nr);
	local_irq_restore(flags);
}
```
- 인자: 정수형인 Soft IRQ 인덱스
- local_irq_restore 함수를 호출해 CPU 라인의 인터럽트를 비활성화
```c
inline void raise_softirq_irqoff(unsigned int nr)
{
	__raise_softirq_irqoff(nr);
	
	if (!in_interrupt())
		wakeup_softirqd();
}
```
- 현재 실행 코드가 인터럽트 컨텍스트인지 점검. 인터럽트 컨텍스트가 아니면 wakeup_softirqd함수를 통해 kosftirqd 스레드를 깨움
- 인터럽트 컨텍스트가 아닐 때도 soft IRQ 서비스 요청을 할 수 있음

### irq_stat 전역변수 분석
- irq_stat[cpu]__softirq_pending
	- 커널은 SoftIRQ 서비스 요청이 있었는지 확인하는 변수
	- irq_stat은 배열이 아니고 percpu 타입 // CPU코어 갯수만큼 메모리 공간이 위치해있겠구나!

- or_softirq_pending() 매크로 함수에 접근하는 코드
```c
void	__raise_softirq_irqoff(unsigned int nt)
{
	trace_softirq_raise(nr);
	or_softirq_pending(1UL << nr);
}
```
- or_softirq_pending() 매크로 함수(Soft IRQ서비스 요청)의 실체
	- irq_stat[cpu]__softirq_pending |= x;
- local_softirq_pending() 매크로 함수(Soft IRQ 서비스 요청 확인)의 실체
	- return irq_stat[cpu]__softirq_pending

- irq_stat 변수의 예시
```c
(static irq_cpustat_t [4]) irq_stat = { 
	[0] = (
		(unsigned int) __ softirq_pending = 0x1,
	[1] = (
		(unsigned int) __ softirq_pending = 0x2,
	...
```

### Soft IRQ 서비스를 요청했는지는 누가 어떻게 점검할까?
- local_softirq_pending() 함수
	- 커널은 Soft IRQ 서비스 요청 여부를 알려주는 local_softirq_pending() 함수를 제공
	- local_softirq_pending() 함수를 호출했는데 true를 반환하면 Soft IRQ 서비스를 요청했다고 판단
- local_softirq_pending() 함수를 호출하는 부분
	- 인터럽트 핸들러 처리를 마무리한 후 호출하는 irq_exit() 함수
	- ksoftirqd 스레드 핸들러 함수인 run_ksoftirqd() 함수
- soft_IRQ는 percpu타입으로 cpu 코어별로 동작한다.

### 동작 정리
1. Soft IRQ 서비스는 어떻게 요청할까?
	- 인터럽트 핸들러에서 raise_softirq() 함수나 __raise_softirq_irqoff() 함수를 호출한다. 이러한 함수에서or_softirq_pending() 함수를 호출해서 Soft IRQ 서비스를 요청

2. Soft IRQ 서비스 요청은 어느 코드에서 점검할까
	- 인터럽트 핸들러 실행을 마친 후 호출되는 irq_exit() 함수 혹은 ksoftirqd 스레드 핸들러인 run_ksoftirqd() 함수에서 local_softirq_pending() 함수를 호출해 Soft IRQ 서비스를 요청했는지 확인

3. Soft IRQ 서비스 요청을 할 때 변경되는 자료구조는 무엇인가?
- percpu 타입의 irq_stat__soft_pending 변수
- or_softirq_pending 함수를 호출하면 percpu 타입의 irq_stat__soft_pending 변수에 Soft IRQ 서비스를 요청했다는 정보를 설정
- local_softirq_pending()함수는 percpu타입의 irq_stat__soft_pending 변수에 접근해 Soft IRQ 서비스 요청이 있었는지 확인
