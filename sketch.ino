#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TM1637Display.h>

#define CLK 8
#define DIO 9
#define START_BUTTON_PIN 13 
#define END_BUTTON_PIN 12
#define CONTINUE_BUTTON_PIN 7


TM1637Display ekran(CLK, DIO);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

#define SENSOR_PIN A1
#define POT_PIN A0
#define LED_PIN_1 2
#define LED_PIN_2 3
#define LED_PIN_3 4

#define BRICK_WIDTH 12
#define BRICK_HEIGHT 6
#define BRICK_GAP 2

#define BALL_SIZE 3 

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int initialBallX, initialBallY;
int ballX, ballY;
float ballSpeedX = 3;
float ballSpeedY = -3;
int palettePos;
int skor=0;
int level = 1;

int ledCount = 0;

const int NUM_BRICKS_X = SCREEN_WIDTH / (BRICK_WIDTH + BRICK_GAP);
const int NUM_BRICKS_Y = SCREEN_HEIGHT * 0.3 / (BRICK_HEIGHT + BRICK_GAP);
bool bricks[NUM_BRICKS_X][NUM_BRICKS_Y];

bool gameStarted = false;
bool endGame=false;
bool gamePaused = false;

bool specialObjectActive = false;
int specialObjectX, specialObjectY;
int specialObjectSpeedY = 2; 


void setup() {
  Serial.begin(9600);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 bağlanti hatasi!"));
    for(;;);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  initialBallX = SCREEN_WIDTH / 2;
  initialBallY = SCREEN_HEIGHT - 20;

  ballX = initialBallX;
  ballY = initialBallY;

  initBricks();

  pinMode(LED_PIN_1, OUTPUT);
  pinMode(LED_PIN_2, OUTPUT);
  pinMode(LED_PIN_3, OUTPUT);
  digitalWrite(2, HIGH);
  digitalWrite(3, HIGH);
  digitalWrite(4, HIGH);

  pinMode(START_BUTTON_PIN, INPUT_PULLUP); 
  pinMode(END_BUTTON_PIN,INPUT_PULLUP);

  ekran.setBrightness(0x0f); 
  ekran.clear(); 
  ekran.showNumberDec(skor,false);

  display.display();
}

void initBricks() {
  for (int i = 0; i < NUM_BRICKS_X; i++) {
    for (int j = 0; j < NUM_BRICKS_Y; j++) {
      bricks[i][j] = false; 

      
      if (level == 1) {
        // Seviye 1 için tuğla düzeni
        // Örnek bir tuğla düzeni:
        // X X X X X X X X X X X X
        // X X X X X X X X X X X X
        // X X X X X X X X X X X X
        // X X X X X X X X X X X X
        if (i % 2 == 0 && j % 2 == 0) {
          bricks[i][j] = true;
        }
      } else if (level == 2) {
        // Seviye 2 için tuğla düzeni
        // Örnek bir tuğla düzeni:
        // X   X   X   X   X   X  
        //   X   X   X   X   X   
        // X   X   X   X   X   X  
        //   X   X   X   X   X   
        if ((i + j) % 2 == 0) {
          bricks[i][j] = true;
        }
      } else if (level == 3) {
        // Seviye 3 için tuğla düzeni
        // Örnek bir tuğla düzeni:
        // X X     X X     X X   
        // X X     X X     X X   
        //     X X     X X       
        //     X X     X X       
        if ((i + j) % 4 == 0 || (i + j) % 4 == 3) {
          bricks[i][j] = true;
        }
      }
    }
  }
  
  
  // Rastgele %10 olasılıkla özel objenin düşmesi
  if (random(0, 10) == 0) {
    specialObjectActive = true;
    specialObjectX = random(0, SCREEN_WIDTH);
    specialObjectY = 0;
  }
}

void drawBricks() {
  for (int i = 0; i < NUM_BRICKS_X; i++) {
    for (int j = 0; j < NUM_BRICKS_Y; j++) {
      if (bricks[i][j]) {
        int brickX = i * (BRICK_WIDTH + BRICK_GAP);
        int brickY = j * (BRICK_HEIGHT + BRICK_GAP);
        display.fillRect(brickX, brickY, BRICK_WIDTH, BRICK_HEIGHT, SSD1306_WHITE);
      }
    }
  }
  
  // Özel objeyi çiz
  if (specialObjectActive) {
    display.fillRect(specialObjectX, specialObjectY, 5, 5, SSD1306_WHITE);
  }
}

void drawCutScene(){

    
    display.clearDisplay();
    display.setCursor(20, 20);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.print("Seviye Sonu! Skor: ");
    display.println(skor);
    display.display();
    delay(5000); 

}

void drawBall(int x, int y) {
  display.fillCircle(x, y, BALL_SIZE, SSD1306_WHITE);
}

void moveBall() {
  ballX += ballSpeedX;
  ballY += ballSpeedY;

  if (ballX >= SCREEN_WIDTH || ballX <= 0) {
    ballSpeedX = -ballSpeedX;
  }
  if (ballY <= 0) {
    ballSpeedY = -ballSpeedY;
  }

  // Özel objeye çarpma kontrolü
  if (specialObjectActive && specialObjectY + 5 >= SCREEN_HEIGHT - 10 && specialObjectX + 5 >= palettePos && specialObjectX <= palettePos + 15) {
    specialObjectActive = false;
    ledCount--;
  }

  if (ballY + BALL_SIZE >= SCREEN_HEIGHT) {
    ledCount++;
    resetBallPosition();
  }

  switch (ledCount) {

      case 0:
      digitalWrite(LED_PIN_1, HIGH);
      digitalWrite(LED_PIN_2, HIGH);
      digitalWrite(LED_PIN_3, HIGH);
        break;
      case 1:
        digitalWrite(LED_PIN_1, LOW);
        digitalWrite(LED_PIN_2, HIGH);
        digitalWrite(LED_PIN_3, HIGH);
        break;
      case 2:
        digitalWrite(LED_PIN_1, LOW);
        digitalWrite(LED_PIN_2, LOW);
        digitalWrite(LED_PIN_3, HIGH);
        break;
      case 3:
        digitalWrite(LED_PIN_1, LOW);
        digitalWrite(LED_PIN_2, LOW);
        digitalWrite(LED_PIN_3, LOW);
        break;
      default:
        break;
    }
 
  if (ballY + BALL_SIZE >= SCREEN_HEIGHT - 10 && ballX + BALL_SIZE >= palettePos && ballX <= palettePos + 15) {
    ballSpeedY = -ballSpeedY;
  }

  checkCollision();
  
  drawBall(ballX, ballY);
}

