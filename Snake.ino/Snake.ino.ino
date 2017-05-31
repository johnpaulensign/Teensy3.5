/*
  This code is loosely base off the project found here http://www.kosbie.net/cmu/fall-10/15-110/handouts/snake/snake.html 
  which was modified by Mario Avenoso <mario@mtechcreations.com> Mario.mtechcreations.com on 11/14/13.

    ---------
  
  Several changes have been made:
    Changed to FastLED instead of NeoPixel
    Using a single strand that is broken into 8 LEDs per row
    Changed snake head to be a different color
    Added multiple "programs"

  Modified by John-Paul Ensign <ensign.johnpaul@gmail.com> 2017-05-31

    ---------
  
*/

#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>

//delay to debounce button interrupts
#define INTERRUPT_DEBOUNCE 200

//data pin for LED strand
#define DATA_PIN 6
#define POT_PIN 0

//this is the number of rows, or height
#define ROWS 16

//this is the number of columns, or width
#define COLUMNS 8

#define UP_BUTTON 7
#define DOWN_BUTTON 8
#define LEFT_BUTTON 9
#define RIGHT_BUTTON 10
#define CHANGE_PROGRAM_BUTTON 11
#define CHANGE_BRIGHTNESS_BUTTON 12

#define NUM_STRIPS 16
#define NUM_LEDS ROWS*COLUMNS

#define DELAY 50

//global vars

// led array
CRGB leds[NUM_LEDS];


//sets the row and col of the snake head
int hrow = 0, hcol = 0;

//start the game with true
//game = false == game over
bool game = true;

//dont autostart
bool start = false;

//When true the game timer will not update due to the update by a keypress
//tells the game timer whether or not it should run
bool ignoreNextTimer = false;

// used to immediately return out of programs if true
bool interrupted = false;

//set snake location
int sx = 4, sy = 3;

//used for the game timer
long previousMillis = 0;

//how long between each update of the game
uint16_t difficulty = 500;

//used for the game timer
unsigned long currentMillis = 0;

//used to keep last direction, start off going up
int8_t sDr = 1, sDc = 0;

uint16_t score = 0;

//int array[Y * X];
uint32_t SetElement(uint32_t, uint32_t);//2D array into a number

//game field, 0 is empty, -1 is food, >0 is snake
int gameBoard[ROWS][COLUMNS] = {
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0}
};

static unsigned long last_interrupt_time_up = 0, last_interrupt_time_down = 0, last_interrupt_time_left = 0, last_interrupt_time_right = 0, last_interrupt_time_change_program = 0, last_interrupt_time_brightness = 0;

uint8_t current_program = 0;
uint16_t brightness_level = 51;

void setup() {

  Serial.begin(57600);//used for debugging
  hrow = sx; //set the row of the snake head
  hcol = sy; //set the col of the snake head
  randomSeed(analogRead(0));//used to help make a better random number
  //  keyboard.begin(DataPin, IRQpin);//start the keybaord
  //  strip.begin();//start the NeoPixel Sheild
  //  strip.show(); // Initialize all pixels to 'off'
  resetGame();//clear and set the game

  LEDS.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  LEDS.setBrightness(84);
  FastLED.show();

  attachInterrupt(UP_BUTTON, up_pressed, FALLING);
  attachInterrupt(DOWN_BUTTON, down_pressed, FALLING);
  attachInterrupt(LEFT_BUTTON, left_pressed, FALLING);
  attachInterrupt(RIGHT_BUTTON, right_pressed, FALLING);
  attachInterrupt(CHANGE_PROGRAM_BUTTON, change_program, FALLING);
  attachInterrupt(CHANGE_BRIGHTNESS_BUTTON, change_brightness, FALLING);

}

