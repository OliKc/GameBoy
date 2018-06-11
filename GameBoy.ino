#include <SPI.h>
#include <Adafruit_GFX.h>
//Written by Limor Fried/Ladyada  for Adafruit Industries.
#include <Adafruit_PCD8544.h>

// Software SPI (slower updates, more flexible pin options):
// pin 7 - Serial clock out (SCLK)
// pin 6 - Serial data out (DIN)
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);

#define AREAWIDTH 84
#define AREAHEIGHT 47     //1px trimmed

#define SEGMENTSIZE 5
#define SEGMENTCOUNT 144  //16*9

//D pad pins
#define UP 8
#define RIGHT 9
#define DOWN 10
#define LEFT 11

#define UP_DIRECTION 0
#define RIGHT_DIRECTION 1
#define DOWN_DIRECTION 2
#define LEFT_DIRECTION 3

#define INITIAL_SNAKE_SIZE 6

long previousMillis = 0;
int interval = 1000;




byte score = 0;

byte buttonState = 0;

boolean paused = false;

//direction flags w/ initial flag
boolean upward = false, rightward = true, downward = false, leftward = false;

//snake body properties
byte x[150], y[150], d[150], snakeSize = INITIAL_SNAKE_SIZE;

int Buzzer;

#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16


void setup()   {
  pinMode(UP, INPUT);
  pinMode(RIGHT, INPUT);
  pinMode(DOWN, INPUT);
  pinMode(LEFT, INPUT);


  pinMode(13, OUTPUT); //LED

  pinMode(Buzzer, OUTPUT);  //Buzzer pin

  Serial.begin(9600);


  display.begin();
  display.clearDisplay();
  display.setContrast(55);


  //draw horizontal borders
  display.drawLine(1, 0, AREAWIDTH - 2, 0, BLACK);
  display.drawLine(1, AREAHEIGHT - 1, AREAWIDTH - 2, AREAHEIGHT - 1, BLACK);

  //draw vertical borders
  display.drawLine(1, 0, 1, AREAHEIGHT - 1, BLACK);
  display.drawLine(AREAWIDTH - 2, 0, AREAWIDTH - 2, AREAHEIGHT - 1, BLACK);


  //initial snake position
  int i;
  for (i = 0; i < snakeSize; i++)
  {
    x[i] = (6 * SEGMENTSIZE + 2) - i * 5; //2+5n - 5i
    y[i] = (4 * SEGMENTSIZE + 1);         //1+5n
  }

  //draw start head
  display.fillRoundRect(x[0], y[0] + 1, 5, 3, 1, BLACK);
  d[0] = RIGHT_DIRECTION;

  //draw start body
  for (i = 1; i < snakeSize - 1; i++)
  {
    display.drawPixel(x[i], y[i] + 2, BLACK);
    display.fillRect(x[i] + 1, y[i] + 1, 4, 3, BLACK);
    d[i] = RIGHT_DIRECTION;
  }

  //draw start tail
  display.fillRect(x[snakeSize - 1], y[snakeSize - 1] + 2, 2, 1, BLACK);
  display.fillRect(x[snakeSize - 1] + 2, y[snakeSize - 1] + 1, 3, 3, BLACK);
  d[snakeSize - 1] = RIGHT_DIRECTION;


  display.display();
}


