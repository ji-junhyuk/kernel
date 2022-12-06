### 5장 인터럽트를 공부하는 방법
- 인터럽트 컨텍스트, 인터럽트 벡터가 제일 중요하다!
  
- 인터럽트 3가지 동작
	- arm architecture 관점(arm processor, exception 처리)
	- irq subsystem 관점
	- interrupt handler 관점

### 인터럽트란?
- 하드웨어 관점
	- 하드웨어의 변화를 감지해서 외부 입력으로 전달되는 전기 신호.
	- 보통 하드웨어 개발자들은 오실로스코프라는 장비로 인터럽트 신호가 제대로 올라오는지 측정.
- 소프트웨어 관점
	- 인터럽트가 발생하면 프로세스는 하던 일을 멈추고 '이미 정해진 코드'를 실행해서 하드웨어의 변화를 처리
	- '이미 정해진 코드': 인터럽트 벡터와 인터럽트 핸들러
	- 인터럽트가 발생하면 소프트웨어적으로 처리하는 과정은 인터럽트 서비스 루틴(Interrupt Service Routine)이라고 부름

- CPU 아키텍처 관점
	- 인터럽트는 CPU 아키텍처별로 다르게 처리
	- x86, ARMv7, ARMv8 아키텍처별로 인터럽트를 처리하는 방식이 다름
	- 라즈베리 파이는 ARMv7 기반 아키텍처이므로 ARMv7 CPU에서 인터럽트를 처리하는 과정을 파악하면 됨.
 
- ARMv7 아키텍처 기반 프로세서 관점
	- 인터럽트는 익셉션(Exception)의 한 종류로 처리
	- 외부 하드웨어 입력이나 오류 이벤트가 발생하면 익셉션 모드로 진입
	- 익셉션이 발생했다고 감지하면 익셉션 종류별로 이미 정해 놓은 주소로 브랜치
 
- ARMv7 기반 프로세서에서 익셉션 벡터(Exception Vector)처리 방식
	- 인터럽트나 소프트웨어적으로 심각한 오류가 발생하면 ARMv7 프로세스는'이미 정해진 주소'에 있는 코드를 실행
	- 이미 정해진 주소 코드를 익셉션 벡터(Exception vector)라 하며, 각 익셉션의 종류에 따라 주소의 위치가 다름
	- 인터럽트를 익셉션 벡터 중 하나의 모드로 처리

### 인터럽트 처리 방식에 대한 진실과 오해
- 인터럽트 핸들러를 빨리 실행해야 하는 이유는?
	- 인터럽트가 발생하면 프로세스(process)가 실행 중인 코드가 멈추기 때문
	- 리눅스 디바이스 드라이버나 커널 코드를 볼 때는 우리가 보고 있거나 실행하는 어떤 커널 코드도 인터럽트가 발생하면 인터럽트 벡터로 실행 흐름이 바뀔 수 있음

- 인터럽트 핸들러에서 처리해야 할 동작이 많으면 어떻게 해야 할까?
	- 인터럽트 후반부 기법을 활용하면 됨.

### 인터럽트 주요 개념
- 인터럽트 핸들러
- 인터럽트 벡터
- 인터럽트 디스크립터
- 인터럽트 컨텍스트

### 인터럽트 핸들러
- 인터럽트가 발생하면 이를 핸들링하기 위한 함수가 호출되는데 이를 인터럽트 핸들러라고 함.
- 키보드를 타이핑해서 인터럽트가 발생하면 키보드 인터럽트를 처리하는 키보드 인터럽트 핸들러가 호출
- 휴대폰에서 화면을 손으로 만지면 터치 인터럽트가 발생하고 터치 인터럽트를 처리하는 터치 인터럽트 핸들러가 호출

- 마우스 인터럽트 핸들러가 호출되는 과정(ARM architecture에 의존적임)
	- 마우스를 움직이면 마우스가 움직였다는 인터럽트가 발생해 인터럽트 벡터가 실행됨.
	- 커널의 인터럽트 내부 함수(IRQ 서브 시스템)에서 해당 인터럽트에 맞는 인터럽트 핸들러를 찾아 호출
	- 많은 하드웨어 디바이스가 이 같은 방식으로 인터럽트를 통해 하드웨어의 변화를 알림
 
