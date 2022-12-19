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

