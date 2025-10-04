#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

// ESP32 GPIO
const int joyX = 34;
const int joySW = 33;
const int buzzerPin = 25;
const int ledPin = 26;

int playerX = 0;
int playerY = 2;

int bombX = 19;
int bombY = 2;

int birdX = 15;
int birdY = 2;

int wallX = 10;
int wallY = 1;

int score = 0;
int lives = 3;
bool gameOver = false;

unsigned long lastMoveTime = 0;
int moveDelay = 600;

// Custom characters
byte dino[8] = {
  B01110,
  B11111,
  B10101,
  B11111,
  B01010,
  B11011,
  B10001,
  B00000
};

byte bomb[8] = {
  B00100,
  B01110,
  B10101,
  B11111,
  B01110,
  B11111,
  B01010,
  B10001
};

byte bird[8] = {
  B00010,
  B00110,
  B01100,
  B11110,
  B11110,
  B01100,
  B00110,
  B00010
};

byte wall[8] = {
  B11111,
  B10101,
  B11111,
  B10101,
  B11111,
  B10101,
  B11111,
  B10101
};

void drawGame() {
  lcd.clear();

  // Top border / title
  lcd.setCursor(0, 0);
  lcd.print("==== JUMP GAME ====");

  // Draw objects (rows 1 or 2 only)
  lcd.setCursor(playerX, playerY); lcd.write(byte(0));
  lcd.setCursor(bombX, bombY); lcd.write(byte(1));
  lcd.setCursor(birdX, birdY); lcd.write(byte(2));
  lcd.setCursor(wallX, wallY); lcd.write(byte(3));

  // Bottom row status
  lcd.setCursor(0, 3); lcd.print("S:"); lcd.print(score);
  lcd.setCursor(5, 14); lcd.print("=========");
  lcd.setCursor(15, 3); lcd.print("L:");
  for (int i = 0; i < lives; i++) lcd.print((char)2);

  // Serial Debug
  Serial.print("Player: ("); Serial.print(playerX); Serial.print(", "); Serial.print(playerY); Serial.print(")  ");
  Serial.print("Score: "); Serial.print(score);
  Serial.print("  Lives: "); Serial.println(lives);
}

void moveObjects() {
  bombX--;
  if (bombX < 0) {
    bombX = 19;
    bombY = random(1, 3);
    Serial.println("Bomb repositioned");
  }

  birdX--;
  if (birdX < 0) {
    birdX = 19;
    birdY = random(1, 3);
    Serial.println("Bird repositioned");
  }

  wallX--;
  if (wallX < 0) {
    wallX = 19;
    wallY = random(1, 3);
    Serial.println("Wall repositioned");
  }
}

void checkCollision() {
  if ((playerX == bombX && playerY == bombY) ||
      (playerX == wallX && playerY == wallY)) {
    tone(buzzerPin, 1000, 200);
    digitalWrite(ledPin, HIGH);
    lives--;
    Serial.println("Collision! Life lost.");
    if (lives <= 0) {
      gameOver = true;
      Serial.println("Game Over!");
    }
  }

  if (playerX == birdX && playerY == birdY) {
    tone(buzzerPin, 2000, 100);
    score++;
    Serial.println("Bird caught! Score increased.");
    birdX = 19;
    birdY = random(1, 3);
    if (score % 3 == 0 && lives < 3) {
      lives++;
      Serial.println("Bonus life earned!");
    }
    if (score % 5 == 0 && moveDelay > 150) {
      moveDelay -= 50;
      Serial.print("Speed increased. New delay: ");
      Serial.println(moveDelay);
    }
  }
}

void handleMovement() {
  int xVal = analogRead(joyX);
  bool sw = !digitalRead(joySW);

  // Left/right movement
  if (xVal < 1000 && playerX > 0) {
    playerX--;
    Serial.println("Moved Left");
  } else if (xVal > 3000 && playerX < 19) {
    playerX++;
    Serial.println("Moved Right");
  }

  // Jump with SW button
  if (sw) {
    if (playerY == 2) playerY = 1;
    else if (playerY == 1) playerY = 2;
    Serial.println("Jumped / Switched Row");
    delay(200); // debounce
  }
}

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();

  pinMode(joySW, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  lcd.createChar(0, dino);
  lcd.createChar(1, bomb);
  lcd.createChar(2, bird);
  lcd.createChar(3, wall);

  randomSeed(analogRead(33));

  drawGame();
  Serial.println("Game Started");
}

void loop() {
  if (!gameOver) {
    unsigned long now = millis();
    if (now - lastMoveTime > moveDelay) {
      handleMovement();
      moveObjects();
      checkCollision();
      drawGame();
      digitalWrite(ledPin, LOW);
      lastMoveTime = now;
    }
  } else {
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("==== GAME OVER =====");
    lcd.setCursor(0, 1); lcd.print("        ***        ");
    lcd.setCursor(5, 2); lcd.print(" Score: ");
    lcd.print(score);
    lcd.setCursor(0, 3); lcd.print("====================");
    Serial.print("Final Score: "); Serial.println(score);
    while (1);
  }
}
