/*
 * 賽車遊戲控制器（RALLY / RAW 2）— Arduino Uno 版
 *
 * 輸入（Arduino → 電腦）：搖桿取代 WASD、一顆按鈕當手煞車（Space），其他功能維持用鍵盤。
 * 輸出（電腦 → Arduino）：車速夠快時風扇吹風（用繼電器開關）；通過檢查點時紅燈亮 1 秒。
 *
 * Uno 不能直接模擬 USB 鍵盤，所以電腦要執行 serial_to_keys.py 當橋樑：
 *   Arduino 送出：+w 按下 W、-w 放開 W；空白鍵寫成 sp
 *   電腦送來  ：S<車速 km/h>，例如 S87；C 代表通過檢查點
 *
 * 接線：
 *   搖桿模組  VCC → 5V、GND → GND、VRx → A0、VRy → A1（SW 不用接）
 *   手煞車按鈕 一腳 → D2、另一腳 → GND（使用內建上拉電阻）
 *   繼電器模組 + → 5V、- → GND、S → D5
 *   風扇      5V → 繼電器 COM，繼電器 NO → 風扇 +，風扇 - → GND，詳見 README
 *   紅燈 LED  D12 → 220Ω → LED 長腳，LED 短腳 → GND
 */

// ---------- 腳位 ----------
const int JOY_X_PIN = A0;
const int JOY_Y_PIN = A1;
const int HANDBRAKE_PIN = 2;
const int FAN_PIN = 5;   // 繼電器 S；改用調速模式時必須是 PWM 腳位（3、5、6、9、10、11）
const int LED_PIN = 12;

// ---------- 搖桿設定 ----------
// 搖桿推離中心超過這個值才算按下（0~512），太敏感就調大
const int DEAD_ZONE = 200;

// 方向相反時把對應的值改成 true
const bool INVERT_X = false;  // true：左右對調
const bool INVERT_Y = false;  // true：前後對調

// ---------- 風扇設定 ----------
// 繼電器只能全開或全關，不能調轉速。
// 之後改用電晶體或馬達驅動模組時，改成 false 就會隨車速調整轉速。
const bool FAN_USE_RELAY = true;
const bool RELAY_ACTIVE_LOW = false;  // 繼電器在 S 給 LOW 時才吸合（風扇反而在停車時轉）就改成 true

// 繼電器模式：車速超過 ON 開風扇、低於 OFF 關風扇
// （兩個值分開，避免車速在門檻附近時繼電器一直切換）
const int FAN_RELAY_ON_SPEED = 40;    // km/h
const int FAN_RELAY_OFF_SPEED = 30;   // km/h
const unsigned long FAN_RELAY_MIN_SWITCH_MS = 500;  // 繼電器最短切換間隔，保護繼電器

// 調速模式（FAN_USE_RELAY = false）
const int FAN_START_SPEED = 5;     // 車速低於這個值（km/h）風扇停止
const int FAN_FULL_SPEED = 150;    // 車速達到這個值（km/h）風扇全速
const int FAN_MIN_PWM = 80;        // 風扇能轉起來的最低 PWM，轉不動就調大

const unsigned long FAN_TIMEOUT_MS = 1000;  // 超過這麼久沒收到車速就關風扇

// ---------- 紅燈設定 ----------
const unsigned long LED_ON_MS = 1000;

// ---------- 按鍵狀態 ----------
struct Key {
  const char *name;
  bool pressed;
};

Key keyUp        = {"w", false};   // 油門
Key keyDown      = {"s", false};   // 煞車/倒車
Key keyLeft      = {"a", false};   // 左轉
Key keyRight     = {"d", false};   // 右轉
Key keyHandbrake = {"sp", false};  // 手煞車

int centerX = 512;
int centerY = 512;

bool handbrakeLastReading = false;
unsigned long handbrakeChangedAt = 0;
const unsigned long DEBOUNCE_MS = 20;

unsigned long lastSpeedAt = 0;
bool fanRelayOn = false;
unsigned long fanRelaySwitchedAt = 0;
unsigned long ledOnAt = 0;
bool ledOn = false;

char rxBuffer[16];
int rxLength = 0;