void loop() {

  if (current_program == 0) {
    snake_program();
  } else if (current_program == 1) {
    backAndForth();
  } else if (current_program == 2) {
    backAndForthBow();
  }  else if (current_program == 3) {
    one_color(Candle);
  } else if (current_program == 4) {
    one_color(Tungsten40W);
  } else if (current_program == 5) {
    one_color(Tungsten100W);
  } else if (current_program == 6) {
    one_color(Halogen);
  } else if (current_program == 7) {
    one_color(CarbonArc);
  } else if (current_program == 8) {
    one_color(HighNoonSun);
  } else if (current_program == 9) {
    one_color(DirectSunlight);
  } else if (current_program == 10) {
    one_color(OvercastSky);
  } else if (current_program == 11) {
    one_color(ClearBlueSky);
  } else if (current_program == 12) {
    one_color(WarmFluorescent);
  } else if (current_program == 13) {
    one_color(StandardFluorescent);
  } else if (current_program == 14) {
    one_color(CoolWhiteFluorescent);
  } else if (current_program == 15) {
    one_color(FullSpectrumFluorescent);
  } else if (current_program == 16) {
    one_color(GrowLightFluorescent);
  } else if (current_program == 17) {
    one_color(BlackLightFluorescent);
  } else if (current_program == 18) {
    one_color(MercuryVapor);
  } else if (current_program == 19) {
    one_color(SodiumVapor);
  } else if (current_program == 20) {
    one_color(MetalHalide);
  } else if (current_program == 21) {
    one_color(HighPressureSodium);
  } else if (current_program == 22) {
    one_color(CRGB::Red);
  } else if (current_program == 23) {
    one_color(CRGB::Green);
  } else if (current_program == 24) {
    one_color(CRGB::Blue);
  } else {
    resetGame();
    current_program = 0;
  }

}

void change_program() {
  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time_change_program > INTERRUPT_DEBOUNCE) {
    current_program++;
    interrupted = true;
  }
  last_interrupt_time_change_program = interrupt_time;
}

void delayPot() {
  delay(map(analogRead(0), 1, 1023, 0, 500));
}

void one_color(int32_t color) {
  for (int i = 0; i < NUM_LEDS; i++) {
    if (interrupted) {
      interrupted = false;
      return;
    }
    leds[i] = color;
  }
  FastLED.show();
}

void change_brightness() {

  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time_brightness > INTERRUPT_DEBOUNCE)
  {
    brightness_level += 17;
    if (brightness_level > 255) {
      brightness_level = 17;
    }
    LEDS.setBrightness(brightness_level);
    FastLED.show();
  }
  last_interrupt_time_brightness = interrupt_time;

}

void backAndForth() {
  static uint8_t hue = 0;
  for (int i = 0; i < NUM_STRIPS; i++) {
    if (interrupted) {
      interrupted = false;
      return;
    }
    for (int j = 0; j < 8; j++) {
      if (interrupted) {
        interrupted = false;
        return;
      }
      leds[i * 8 + j] = CHSV(hue, 255, 255);
    }

    hue++;
    FastLED.show();
    fadeall(230);
    delayPot();
  }

  for (int i = (NUM_STRIPS) - 1; i >= 0; i--) {
    if (interrupted) {
      interrupted = false;
      return;
    }
    for (int j = 0; j < 8; j++) {
      if (interrupted) {
        interrupted = false;
        return;
      }
      leds[i * 8 + j] = CHSV(hue, 255, 255);
    }

    hue++;
    FastLED.show();
    fadeall(230);
    delayPot();
  }
}

