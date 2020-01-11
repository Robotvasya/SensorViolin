
/* Current Ver 3.0*/

/*
 
Ver 1.0 Переделаны струны на сенсорную кнопку tpp223
Ver 2.0 Добавлены кнопки управления громкостью и перемотка треков
Ver 3.0 ДОБАВЛЕНЫ ПРЕРЫВАНИЯ И ЭНЕРГОСБЕРЕЖЕНИЕ

*/


#include <DFRobotDFPlayerMini.h>
#include "LowPower.h"  // https://github.com/rocketscream/Low-Power/
#include <SoftwareSerial.h>

// Переменные для обработки событий без delay():
static unsigned long timer = 0;
static unsigned long clktimer = 0;
static unsigned long clktimer4 = 0;
static unsigned long pwrtimer = 0;
long total1;

const uint8_t  pinBTN1 = 2, pinBTN2 = 3, pinBTN4 = 4;     // Определяем номер вывода к которому подключёна кнопка 1 (струны), кнопка 2, кнопка 4    // BTN=BTN1 лень было переделывать, после удаления одной кнопки
uint8_t  evnBTN = 0, evnBTN2 = 0,   evnBTN4 = 0;          // Определяем переменную для хранения состояний и событий кнопки 2, 4  и событий струн кнопка 1
uint8_t  statusBTN2 = 0;                // Определяем переменную для хранения текущего статуса кнопки 2

uint8_t  playCtrl = 0;                  // Определяем переменную для хранения переключения состояния плеера
uint8_t  playStat = 0;                  // Определяем переменную для хранения текущего статуса плеера



bool     pause = true;
bool     valBTN = false, valBTN2 = false, valBTN4 = false;  // Определяем переменную для хранения логического уровня 1,2 , струны
bool     startCtrl = false;                                 //Переменная для хранения статуса воспроизведения плеером
bool     lockBTN2 = false;                                  //Блокировка обработчика кнопки 2 до отпускания
bool     lockBTN4 = false;                                  //Блокировка обработчика кнопки 4 до отпускания
                    


//SoftwareSerial PIN 10, 11
SoftwareSerial SSerial(10, 11);//  RX, TX

DFRobotDFPlayerMini myDFPlayer;
int volume = 17;

void wakeUp()
{
  Serial.println(F("I'm awake!"));
}

