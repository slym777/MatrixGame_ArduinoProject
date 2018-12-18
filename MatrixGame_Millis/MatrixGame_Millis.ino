#include <LedControl.h>  // library for Driver
#include <LiquidCrystal.h> // library for LCD
#include <time.h>     // library for random function
#include <stdlib.h>   // library for random function
#include <EEPROM.h>   //need EEPROM to store the best score
#define V0_Pin 5 // PWN instead of Potentiometer
#define SW_Pin 2 // Joystick's button

LedControl lc = LedControl(12, 11, 10, 1); //DIN, CLK, LOAD, No. DRIVER - setting Driver for Matrix

const int RS = 4, E = 3, D4 = 6, D5 = 7, D6 = 8, D7 = 9; // setings for LCD
LiquidCrystal lcd(RS, E, D4, D5, D6, D7);

const int buzzer = 13; // pin for buzzer

// JOYSTICK
const int joyX = A0;
const int joyY = A1;

// Game Variables
bool init_matrix[8][8];  // initial matrix of every level
bool user_matrix[8][8];  // user matrix during a level 
int nr_points, lives, nr_level, score;
bool user_wins;   // specifies if the user has passed the game or not
bool time_game;   // specifies if the game complexity (number - false/ time - true)
int time_print;    // time to remember points 

void lcd_dificulty(int pos)  // lcd during choose_dificulty
{
  lcd.clear();
  lcd.print("Choose Dificulty");
  lcd.setCursor(0,1);
  if (pos == 0)
  {
    lcd.print(">Number");
    lcd.setCursor(12,1);
    lcd.print("Time");
  }
  else 
  {
    lcd.print("Number");
    lcd.setCursor(11,1);
    lcd.print(">Time");
  }
}

bool choose_dificulty()   // function which let the user to choose complexity
{
  int posX = 0,valX;
  bool movedX = false;
  int button_state = digitalRead(SW_Pin);
  unsigned long prev_millis = millis();
  while(button_state)  
  {
    button_state = digitalRead(SW_Pin);
    if (millis() > prev_millis + 200)
    {
      valX = analogRead(joyX);
      if (valX < 300 && movedX == false)
      {
        posX--;
        movedX = true;
      }
    else if (valX > 700 && movedX == false)
    {
      posX++;
      movedX = true;
    }
    else movedX = false;
    if (posX == 2) posX = 0;
    if (posX == -1) posX = 1;
    lcd_dificulty(posX);
    prev_millis = millis();
    }
  }
  if(posX == 0)
  {
    nr_points -= 2;
    return false;
  }
  else 
    return true;
}

void setup()
{
  pinMode(buzzer, OUTPUT);      // setting buzzer pin as output
  for (int i = 1; i < 5; i++)   // a specific sound which mark the start of the game
  {
    tone(buzzer, i*1000, i*100);
    delay(i*100+200); /// is used to make short interupts between sounds
  }
  lc.shutdown(0, false); // turn off power saving, enables display
  lc.setIntensity(0, 5); // sets brightness (0~15 possible values)
  lc.clearDisplay(0); // clear screen
  lcd.begin(16, 2);   // marks the space of the lcd
  pinMode(V0_Pin, OUTPUT); // setting pwm pin as an output
  analogWrite(V0_Pin, 90);
  digitalWrite(SW_Pin, HIGH);   //SW_PIN has the value 1 when is pressed and reverse
  randomSeed(analogRead(3));
  nr_points = 4, lives = 5, score = 0, nr_level = 0;
  user_wins = false;   
  time_game = false;   
  time_print = 7000;    
  lcd.clear(); // clear the lcd
  lcd.setCursor(3,0);
  lcd.print("Welcome to");
  lcd.setCursor(0,1);
  lcd.print("Test your Memory!");
  delay(4000); // let the user to read from lcd
  time_game = choose_dificulty();  // let the user to choose complexity
  next_level();    // initialize the first level
}

void clear_matrix(bool matrix[8][8])   // clear the needed matrix
{
  for (int i = 0; i < 8; i++)
    for(int j = 0; j < 8; j++)
      matrix[i][j] = false;
}

void lcd_during_game()    // lcd during a level 
{
  lcd.clear();
  lcd.print("Level:");
  lcd.print(nr_level, DEC);
  lcd.print(" Lives:");
  lcd.print(lives, DEC);
  lcd.setCursor(0,1);
  lcd.print("Score:");
  lcd.print(score, DEC);
}

