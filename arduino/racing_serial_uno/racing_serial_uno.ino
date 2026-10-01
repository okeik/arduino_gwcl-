/*
 * 賽車遊戲控制器（RALLY / RAW 2）— Arduino Uno 版
 * Uno 不能直接模擬 USB 鍵盤，所以改成：
 *   Uno 透過 Serial 送出「按下 / 放開」訊息 → 電腦上的 serial_to_keys.py 代按鍵盤。
 *
 * 訊息格式（每行一個）：+w 代表按下 W，-w 代表放開 W；空白鍵寫成 sp。
 *
 * 接線：
 *   搖桿模組  VCC → 5V、GND → GND、VRx → A0、VRy → A1、SW → D10（手煞車 Space）
 *   按鈕      一腳接腳位、另一腳接 GND（使用內建上拉電阻，不需外加電阻）
 *             D2 → Q 降檔      D3 → E 升檔      D4 → X 倒車檔
 *             D5 → M 自排/手排  D6 → C 切換視角  D7 → T 遙測面板
 *             D8 → R 扶正車輛   D9 → P 暫停
 */

// ---------- 搖桿設定 ----------
const int JOY_X_PIN = A0;
const int JOY_Y_PIN = A1;

// 搖桿推離中心超過這個值才算按下（0~512），太敏感就調大
const int DEAD_ZONE = 200;

// 方向相反時把對應的值改成 true
const bool INVERT_X = false;  // true：左右對調
const bool INVERT_Y = false;  // true：前後對調

// ---------- 按鈕設定 ----------
struct Button {
  uint8_t pin;
  const char *key;
  bool pressed;          // 目前送出的狀態
  bool lastReading;      // 上一次讀到的狀態
  unsigned long changedAt;
};

Button buttons[] = {
  {10, "sp", false, false, 0},  // 搖桿 SW：手煞車
  {2,  "q",  false, false, 0},  // 降檔
  {3,  "e",  false, false, 0},  // 升檔
  {4,  "x",  false, false, 0},  // 倒車檔
  {5,  "m",  false, false, 0},  // 自排/手排
  {6,  "c",  false, false, 0},  // 切換視角
  {7,  "t",  false, false, 0},  // 遙測面板
  {8,  "r",  false, false, 0},  // 扶正車輛
  {9,  "p",  false, false, 0},  // 暫停
};
const int BUTTON_COUNT = sizeof(buttons) / sizeof(buttons[0]);
const unsigned long DEBOUNCE_MS = 20;

// ---------- 搖桿方向對應的按鍵 ----------
struct Direction {
  const char *key;
  bool pressed;
};

Direction dirUp    = {"w", false};  // 油門
Direction dirDown  = {"s", false};  // 煞車/倒車
Direction dirLeft  = {"a", false};  // 左轉
Direction dirRight = {"d", false};  // 右轉

int centerX = 512;
int centerY = 512;

void sendKey(const char *key, bool press) {
  Serial.print(press ? '+' : '-');
  Serial.println(key);
}

void setKey(Direction &dir, bool shouldPress) {
  if (shouldPress == dir.pressed) {
    return;
  }
  sendKey(dir.key, shouldPress);
  dir.pressed = shouldPress;
}

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < BUTTON_COUNT; i++) {
    pinMode(buttons[i].pin, INPUT_PULLUP);
  }

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
  // 搖桿 → WASD
  int dx = analogRead(JOY_X_PIN) - centerX;
  int dy = analogRead(JOY_Y_PIN) - centerY;
  if (INVERT_X) dx = -dx;
  if (INVERT_Y) dy = -dy;

  // 多數搖桿模組往前推時 VRy 變小，所以 dy < 0 代表往前
  setKey(dirUp,    dy < -DEAD_ZONE);
  setKey(dirDown,  dy >  DEAD_ZONE);
  setKey(dirLeft,  dx < -DEAD_ZONE);
  setKey(dirRight, dx >  DEAD_ZONE);

  // 按鈕 → 其他按鍵（按下時腳位為 LOW）
  unsigned long now = millis();
  for (int i = 0; i < BUTTON_COUNT; i++) {
    Button &b = buttons[i];
    bool reading = digitalRead(b.pin) == LOW;

    if (reading != b.lastReading) {
      b.lastReading = reading;
      b.changedAt = now;
    }

    if (now - b.changedAt >= DEBOUNCE_MS && reading != b.pressed) {
      sendKey(b.key, reading);
      b.pressed = reading;
    }
  }

  delay(5);
}