void setup() {

  pinMode(pinBTN1, INPUT );                                      // Конфигурируем вывод к которому подключена кнопка 1 (tpp223)  как вход
  pinMode(pinBTN2, INPUT );                                      // Конфигурируем вывод к которому подключена кнопка 2 как вход
  pinMode(pinBTN4, INPUT );                                      // Конфигурируем вывод к которому подключена кнопка 4 как вход

  Serial.begin(115200);
  SSerial.begin(9600);
  delay (100);
  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));

  if (!myDFPlayer.begin(SSerial, true, false)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    printDetail(myDFPlayer.readType(), myDFPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
    while (true) {
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  Serial.println(F("DFPlayer Mini online."));
  if (myDFPlayer.available()) {
    printDetail(myDFPlayer.readType(), myDFPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
  }
  delay (100);
  
  //Настройки плеера
  myDFPlayer.setTimeOut(500); delay (100);
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD); delay (100);
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL); delay (100);
  myDFPlayer.volume(volume); delay (100);
  myDFPlayer.enableLoopAll(); delay (100);
  myDFPlayer.pause(); 
  delay (100);
}

void loop() {

  if (myDFPlayer.available()) {
    printDetail(myDFPlayer.readType(), myDFPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
  }

  //кнопка 1 это сенсорные Струны на TPP223 (см. описание проекта)
  valBTN = digitalRead(pinBTN1);                                 // Сохраняем логический уровень со входа pinBTN1 в переменную valBTN
  switch (evnBTN) {                                              // Определяем состояния и события кнопки
    case 0:  if ( valBTN) {
        evnBTN = 1;
        delay(10);
      } break;     // Фиксируем событие:   нажатие    (подавляем дребезг)
    case 2:  if (!valBTN) {
        evnBTN = 3;
        delay(50);
      } break;     // Фиксируем событие:   отпускание (подавляем дребезг)
    default: if ( valBTN) {
        evnBTN = 2;
      } else {
        evnBTN = 0;
      } break;     // Фиксируем состояние: удерживается или отпущена
  }

  //кнопка 2   перемотка   << и >> на одной тактовой кнопке
  valBTN2 = digitalRead(pinBTN2);                                 // Сохраняем логический уровень со входа pinBTN2 в переменную valBTN
  switch (evnBTN2) {                                              // Определяем состояния и события кнопки
    case 0:  if ( valBTN2) {
        evnBTN2 = 1;
        delay(10);
      } break;      // Фиксируем событие:   нажатие    (подавляем дребезг)
    case 2:  if (!valBTN2) {
        evnBTN2 = 3;
        delay(50);
      } break;     // Фиксируем событие:   отпускание (подавляем дребезг)
    default: if ( valBTN2) {
        evnBTN2 = 2;
      } else {
        evnBTN2 = 0;
      } break;    // Фиксируем состояние: удерживается или отпущена
  }                                                               //

  //кнопка 4   громкость     + и - повешаны на одну тактовую кнопку.
  valBTN4 = digitalRead(pinBTN4);                                 // Сохраняем логический уровень со входа pinBTN4 в переменную valBTN
  switch (evnBTN4) {                                              // Определяем состояния и события кнопки
    case 0:  if ( valBTN4) {
        evnBTN4 = 1;
        delay(10);
      } break;      // Фиксируем событие:   нажатие    (подавляем дребезг)
    case 2:  if (!valBTN4) {
        evnBTN4 = 3;
        delay(50);
      } break;     // Фиксируем событие:   отпускание (подавляем дребезг)
    default: if ( valBTN4) {
        evnBTN4 = 2;
      } else {
        evnBTN4 = 0;
      } break;    // Фиксируем состояние: удерживается или отпущена
  }


  /*Обработка сигналов с кнопки:*/

  // ---- Кнопка 2 перемотка BTN 2 повешано на отпускание, так как отрабатывается длительность нажатия кнопки
  if (evnBTN2 == 0) {
    /* КНОПКА 2 ОТПУЩЕНА     */
  }                // Код выполняется постоянно  при отпущенной кнопке
  if (evnBTN2 == 1) {
    lockBTN2 = true;  // Код выполняется однократно при нажимании на кнопку
    clktimer = millis(); /*clk=1; КНОПКА 2 НАЖИМАЕТСЯ*/
  }
  if (evnBTN2 == 2) {
    /* КНОПКА 2 УДЕРЖИВАЕТСЯ */
  }                // Код выполняется постоянно  при нажатой кнопке
  if (evnBTN2 == 3) {
    lockBTN2 = false;
    clktimer = millis() - clktimer;
    prevNext();
  } /* КНОПКА 2 ОТПУСКАЕТСЯ  */                // Код выполняется однократно при отпускании кнопки



  // ---- Кнопка 4 громкость BTN4 повешано на отпускание, так как отрабатывается длительность нажатия кнопки
  if (evnBTN4 == 0) {
    //Serial.println(F( "/* КНОПКА 4 ОТПУЩЕНА     */")); /* КНОПКА 4 ОТПУЩЕНА     */
  }                // Код выполняется постоянно  при отпущенной кнопке
  if (evnBTN4 == 1) {
    lockBTN4 = true;  // Код выполняется однократно при нажимании на кнопку
    clktimer4 = millis();
    // Serial.println(F( "/* КНОПКА 4 НАЖИМАЕТСЯ*/"));
  }
  if (evnBTN4 == 2) {
    // Serial.println(F( "/* КНОПКА 4 УДЕРЖИВАЕТСЯ*/"));
  }                // Код выполняется постоянно  при нажатой кнопке
  if (evnBTN4 == 3) {
    lockBTN4 = false;
    clktimer4 = millis() - clktimer4;
    volCtrl();
    //Serial.println(F( "/* КНОПКА 4 ОТПУСКАЕТСЯ*/"));
  } /* КНОПКА 4 ОТПУСКАЕТСЯ  */                // Код выполняется однократно при отпускании кнопки




  //------  СТРУНЫ  ----- BTN1
  if (evnBTN == 0) {   // Код выполняется постоянно  при отпущенной кнопке
    /*Serial.println(" КНОПКА ОТПУЩЕНА") ;*/
  }
  if (evnBTN == 1) {  // Код выполняется однократно при нажимании на кнопку
    // Serial.println(F("/* КНОПКА НАЖИМАЕТСЯ   */"));  // Отладка
    playCtrl = 1;
    startCtrl = false;
  }
  if (evnBTN == 2) {    // Код выполняется постоянно  при нажатой кнопке
    /*Serial.println("КНОПКА УДЕРЖИВАЕТСЯ ") ;*/
  }
  if (evnBTN == 3) {   // Код выполняется однократно при отпускании кнопки}
    // Serial.println(F("Таймер запущен /* КНОПКА ОТПУСКАЕТСЯ  */"));   // Отладка
    startCtrl = true;
    timer = millis();
  }

  //Выполняем обработчик постановки плеера на паузу 
  pauseDelay();

  if (pause == true && playCtrl == 1) {
    myDFPlayer.start();
    pause = false;
  }

  // сбрасываем начало счетчика ухода в сон, если идет воспроизведение
  if (!pause) {
    pwrtimer = millis();
  }

  // режим сна (энергосбережения) по событию Х секунд с момента когда включилась пауза и ничего больше не происходило
  if (pause == true && ((millis() - pwrtimer) > 10000)) {
    // Serial.println(pwrtimer); // Отладка выводим значение таймера работы
    sleepNow();
  }

}

/*=== ПЕРЕМОТКА ТРЕКОВ ===*/
void prevNext() {
  if (clktimer < 700 && pause && !lockBTN2) { //если короткое нажатие кнопки
    myDFPlayer.next();                        //переключаем трек на следующий
    //  Serial.println(F("Next>")); // Отладка
    myDFPlayer.pause();                       //сразу ставим на паузу
  }

  if (clktimer >= 700 && pause && !lockBTN2) {  //если долгое удержание кнопки
    myDFPlayer.previous();                      //переключаем трек на предыдущий
    // Serial.println(F("Prev>")); // Отладка
    myDFPlayer.pause();                         //сразу ставим на паузу
  }
}

/*=== Переключение горомкости ===*/
void volCtrl() {
  if (clktimer4 < 600 && !lockBTN4 && volume <= 22 ) { //если короткое нажатие кнопки прибавляем громкость
    ++volume;
    myDFPlayer.volume(volume);
    //Serial.println(F("vol+"));
  }

  if (clktimer4 >= 600 && !lockBTN4 && volume >= 3) { //если долгое удержание кнопки убавляем громкость. сразу на 3 единицы за одно нажатие для скорости и поэтому мин значение не меньше 3, чтоб было из чего вычитать
    volume = volume - 3;
    myDFPlayer.volume(volume);
    //Serial.println(F("vol-"));
  }
}

/*== Обработчик паузы ==*/
void pauseDelay() {
  if (startCtrl) {
    if (millis() - timer > 600) {
      myDFPlayer.pause();  //Pause after 600ms.
      playCtrl = 0;
      pause = true;
      startCtrl = false;
      // Serial.print(F("Timer: ")); Serial.println(timer); // Отладка таймера паузы
    }
  }
}

/*===Обработка ошибок DFPlayer===*/
void printDetail(uint8_t type, int value) {
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerUSBInserted:
      Serial.println(F("USB Inserted!"));
      break;
    case DFPlayerUSBRemoved:
      Serial.println(F("USB Removed!"));
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

/*
 == Функция увода ардуины в сон. Библиотека и ее описание:  ==
 == https://github.com/rocketscream/Low-Power/              ==
*/

void sleepNow()  {       

  //Serial.println(F("I go to sleep")); //Отладка

  attachInterrupt(0, wakeUp, HIGH);
  // Enter power down state with ADC and BOD module disabled.
  // Wake up when wake up pin is low.
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
  // Disable external pin interrupt on wake up pin.
  detachInterrupt(0);
} 

//Увод в сон DFPlayer оключен а код удален из скетча, поскольку мой модуль не выходил из сна 