- 리눅스 커널에서 구현된 익셉션 벡터
	- 0xFFFF0000 주소에 익셉션 종류 별로 익셉션 벡터 테이블 구현됨.
	- 0xFFFF0000 주소 기준으로 +0x18 주소에 인터럽트 벡터의 코드가 구현돼 있음.

- 인터럽트 디스크립터란
	- 인터럽트 종류별로 다음과 같은 인터럽트의 세부 속성을 관리하는 자료 구조
		- 인터럽트 핸들러
		- 인터럽트 핸들러 매개변수
		- 논리 인터럽트 번호
		- 인터럽트 실행 횟수
	- 프로세스의 세부 속성을 표현하는 자료구조가 태스크 디스크립터이듯이 인터럽트에 대한 속성 정보를 저장하는 자료구조가 인터럽트 디스크립터임.
	- 커널 인터럽트의 세부 함수에서는 인터럽트 디스크립터에 접근해 인터럽트 종류별로 세부적인 처리를 수행

### 인터럽트 처리하는 방식을 잘 알아야 하는 이유
- 대부분의 리눅스 드라이버는 인터럽트를 통해 하드웨어 디바이스와 통신하므로 인터럽트의 동작 방식을 잘 알고 있으면 디바이스 드라이버 코드를 빨리 이해할 수 있음.
- 인터럽트가 발생하면 프로세스는 이미 정해진 동작을 수행하므로 인터럽트 처리 과정을 숙지하면 프로세스가 스택 메모리 공간에서 어떻게 실행되는지 알게 됨.
-  CPU 아키텍처 (x86, ARM)에 따라 인터럽트 벡터는 달리 동작하는데 인터럽트 벡터가 어떻게 동작하는지 잘 알면 자연히 ARM 아키텍처의 동작 원리에 대해 더 많이 파악함.
- 리눅스 커널의 핵심 동작을 이해하기 위해
	- 스케줄링에서 선점(preemptive) 진입 경ㄹ로 중 하나가 인터럽트 처리를 끝낸 시점
	- 유저 공간에서 등록한 시그널 핸들러는 인터럽트 핸들러를 실행한 다음 처리를 시작
	- 레이스 컨디션이 발생하는 가장 큰 이유 중 하나는 비동기적으로 인터럽트가 발생해서 임계 영역의 코드를 오염시키는 것임

### 리눅스 커널에서의 인터럽트 처리 흐름
1. ARM
	- 인터럽트 벡터 주소 실행
	- 실행 중 레지스터를 스택 공간에 푸시
```
인터럽트가 발생하면 실행 중인 코드를 멈춘 후 인터럽트 벡터로 실행 흐름을 이동 
```
- 인터럽트가 발생하면 프로세스는 실행 도중 프로그램 카운터가 인터럽트 벡터로 이동
- 인터럽트 벡터에서 인터럽트 처리를 마무리한 후 다시 프로세스를 실행하기 위해 실행 중인 프로세스의 레지스터 세트를 스택에 저장
- IRQ 서브시스템을 구성하는 함수들이 호출됨
 
2. 리눅스 커널
	- __irq_svc
	- gic_handle_irq
	- handle_domain_irq
	- generic_handle_irq
	- handle_fasteoi_irq
	- handle_irq_event
	- handle_irq_event_percpu
```
인터럽트 벡터로 프로그램 카운터를 브랜치
인터럽트 자료 구조를 관리하고 예외 처리를 수행하는 커널 내부
인터럽트 디스크립터를 읽어 해당 인터럽트 핸들러를 호출 
```
- 커널 내부에서는 발생한 인터럽트에 대응하는 인터럽트 디스크립터를 읽어서 인터럽트 핸들러를 호출

3. 디바이스 드라이버
	- 인터럽트 핸들러 실행
		- 하드웨어 설정
		- 인터럽트 변화에 대한 처리(ex) 화면 업데이트)
```
각 디바이스 드라이버에서 등록한 인터럽트 핸들러를 실행해 인터럽트 발생에 대한 처리를 수행 
```
- 인터럽트 핸들러에서 하드웨어를 직접 제어하고 유저 공간에 이 변화를 알림 

