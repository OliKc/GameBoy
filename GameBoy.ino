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

#define SEGMENT_SIZE 5
#define SEGMENTS_WIDTH 16
#define SEGMENTS_HEIGHT 9
#define SEGMENT_COUNT 144 //16*9

//D pad pins
#define UP 8
#define RIGHT 9
#define DOWN 10
#define LEFT 11

#define UP_DIRECTION 0
#define RIGHT_DIRECTION 1
#define DOWN_DIRECTION 2
#define LEFT_DIRECTION 3

#define INITIAL_SNAKE_LENGTH 6

long previousMillis = 0;
int interval = 1000;


byte score;

byte buttonState = 0;

boolean paused = false;


byte egg_x;
byte egg_y;
boolean egg = false;

byte excessTail_x;
byte excessTail_y;

//direction flags w/ initial flag
boolean
upward = false,
rightward = true,
downward = false,
leftward = false;

byte anchor_x[SEGMENTS_WIDTH];
byte anchor_y[SEGMENTS_HEIGHT];

//snake body properties
byte
x[SEGMENT_COUNT],
y[SEGMENT_COUNT],
d[SEGMENT_COUNT],
snakeLength = INITIAL_SNAKE_LENGTH;

int Buzzer;

#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16


void setup()
{
  randomSeed(analogRead(0));

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

  display.display();


  byte i;

  //fill anchors
  for (i = 0; i < SEGMENTS_WIDTH; i++) {
    anchor_x[i] = i * 5 + 2;
  }
  for (i = 0; i < SEGMENTS_HEIGHT; i++) {
    anchor_y[i] = i * 5 + 1;
  }


  //initial snake properties
  for (i = 0; i < INITIAL_SNAKE_LENGTH; i++)
  {
    x[i] = anchor_x[6 - i];
    y[i] = anchor_y[4];
    d[i] = RIGHT_DIRECTION;
  }

  draw();
}


void loop()
{
  steering();

  if (!paused)
  {
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis > interval)
    {
      //moveSnake();

      if (!collision())
      {
        draw();

        if (!egg) {
          addEgg();
        }
      }
      else
      {
        paused = true;
      }
      
      previousMillis = currentMillis;
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


  //move up
  if (upward == true)
  {
    //Serial.print("upward\n");
    if (y[0] - SEGMENT_SIZE < 1) {
      tempy = AREAHEIGHT - SEGMENT_SIZE - 1;
    }
    else {
      tempy = y[0] - SEGMENT_SIZE;
    }
    tempx = x[0];
    tempd = UP_DIRECTION;
  }

  //move right
  else if (rightward == true)
  {
    //Serial.print("rightward\n");
    if (x[0] + SEGMENT_SIZE > AREAWIDTH - SEGMENT_SIZE - 2) {
      tempx = 2;
    }
    else {
      tempx = x[0] + SEGMENT_SIZE;
    }
    tempy = y[0];
    tempd = RIGHT_DIRECTION;
  }

  //move down
  else if (downward == true)
  {
    //Serial.print("downward\n");
    if (y[0] + SEGMENT_SIZE > AREAHEIGHT - SEGMENT_SIZE - 1) {
      tempy = 1;
    }
    else {
      tempy = y[0] + SEGMENT_SIZE;
    }
    tempx = x[0];
    tempd = DOWN_DIRECTION;
  }

  //move left
  else if (leftward == true)
  {
    //Serial.print("leftward\n");
    if (x[0] - SEGMENT_SIZE < 2) {
      tempx = AREAWIDTH - SEGMENT_SIZE - 2;
    }
    else {
      tempx = x[0] - SEGMENT_SIZE;
    }
    tempy = y[0];
    tempd = LEFT_DIRECTION;
  }


  //snake position shift
  byte i;
  for (i = 0; i <= snakeLength; i++)
  {
    byte _x = x[i];
    byte _y = y[i];
    byte _d = d[i];
    x[i] = tempx;
    y[i] = tempy;
    d[i] = tempd;
    tempx = _x;
    tempy = _y;
    tempd = _d;
  }
}

boolean collision()
{
  //check head collision
  byte i;
  for (i = 1; i < snakeLength; i++)
  {
    if (x[i] == x[0] && y[i] == y[0])
    {
      //game over
      score = snakeLength - INITIAL_SNAKE_LENGTH;

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

void draw()
{
  //clear excess tail
  if (x[snakeLength]) {
    display.fillRect(x[snakeLength], y[snakeLength], 5, 5, WHITE);
  }

  //reset excess tail properties
  x[snakeLength] = 0; //or :=NULL
  y[snakeLength] = 0; //or :=NULL


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

void addEgg()
{
  byte available_x[SEGMENT_COUNT - snakeLength];
  byte available_y[SEGMENT_COUNT - snakeLength];

  byte i, ii, j = 0;
  byte a_xi = 0, a_yi = 0;
  boolean available_position;

  for (i = 0; i < SEGMENT_COUNT; i++)
  {
    if (a_xi >= SEGMENTS_WIDTH)
    {
      a_xi = 0;
      a_yi++;
    }

    available_position = true;
    for (ii = 0; ii < snakeLength; ii++)
    {
      if (anchor_x[a_xi] == x[ii] && anchor_y[a_yi] == y[ii])//test
      {
        available_position = false;
        break;
      }
    }

    if (available_position)
    {
      //      Serial.println("wpisano do available");
      available_x[j] = anchor_x[a_xi];
      available_y[j] = anchor_y[a_yi];
      j++;
    }
    else {
      //      Serial.println("!!!SNAKE!!!");
    }

    //    Serial.print(a_yi);
    //    Serial.print(": ");
    //    Serial.println(a_xi);

    a_xi++;
  }

  byte randIndex = random(0, j - 1);
  egg_x = available_x[randIndex];
  egg_y = available_y[randIndex];

  egg = true;

  Serial.println(randIndex);
  Serial.println(j);
  Serial.println(SEGMENT_COUNT - snakeLength);

}


