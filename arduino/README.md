# Arduino 賽車控制器

用 Arduino 操控 [賽車遊戲](../car-racing-1/index.html)：搖桿取代 WASD，按鈕取代其他按鍵。

| 資料夾 | 適用板子 | 做法 |
|---|---|---|
| [`racing_serial_uno/`](racing_serial_uno/) | **Arduino Uno**（目前用的板子） | Uno 透過 USB 傳訊號 → 電腦執行 `serial_to_keys.py` 代按鍵盤 |
| [`racing_keyboard/`](racing_keyboard/) | Leonardo / Micro / Pro Micro / Uno R4 | 板子直接被電腦當成 USB 鍵盤，不用另外執行程式 |

兩個版本的接線完全一樣。

## 按鍵對應

| 控制元件 | Arduino 腳位 | 鍵盤按鍵 | 遊戲功能 |
|---|---|---|---|
| 搖桿往前 | A1（VRy） | W | 油門 |
| 搖桿往後 | A1（VRy） | S | 煞車／倒車 |
| 搖桿往左 | A0（VRx） | A | 左轉 |
| 搖桿往右 | A0（VRx） | D | 右轉 |
| 搖桿按下 | D10（SW） | Space | 手煞車 |
| 按鈕 1 | D2 | Q | 降檔 |
| 按鈕 2 | D3 | E | 升檔 |
| 按鈕 3 | D4 | X | 倒車檔 |
| 按鈕 4 | D5 | M | 自排／手排切換 |
| 按鈕 5 | D6 | C | 切換視角 |
| 按鈕 6 | D7 | T | 顯示／隱藏遙測面板 |
| 按鈕 7 | D8 | R | 扶正車輛 |
| 按鈕 8 | D9 | P | 暫停 |

## 接線

材料：搖桿模組 ×1、按鈕（輕觸開關）×8、麵包板、杜邦線。**不需要電阻**，程式使用 Arduino 內建的上拉電阻。

### 搖桿模組（5 支腳）

```
搖桿模組          Arduino
  GND   ───────►  GND
  +5V   ───────►  5V
  VRx   ───────►  A0
  VRy   ───────►  A1
  SW    ───────►  D10
```

### 按鈕（每顆都一樣）

每顆按鈕的一支腳接到 Arduino 腳位，另一支腳接 GND。

```
D2 ──┤按鈕├── GND      (Q 降檔)
D3 ──┤按鈕├── GND      (E 升檔)
D4 ──┤按鈕├── GND      (X 倒車檔)
D5 ──┤按鈕├── GND      (M 自排/手排)
D6 ──┤按鈕├── GND      (C 視角)
D7 ──┤按鈕├── GND      (T 遙測)
D8 ──┤按鈕├── GND      (R 扶正)
D9 ──┤按鈕├── GND      (P 暫停)
```

麵包板的做法：把 Arduino 的 GND 接到麵包板的藍色（−）長條，所有按鈕的另一支腳都接到這條。

> 四腳的輕觸開關：同一側相鄰的兩支腳是「按下才導通」，對角的兩支腳一定會切換。不確定的話，接**對角**的兩支腳最保險。

## 使用方式：Uno 版（`racing_serial_uno`）

1. 用 Arduino IDE 打開 `racing_serial_uno/racing_serial_uno.ino`，板子選 **Arduino Uno**、序列埠選 **COM3**，按上傳。
2. 上傳完**關掉 IDE 的序列埠監控視窗**（不然 COM3 會被佔用）。
3. 安裝 Python 套件（只需一次）：

   ```bash
   pip install pyserial
   ```

4. 執行轉換程式：

   ```bash
   python arduino/racing_serial_uno/serial_to_keys.py COM3
   ```

5. 用 Chrome 打開賽車遊戲，**點一下遊戲畫面讓它取得焦點**，就可以用搖桿和按鈕操作。結束時在終端機按 `Ctrl+C`。

## 使用方式：Leonardo 版（`racing_keyboard`）

1. Arduino IDE 的程式庫管理員安裝 **Keyboard**。
2. 打開 `racing_keyboard/racing_keyboard.ino`，板子選 **Arduino Leonardo**（或你的板子），上傳。
3. 打開遊戲就能直接用，不需要執行 Python。

> 注意：Leonardo 版一插上就會開始按鍵盤。如果搖桿接錯導致一直送出按鍵，可以按住板子的 Reset 鍵再上傳修正後的程式。

## 調整

- **方向相反**：把程式裡的 `INVERT_X` 或 `INVERT_Y` 改成 `true`。
- **太敏感／不夠敏感**：調整 `DEAD_ZONE`（預設 200，越大要推越多才會觸發）。
- **開機校正**：插上電的前 0.5 秒會讀取搖桿中心位置，這段時間不要碰搖桿。