void moveSpecialObject() {
  if (specialObjectActive) {
    specialObjectY += specialObjectSpeedY;
    
    // Ekranın dışına çıktığında özel nesneyi devre dışı bırak
    if (specialObjectY >= SCREEN_HEIGHT) {
      specialObjectActive = false;
    }
  }
}

void checkCollision() {
  int ballGridX = ballX / (BRICK_WIDTH + BRICK_GAP);
  int ballGridY = ballY / (BRICK_HEIGHT + BRICK_GAP);

  if (ballGridX >= 0 && ballGridX < NUM_BRICKS_X && ballGridY >= 0 && ballGridY < NUM_BRICKS_Y) {
    if (bricks[ballGridX][ballGridY]) {
      skor++;
      ekran.showNumberDec(skor,false);
      bricks[ballGridX][ballGridY] = false;
      ballSpeedY = -ballSpeedY;
      
      // Tuğla kırıldığında özel objenin düşmesi
      if (random(0, 10) == 0) {
        specialObjectActive = true;
        specialObjectX = ballGridX * (BRICK_WIDTH + BRICK_GAP);
        specialObjectY = ballGridY * (BRICK_HEIGHT + BRICK_GAP);
      }
    }
  }

  if (checkEmpty()) {
    level++;
    if (level <= 3) {
      drawCutScene();
      initBricks(); 
      ballSpeedX += ballSpeedX * 0.2; // Topun hızını artır
      ballSpeedY += ballSpeedY * 0.2;
    } else {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.setTextSize(2);
      display.setTextColor(SSD1306_WHITE);
      display.println("Oyun bitti!");
      display.display();
      while (true) {}
    }
  }
}

bool checkEmpty() {
  for (int i = 0; i < NUM_BRICKS_X; i++) {
    for (int j = 0; j < NUM_BRICKS_Y; j++) {
      if (bricks[i][j]) {
        return false;
      }
    }
  }
  return true;
}

void resetBallPosition() {
  ballSpeedX = 0;
  ballSpeedY = 0; 
  ballX = palettePos+7;
  ballY = SCREEN_HEIGHT-13;
  gamePaused=true;
}

void drawStartScreen() {
  display.clearDisplay();
  display.setCursor(20, 20);
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.println("Basla");
  display.setCursor(20, 40);
  display.println("Cikis");
  display.display();
}

void drawGameOverScreen() {
  display.clearDisplay();
  display.setCursor(20, 20);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.println("Oyun bitti!");
  display.setCursor(20, 40);
  display.print("Skor: ");
  display.println(skor);
  display.display();
}

void drawStopScreen(){
  display.clearDisplay();
  display.setCursor(20, 20);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.println("Oyunumuza gosterdiginiz");
  display.setCursor(20, 40);
  display.println("Ilgi icin tesekkurler");
  display.display();

}

void resetGame() {
  level = 1; 
  initBricks(); 
}

void gameOver(){

    drawGameOverScreen();
    skor=0;
    ekran.showNumberDec(skor,false);
    ledCount=0;
    gameStarted=false;
    resetGame();
    delay(3000);
    drawStartScreen();

}

void loop() {
  int startButtonState = digitalRead(START_BUTTON_PIN);
  int endButtonState = digitalRead(END_BUTTON_PIN);
  int continueButtonState= digitalRead(CONTINUE_BUTTON_PIN);
  int sensorValue=analogRead(SENSOR_PIN);
  
  if(sensorValue>500){
    display.invertDisplay(true);

  }
  else{
    display.invertDisplay(false);
  }

  if (endButtonState == LOW) {
    endGame = true;
    drawStopScreen();
    while (endGame == true) {}
  }

  if (continueButtonState == LOW && gamePaused) {
    gamePaused = false;
    if(level==1){
      ballSpeedX=3;
      ballSpeedY=-3;
    }
    else if(level==2){
      ballSpeedX=3+3*0.2;
      ballSpeedY=-3-3*0.2;
    }
    else if(level==3){
      ballSpeedX=3.6+3.6*0.2;
      ballSpeedY=-3.6-3.6*0.2;
    }
    
    delay(500); 
  }

  if (!gameStarted) {
    drawStartScreen();
    if (startButtonState == LOW) {
      gameStarted = true;
      delay(700); 
    }
  } else if (gameStarted && !gamePaused) {
    int potValue = analogRead(POT_PIN);
    palettePos = map(potValue, 0, 1023, 0, SCREEN_WIDTH - 10);
    display.clearDisplay();
    drawBricks();
    display.fillRect(palettePos, SCREEN_HEIGHT - 10, 30, 3, SSD1306_WHITE);
    moveSpecialObject(); 
    moveBall();
    
    display.display();
    delay(50);
  }

  if (ledCount >= 3) {
    gameOver(); 
  }
  
}
