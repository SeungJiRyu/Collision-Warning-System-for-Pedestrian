/* Title : Collision Warning Systems(CWS) for personal mobiltiy */
/* >>> For PWM board */

/* Constant for setup */
#define pushButton 5
#define PWM_rear 6
#define frontDirection1 10
#define frontDirection2 9
#define rearDirection1 8
#define rearDirection2 7
#define PWM_front 11



void setup() {
  Serial.begin(9600);
  pinMode(PWM_front,OUTPUT); //ENA 1,2번에 대한 속도제어(PWM)
  pinMode(PWM_rear,OUTPUT); //ENB, 3,4번에 대한 속도제어(PWM)
  pinMode(frontDirection1,OUTPUT); //IN1
  pinMode(frontDirection2,OUTPUT); //IN2
  pinMode(rearDirection1,OUTPUT); //IN3
  pinMode(rearDirection2,OUTPUT); //IN4
  pinMode(pushButton, INPUT);
}

/* Constant for PWM */
volatile int brk=0;
volatile int num=1; //전진, 후진 (임시)

void loop() {
  uint8_t speedControl = analogRead(A5)/4; //가변저항 output(range:0~255)
  int brk = digitalRead(pushButton);

  if(brk == 0){
    analogWrite(PWM_front,speedControl);
    analogWrite(PWM_rear,speedControl);
    digitalWrite(frontDirection1,HIGH);
    digitalWrite(frontDirection2,LOW); // +- 라서 정방향 회전
    digitalWrite(rearDirection1,HIGH);
    digitalWrite(rearDirection2,LOW);
  }else if(brk == 1){
    analogWrite(PWM_front,speedControl);
    analogWrite(PWM_rear,speedControl);
    digitalWrite(frontDirection1,HIGH);
    digitalWrite(frontDirection2,HIGH); // -+ 라서 역방향 회전
    digitalWrite(rearDirection1,HIGH);
    digitalWrite(rearDirection2,HIGH);     
  }
}