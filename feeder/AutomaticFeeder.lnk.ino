#include <LiquidCrystal_I2C.h>
#include <string.h>
#include <avr/eeprom.h>


// пины шаговика - 2 ,4 , 6 , 8
int engine[4] = {2, 4, 6, 8};
int counter = 0; // для отладки было
// кнопка
const byte button = 12;
const byte button_input = 13; 
LiquidCrystal_I2C lcd(0x27,16,2);

// пока что  переменные 
// unsigned int portion ;
// unsigned int period ; // в секундах

// шаги вперед назад шаговика
const uint8_t f = 17;
const uint8_t b = 3;
//обновление экрана
const uint8_t frequency = 500;
// так проще записать в eeprom память
struct Options { 
  unsigned int portion; // кол-во кругов 
  unsigned int period;
};

Options Params;





void setup()
{ 
  Serial.begin(9600);
  for (int i; i <=3; i++) // настройка пинов шаговика
  { digitalWrite(engine[i],LOW);
    pinMode(engine[i],OUTPUT);
   }

  pinMode(button,INPUT); // настройка кнопки
  //pinMode(button_input,OUTPUT);
  digitalWrite(button,1); // по умолчанию на кнопке 1
  //digitalWrite(button_input,1);
  

  eeprom_read_block((void*)&Params, 0, sizeof(Params));  // читаем period и portion из энергонезависимой памяти (пока что каждый раз обновляется, тк так проверять проще)
  Params.portion = 1; 
  Params.period = 20;
  display_update(Params.portion,Params.period);

    

Serial.println("END OF SETUP");
}// КОНЕЦ SETUP


 uint32_t rate = 1000; // при  rate = 1000 переводит ожидание period из миллисек в сек
 uint32_t last = -1;
 uint32_t waiting = 0;
void loop() {

  
  if (digitalRead(button)==0)
  {
    button_handler();
    Serial.println("ПРОВЕРКА КНОПКИ:\nportion    period");
    Serial.print(Params.portion); Serial.print("        ") ;Serial.println(Params.period);
     
  }


  waiting = Params.period*rate; 

  if (  ( uint32_t(millis()-last) >= waiting ) ) // если пришло время 
  {
    
    Serial.println("portion:     period");
    last = (uint32_t)millis();
    Serial.print(Params.portion); Serial.print(" ") ;Serial.println(Params.period);
    Serial.println(f);
    motor_move(Params.portion*64*8*8/9, f , b); // 64*8 == 1 круг
  }
 if (millis()%frequency <= 10) {display_update(Params.portion,Params.period);}
  
}// КОНЕЦ







void move_motor_forward(int n)
  {   for (int t = 0 ; t <= n;  t++ )
        { // цикл n оборотов вперед
        for (int i = 0; i<=3; i++ )
          { // цикл 1 оборота
          digitalWrite(engine[i],HIGH);
          delayMicroseconds(3000); 
          digitalWrite(engine[i],LOW);
          }
        } 
  }



void move_motor_back(int n )
  {
      for (int t = 0 ; t < int(n);  t++ )
      { // кол во оборотов назад
        for (int i = 3; i>=0; i-=1 )
        {
          digitalWrite(engine[i],HIGH);
          delayMicroseconds(3000);
          digitalWrite(engine[i],LOW);
        }
      }
  
  }


int motor_move(int portion, int f, int b )
  { 
    Serial.println(portion/(f-b));
    for (int c = 0 ; c <= portion/(f-b); c++ )
    { 
      if (digitalRead(button)==0 ) {delay(1500); return ;} // можно нажать кнопку для остановки
       move_motor_forward(f);
       move_motor_back(b);
    }
  }


int button_handler()
  { // прибавяет к периоду "increment" сек если тап по кнопке или меняет portion если зажать больше чем на 2 сек
    static uint32_t last_call = millis() + 120;
    
    unsigned int port = 0;
    uint32_t start = millis();
    uint8_t increment = 20; // увеличивает period на 20
    uint32_t params_reset = 11*1000;
    uint16_t click_time = 2000; // в миллисек

    
    if ( last_call - millis() < 100 ) {return ; } // проверка на дребезг кнопки
    Serial.println(digitalRead(button));
    Serial.println(( millis() - start  ));
    while (digitalRead(button)==0 and ( ( millis() - start  ) <= params_reset ))
    
      {
        uint16_t seconds = millis() - start;
        Serial.print("seconds "); Serial.println(seconds);
        if ( seconds >= click_time and seconds%1000 <= 10 )
          {
           Serial.println("ПРОШЛА 1 сек");
           Params.portion+=1;
           display_update(Params.portion,Params.period);
          }
           
      }
      uint16_t seconds = millis() - start;
      if (seconds <= click_time)
      {
        Params.period+=increment;
      }
      else if (seconds >= params_reset) 
      {
        Params.period = 60;
        Params.portion = 0;
      }
     
    
    Params.period%=(12*3600);
    last_call = millis();
  } 
  



int display_update(uint16_t portion , uint16_t period)
  {
    static uint16_t value_portion = portion; 
    static uint16_t value_period = period;
    static String  text_portion = "eat:";
    static String  text_period = "delay:";
    static String  time_until_start = "after:";
    static LiquidCrystal_I2C lcd(0x27,16,2);
    static boolean called = false;
    if (not called) {lcd.init();}  // настройка дисплея если функц вызывается впервые
    else if (value_portion != portion or value_period != period )
      {
        value_portion  = portion;
        value_period = period;
      }
    lcd.clear();
    lcd.backlight();
    lcd.setCursor(0, 0); // 1 строка
    lcd.print(text_portion);
    lcd.print(value_portion);
    lcd.setCursor(0, 1);
    lcd.print(text_period);
    lcd.print(value_period);
    Serial.print("waiting - (mil()- last) "); Serial.println((waiting-( millis()-last ) )/1000);
    lcd.print(time_until_start); // 
    if ( (waiting-( millis()-last ) )/1000  != 4294966) {lcd.print((waiting-( millis()-last ) )/1000 );}
    
    called = true;
  }


void print_array(int *a,int N){
  for (int i; i<N; i++){
    Serial.print(i);Serial.print(": ") ; Serial.println(a[i]);
    
    }  
}
