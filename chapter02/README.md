- [Raspberry Pi settings](#raspberry-pi-settings)
	* [명령어](#명령어)
	* [라즈베리 파이 설치 과정은 왜 배워야 할까?](#라즈베리-파이-설치-과정은-왜-배워야-할까)
	* [라즈베리 파이 설치](#라즈베리-파이-설치)
	* [설정](#설정)
	* [pi 이름 변경하기](#pi-이름-변경하기)
	* [설치 순서](#설치-순서)
	* [리눅스 커널 소스의 구조](#리눅스-커널-소스의-구조)
	* [objdump](#objdump)
# Raspberry Pi settings

### 명령어
```bash
$ sudo reboot
$ sudo shutdown -h now
```
### 라즈베리 파이 설치 과정은 왜 배워야 할까?
- 라즈베리 파이를 설치하면서 실전 개발에서 활용하는 과정 체험

| 라즈베리 파이 설치                  | 실전 개발                                                |
|-------------------------------------|----------------------------------------------------------|
| 전원, HDMI 케이블 연결, SD카드 삽입 | 보드에 주변 장치 연결(USB, UART, 전원                    |
| win32 disk imager 프로그램          | 개발 환경으로 다운로드 툴 제공, 파티션 이미지별 다운로드 |
| 보드에 UART 연결                    | 보드에 UART 연결                                         |
| Geany 에디터(소스 수정)             | VI 에디터(소스 수정)                                     |
| 컴파일: ARM_GCC                     | 컴파일: ARM-GCC(크로스 컴파일러)                         |
| 리눅스 터미널에서 명령어 입력       | 리눅스 터미널에서 명령어 입력                            |

### 라즈베리 파이 설치
- 유튜브 참고
	- https://www.youtube.com/watch?v=_LB6z7e0kIE&t=103s
	- https://www.youtube.com/watch?v=tenLLerqop8&t=205s

### 설정
```bash
$ sudo apt-get install neovim
$ raspi-config	
$ sudo apt-get install ibus
$ sudo apt-get install ibus-hangul
$ sudo apt-get install fonts-unfonts-core
$ sudo apt-get update && sudo apt-get upgrade
```
- raspi-config: `en_GB.UTF-8 UTF-8`, `en_US.UTF-8 UTF-8`, `ko_kr.UTF-8 UTF-8`

### pi 이름 변경하기
- 계정을 변경하기 위해선, 그 계정에서 실행되는 모든 프로세스가 종료되어야 함.
1. 다른 temp 계정을 만든다
```bash
$ adduser [temp]
```
- reboot 후 temp로 로그인한다.
(desktop gui를 지원하는 환경에서는 pi프로세스를 완전히 종료하기 위해 reboot함)

2. temp로 로그인 후 root로 로그인한다. username 변경
```bash
$ sudo su
$ usermod -l junto pi
$ usermod -m -d /home/user newuser
```

3. pi 프로세스가 살아있다면, kill로 종료시킨다.
```bash
kill [pid]
```

4. temp 계정 삭제
```bash
$ sudo deluser -remove-all temp
```

### 설치 순서
https://www.raspberrypi.com/documentation/computers/linux_kernel.html#building
1. 라즈비안 커널 소스코드 내려받기
```bash
$ git clone --depth=1 https://github.com/raspberrypi/linux
$ git branch // version
$ git clone --depth=1 --branch [version] https://github.com/raspberrypi/linux
```
2. 라즈비안 리눅스 커널 빌드(make)
3. 라즈비안 리눅스 커널 설치(커널 이미지 make)

### 리눅스 커널 소스의 구조
- arch: 아키텍처별로 동작하는 커널 코드
- include
- Documentation: 커널 기술 문서
- Kernel: 핵심 코드
	- irq: 인터럽트 관련 코드
	- sched: 스케줄링 코드
	- power: 커널 파워 매니지먼트 
	- locking: 커널 동기화 관련 코드
	- printk: 커널 콘솔 관련 코드
	- trace: ftrace 관련 코드
- mm: 가상 메모리 및 페이징 관련 코드
- drivers: 디바이스 드라이버 코드
- fs: 파일 시세틈 코드
- lib: 커널에서 제공하는 라이브러리 코드

### objdump
- 리눅스 커널 어셈블리 코드와 섹션 정보를 볼 수 있는 바이너리 유틸리티
```bash
$ objdump -x vmlinux
```
- 커널 이미지를 빌드하면 `System.map`파일이 생성된다.
	- 커널에서 가장 유명한 schedule함수가 주소 `c09d6270` ~ `c09d6c5c`에 있음을 알 수 있다.
- 사용법
```bash
$ objdump --start-address=[시작주소]--stop-address=[끝주소] -d vmlinux
```
