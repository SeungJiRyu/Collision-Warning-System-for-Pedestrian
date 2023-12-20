/* Title : Collision Warning Systems(CWS) for personal mobiltiy */
/* >>> For sensor board */

/* Constant for setup */
#define trigPin 12 //ultrasonic output
#define echoPin 13 //ultrasonic input
#define ENCODER 2 //encoder input - PD2
#define BUZZER 3 // buzzer output - PD3
#define bit1ForInterval 4
#define bit2ForInterval 5
#define bit3ForControllingdB  6
#define bit4ForSpeedmeasure 7

/* Constant for HOG Descriptor */
#define HOG A0 //라즈베리파이에서 받을 핀 번호 설정

/* Ultrasonic code */
//Warning : 둘 다 실수로 나오므로 Serial.print로 확인하기 위해서는 강제로 형변환 필요 - Serial.print(String(float_value));
#define offsetForZero 13 //초음파센서 영점을 맞추기 위한 변수
#define delayTime 200 //초음파센서, 엔코더 딜레이
volatile float filterdistance=30; //필터링을 위한 배열 선언, 시작점을 30으로,

volatile float distance_right = 0;
volatile float filterdistance_right=80;

volatile float distance_left = 0;
volatile float filterdistance_left=80;

volatile float sensitivity_dis=0.2;

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
  filterdistance = filterdistance *(1-sensitivity_dis)+distance*sensitivity_dis;
  
  //테스트용
  //Serial.println(filterdistance);

  return filterdistance;
}


float measure_dist_right(){
  int volt = map(analogRead(A5), 0, 1023, 0, 5000);
     
  distance_right = (27.61 / (volt - 0.1696)) * 1000.0;
  if (distance_right < 0){
    distance_right=-distance_right;
  }
  if (distance_right>=200){
    distance_right=200;
  }
  filterdistance_right=filterdistance_right*(1-sensitivity_dis)+distance_right*sensitivity_dis;
  
  return filterdistance_right;
}

float measure_dist_left(){
  int volt = map(analogRead(A4), 0, 1023, 0, 5000);
     
  distance_left = (27.61 / (volt - 0.1696)) * 1000.0;
  if (distance_left < 0){
    distance_left=-distance_left;
  }
  if (distance_left>=150){
    distance_left=200;
  }
  filterdistance_left=filterdistance_left*(1-sensitivity_dis)+distance_left*sensitivity_dis;
  
  return filterdistance_left;
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

volatile float boundary_for_partial_brake = 20.0;
volatile float boundary_for_full_brake = 7.0;

volatile int interval; //충돌예상거리에 따른 상황분류를 위한 변수

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

//int runflag = 0;

/* Collision distance */
float estimate_collision_distance() { //값은 모두 소수점 붙여야 함
// 킥보드의 속도에 따라 '충돌예상거리'를 결정해주는 함수
  float vel = measure_velocity_using_encoder();
  float col_dist = max(40.0,min(80.0,40.0+(80.0-40.0)/40.0*(vel))); //40: speed max value

  // if(vel<=2.0){
  //   runflag=0;
  // }
  // else{
  //   runflag=1;
  // }
  return col_dist;

}

/* Buzzer code */
#define Interval1_no_detection 990 //00
#define Interval2_detect 991 //01
#define Interval3_partial_brake 992 //10
#define Interval4_full_brake 993 //11

//volatile int brakeflag=0;

void make_warning_sound(){

  
  
/*******/
  
  

  //Serial.println(boundary_for_detect);
}

/* variable for HOG dectection */
unsigned long lastestDetectionTime = 0;
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

  /* HOG Descriptor() */
  pinMode(HOG,INPUT);
  pinMode(bit3ForControllingdB,OUTPUT); 

  /* make warning sound */
  interval = Interval1_no_detection;
}

void loop(){
  /* check whether person is detected */
  int detected = analogRead(HOG); //3.3/5 * 1023 = 675.18
  bool whether_person_detected = false;

  if((detected < 690) && (detected > 650)){
    whether_person_detected = true;
    digitalWrite(bit3ForControllingdB,HIGH);
  }
  
  volatile float distance_front = measure_distance();
  volatile float distance_right = measure_dist_right();
  volatile float distance_left = measure_dist_left();
  volatile float boundary_for_detect = estimate_collision_distance();
  
    /* Rasberrypi signal debouncing */
  if(whether_person_detected){
    lastestDetectionTime = millis();
  }
  gap = millis() - lastestDetectionTime;

  if(gap > 500){ // No person detection - gap이 0.3s보다 크면 정상 주행
    whether_person_detected = false;
  }else{ // person detection
    whether_person_detected = true;
  }
  
  /* 상황 판단 */
  if((distance_front < boundary_for_full_brake)){ //&& (runflag==true)
    interval = Interval4_full_brake;
    //brakeflag=1;
  }else if((distance_front < boundary_for_partial_brake)){
    interval = Interval3_partial_brake;
  }else if((distance_front < boundary_for_detect)&&(whether_person_detected)){//
    interval = Interval2_detect;
  }else{
    if (((distance_right <= 40) || (distance_left <= 40)) && whether_person_detected){
      interval = Interval2_detect;
    }
    else{
      interval = Interval1_no_detection;
    }
  }

  // if (brakeflag==1){
  //   interval = Interval4_full_brake;
  // }

  // if(distance_front==0){
  //   brakeflag=0;
  // }

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
  Serial.print(distance_right);
  Serial.print(',');
  Serial.println(distance_left);
  // Serial.print(digitalRead(4));
  // Serial.println(digitalRead(5));
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

  /* Test for ultra-sonic*/
  // float value = measure_distance();
  // Serial.print("거리:");
  // Serial.print(String(value));
  // Serial.println(",");
  //Serial.print(digitalRead(4));
  //Serial.println(digitalRead(5));

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
