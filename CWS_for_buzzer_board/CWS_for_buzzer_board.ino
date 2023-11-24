/* Buzzer code */
#define BUZZER 3 // buzzer output - PD3
#define frequency 440.00; //라 음계, 사람이 가장 잘 인지하는 주파수
volatile uint8_t DUTY = 1; // dB를 제어하기 위한 duty값 : (PWM / [5, 15, 30, 95] 4단계로 구분

//1차 경보 : 위험 감지 시(With partial braking), 띠 띠 띠 띠
void buzzer_sound_mode1(uint8_t DUTY){
  OCR2A = F_CPU / 256 / frequency -1; //make ra
  OCR2B = OCR2A *DUTY/100; //for dB, 이 값에 비례해서 데시벨 결정
  //pinMode(3,HIGH);
  DDRD |= (1<<BUZZER);//on

  delay(40);
  DDRD &= ~(1<<BUZZER);//off
  delay(70);
}

//2차 경보 : 충돌 예상 시(close at full braking), 띠----
void buzzer_sound_mode2(uint8_t DUTY){
  OCR2A = F_CPU / 256 / frequency -1;
  OCR2B = OCR2A *DUTY/100;
  //pinMode(3,HIGH);
  DDRD |= (1<<BUZZER);
  OCR2B = OCR2A *DUTY/100 ;

  //Delay 10 * 2ms = 20ms, 20ms 동안 경보
  delay(2000);
}

void setup() {
  Serial.begin(57600);
  pinMode(BUZZER,OUTPUT);
  pinMode(6,INPUT);
  pinMode(7,INPUT);

  //Timercounter setting
  TCCR2A |= (1<<COM2B1);
  TCCR2A |= (1<<WGM21) |(1<<WGM20);
  TCCR2B |= (1<<WGM22); // Fast PWM, TOP을 OCRA로 지정하였으므로, OCR2A의 값을 계산한다.
  TCCR2B |= (1<<CS22) | (1<<CS21) | (0<<CS20);

  OCR2A = F_CPU / 256 / frequency -1;
  OCR2B = OCR2A *DUTY/100;
}

void loop() {

  Serial.print(digitalRead(6));
  Serial.print(",");
  Serial.print(digitalRead(7));
  Serial.println();

  if((digitalRead(6) == 1) && (digitalRead(7) == 0)){
    buzzer_sound_mode1(DUTY);
  }else if((digitalRead(6) == 1) && (digitalRead(7) == 1)){
    buzzer_sound_mode2(DUTY);
  }else if((digitalRead(6) == 0) && (digitalRead(7) == 0)){
    DDRD &= ~(1<<BUZZER);
  }
}
