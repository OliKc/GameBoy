/*********************************************************************
  This is an example sketch for our Monochrome Nokia 5110 LCD Displays

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/338

  These displays use SPI to communicate, 4 or 5 pins are required to
  interface

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada  for Adafruit Industries.
  BSD license, check license.txt for more information
  All text above, and the splash screen must be included in any redistribution
*********************************************************************/

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

// Software SPI (slower updates, more flexible pin options):
// pin 7 - Serial clock out (SCLK)
// pin 6 - Serial data out (DIN)
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);

// Hardware SPI (faster, but must use certain hardware pins):
// SCK is LCD serial clock (SCLK) - this is pin 13 on Arduino Uno
// MOSI is LCD DIN - this is pin 11 on an Arduino Uno
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
// Adafruit_PCD8544 display = Adafruit_PCD8544(5, 4, 3);
// Note with hardware SPI MISO and SS pins aren't used but will still be read
// and written to during SPI transfer.  Be careful sharing these pins!


#define AREAWIDTH 84
#define AREAHEIGHT 47


const byte UP_DIRECTION = 0;
const byte RIGHT_DIRECTION = 1;
const byte DOWN_DIRECTION = 2;
const byte LEFT_DIRECTION = 3;

long previousMillis = 0;
long interval = 2000;


//D pad pins
const byte UP = 8;
const byte RIGHT = 9;
const byte DOWN = 10;
const byte LEFT = 11;


byte buttonState = 0;


//direction flags w/ initial flag
boolean upward = false, rightward = true, downward = false, leftward = false;

//snake body properties
byte x[150], y[150], d[150], snakeLength = 6;

int Buzzer;

#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16


void setup()   {
  pinMode(UP, INPUT);
  pinMode(RIGHT, INPUT);


  pinMode(13, OUTPUT); //LED

  pinMode(Buzzer, OUTPUT);  //Buzzer pin

  Serial.begin(9600);


  display.begin();
  display.clearDisplay();
  display.setContrast(55);


  //draw horizontal borders
  display.drawLine(0, 0, AREAWIDTH - 1, 0, BLACK);
  display.drawLine(0, AREAHEIGHT - 1, AREAWIDTH - 1, AREAHEIGHT - 1, BLACK);

  //draw vertical borders
  display.drawLine(1, 0, 1, AREAHEIGHT - 1, BLACK);
  display.drawLine(AREAWIDTH - 2, 0, AREAWIDTH - 2, AREAHEIGHT - 1, BLACK);

  int i;
  //initial snake position
  for (i = 0; i <= snakeLength; i++)
  {
    x[i] = 32 - i * 5;  //2+5n
    y[i] = 21;          //1+5n
  }

  //draw head
  display.fillRoundRect(x[0], y[0] + 1, 5, 3, 1, BLACK);
  d[0] = RIGHT_DIRECTION;

  //draw body
  for (i = 1; i < snakeLength - 1; i++)
  {
    display.drawPixel(x[i], y[i] + 2, BLACK);
    display.fillRect(x[i] + 1, y[i] + 1, 4, 3, BLACK);
    d[i] = RIGHT_DIRECTION;
  }

  //draw tail
  display.fillRect(x[snakeLength - 1], y[snakeLength - 1] + 2, 2, 1, BLACK);
  display.fillRect(x[snakeLength - 1] + 2, y[snakeLength - 1] + 1, 3, 3, BLACK);
  d[snakeLength - 1] = RIGHT_DIRECTION;


  display.display();
}


void loop() {

  steering();

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis > interval)
  {
    previousMillis = currentMillis;

    //Serial.print("flag\n");
    moveSnake();
    drawSnake();
  }

}


void steering()
{
  if (digitalRead(UP) == HIGH) {
    upward = true;
    rightward = false;
    downward = false;
    leftward = false;
  }
  else if (digitalRead(RIGHT) == HIGH) {
    rightward = true;
    upward = false;
    downward = false;
    leftward = false;
  }
  else if (digitalRead(DOWN) == HIGH) {
    downward = true;
    upward = false;
    rightward = false;
    leftward = false;
  }
  else if (digitalRead(LEFT) == HIGH) {
    leftward = true;
    upward = false;
    rightward = false;
    downward = false;
  }
}

void moveSnake()
{
  byte tempx, tempy, tempd;

  //if wrong key pressed - impossible move
  if (d[0] == UP_DIRECTION && downward == true) {
    downward = false;
    upward = true;
//    Serial.println('a');
  }
  else if (d[0] == RIGHT_DIRECTION && leftward == true) {
    leftward = false;
    rightward = true;
//    Serial.println('b');
  }
  else if (d[0] == DOWN_DIRECTION && upward == true) {
    upward = false;
    downward = true;
//    Serial.println('c');
  }
  else if (d[0] == LEFT_DIRECTION && rightward == true) {
    rightward = false;
    leftward = true;
//    Serial.println('d');
  }

  //  Serial.println(d[0]);
    Serial.println(upward);
    Serial.println(rightward);
    Serial.println(downward);
    Serial.println(leftward);

  //move up
  if (upward == true)
  {
    //Serial.print("upward\n");
    tempx = x[0];
    tempy = y[0] - 5;
    tempd = UP_DIRECTION;
  }
  //move right
  else if (rightward == true)
  {
    //Serial.print("rightward\n");
    tempx = x[0] + 5;
    tempy = y[0];
    tempd = RIGHT_DIRECTION;
  }
  //move down
  else if (downward == true)
  {
    //Serial.print("downward\n");
    tempx = x[0];
    tempy = y[0] + 5;
    tempd = DOWN_DIRECTION;
  }
  //move left
  else if (leftward == true)
  {
    //Serial.print("leftward\n");
    tempx = x[0] - 5;
    tempy = y[0];
    tempd = LEFT_DIRECTION;
  }


  //snake position shift
  int i;
  for (i = 0; i <= snakeLength; i++)
  {
    byte xx = x[i];
    byte yy = y[i];
    byte dd = d[i];
    x[i] = tempx;
    y[i] = tempy;
    d[i] = tempd;
    tempx = xx;
    tempy = yy;
    tempd = dd;
  }

  Serial.print('\n');

}