void backAndForthBow() {
  static uint8_t hue = 0;
  for (int i = 0; i < NUM_STRIPS; i++) {

    if (interrupted) {
      interrupted = false;
      return;
    }

    if (21 + i * 8 < NUM_LEDS) {
      leds[i * 8] = CHSV(hue, 255, 255);
      leds[9 + i * 8] = CHSV(hue, 255, 255);
      leds[18 + i * 8] = CHSV(hue, 255, 255);
      leds[19 + i * 8] = CHSV(hue, 255, 255);
      leds[20 + i * 8] = CHSV(hue, 255, 255);
      leds[21 + i * 8] = CHSV(hue, 255, 255);
      leds[14 + i * 8] = CHSV(hue, 255, 255);
      leds[7 + i * 8] = CHSV(hue, 255, 255);

    } else {
      continue;
    }

    hue++;
    FastLED.show();
    fadeall(200);
    delayPot();
  }

  for (int i = NUM_STRIPS; i >= 0; i--) {
    if (interrupted) {
      interrupted = false;
      return;
    }

    if (-21 + i * 8 - 1 >= 0) {
      leds[i * 8 - 1] = CHSV(hue, 255, 255);
      leds[-9 + i * 8 - 1] = CHSV(hue, 255, 255);
      leds[-18 + i * 8 - 1] = CHSV(hue, 255, 255);
      leds[-19 + i * 8 - 1] = CHSV(hue, 255, 255);
      leds[-20 + i * 8 - 1] = CHSV(hue, 255, 255);
      leds[-21 + i * 8 - 1] = CHSV(hue, 255, 255);
      leds[-14 + i * 8 - 1] = CHSV(hue, 255, 255);
      leds[-7 + i * 8 - 1] = CHSV(hue, 255, 255);

    } else {
      continue;
    }

    hue++;
    FastLED.show();
    fadeall(200);
    delayPot();
  }
}

void fadeall(int fadeSpeed) {
  for (int i = 0; i < NUM_LEDS; i++) {

    if (interrupted) {
      interrupted = false;
      return;
    }

    leds[i].nscale8(fadeSpeed);
  }
}

void snake_program() {

  difficulty = 500 - score * 25;

  if (difficulty < 100) {
    difficulty = 100;
  }

  currentMillis = millis();//get the current time
  //game clock
  if (currentMillis - previousMillis >= difficulty) { //check if the
    // current time - the last time is greater the the delay time you want
    previousMillis = currentMillis;
    if (game && start && ignoreNextTimer == false) //checks to make sure the game is good and running
    {
      drawBoard();
      updateGame();
    }
    ignoreNextTimer = false; //resets the ignore bool
  }
}

void up_pressed() {

  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time_up > INTERRUPT_DEBOUNCE)
  {
    if (game && !start) {
      start = true;
      drawBoard();
    } else {
      //moveSnake(-1, 0);
      Serial.print("[Up]");
      //      ignoreNextTimer = true;
    }
    sDr = -1;
    sDc = 0;
  }
  last_interrupt_time_up = interrupt_time;
}

void down_pressed() {

  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time_down > INTERRUPT_DEBOUNCE)
  {
    if (game && !start) {
      start = true;
      drawBoard();
    } else {
      //      moveSnake(1, 0);
      Serial.print("[Down]");
      //      ignoreNextTimer = true;
    }
    sDr = 1;
    sDc = 0;
  }
  last_interrupt_time_down = interrupt_time;
}

void left_pressed() {
  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time_left > INTERRUPT_DEBOUNCE)
  {
    if (game && !start) {
      start = true;
      drawBoard();
    } else {
      //      moveSnake(0, -1);
      Serial.print("[Left]");
      //      ignoreNextTimer = true;
    }
    sDr = 0;
    sDc = -1;
  }
  last_interrupt_time_left = interrupt_time;
}

void right_pressed() {

  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time_right > INTERRUPT_DEBOUNCE)
  {
    if (game && !start) {
      start = true;
      drawBoard();
    } else {
      Serial.print("[Right]");
    }
    sDr = 0;
    sDc = 1;
  }
  last_interrupt_time_right = interrupt_time;
}

void updateGame() {
  if (game && start)//checks to make sure the game is running and good
  {
    clear_dsp();
    moveSnake(sDr, sDc); //keeps the Snake moving in the current direction
    drawBoard();//update the screen
  }

}

void resetGame() {
  resetBoard();
  sDr = -1;
  sDc = 0;
  loadSnake();
  placeFood();
  findSnakeHead();//find where the snake is starting from
  score = 0;
  game = true;
  start = false;
  ignoreNextTimer = false;
  drawBoard();
}

