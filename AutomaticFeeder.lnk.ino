#include <string.h>
#include <avr/eeprom.h>
// пины шаговика - 2 ,4 , 6 , 8
int engine[4] = {2, 4, 6, 8};
int counter = 0; // для отладки было
// кнопка
const byte button = 12;
const byte button_input = 13; 

// пока что  переменные 
// unsigned int portion ;
// unsigned int period ; // в секундах

// шаги вперед назад шаговика
const uint8_t f = 7;
const uint8_t b = 2;

// так проще записать в eeprom память
struct Options { 
  unsigned int portion;
  unsigned int period;
};

Options Params;




// SЕТУП БЛИН
void setup()
{ 
  Serial.begin(9600);
  for (int i; i <=3; i++)
  { digitalWrite(engine[i],LOW);
    pinMode(engine[i],OUTPUT);
   }

  pinMode(button,INPUT);
  pinMode(button_input,OUTPUT);
  digitalWrite(button,0);
  digitalWrite(button_input,1);
  

  eeprom_read_block((void*)&Params, 0, sizeof(Params));
  
    

Serial.println("END OF SETUP");
}// КОНЕЦ SETUP

 uint32_t last = 0;
 uint32_t waiting = 0;
 uint32_t kk = 1000;
void loop() {


  
  if (digitalRead(button))
  {
    button_handler();
    Serial.println(Params.portion); Serial.println(Params.period);Serial.println("ПРОВЕРКА КНОНПКИ");
    
  }
  //Serial.println(waiting);
  waiting = Params.period*kk;
  if ( uint32_t(millis()-last) >= waiting )
  {
    Serial.println( uint32_t(millis()-last));
    Serial.println(waiting);
Serial.println(Params.period);
    Serial.println("конец дебаг 1)делта 2)условие");
    last = (uint32_t)millis();
    Serial.println(Params.portion); Serial.println(Params.period);
    motor_move(Params.portion, f , b);
  }
 
  
}// КОНЕЦ







void move_motor_back(int n)
  {   for (int t = 0 ; t <= n;  t++ )
        { // цикл n оборотов вперед
        for (int i = 0; i<=3; i++ )
          { // цикл 1 оборота
          digitalWrite(engine[i],HIGH);
          delayMicroseconds(2000); // лучше использовать асинхрон. делей
          digitalWrite(engine[i],LOW);
          }
        } 
  }



void move_motor_forward(int n )
  {
      for (int t = 0 ; t <= int(n);  t++ )
      { // кол во оборотов назад
        for (int i = 3; i>=0; i-=1 )
        {
          digitalWrite(engine[i],HIGH);
          delayMicroseconds(2000);
          digitalWrite(engine[i],LOW);
        }
      }
  
  }


void motor_move(int portion, int f, int b )
  {
    for (int c = 0 ; c <= portion; c++ )
    {
       move_motor_forward(f);
       move_motor_back(b);
    }
  }


int button_handler()
  { // прибавяет к периоду +10 сек если тап по кнопке или меняет portion если зажать больше чем на 20 сек
    static uint32_t last_call = millis() + 120;
    
    unsigned int port = 0;
    uint32_t start = millis();
    uint8_t increment = 20;
    uint32_t params_reset = 20*1000;
    uint16_t click_time = 2000; // в миллисек

    
    if ( last_call - millis() < 100 ) {return 10; }
    Serial.println(digitalRead(button));
    Serial.println(( millis() - start  ));
    while (digitalRead(button) and ( ( millis() - start  ) <= params_reset ))
    
      {
        //Serial.println("ЖДУМС");
        uint16_t seconds = millis() - start;
        Serial.print("seconds "); Serial.println(seconds);
        if ( seconds >= click_time )
          {
           // VRUM VRUM
           motor_move(5, f , b);
           port+= 5;
          }
           
      }
      uint16_t seconds = millis() - start;
      if (seconds <= click_time)
      {
        Params.period+=increment;
      }
      else if (seconds >= params_reset) //  было <= params_reset но я тупняк словил астаньте
      {
        Params.period = 10;
        Params.portion = 0;
      }
     
    
    if (seconds >=click_time and seconds < params_reset){Params.portion = port;}
    
    Params.period%=(12*3600);
    last_call = millis();
  } 
  






void print_array(int *a,int N){
  for (int i; i<N; i++){
    Serial.print(i);Serial.print(": ") ; Serial.println(a[i]);
    
    }  
}
