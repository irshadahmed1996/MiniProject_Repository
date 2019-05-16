#include <avr/io.h>

#include<util/delay.h>

#include <avr/sfr_defs.h>

#include<avr/interrupt.h>

#include<LiquidCrystal.h>

#define SET_BIT(PORT, BIT) PORT |= (1 << BIT)
#define CLR_BIT(PORT, BIT) PORT &= ~(1 << BIT)
LiquidCrystal lcd(13, 12, 11, 10, 9, 8);
float distance = 0;
float distance1 = 0;
int U_Turn_speed = 10;
int hospital_speed = 20;
int school_speed = 30;
int city_limit = 40;
int Normal_speed = 60;
uint16_t adcValue = 0;
int c = 1;
struct {

  volatile unsigned int ISR0: 1;

}
FLAG_BIT;

void INIT_T0_ON() {
  TCCR0A |= (1 << WGM01);
  TCCR0B &= ~((1 << CS00) | (1 << CS01));
  TCCR0B |= (1 << CS02);
  OCR0A = 255;
  OCR0B = 255;
  TIMSK0 |= (1 << OCIE0A) | (1 << OCIE0B);
}

void adcInit() {
  ADCSRA |= (1 << ADEN);
}

void adcRead() {
  ADCSRA |= (1 << ADSC);
  while (ADCSRA & (1 << ADSC));
  adcValue = ADC;
}

void Setup() {
  Serial.begin(9600);
}