### 터치 드라이버 관점 인터럽트 처리 흐름
- 1단계 : 터치 인터럽트 발생
	- 하드웨어적인 터치 모듈이 변화를 감지하고 터치 모듈에 대한 인터럽트를 발생시키면 인터럽트 벡터가 실행

- 2단계: 터치 인터럽트 핸들러 호출
	- 커널은 터치 인터럽트 번호로 해당 인터럽트 디스크립터를 읽은 다음 인터럽트 디스크립터에 저장된 인터럽트 핸들러의 주소를 찾아 인터럽트 핸들러를 호출 
- 3단계: 터치 인터럽트 핸들러 실행
	- 결국 터치 인터럽트 핸들러는 터치 인터럽트를 받아 정해진 처리
	- 화면을 업데이트 하거나 하드웨어 터치 디바이스에 인터럽트를 잘 받았다는 사실을 알림 

### 인터럽트 컨텍스트
- 인터럽트 컨텍스트란 무엇인가?
	- 인터럽트를 처리 중
	- 현재 인터럽트 핸들러를 실행 중이면 인터럽트를 핸들링하는 중이므로 인터럽트 컨텍스트임.
	- 인터럽트 핸들러에서 호출된 서브 함수 중 하나가 실행될 때도 인터럽트 컨텍스트임.
- 리눅스 커널에서 인터럽트 컨텍스트를 정의한 이유
	- 인터럽트가 발생하면 이를 핸들링하는 코드는 빨리 실행돼야 하기 때문 
- 리눅스 커널에서 컨텍스트란?
	- 컨텍스트란 '프로세스 실행 그 자체'를 의미
	- 현재 실행 중인 프로세스 정보를 담고 있는 레지스터 세트
	- 컨텍스트(레지스터 세트)와 관련한 자료구조
	- cpu_cotext_save 구조체는 프로세스 스택의 최상단 주소에 위치한 thread_info 구조체의 cpu_context 필드에 저장됨.

### 인터럽트 컨텍스트란: in_interrupt() 함수
- in_interrupt() 함수란
	- 현재 실행 중인 코드가 인터럽트 컨텍스트 구간인지 알려주는 역할
	- in_interrupt()함수가 true를 반환하면 인터럽트 컨텍스트이고, 반대로 false를 반환하면 프로세스 컨텍스트
- in_interrupt() 함수를 왜 사용할까?
	- 함수들은 복잡하게 호출되므로 함수 호출 흐름을 간단히 파악하기 어려워 커널 혹은 드라이버 코드에서 볼 수 있는 함수가 '인터럽트 컨텍스트'에서 실행 중인지 분간하기 힘듦.
- patch code
```c
static struct mmc_blk_ioc_data *mmc_blk_ioctl_copy_from_user(
	struct mmc_blk_ioc_data *idata;
	int err;
	
-	idata = kmalloc(sizeof(*idata), GFP_KERNEL);
+	idata = kmalloc(sizeof(*idata), in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
```
- in_interrupt() 함수가 true를 반환하면 GFP_ATOMIC 플래그, 반대의 경우 GFP_KERNEL 플래그를 적용해 kmalloc() 함수를 호출
- in_interrupt() 함수가 true를 반환하는 경우 인터럽트 컨텍스트이며 '인터럽트 처리 중'이라고 볼 수 있음
- 인터럽트를 처리하는 도중에는 빨리 메모리를 할당하는 목적의 코드
### in_interrupt 함수 코드 분석하기
- in_interrupt 함수 구현부
```c
#define in_interrupt() (irq_count()) // irq_count()함수로 치환
```
- irq_count 함수 선언부
```c
#define irq_count() (preempt_count() & (HARDIRQ_MASK | SOFTIRQ_MASK | NMI_MASK()
```
	- preempt_count() 함수가 반환하는 값과 HARDIRQ_MASK | SOFTIRQ_MASK 비트 마스크에 대해 OR 비트 연산을 수행
- irq_count() 매크로 함수는 다음과 같이 변환
	- preempt_count() & 0x1fff00
- preempt_count() 함수의 정체는 무엇일까?
	- preempt_count() 함수는 실행 중인 프로세스의 thread_info 구조체의 preempt_count 필드값을 반환