void random_matrix()   // a function which generate random points on the matrix each level
{
  int k = nr_points;
  while (k > 0)
  { 
    int x = random(8);
    int y = random(8);
    if (!init_matrix[x][y])
    {
      init_matrix[x][y] = true;
      k--;
    }
  }
}

void printing(bool matrix[8][8])  // print the matrix
{
  for (int i = 0; i < 8; i++)
    for (int j = 0; j < 8; j++)
      if (matrix[i][j])
        lc.setLed(0,j,i,true);
      else 
        lc.setLed(0,j,i,false);
}

bool user_move(int &posX, int &posY)    // let the user to move on the matrix and choos points he remembered
{
  int valX, valY,button_state;
  bool movedX = false, movedY = false;
  unsigned long prev_millis = millis();
  button_state = 1;
  while(button_state)  
  {
    lc.setLed(0, posX, posX, false);
    if (millis() > prev_millis + 100)
    {
      button_state = digitalRead(SW_Pin);
      valX = analogRead(joyX);
      valY = analogRead(joyY);
      if (valY < 300 && movedY == false)
      {
        posY--;
        movedY = true;
      }
      else if (valY > 700 && movedY == false)
      {
        posY++;
        movedY = true;
      }
      else movedY = false;
      if (valX < 300 && movedX == false)
      {
        posX--;
        movedX = true;
      }
      else if (valX > 700 && movedX == false)
      {
        posX++;
        movedX = true;
      }
      else movedX = false;
 
      if (posY == 8) posY = 0;
      if (posY == -1) posY = 7;
      if (posX == 8) posX = 0;
      if (posX == -1) posX = 7;
      printing(user_matrix);
      if (!button_state)
      {
        if (init_matrix[posX][posY] == true)
        {
          user_matrix[posX][posY] = true;
          return true;
        }
        else 
          return false;
      }
      prev_millis = millis();
    }
    lc.setLed(0, posY, posX, true);
  }
}

bool user_guessing()    // a function which verifies even the user guessed the points or not
{
  int nr_guessed = 0;
  lcd_during_game();
  int posX = 0, posY = 0;
  while (nr_guessed != nr_points && lives > 0)
  {
    if (user_move(posX, posY))
    {
      tone(buzzer, 2000, 200);
      nr_guessed++;
      score += nr_guessed * nr_level;
      posX = 0, posY = 0;
    }
    else 
    {
      for (int i = 1; i >= 0; i--)  // notify the user that he lost a live-point
      {
        tone(buzzer,i*3000 + 500,100);
        delay(100); // need it to wait the first sound 
      }
      lives--;
    }
    lcd_during_game();
  }
  if (lives == 0)
    return false;
  return true;
  
}

void lcd_play_again(int pos)   // lcd during choose_play_again
{
  lcd.clear();
  lcd.print("  Play Again ?");
  lcd.setCursor(0,1);
  if (pos == 0)
  {
    lcd.print(">Yes");
    lcd.setCursor(14,1);
    lcd.print("No");
  }
  else{
    lcd.print("Yes");
    lcd.setCursor(13,1);
    lcd.print(">No");
  }
}

bool choose_play_again()  // let the user to choose to play again or not
{
  int posX = 0,valX;
  int button_state = digitalRead(SW_Pin);
  bool movedX = false;
  lcd_play_again(posX);
  unsigned long prev_millis = millis();
  while(button_state)  
  {
    button_state = digitalRead(SW_Pin);
    if (millis() > prev_millis + 200)
    {
      valX = analogRead(joyX);
      if (valX < 300 && movedX == false)
      {
        posX--;
        movedX = true;
      }
      else if (valX > 700 && movedX == false)
      {
        posX++;
        movedX = true;
      }
      else movedX = false;
      if (posX == 2) posX = 0;
      if (posX == -1) posX = 1;
      lcd_play_again(posX);
      prev_millis = millis();
    }
  }
  tone(buzzer, 3000, 50); // notify that user has choosen 
  if(posX == 0)
    return true;
  else 
    return false;
}