void placeFood() {
  int rx = 0, ry = 0;
  rx = random(0, ROWS - 1);
  ry = random(0, COLUMNS - 1);
  if (gameBoard[rx][ry] > 0)
  {

    while (gameBoard[rx][ry] > 0)
    {

      rx = random(0, ROWS - 1);
      ry = random(0, COLUMNS - 1);
      if (gameBoard[rx][ry] == 0)
      {
        gameBoard[rx][ry] = -1;
        //drawBoard();
        break;
      }
    }
  }
  else
  {
    gameBoard[rx][ry] = -1;
    //drawBoard();
  }

}

void loadSnake() {
  gameBoard[sx][sy] = 1;

}

void resetBoard() {
  for (int x = 0; x < ROWS; x++)
  {
    for (int y = 0; y < COLUMNS; y++)
    {
      gameBoard[x][y] = 0;
    }

  }
  loadSnake();
}

void gameOver() {
  game = false;
  start = false;
  for (int light = 0; light < 255; light++)
  {
    for (int i = 0; i < NUM_LEDS; i++)
    {
      leds[i] = CRGB::Red;
    }
    FastLED.show();
    delay(15);
  }

  FastLED.show();

  delay(1000);
  resetGame();
}

void moveSnake(int row, int col) {
  sDr = row;
  sDc = col;
  Serial.print(sDr);
  Serial.print(",");
  Serial.println(sDc);
  int new_r = 0, new_c = 0;
  new_r = hrow + row;
  new_c = hcol + col;
  if (new_r >= ROWS || new_r < 0 || new_c >= COLUMNS || new_c < 0)
  {
    gameOver();
  }
  else if (gameBoard[new_r][new_c] > 0)
  {
    gameOver();
  }
  else if (gameBoard[new_r][new_c] == -1)
  {
    gameBoard[new_r][new_c] = 1 + gameBoard[hrow][hcol];
    hrow = new_r;
    hcol = new_c;
    placeFood();
    score++;
    drawBoard();
  }
  else
  {
    gameBoard[new_r][new_c] = 1 + gameBoard[hrow][hcol];
    hrow = new_r;
    hcol = new_c;
    removeTail();
    drawBoard();
  }

}

void removeTail() {
  for (int x = 0; x < ROWS; x++)
  {
    for (int y = 0; y < COLUMNS; y++)
    {
      if (gameBoard[x][y] > 0)
      {
        gameBoard[x][y]--;
      }
    }
  }
}

void drawBoard() {
  for (int x = 0; x < ROWS; x++)
  {
    for (int y = 0; y < COLUMNS; y++)
    {
      if (gameBoard[x][y] == -1) //food
      {
        leds[SetElement(x, y)] = CRGB::Green;
        //        strip.setPixelColor(SetElement(x,y),strip.Color(0,100,0));
      }

      else if (gameBoard[x][y] == 0) //empty
      {
        leds[SetElement(x, y)] = CRGB::Black;
        //        strip.setPixelColor(SetElement(x,y),strip.Color(0,0,0));
      }
      else
      {
        leds[SetElement(x, y)] = CRGB::Blue;
        leds[SetElement(hrow, hcol)] = CRGB::Purple;
        //        strip.setPixelColor(SetElement(x,y),strip.Color(0,0,255));
      }

    }
  }
  FastLED.show();
}

void findSnakeHead() {
  hrow = 0; //clearing out old location
  hcol = 0; //clearing out old location

  for (int x = 0; x < ROWS; x++)
  {
    for (int y = 0; y < COLUMNS; y++)
    {
      if (gameBoard[x][y] > gameBoard[hrow][hcol])
      {
        hrow = x;
        hcol = y;
      }
    }
  }
}

void clear_dsp() {
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
}

uint32_t SetElement(uint32_t row, uint32_t col) {
  if (row % 2 == 0) {
    return row * COLUMNS + col;
  } else {
    return row * COLUMNS + (COLUMNS - 1 - col);
  }
}


