/* Buzzer code */
#define BUZZER 3 // buzzer output - PD3
#define frequency 440.00; //라 음계, 사람이 가장 잘 인지하는 주파수
volatile uint8_t DUTY = 1; // dB를 제어하기 위한 duty값 : (PWM / [5, 15, 30, 95] 4단계로 구분

//Delay 없이 부저를 제어하기 위해 시간을 측정하는 변수
unsigned long previousMillis = 0;
int flag =0; //0이 부저가 꺼져있다는 뜻, 1은 부저가 켜져있다는 뜻 -> Toggle을 위해 사용

//1차 경보 : 위험 감지 시(With partial braking), 띠 띠 띠 띠
void buzzer_sound_mode1(uint8_t DUTY){
  OCR2A = F_CPU / 256 / frequency -1; //make ra
  OCR2B = OCR2A *DUTY/100; //for dB, 이 값에 비례해서 데시벨 결정
  
  unsigned long currentMillis = millis();
  if ((currentMillis-previousMillis)>1000){
    flag=0; //현재시간과 기존 시간이 1초 이상 차이 나면, flag를 초기화한다, flag=1에서 부저가 종료되어도 1초 후에는 정상동작할 수 있음
  }

  if (flag==0){
    //pinMode(3,HIGH);
    if((currentMillis-previousMillis)>=40){
      previousMillis = currentMillis;
      DDRD |= (1<<BUZZER);//on
      toggle ^= 1;
    }
  }
  else{
    if((currentMillis-previousMillis)>=70){
      previousMillis = currentMillis;
      DDRD &= ~(1<<BUZZER);//off
      toggle ^= 1;
    }
  }
}

//2차 경보 : 충돌 예상 시(close at full braking), 띠----
void buzzer_sound_mode2(uint8_t DUTY){
  OCR2A = F_CPU / 256 / frequency -1;
  OCR2B = OCR2A *DUTY/100;
  
  OCR2B = OCR2A *DUTY/100 ;  // 왜 두번있지..?
  
  //pinMode(3,HIGH);
  DDRD |= (1<<BUZZER);

  // //Delay 10 * 2ms = 20ms, 20ms 동안 경보
  // unsigned long currentMillis = millis();

  // delay(2000); 딜레이가 없어도 문제 없는지 확인 필요
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
  
  //Reset 등 초기화 시 flag 초기화
  flag=0;
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
