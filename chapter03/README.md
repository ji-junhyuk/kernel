* [kernel debugging and code learning](#kernel-debugging-and-code-learning)
	* [임베디드 리눅스 BSP 디버깅 툴](#임베디드-리눅스-bsp-디버깅-툴)
	* [커널 로그 분석](#커널-로그-분석)
	* ['cat /proc/interrupts' 디버깅](#cat-procinterrupts-디버깅)
	* [디버깅 순서](#디버깅-순서)
	* [ftrace 로그에서 알 수 있는 정보](#ftrace-로그에서-알-수-있는-정보)
	* [단순히 코드 분석이 아닌 디버깅을 해야 하는 이유](#단순히-코드-분석이-아닌-디버깅을-해야-하는-이유)

# kernel debugging and code learning

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