void setKey(Key &key, bool shouldPress) {
  if (shouldPress == key.pressed) {
    return;
  }
  Serial.print(shouldPress ? '+' : '-');
  Serial.println(key.name);
  key.pressed = shouldPress;
}

void writeRelay(bool on) {
  digitalWrite(FAN_PIN, on != RELAY_ACTIVE_LOW ? HIGH : LOW);
}

void setFanRelay(bool on) {
  if (on == fanRelayOn || millis() - fanRelaySwitchedAt < FAN_RELAY_MIN_SWITCH_MS) {
    return;
  }
  writeRelay(on);
  fanRelayOn = on;
  fanRelaySwitchedAt = millis();
}

void fanOff() {
  if (FAN_USE_RELAY) {
    setFanRelay(false);
  } else {
    analogWrite(FAN_PIN, 0);
  }
}

void setFanSpeed(int speedKmh) {
  if (FAN_USE_RELAY) {
    if (speedKmh >= FAN_RELAY_ON_SPEED) {
      setFanRelay(true);
    } else if (speedKmh < FAN_RELAY_OFF_SPEED) {
      setFanRelay(false);
    }
    return;
  }

  int pwm = 0;
  if (speedKmh >= FAN_START_SPEED) {
    pwm = map(constrain(speedKmh, FAN_START_SPEED, FAN_FULL_SPEED),
              FAN_START_SPEED, FAN_FULL_SPEED, FAN_MIN_PWM, 255);
  }
  analogWrite(FAN_PIN, pwm);
}

void handleCommand(const char *cmd) {
  if (cmd[0] == 'S') {
    setFanSpeed(atoi(cmd + 1));
    lastSpeedAt = millis();
  } else if (cmd[0] == 'C') {
    digitalWrite(LED_PIN, HIGH);
    ledOn = true;
    ledOnAt = millis();
  }
}

void readSerial() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (rxLength > 0) {
        rxBuffer[rxLength] = '\0';
        handleCommand(rxBuffer);
        rxLength = 0;
      }
    } else if (rxLength < (int)sizeof(rxBuffer) - 1) {
      rxBuffer[rxLength++] = c;
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(HANDBRAKE_PIN, INPUT_PULLUP);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  if (FAN_USE_RELAY) {
    writeRelay(false);
  } else {
    analogWrite(FAN_PIN, 0);
  }
  digitalWrite(LED_PIN, LOW);

  // 開機時搖桿不要碰，用來校正中心點
  delay(500);
  long sumX = 0;
  long sumY = 0;
  for (int i = 0; i < 16; i++) {
    sumX += analogRead(JOY_X_PIN);
    sumY += analogRead(JOY_Y_PIN);
    delay(5);
  }
  centerX = sumX / 16;
  centerY = sumY / 16;
}

void loop() {
  unsigned long now = millis();

  // 搖桿 → WASD
  int dx = analogRead(JOY_X_PIN) - centerX;
  int dy = analogRead(JOY_Y_PIN) - centerY;
  if (INVERT_X) dx = -dx;
  if (INVERT_Y) dy = -dy;

  // 多數搖桿模組往前推時 VRy 變小，所以 dy < 0 代表往前
  setKey(keyUp,    dy < -DEAD_ZONE);
  setKey(keyDown,  dy >  DEAD_ZONE);
  setKey(keyLeft,  dx < -DEAD_ZONE);
  setKey(keyRight, dx >  DEAD_ZONE);

  // 手煞車按鈕（按下時腳位為 LOW）
  bool reading = digitalRead(HANDBRAKE_PIN) == LOW;
  if (reading != handbrakeLastReading) {
    handbrakeLastReading = reading;
    handbrakeChangedAt = now;
  }
  if (now - handbrakeChangedAt >= DEBOUNCE_MS) {
    setKey(keyHandbrake, reading);
  }

  // 接收車速與檢查點
  readSerial();

  // 太久沒收到車速（遊戲關掉或橋接程式停止）就關風扇
  if (now - lastSpeedAt > FAN_TIMEOUT_MS) {
    fanOff();
  }

  // 紅燈亮 1 秒後熄滅
  if (ledOn && now - ledOnAt >= LED_ON_MS) {
    digitalWrite(LED_PIN, LOW);
    ledOn = false;
  }

  delay(5);
}