void loop()
{
  steering();

  if (!paused)
  {
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis > interval)
    {
      previousMillis = currentMillis;

      //Serial.print("flag\n");
      moveSnake();
      if (!collision())
      {
        drawSnake();
        addEgg();
      }
      else
      {
        paused = true;
      }
    }
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
  byte newx, newy, newd;

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


  //move up
  if (upward == true)
  {
    //Serial.print("upward\n");
    if (y[0] - SEGMENTSIZE < 1) {
      newy = AREAHEIGHT - SEGMENTSIZE - 1;
    }
    else {
      newy = y[0] - SEGMENTSIZE;
    }
    newx = x[0];
    newd = UP_DIRECTION;
  }

  //move right
  else if (rightward == true)
  {
    //Serial.print("rightward\n");
    if (x[0] + SEGMENTSIZE > AREAWIDTH - SEGMENTSIZE - 2) {
      newx = 2;
    }
    else {
      newx = x[0] + SEGMENTSIZE;
    }
    newy = y[0];
    newd = RIGHT_DIRECTION;
  }

  //move down
  else if (downward == true)
  {
    //Serial.print("downward\n");
    if (y[0] + SEGMENTSIZE > AREAHEIGHT - SEGMENTSIZE - 1) {
      newy = 1;
    }
    else {
      newy = y[0] + SEGMENTSIZE;
    }
    newx = x[0];
    newd = DOWN_DIRECTION;
  }

  //move left
  else if (leftward == true)
  {
    //Serial.print("leftward\n");
    if (x[0] - SEGMENTSIZE < 2) {
      newx = AREAWIDTH - SEGMENTSIZE - 2;
    }
    else {
      newx = x[0] - SEGMENTSIZE;
    }
    newy = y[0];
    newd = LEFT_DIRECTION;
  }


  //snake position shift
  byte i;
  for (i = 0; i <= snakeSize; i++)
  {
    byte _x = x[i];
    byte _y = y[i];
    byte _d = d[i];
    x[i] = newx;
    y[i] = newy;
    d[i] = newd;
    newx = _x;
    newy = _y;
    newd = _d;
  }

  //Serial.print('\n');

}

boolean collision()
{
  //check head collision
  byte i;
  for (i = 1; i < snakeSize; i++)
  {
    if (x[i] == x[0] && y[i] == y[0])
    {
      //game over
      score = snakeSize - INITIAL_SNAKE_SIZE;

      display.fillRect(14, 14, 55, 18, WHITE);

      display.setTextColor(BLACK);
      display.setTextSize(1);

      display.setCursor(15, 15);
      display.print("Game Over");

      display.setCursor(15, 24);
      if (score < 10) {
        display.print("score:  ");
      }
      else if (score < 100) {
        display.print("score: ");
      }
      else {
        display.print("score:");
      }
      display.print(score);

      display.display();

      return true;
    }
  }
  return false;
}

void drawSnake()
{
  //clear excess tail
  display.fillRect(x[snakeSize], y[snakeSize], 5, 5, WHITE);

  //reset excess tail properties
  x[snakeSize] = 0; //or :=NULL
  y[snakeSize] = 0; //or :=NULL


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
  for (i = 1; i < snakeSize - 1; i++)
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
  display.fillRect(x[snakeSize - 1], y[snakeSize - 1], 5, 5, WHITE);
  if (d[snakeSize - 1] == UP_DIRECTION)
  {
    //    Serial.print("tail UP\n");
    display.fillRect(x[snakeSize - 1] + 1, y[snakeSize - 1], 3, 3, BLACK);
    display.fillRect(x[snakeSize - 1] + 2, y[snakeSize - 1] + 3, 1, 2, BLACK);
  }
  else if (d[snakeSize - 1] == RIGHT_DIRECTION)
  {
    //    Serial.print("tail RIGHT\n");
    display.fillRect(x[snakeSize - 1], y[snakeSize - 1] + 2, 2, 1, BLACK);
    display.fillRect(x[snakeSize - 1] + 2, y[snakeSize - 1] + 1, 3, 3, BLACK);
  }
  else if (d[snakeSize - 1] == DOWN_DIRECTION)
  {
    //    Serial.print("tail DOWN\n");
    display.fillRect(x[snakeSize - 1] + 2, y[snakeSize - 1], 1, 2, BLACK);
    display.fillRect(x[snakeSize - 1] + 1, y[snakeSize - 1] + 2, 3, 3, BLACK);
  }
  else if (d[snakeSize - 1] == LEFT_DIRECTION)
  {
    //    Serial.print("tail LEFT\n");
    display.fillRect(x[snakeSize - 1], y[snakeSize - 1] + 1, 3, 3, BLACK);
    display.fillRect(x[snakeSize - 1] + 3, y[snakeSize - 1] + 2, 2, 1, BLACK);
  }
  else
  {
    //error
    Serial.print("tail error\n");
  }

  display.display();
}

void addEgg()
{
//  for (;;)
//  {
//    
//  }
}


