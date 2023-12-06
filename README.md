# Collision-Warning-System-for-Pedestrian
## 개요
본 프로젝트는 마이크로프로세서응용시스템 수업에서 진행하는 main 프로젝트로 아두이노 우노(Atmega328p)를 활용하여 스케일 모델에 적용할 "보행자 안전사고 방지를 위한 Personal Mobility 충돌경보 시스템"의 구현을 목적으로 한다. 

CWS-충돌경보시스템은 Collision-Warning-System-for-Pedestrian에서 약자를 활용하여 CWS라 하겠다-는 크게 2가지 기능으로 구성되어 있다. 

![purpose_img](https://github.com/SeungJiRyu/Collision-Warning-System-for-Pedestrian/assets/108774002/3bf86e60-4532-4dc5-bfb3-b774a610dffc)

첫 번째 기능은 보행자를 위한 경보음 발생 기능이다.전동킥보드가 도로를 주행하는 상황에서 충돌예상거리 이내에 보행자 식별 시 경보음을 발생하여 전동킥보드를 인지하지 못하는 보행자에게 전동킥보드가 주행 중임을 알린다.

두 번째 기능은 충돌예상거리 이내에 보행자가 감지된 경우, 보행자와의 거리에 따라 충돌이 예상되는 위험도를 판단하여 자동으로 제동하는 Partial Braking System기능이다.
 
## 프로젝트 경과
#### 해야할 일 
1. encoder에서 측정한 속도를 바탕으로 collision distance 결정
2. 라즈베리파이 HOG
- ardum cam, esp32 사용할 수 있는지 판단
- 안되면 PICam(라즈베리파이 전용 카메라) 구입 -> Raspberry Pi Camera Module2 사용 결정
- 파이캠 연결하고 정상 작동하는지 테스트(사진촬영 해보기)
- 모든 테스트 완료 후에 라즈베리파이 터미널에서 GUI를 거치지 않고 부팅시에 HOG.py가 실행되게 세팅 및 테스트 필요
3. 필요시 주행모드, 주차모드를 구별하는 스위치 구현
4. 킥보드 하드웨어 디자인 설계
5. 프로젝트 시연 방법 결정
