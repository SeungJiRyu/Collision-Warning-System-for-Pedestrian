/* Title : Collision Warning Systems(CWS) for personal mobiltiy */
/* >>> For driver controller & buzzer board */

/* Constant for buzzer */
#define BUZZER 3 // buzzer output - PD3
#define frequency 440.00; //라 음계, 사람이 가장 잘 인지하는 주파수

volatile uint8_t DUTY = 95; // dB를 제어하기 위한 duty값 : (PWM / [5, 15, 30, 95] 4단계로 구분
volatile unsigned long previousMillis = 0; //Delay 없이 부저를 제어하기 위해 시간을 측정하는 변수
volatile int flag =0; //0이 부저가 꺼져있다는 뜻, 1은 부저가 켜져있다는 뜻 -> Toggle을 위해 사용

/* Constant for driver controller */
#define pushButton 5
#define PWM_motor 6
#define frontDirection1 10
#define frontDirection2 9
#define rearDirection1 8
#define rearDirection2 7
#define bit1ForInterval 4
#define bit2ForInterval 5
#define ledWarning 2
#define limitPWM  120
#define PWMControl  60
#define Interval1_no_detection 990
#define Interval2_detect 991
#define Interval3_partial_brake 992
#define Interval4_full_brake 993
volatile int interval = Interval1_no_detection;

#define situation1_normal 555
#define situation2_sudden_dectection 556

//충돌거리에 따라 구간을 구별하는 함수
int distinguish_Interval(){
  volatile int bit1 = digitalRead(bit1ForInterval);
  volatile int bit2 = digitalRead(bit2ForInterval);

  if((bit1 == 0) && (bit2 == 1)){
    interval = Interval2_detect;
  }else if((bit1 == 1) && (bit2 == 0)){
    interval = Interval3_partial_brake;
  }else if((bit1 == 1) && (bit2 == 1)){
    interval = Interval4_full_brake;
  }else if((bit1 == 0) && (bit2 == 0)){
    interval = Interval1_no_detection;
  }
  return interval;
}

/* function for buzzer */
//1차 경보 : 위험 감지 시(With partial braking), 띠 띠 띠 띠
void buzzer_sound_mode1(uint8_t DUTY){
  OCR2A = F_CPU / 256 / frequency -1; //make ra
  OCR2B = OCR2A *DUTY/100; //for dB, 이 값에 비례해서 데시벨 결정
 
  unsigned long currentMillis = millis();

  if (flag==0){
    //pinMode(3,HIGH);
    DDRD |= (1<<BUZZER);//on
    if((currentMillis-previousMillis)>=50){
      previousMillis = currentMillis;
      flag ^= 1;
    }
  }
  else{
    DDRD &= ~(1<<BUZZER);//off
    if((currentMillis-previousMillis)>=70){
      previousMillis = currentMillis;
      flag ^= 1;
    }
  }
}

//2차 경보 : 충돌 예상 시(close at full braking), 띠----
void buzzer_sound_mode2(uint8_t DUTY){
  OCR2A = F_CPU / 256 / frequency -1;
  OCR2B = OCR2A *DUTY/100;
 
  flag = 0; //partial brake시 토글하던 flag값을 초기화

  //pinMode(3,HIGH);
  DDRD |= (1<<BUZZER);

  // //Delay 10 * 2ms = 20ms, 20ms 동안 경보
  // unsigned long currentMillis = millis();
}

/* brake system(Driver Controller) for each interval */
void normal_drive(uint8_t speedControl){
  analogWrite(PWM_motor,speedControl);
  digitalWrite(frontDirection1,HIGH);
  digitalWrite(frontDirection2,LOW);
  digitalWrite(rearDirection1,HIGH);
  digitalWrite(rearDirection2,LOW);
}
void partial_brake(uint8_t speedControl){
  if(speedControl<10){
    analogWrite(PWM_motor,speedControl);
    }
    else{
      analogWrite(PWM_motor,PWMControl);
    }
    digitalWrite(frontDirection1,HIGH);
    digitalWrite(frontDirection2,LOW); // +- 라서 정방향 회전
    digitalWrite(rearDirection1,HIGH);
    digitalWrite(rearDirection2,LOW);
}
void full_brake(uint8_t speedControl){
  analogWrite(PWM_motor,speedControl);
  digitalWrite(frontDirection1,LOW);
  digitalWrite(frontDirection2,LOW); //정지
  digitalWrite(rearDirection1,LOW);
  digitalWrite(rearDirection2,LOW);
}




void setup() {
  /* Serial communicaton for distinguishing Interval */
  pinMode(bit1ForInterval,INPUT);
  pinMode(bit2ForInterval,INPUT);

  /* PWM */
  Serial.begin(9600);
  pinMode(PWM_motor,OUTPUT); //ENA 1,2번에 대한 속도제어(PWM)
  pinMode(PWM_motor,OUTPUT); //ENB, 3,4번에 대한 속도제어(PWM)
  pinMode(frontDirection1,OUTPUT); //IN1
  pinMode(frontDirection2,OUTPUT); //IN2
  pinMode(rearDirection1,OUTPUT); //IN3
  pinMode(rearDirection2,OUTPUT); //IN4

  /* buzzer */
  pinMode(BUZZER,OUTPUT);

  //Timercounter setting
  TCCR2A |= (1<<COM2B1);
  TCCR2A |= (1<<WGM21) |(1<<WGM20);
  TCCR2B |= (1<<WGM22); // Fast PWM, TOP을 OCRA로 지정하였으므로, OCR2A의 값을 계산한다.
  TCCR2B |= (1<<CS22) | (1<<CS21) | (0<<CS20);

  OCR2A = F_CPU / 256 / frequency -1;
  OCR2B = OCR2A *DUTY/100;

}


void loop() {
  //충돌거리에 따른 구간 구별
  interval = distinguish_Interval();

  /* loop for driver controller and buzzer */

  /* speedController using potentiometer */
  uint8_t speedControl = min(analogRead(A5)/4,limitPWM); //가변저항 output(range:0~255)
  

 /* driver controller */
  if(interval == Interval4_full_brake){ 
    full_brake(speedControl);
    DDRD &= ~(1<<BUZZER);
    digitalWrite(ledWarning, LOW);\
    //delay(5000); //급제동 후 다시 출발하지 않도록 구동부를 정지
  }else if(interval == Interval2_detect){
    normal_drive(speedControl);
    buzzer_sound_mode1(DUTY);
    digitalWrite(ledWarning, LOW);
  }else if(interval == Interval3_partial_brake){
    partial_brake(speedControl);
    buzzer_sound_mode2(DUTY);
    digitalWrite(ledWarning, HIGH);
  }else{ //interval == Interval1_no_detection
    normal_drive(speedControl);
    DDRD &= ~(1<<BUZZER); //buzzer off
    digitalWrite(ledWarning, LOW);
  }

  Serial.print(digitalRead(4));
  Serial.print(",");
  Serial.println(digitalRead(5));
}