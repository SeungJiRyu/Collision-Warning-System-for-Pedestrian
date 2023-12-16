/* Title : Collision Warning Systems(CWS) for personal mobiltiy */
/* >>> For sensor board */

/* Constant for setup */
#define trigPin 12 //ultrasonic output
#define echoPin 13 //ultrasonic input
#define ENCODER 2 //encoder input - PD2
#define BUZZER 3 // buzzer output - PD3
#define bit1ForInterval 4
#define bit2ForInterval 5

/* Constant for HOG Descriptor */
#define HOG 6 //라즈베리파이에서 받을 핀 번호 설정

/* Ultrasonic code */
//Warning : 둘 다 실수로 나오므로 Serial.print로 확인하기 위해서는 강제로 형변환 필요 - Serial.print(String(float_value));
#define offsetForZero 13 //초음파센서 영점을 맞추기 위한 변수
#define delayTime 200 //초음파센서, 엔코더 딜레이
volatile float filteredistance=0; //필터링을 위한 배열 선언
volatile float sensitivity_dis=0.1;

/* 초음파 센서로 전방거리를 측정하는 함수 */
float measure_distance(){
  digitalWrite(echoPin,LOW);
  digitalWrite(trigPin,LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin,HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin,LOW);

  unsigned long duration = pulseIn(echoPin,HIGH);
  float distance = ((float) (340*duration)/10000)/2;
  filteredistance = filteredistance *(1-sensitivity_dis)+distance*sensitivity_dis;
  
  return filteredistance;
}

 /* 초음파 센서로 상대속도를 측정,계산하는 함수 */
float calculate_relative_velocity_using_Ultrasonic(){
  float distance1, distance2, velocity, filtered_velocity;
  unsigned long duration1, duration2;

  digitalWrite(echoPin,LOW);
  digitalWrite(trigPin,LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin,HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin,LOW);

  duration1 = pulseIn(echoPin,HIGH);
  distance1 = ((float) (340*duration1)/10000)/2;

  delay(delayTime);
  digitalWrite(echoPin,LOW);
  digitalWrite(trigPin,LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin,HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin,LOW);

  duration2 = pulseIn(echoPin,HIGH);
  distance2 = ((float) (340*duration2)/10000)/2; //340m/s * (1s / 10^6ms) * duaration ms * 100 cm / 1m

  velocity = ((float)(distance2-distance1)/delayTime*0.001) - offsetForZero;

  /* Low pass filter */
  for(int i=0 ; i < 100 ; i++){ //0.1초에 대한 평균값
    filtered_velocity += velocity;
    delayMicroseconds(100);
  }
  filtered_velocity /= 100;

  return filtered_velocity;
}



/* Encoder code */
//초를 표시하기 위한 코드 : Serial.print(timeCurr / 1000);

//For setup() setting for encoder
volatile int encoder = 0;

void ISR_encoder(){
  encoder++;
}

#define wheelDiameter  6.6 // 바퀴 지름 (cm)
#define rotationPerRevolution 75
#define distancePerRevolution  2 * PI * (wheelDiameter / 2) //한 바퀴당 이동거리(cm)
unsigned long timePrev = 0;
unsigned long timeCurr = 0;
volatile int encoderPrev = 0;
volatile float revolutions = 0;
volatile float rpm = 0;
volatile float velocity = 0;
volatile float filteredVelocity = 0;
volatile float sensitivity_vel = 0.5;

float measure_velocity_using_encoder(){
  timeCurr = millis();
  if(timeCurr - timePrev > delayTime){ //1초마다 출력
    timePrev = timeCurr;
    noInterrupts();

    revolutions = float(encoder-encoderPrev)/float(rotationPerRevolution); //delayTime동안의 회전수
    rpm = revolutions * delayTime; //초당 회전수 1초 : (timeCurr - timePrev)

    // RPM을 cm/s로 변환
    velocity = (revolutions * distancePerRevolution * 1000)/ delayTime; // cm/s로 변환
    filteredVelocity = filteredVelocity * (1-sensitivity_vel) + velocity * sensitivity_vel;
    encoderPrev = encoder;
    interrupts();

    return filteredVelocity;
  }
}

volatile boolean runflag = true;

/* Collision distance */
float estimate_collision_distance() { //값은 모두 소수점 붙여야 함
// 킥보드의 속도에 따라 '충돌예상거리'를 결정해주는 함수
  float vel = measure_velocity_using_encoder();
  float col_dist = max(30.0,min(50.0,30.0+(50.0-30.0)/40.0*(vel-10))); //25: speed max value

  // if(vel<=2.0){
  //   runflag=false;
  // }
  // else{
  //   runflag=true;
  // }
  return col_dist;
}

