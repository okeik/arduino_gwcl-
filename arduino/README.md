# Arduino 賽車控制器

用 Arduino Uno 操控 [賽車遊戲](../car-racing-1/index.html)，並把遊戲狀態回饋到實體元件：

- **輸入**：搖桿取代 WASD，一顆按鈕當手煞車。其他功能（換檔、視角、暫停等）維持用電腦鍵盤。
- **輸出**：風扇轉速隨車速變化，模擬吹風的感覺；通過檢查點時紅燈亮 1 秒。

程式在 [`racing_serial_uno/`](racing_serial_uno/)：

| 檔案 | 說明 |
|---|---|
| `racing_serial_uno.ino` | 上傳到 Arduino Uno 的程式 |
| `serial_to_keys.py` | 電腦端的橋接程式：把搖桿／按鈕轉成鍵盤按鍵，並把遊戲的車速、檢查點傳給 Arduino |

```
搖桿、按鈕 ──► Arduino ──USB──► serial_to_keys.py ──鍵盤按鍵──► 遊戲
風扇、紅燈 ◄── Arduino ◄──USB── serial_to_keys.py ◄──車速、檢查點── 遊戲
```

## 控制對應

| 元件 | Arduino 腳位 | 作用 |
|---|---|---|
| 搖桿往前／往後 | A1（VRy） | W 油門／S 煞車、倒車 |
| 搖桿往左／往右 | A0（VRx） | A 左轉／D 右轉 |
| 手煞車按鈕 | D2 | Space 手煞車 |
| 風扇 | D5（PWM） | 車速越快轉越快，5 km/h 以下停止，150 km/h 全速 |
| 紅燈 LED | D12 | 通過檢查點亮 1 秒 |

其他按鍵照常用鍵盤：Q／E 降檔／升檔、X 倒車檔、M 自排／手排、C 視角、T 遙測、R 扶正、P 暫停。

## 接線

材料：搖桿模組 ×1、按鈕 ×1、5V 小風扇（直流馬達）×1、NPN 電晶體（2N2222 或 S8050）×1、二極體 1N4007 ×1、紅色 LED ×1、電阻 1kΩ ×1、電阻 220Ω ×1、麵包板、杜邦線。

### 搖桿模組

```
搖桿模組          Arduino
  GND   ───────►  GND
  +5V   ───────►  5V
  VRx   ───────►  A0
  VRy   ───────►  A1
  SW    （不用接）
```

### 手煞車按鈕

```
D2 ──┤按鈕├── GND
```

不需要電阻，程式使用內建上拉電阻。四腳按鈕接**對角**的兩支腳最保險。

### 風扇（用電晶體驅動）

Arduino 腳位的電流不夠直接推動風扇，**不能把風扇直接接在 D5**，要用電晶體當開關：

```
            5V
             │
     ┌───────┼────────┐
     │       │        │
  1N4007   風扇 +      │
 (白線端朝上) │        │
     │     風扇 −      │
     └───────┤        │
             │ C（集極）
D5 ─[1kΩ]─── B（基極）  NPN 電晶體
             │ E（射極）
             │
            GND
```

- 風扇 + → 5V，風扇 − → 電晶體 C（集極）。
- 電晶體 E（射極）→ GND。
- D5 → 1kΩ 電阻 → 電晶體 B（基極）。
- 二極體 1N4007 跨接在風扇兩端：**有白線的那端接 5V**，另一端接風扇 −。它保護電路不被馬達停轉時的反向電壓打壞。
- 2N2222 和 S8050 平面朝自己、腳朝下時，腳位由左到右都是 **E、B、C**。不同型號可能不同，請查元件的資料表。

> 用套件裡的 **L9110 風扇模組**：VCC → 5V、GND → GND、INA → D5、INB → GND，就不需要電晶體和二極體。若風扇反轉，把 INA、INB 對調。

### 紅燈 LED

```
D12 ──[220Ω]──► LED 長腳（+）   LED 短腳（−）──► GND
```

## 使用方式

1. 用 Arduino IDE 打開 `racing_serial_uno/racing_serial_uno.ino`，板子選 **Arduino Uno**、序列埠選 **COM3**，按上傳。
2. 上傳完**關掉 IDE 的序列埠監控視窗**（不然 COM3 會被佔用）。
3. 安裝 Python 套件（只需一次）：

   ```bash
   pip install pyserial
   ```

4. 執行橋接程式，它會自動用瀏覽器打開我們發布在 GitHub Pages 的遊戲 <https://okeik.github.io/arduino_gwcl-/car-racing-1/>：

   ```bash
   python arduino/racing_serial_uno/serial_to_keys.py COM3
   ```

5. **點一下遊戲畫面讓它取得焦點**，按「開始駕駛」就能用搖桿操作，風扇和紅燈會跟著遊戲動作。結束時在終端機按 `Ctrl+C`，會自動關掉風扇。

> - 遊戲透過 `http://localhost:8000/telemetry` 把車速和檢查點傳給橋接程式。Chrome 第一次可能會跳出「允許存取區域網路上的裝置」之類的詢問，請按**允許**，否則風扇和紅燈不會動。
> - 沒有網路時，可以加上 `--local`，改開電腦裡的遊戲（`http://localhost:8000/`）：
>
>   ```bash
>   python arduino/racing_serial_uno/serial_to_keys.py COM3 --local
>   ```

## 調整

程式開頭的設定值：

- **方向相反**：`INVERT_X` 或 `INVERT_Y` 改成 `true`。
- **搖桿太敏感／不夠敏感**：`DEAD_ZONE`（預設 200，越大要推越多才會觸發）。
- **風扇低速轉不動**：調大 `FAN_MIN_PWM`（預設 80，最大 255）。
- **風扇範圍**：`FAN_START_SPEED`（開始轉的車速，預設 5 km/h）、`FAN_FULL_SPEED`（全速的車速，預設 150 km/h）。
- **紅燈時間**：`LED_ON_MS`（預設 1000 毫秒）。
- **開機校正**：插上電的前 0.5 秒會讀取搖桿中心位置，這段時間不要碰搖桿。