void drawSnake()
{
  //delete tail
  display.fillRect(x[snakeLength], y[snakeLength], 5, 5, WHITE);


  //draw head
  display.fillRect(x[0], y[0], 5, 5, WHITE);
  if (d[0] == UP_DIRECTION)
  {
    //    Serial.print("head UP\n");
    display.fillRoundRect(x[0] + 1, y[0], 3, 5, 1, BLACK);
  }
  else if (d[0] == RIGHT_DIRECTION)
  {
    //    Serial.print("head RIGHT\n");
    display.fillRoundRect(x[0], y[0] + 1, 5, 3, 1, BLACK);
  }
  else if (d[0] == DOWN_DIRECTION)
  {
    //    Serial.print("head DOWN\n");
    display.fillRoundRect(x[0] + 1, y[0], 3, 5, 1, BLACK);
  }
  else if (d[0] == LEFT_DIRECTION)
  {
    //    Serial.print("head LEFT\n");
    display.fillRoundRect(x[0], y[0] + 1, 5, 3, 1, BLACK);
  }
  else
  {
    //error
    Serial.print("head error\n");
  }


  //draw body
  byte i;
  for (i = 1; i < snakeLength - 1; i++)
  {
    display.fillRect(x[i], y[i], 5, 5, WHITE);

    if (d[i] == UP_DIRECTION)
    {
      //      Serial.print("UP\n");
      //      Serial.println(x[i]);
      //      Serial.println(y[i]);
      display.fillRect(x[i] + 1, y[i], 3, 4, BLACK);
      display.drawPixel(x[i] + 2, y[i] + 4, BLACK);
    }
    else if (d[i] == RIGHT_DIRECTION)
    {
      //      Serial.print("RIGHT\n");
      //      Serial.println(x[i]);
      //      Serial.println(y[i]);
      display.drawPixel(x[i], y[i] + 2, BLACK);
      display.fillRect(x[i] + 1, y[i] + 1, 4, 3, BLACK);
    }
    else if (d[i] == DOWN_DIRECTION)
    {
      //      Serial.print("DOWN\n");
      //      Serial.println(x[i]);
      //      Serial.println(y[i]);
      display.drawPixel(x[i] + 2, y[i], BLACK);
      display.fillRect(x[i] + 1, y[i] + 1, 3, 4, BLACK);
    }
    else if (d[i] == LEFT_DIRECTION)
    {
      //      Serial.print("LEFT\n");
      //      Serial.println(x[i]);
      //      Serial.println(y[i]);
      display.fillRect(x[i], y[i] + 1, 4, 3, BLACK);
      display.drawPixel(x[i] + 4, y[i] + 2, BLACK);
    }
    else
    {
      //error
      Serial.print("body error\n");
    }
  }

  //draw tail
  display.fillRect(x[snakeLength - 1], y[snakeLength - 1], 5, 5, WHITE);
  if (d[snakeLength - 1] == UP_DIRECTION)
  {
    //    Serial.print("tail UP\n");
    display.fillRect(x[snakeLength - 1] + 1, y[snakeLength - 1], 3, 3, BLACK);
    display.fillRect(x[snakeLength - 1] + 2, y[snakeLength - 1] + 3, 1, 2, BLACK);
  }
  else if (d[snakeLength - 1] == RIGHT_DIRECTION)
  {
    //    Serial.print("tail RIGHT\n");
    display.fillRect(x[snakeLength - 1], y[snakeLength - 1] + 2, 2, 1, BLACK);
    display.fillRect(x[snakeLength - 1] + 2, y[snakeLength - 1] + 1, 3, 3, BLACK);
  }
  else if (d[snakeLength - 1] == DOWN_DIRECTION)
  {
    //    Serial.print("tail DOWN\n");
    display.fillRect(x[snakeLength - 1] + 2, y[snakeLength - 1], 1, 2, BLACK);
    display.fillRect(x[snakeLength - 1] + 1, y[snakeLength - 1] + 2, 3, 3, BLACK);
  }
  else if (d[snakeLength - 1] == LEFT_DIRECTION)
  {
    //    Serial.print("tail LEFT\n");
    display.fillRect(x[snakeLength - 1], y[snakeLength - 1] + 1, 3, 3, BLACK);
    display.fillRect(x[snakeLength - 1] + 3, y[snakeLength - 1] + 2, 2, 1, BLACK);
  }
  else
  {
    //error
    Serial.print("tail error\n");
  }


  display.display();
}