### 인터럽트 컨텍스트란
- ftrace에서 확인한 인터럽트 컨텍스트
- 커널 로그에서 확인한 인터럽트 컨텍스트

### 인터럽트 컨텍스트에서 스케줄링을 하면 어떻게 될까?
- 스케줄링 관련 함수를 호출하면 커널 내부에서 많은 연산을 수행하므로 실행시간이 오래 걸린다. 짧은 시간에 인터럽트 핸들러를 실행하고 인터럽트 발생으로 실행을 멈춘 코드로 돌아가야 하는데 프로세스가 휴면 상태에 진입하면 시스템이 오동작할 수 있다.
(다른 프로세스가 뮤택스 획득해서 임계영역을 실행, 같은 뮤택스를 획득한 프로세스는 sleep, 뮤택스를 다 사용한 프로세스는 sleep 들어간 프로세스를 깨워준다. mutex의 경우 slow_path 실행흐름으로 이미 뮤택스를 획득한 상태에서 다른 프로세스가 획득하려고 하면 sleep에 들어간다. 인터럽트 컨텍스트에서 mutex의 fast_path 실행흐름으로  획득하고 해제하면 큰 문제가 되지 않을 거 같은데, 만약에 slow_path로 동작하게 되면. 인터럽트 컨텍스트 과정에서 슬립에 들어간다면 그 프로세스가 언제 깨어날지 예측하기 어렵기 때문이다. 
- 인터럽트 컨텍스트에서 스케줄링 지원하는 함수를 호출한다면, 에러메시지를 출력하거나 커널 패닉을 유발한다.

### 인터럽트 벡터: ARM 프로세서 관점
- 인터럽트가 발생하면 어떤 코드가 가장 먼저 실행될까?
	- 인터럽트 벡터인 vector_irq 레이블 실행

- Arm architecture에서는 인터럽트를 exception의 한 종류로 처리한다.
	- exception vector의 주소를 찾는다(에러마다 offset을 가짐)

- 리눅스 커널에서 구현된 익셉션 벡터
	- vector_irq 레이블 구현부
- 인터럽트가 발생한 모드(유저/커널 모드)에 따라 다음 레이블로 브랜치함
	- __irq_svc: 커널 모드(ARM 프로세서 기준: Supervisor Mode)
	- __irq_usr: 유저 모드(ARM 프로세서 기준: User Mode)

### 인터럽트 핸들러는 언제 호출될까?
- 인터럽트가 발생하면 어떤 코드가 가장 먼저 실행될까?
	- 인터럽트 벡터인 vector_irq 레이블이 실행

- 각 모드 별로 호출되는 인터럽트 벡터
	- 유저모드 인터럽트 벡터: vector_irq 레이블 -> __irq_usr 레이블
	- 커널모드 인터럽트 벡터: vector_irq 레이블 -> __irq_svc 레이블
- 커널에서 인터럽트 핸들러를 호출하는 단계
	- 인터럽트 벡터를 실행. 실행 중인 프로세스의 레지스터 세트를 프로세스 스택에 저장
	- 커널의 IRQ 서브시스템을 구성하는 함수들이 호출된 후 __handle_irq_event_percpu() 함수를 실행.
	- 인터럽트에 해당하는 인터럽트 디스크립터를 읽어 속성 정보를 업데이트
	- 인터럽트 핸들러 함수를 호출

### __irq_svc 동작
sp 지시자 : 스택 주소
sup sp, sp #76 ; 0x4c // 현재 스택 주소에서 0x4c만큼 빼는 연산은 프로세스의 스택 공간을 0x4c바이트 만큼 확보
stm sp, (r1 r2 .. r9, s1 fp ip) // 프로세스 스택 공간에 r1부터 ip 레지스터까지 저장
ldr r1, [pc, #36] ; // handle_arch_irq 전역변수 로딩
ldr pc, [r1] // bcm2836_arm_irqchip_handle_irq() 함수 호출

- in_interrupt() 함수 동작 원리
	- 실행 중인 프로세스의 thread_info 구조체의 preempt_count 필드값에 접근함
	- 0x1fff00와 AND 비트 연산한 결과
- __irq_svc 레이블에서 레지스터 스택 푸시 확인