int main(void) {
  int left, right, max, sign;
  Serial.begin(9600);
  SET_BIT(DDRD, PD1);
  SET_BIT(DDRD, PD5);
  CLR_BIT(DDRD, PD2);
  SET_BIT(PORTD, PD2);
  CLR_BIT(DDRD, PD4); // Back Echo pin input
  SET_BIT(DDRD, PD6); //motor
  CLR_BIT(DDRD, PD3); // Front Echo pin input
  EICRA |= (1 << ISC00);
  EIMSK |= (1 << INT0);
  SREG |= (1 << 7);

  while (1) {
    if (FLAG_BIT.ISR0 == 1) {
      lcd.clear();
      SET_BIT(DDRD, PD4);
      SET_BIT(PORTD, PD4);
      _delay_ms(5);
      CLR_BIT(PORTD, PD4);
      CLR_BIT(DDRD, PD4);
      float time_duration = pulseIn(PD4, HIGH);
      distance = 0.343 * time_duration * 0.5 * 0.1;
      // Serial.println(distance);

      if (distance < 100) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Distance=");
        lcd.setCursor(9, 1);
        lcd.print(distance);
        lcd.print("cm");
        _delay_ms(1000);
      }
      if (distance <= 50) {
        SET_BIT(PORTD, PD5);
      } else
        CLR_BIT(PORTD, PD5);

      SET_BIT(DDRD, PD3);
      SET_BIT(PORTD, PD3);
      _delay_ms(5);
      CLR_BIT(PORTD, PD3);
      CLR_BIT(DDRD, PD3);
      float time_duration1 = pulseIn(PD3, HIGH);
      distance1 = 0.343 * time_duration1 * 0.5 * 0.1;
      if (distance1 > 100) {
        SET_BIT(PORTD, PD6);
        CLR_BIT(PORTD, PD5);
      } else {
        SET_BIT(PORTD, PD5);
        CLR_BIT(PORTD, PD6);
      }
      ADMUX = 0X00;
      ADMUX |= (1 << REFS0);
      adcInit();
      adcRead();
      left = adcValue;
      adcValue = 0;
      // Serial.println('left');
      //  Serial.println(left);

      ADMUX = 0X00;
      ADMUX |= (1 << REFS0);
      ADMUX |= (1 << MUX1);
      adcInit();
      adcRead();
      right = adcValue;
      Serial.println(adcValue);
      adcValue = 0;
      Serial.println(adcValue);
      // Serial.println('right');
      // Serial.println(right);
      _delay_ms(500);
      //sei();

      if (left > right)
        max = left;
      else
        max = right;

      //Serial.println(max);
      //Serial.println(right);
      //Serial.println(left);
      if (max > 512) {

        if (max == left && right < 512) //left lane condtion
        {
          lcd.clear();
          lcd.setCursor(0, 1);
          lcd.begin(16, 2);
          lcd.print("lane detected");
          _delay_ms(1000);
          lcd.begin(16, 2);
          lcd.print("left lane");
          lcd.setCursor(0, 1);
          lcd.print("detected");
          _delay_ms(1000);

        }
        if (max == left && right > 512) {
          lcd.clear();
          lcd.setCursor(0, 1);
          lcd.begin(16, 2);
          lcd.print("lane interrupted");
          _delay_ms(1000);
          lcd.begin(16, 2);
          lcd.print("vehicle turning");
          lcd.setCursor(0, 1);
          lcd.print("left");
          _delay_ms(1000);

        }

        if (max == right && left < 512) //left lane condtion
        {
          lcd.clear();
          lcd.setCursor(0, 1);
          lcd.begin(16, 2);
          lcd.print("lane detected");
          _delay_ms(1000);
          lcd.begin(16, 2);
          lcd.print("right lane");
          lcd.setCursor(0, 1);
          lcd.print("detected");
          _delay_ms(1000);

        }
        if (max == right && left > 512) {
          lcd.clear();
          lcd.setCursor(0, 1);
          lcd.begin(16, 2);
          lcd.print("lane interrupted");
          _delay_ms(1000);
          lcd.begin(16, 2);
          lcd.print("vehicle turning");
          lcd.setCursor(0, 1);
          lcd.print("right");
          _delay_ms(1000);

        }
        left = 0;
        right = 0;
        max = 0;
      }
      INIT_T0_ON();
      ADMUX = 0X00;
      ADMUX |= (1 << REFS0);
      ADMUX |= (1 << MUX0);
      adcInit();
      adcRead();
      sign = adcValue;
      adcValue = 0;
      if (sign > 0 && sign <= 200) {
        Serial.println("U-Turn");
        // OCR0B=50;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("U Turn Sign");
        _delay_ms(1000);
        lcd.setCursor(0, 1);
        lcd.print("SPEED :");
        lcd.print(U_Turn_speed);
        lcd.print("kmph");
        _delay_ms(1000);

      } else if (sign >= 201 && sign <= 400) {
        Serial.println("Hospital");
        OCR0B = 100;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Hospital-signal");
        _delay_ms(1000);
        lcd.setCursor(0, 1);
        lcd.print("SPEED :");
        lcd.print(hospital_speed);
        lcd.print("kmph");
        _delay_ms(1000);
      } else if (sign >= 401 && sign <= 600) {
        Serial.println("School");
        OCR0B = 150;
        lcd.clear();
        lcd.print("School-signal");
        _delay_ms(1000);
        lcd.setCursor(0, 1);
        lcd.print("SPEED :");
        lcd.print(school_speed);
        lcd.print("kmph");
        _delay_ms(1000);
      } else if (sign >= 601 && sign <= 800) {
        Serial.println("City limit");
        OCR0B = 200;
        lcd.clear();
        lcd.print("City Limit");
        _delay_ms(1000);
        lcd.setCursor(0, 1);
        lcd.print("SPEED :");
        lcd.print(city_limit);
        lcd.print("kmph");
        _delay_ms(1000);
      } else {

        SET_BIT(PORTD, PD6);

      }

    } else
      CLR_BIT(PORTD, PD6);
  }
  return 0;
}

ISR(INT0_vect) //interrupt service routine when there is a change in logic level
{
  c++;
  if (c % 2 == 0)
    FLAG_BIT.ISR0 = 1;
  else
    FLAG_BIT.ISR0 = 0;
}

ISR(TIMER0_COMPA_vect) {
  SET_BIT(PORTD, PORTD6);
}
ISR(TIMER0_COMPB_vect) {

  CLR_BIT(PORTD, PORTD6);
}