void record() // function which verifies if the user has set a new record or not
{
  int bestscore = 0;
  int ee_adress = 0;
  EEPROM.get(ee_adress, bestscore);
  if (bestscore > score)
  {
    lcd.clear();
    lcd.print("Your Score:");
    lcd.print(score, DEC);
    delay(2500); // let the user to read the text from lcd
    lcd.clear();
    lcd.setCursor(1,0);
    lcd.print("Best Score:");
    lcd.print(bestscore, DEC);
    delay(2500); // let the user to read the text from lcd
  }
  else 
  {
    for (int i = 0 ; i < EEPROM.length() ; i++)  
      EEPROM.write(i, 0);
    EEPROM.put(ee_adress, score);
    lcd.setCursor(4,0);
    lcd.print("You Set");
    lcd.setCursor(1,1);
    lcd.print("A New Record");
    delay(2500); // let the user to read the text from lcd
    lcd.clear();
    lcd.print("Your Record:");
    lcd.print(score, DEC);
    delay(2500); // let the user to read the text from lcd
  }
}

void scroll(char* s, int &my_cursor)
{
  int txt_size = strlen(s);
  if (my_cursor == 0)
  {
    my_cursor = 16;
    lcd.clear();
  }
  lcd.setCursor(0,0);

  for(int c = my_cursor; c <= txt_size - 1; c++)
    lcd.print(s[c]);

   for(int c = 0; c < my_cursor; c++)
    lcd.print(s[c]);
    
  my_cursor--;
}

void play_again()   // execute the user's choose from choose_play_again
{
  if (choose_play_again())
  {
    nr_points = 4;
    user_wins = false;
    nr_level = 0;
    score = 0;
    time_game = choose_dificulty();
    next_level();
  }
  else
  {
    lcd.clear();
    lcd.setCursor(2,0);
    lcd.print("See You Soon!");
    delay(2500); // let the user to read from lcd
    char* s = " Press to start ";
    int my_cursor = 16;
    int button_state = digitalRead(SW_Pin);
    while(button_state)
    {
      button_state = digitalRead(SW_Pin);
      scroll(s, my_cursor);
      delay(350);
    }
    lcd.clear();
    setup();
  }
}

void update_matrix()  // is used between levels and game events
{
  for (int i = 0; i < 8; i++)
    for (int j = 0; j < 8; j++)
    {
      unsigned long previous_millis = millis();
      while (true)
      {
        if (millis() > previous_millis + 30)
        {
          lc.setLed(0, j, i, true);
          break;
        }
      }
    }
  for (int i = 0; i < 8; i++)
    for (int j = 0; j < 8; j++)
    {
      unsigned long previous_millis = millis();
      while (true)
      {
        if (millis() > previous_millis + 30)
        {
          lc.setLed(0, j, i, false);
          break;
        }
      }
    }  
}
void next_level()   // initialize the next level
{
  for (int i = 1; i <= 4; i++)
  {
    tone(buzzer,i*500 + 1000,100);  // notify the user that he passed the level
    delay(100);
  } 
  clear_matrix(init_matrix);
  clear_matrix(user_matrix);
  if (time_game)
    time_print -= 500;
  else
    nr_points++;
  if (nr_level == 11)
    user_wins = true;
  nr_level++;
  lives = 5;
  if (!user_wins)
  {
    lcd.clear();
    lcd.setCursor(3,0);
    lcd.print("Get Ready!");
    update_matrix(); 
    lcd.clear();
  }
}

void game_over()  // notify the user that he lost and the game is over
{
  for (int i = 8; i > 0; i--)
  {
    tone(buzzer,(i)*500,100);
    delay(100); // need it to wait the first sound 
  }
  lcd.clear();
  lcd.setCursor(4,0);
  lcd.print("Game Over");
  update_matrix();
  record();  // check for a record
  play_again(); // ask user to play again
}

void level()
{
  random_matrix();  // initialize the matrix with points that user has to remember
  printing(init_matrix);  //print it
  lcd.clear();
  lcd.setCursor(5,0);
  lcd.print("Level ");
  lcd.print(nr_level, DEC);
  delay(time_print);  // give time to memorise the points
  if (user_guessing()) // if user guessed the points go to next level otherwise he lost
    next_level();
  else
    game_over();
}

void loop()
{
  if (!user_wins)  // if user hasn't won yet play next level 
    level();
  else   // notify the user that he passed the game
  {
    lcd.setCursor(2,0);
    lcd.print("You Have Won!");
    lcd.setCursor(0,1);
    lcd.print("Congratulations");
    update_matrix();
    record();  // check for a record
    play_again(); // ask user to play again
  }
}
