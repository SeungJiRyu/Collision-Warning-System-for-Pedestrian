/* Title : Collision Warning Systems(CWS) for personal mobiltiy */
/* >>> For sensor board */

/* Constant for setup */
#define trigPin 12 //ultrasonic output
#define echoPin 13 //ultrasonic input
#define ENCODER 2 //encoder input - PD2
#define BUZZER 3 // buzzer output - PD3
#define bit1ForSituation 4
#define bit2ForSituation 5

/* Ultrasonic code */
//Warning : 둘 다 실수로 나오므로 Serial.print로 확인하기 위해서는 강제로 형변환 필요 - Serial.print(String(float_value));
#define offsetForZero 13 //초음파센서 영점을 맞추기 위한 변수
#define delayTime 50 //초음파센서, 엔코더 딜레이
volatile float sensorValues[20]; //필터링을 위한 배열 선언

/* 초음파 센서로 전방거리를 측정하는 함수 */
float measure_distance(){
  for(int i = 0 ; i < 20-1 ; i++){
    sensorValues[i]= sensorValues[i+1];
  }
  digitalWrite(echoPin,LOW);
  digitalWrite(trigPin,LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin,HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin,LOW);

  unsigned long duration1 = pulseIn(echoPin,HIGH);
  float distance = ((float) (340*duration1)/10000)/2;

  sensorValues[20-1] = distance;
  float filterDistance = 0;
  for (int i=0; i< 20; i++){
    filterDistance += sensorValues[i];
  }
  filterDistance /=20;
  return filterDistance;
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

float measure_velocity_using_encoder(){
  #define wheelDiameter  6.6 // 바퀴 지름 (cm)
  #define rotationPerRevolution  130
  #define distancePerRevolution  2 * PI * (wheelDiameter / 2) //한 바퀴당 이동거리(cm) 
  unsigned long timePrev = 0;
  unsigned long timeCurr = 0;
  volatile int encoderPrev = 0;
  volatile float revolutions = 0;
  volatile float rpm = 0;
  volatile float velocity = 0;
  volatile float filteredVelocity = 0;
  
  timeCurr = millis();
  if(timeCurr - timePrev > delayTime){ //1초마다 출력
    timePrev = timeCurr;
    noInterrupts();

    revolutions = float(encoder-encoderPrev)/rotationPerRevolution;
    rpm = revolutions * 60; //초당 회전수 1초 : (timeCurr - timePrev)

    // RPM을 cm/s로 변환
    velocity = (rpm * distancePerRevolution) / 60; // cm/s로 변환
    
    encoderPrev = encoder;
    interrupts();

    return velocity;
  }
}


/* Buzzer code */
#define situation1_no_detection 990 //00
#define situation2_partial_break 991 //10
#define situation3_full_break 992 //11

void make_warning_sound(){
  volatile int situation = situation1_no_detection; //충돌예상거리에 따른 상황분류를 위한 변수
  volatile float boundary_for_partial_break = 30; /////////////////////////////나중에 엔코더 속도 맞춰서 수식 써야하는 부분//////
  volatile float boundary_for_full_break = 10;////////////////////////////////////////////////////////////////////////

  if(measure_distance() < boundary_for_full_break){
    situation = situation3_full_break;
  }else if(measure_distance() < boundary_for_partial_break){
    situation = situation2_partial_break;
  }else{
    situation = situation1_no_detection;
  }

  if(situation == situation2_partial_break){
    digitalWrite(bit1ForSituation,HIGH); 
    digitalWrite(bit2ForSituation,LOW); 
  }else if(situation == situation3_full_break){
    digitalWrite(bit1ForSituation,HIGH); 
    digitalWrite(bit2ForSituation,HIGH);
  }else{
    digitalWrite(bit1ForSituation,LOW); 
    digitalWrite(bit2ForSituation,LOW); 
  }
}

void setup(){
  /* Ultrasonic */
  pinMode(echoPin,INPUT);
  pinMode(trigPin,OUTPUT);

  /* Encoder */
  Serial.begin(57600);
  pinMode(ENCODER, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER), ISR_encoder, FALLING);

  /* Buzzer */
  pinMode(bit1ForSituation,OUTPUT);
  pinMode(bit2ForSituation,OUTPUT);
}

void loop(){
  /* Test for ultra-sonic */
  /*
  DDRD &= ~(1<<BUZZER);
  float value = measure_distance();
  Serial.print(String(value));
  Serial.println();
  delay(200);
  */

  /* Test for encoder */
  float value = measure_distance();
  Serial.print("거리:");
  Serial.print(String(value));
  Serial.print(",");
  make_warning_sound();
  Serial.print(digitalRead(4));
  Serial.print(digitalRead(5));
  Serial.println();
  //delay(50);
}

/* Sound Sensor
#define SOUND A0
int vol = 0;
int count = 0;


void setup() {
  Serial.begin(9600);
}

void loop() {
  vol = analogRead(SOUND);

  if(vol>300){
    count++;
    Serial.print("sound: ");
    Serial.println(count);
    Serial.print("volume: ");
    Serial.println(vol);
    delay(500);
  }

  /*
  Serial.print("sound volume : ");
  Serial.print(vol);
  Serial.println();
  delay(100);
  */


/* 바퀴 지름 6.6 */