/* Buzzer code */
#define Interval1_no_detection 990 //00
#define Interval2_detect 991 //01
#define Interval3_partial_brake 992 //10
#define Interval4_full_brake 993 //11

volatile int brakeflag=0;

void make_warning_sound(){
  volatile int interval = Interval1_no_detection; //충돌예상거리에 따른 상황분류를 위한 변수
  volatile float boundary_for_detect = estimate_collision_distance();
  volatile float boundary_for_partial_brake = 28.0;
  volatile float boundary_for_full_brake = 10.0;

/*******/
  if((measure_distance() < boundary_for_full_brake) && (runflag==true)){ //&& (runflag==true)
    interval = Interval4_full_brake;
    brakeflag=1;
    delay(5000); //급제동 후 다시 출발하지 않도록 구동부를 정지
  }else if((measure_distance() < boundary_for_partial_brake)&& (runflag==true)){
    interval = Interval3_partial_brake;
  }else if((measure_distance() < boundary_for_detect)&& (runflag==true)){
    interval = Interval2_detect;
    brakeflag=0;
  }else{
    interval = Interval1_no_detection;
    brakeflag=0;
  }

  if (brakeflag==1){
    interval = Interval4_full_brake;
  }

  if(interval == Interval2_detect){
    digitalWrite(bit1ForInterval,LOW);
    digitalWrite(bit2ForInterval,HIGH);
  }
  else if(interval == Interval3_partial_brake){
    digitalWrite(bit1ForInterval,HIGH);
    digitalWrite(bit2ForInterval,LOW);
  }else if(interval == Interval4_full_brake){
    digitalWrite(bit1ForInterval,HIGH);
    digitalWrite(bit2ForInterval,HIGH);
  }else{
    digitalWrite(bit1ForInterval,LOW);
    digitalWrite(bit2ForInterval,LOW);
  }

  Serial.println(boundary_for_detect);
}

/* variable for HOG dectection */
unsigned long lastestDetectionTime = 0;
volatile float previousDistance = 0;
volatile float gap = 0;

void setup(){
  /* Ultrasonic */
  pinMode(echoPin,INPUT);
  pinMode(trigPin,OUTPUT);

  /* Encoder */
  Serial.begin(9600);
  pinMode(ENCODER, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER), ISR_encoder, FALLING);

  /* Buzzer */
  pinMode(bit1ForInterval,OUTPUT);
  pinMode(bit2ForInterval,OUTPUT);
  brakeflag=0;

  /* HOG Descriptor() */
  pinMode(HOG,INPUT);
}

void loop(){
  int whether_person_detected = digitalRead(HOG);
  previousDistance = measure_distance();

  if(measure_distance() < 10){// 갑자기 10cm 이내에 물체가 감지되면
    digitalWrite(bit1ForInterval,HIGH);
    digitalWrite(bit2ForInterval,HIGH);
  }else{ //갑자기 감지된 물체가 없으면 정상적으로 시스템 작동
    
    // 사람 감지한 최근 시간을 업데이트
    if(whether_person_detected){
      lastestDetectionTime = millis();
    }

    gap = millis() - lastestDetectionTime;

    if(gap > 500){ // No person detection - gap이 0.5s보다 크면 정상 주행
      // send signal for Interval1_no_detection
      digitalWrite(bit1ForInterval,LOW);
      digitalWrite(bit2ForInterval,LOW);
    }else{ // person detection
      make_warning_sound();
    }
  }
}

  //Test code
  //Serial.print(digitalRead(4));
  //Serial.println(digitalRead(5));
  //Serial.println(runflag);
  //measure_velocity_using_encoder();
  
  /* Test for ultra-sonic 
  float value = measure_distance();
  Serial.print(String(value));
  Serial.println();
  */

  /* Test for ultra-sonic */
  //float value = measure_distance();
  // Serial.print("거리:");
  // Serial.print(String(value));
  // Serial.print(",");
  // make_warning_sound();
  //
  // Serial.println();
  //delay(50);
 
  /* Test for collison distance
  Serial.println(estimate_collision_distance());
  */
  /* Test for Encoder
  Serial.print("초속:");
  float tvelocity=measure_velocity_using_encoder();
  Serial.println(tvelocity);
  */
