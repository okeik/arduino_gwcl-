# 物理課 exp2 紀錄

- 日期：2026-10-01
- 專案 repo：<https://github.com/okeik/arduino_gwcl-.git>
- 遊戲網址（GitHub Pages）：<https://okeik.github.io/arduino_gwcl-/car-racing-1/>
- 上一次紀錄：[物理課exp1.md](物理課exp1.md)

## 今天的目標

用 Arduino Uno 做一個賽車遊戲的實體控制器，並把遊戲狀態回饋到實體元件上：

- **輸入**：搖桿取代 WASD，一顆按鈕當手煞車，其他功能維持用鍵盤。
- **輸出**：用風扇模擬開車時吹風的感覺；通過檢查點時紅燈亮 1 秒。

---

## 1. 第一版：搖桿 + 8 顆按鈕模擬鍵盤

- 先從遊戲程式找出所有按鍵：WASD／方向鍵（油門、煞車、轉向）、Space（手煞車）、Q／E（降檔／升檔）、X（倒車檔）、M（自排／手排）、C（視角）、T（遙測）、R（扶正）、P（暫停）。
- 發現我們的板子是 **Arduino Uno**，它**不能直接模擬 USB 鍵盤**（只有 Leonardo、Micro 這類板子可以）。
- 解法：Uno 透過 USB 序列埠（Serial）送出「按下／放開」訊息，電腦上的 Python 程式 `serial_to_keys.py` 收到後代按鍵盤。
  - 按鍵要用「掃描碼」送出，瀏覽器收到的按鍵代碼才會是 `KeyW` 這類，遊戲才認得。

## 2. 第二版：搖桿 + 手煞車按鈕 + 風扇 + 紅燈

需求調整：只留搖桿和一顆手煞車按鈕，其他功能改回用鍵盤；另外加上風扇和紅燈。

風扇和紅燈需要知道遊戲裡的**車速**和**有沒有通過檢查點**，所以資料要從遊戲送回 Arduino：

```
搖桿、按鈕 ──► Arduino ──USB──► serial_to_keys.py ──鍵盤按鍵──► 遊戲
風扇、紅燈 ◄── Arduino ◄──USB── serial_to_keys.py ◄──車速、檢查點── 遊戲
```

- **修改遊戲**（`car-racing-1/index.html`）：每 0.1 秒把車速和「是否通過檢查點」送到 `http://localhost:8000/telemetry`。沒有執行電腦端程式時，改成每 3 秒才重試一次，不影響遊戲。
- **電腦端程式**（`serial_to_keys.py`）：開一個本機網頁伺服器接收遊戲的資料，再轉給 Arduino：
  - `S<車速>`，例如 `S87`，代表車速 87 km/h。
  - `C` 代表通過檢查點。
- **Arduino 程式**：收到車速就控制風扇；收到 `C` 就讓紅燈亮 1 秒。超過 1 秒沒收到車速（例如遊戲關掉），風扇自動停止。

## 3. 發布遊戲到 GitHub Pages

- 開啟 repo 的 GitHub Pages（從 `main` 分支發布）。
- 遊戲網址改成我們自己的：<https://okeik.github.io/arduino_gwcl-/car-racing-1/>
- 電腦端程式執行時會自動打開這個網址。
- 從 GitHub Pages 開的遊戲要連到電腦上的 `localhost`，所以電腦端程式加上了允許跨網域連線的設定。Chrome 第一次可能會詢問是否允許存取本機或區域網路，要按「允許」。

## 4. 實際操作遇到的問題

### 4.1 安裝 pyserial

電腦端程式需要 Python 套件 `pyserial`：

```bash
pip install pyserial
```

### 4.2 COM3 撞號：上傳失敗、程式連不上

錯誤訊息：`The semaphore timeout period has expired`（Python 和 Arduino IDE 上傳時都出現）。

- **原因**：電腦上有兩個裝置都叫 **COM3**，一個是 Arduino Uno，另一個是藍牙裝置產生的序列埠。程式去開 COM3 時打到藍牙那個，藍牙沒回應就逾時。
- **解法**：把藍牙關掉後，只剩 Arduino 是 COM3，就能正常上傳了。另一種一勞永逸的做法，是在裝置管理員把 Arduino 改成沒人用的編號（例如 COM9）。

### 4.3 風扇只有正負兩條線

- 風扇是兩條線的直流馬達，**不能直接接在 Arduino 腳位上**。腳位電流只有約 20～40 mA，風扇通常要 100 mA 以上。
- 手上能用的是**繼電器模組**（控制端 `+`、`-`、`S`）。繼電器只能全開或全關，不能調轉速，所以程式改成：
  - 車速**超過 40 km/h** 風扇開，**低於 30 km/h** 風扇關。兩個門檻分開，避免繼電器在門檻附近一直切換。
  - 繼電器最短 0.5 秒才能切換一次，保護繼電器。
  - 如果之後改用電晶體或馬達驅動模組，把程式裡的 `FAN_USE_RELAY` 改成 `false`，風扇轉速就會隨車速變化。

### 4.4 四腳按鈕

四腳按鈕的腳是兩兩一組，在按鈕裡面已經接通。接**對角**的兩支腳最保險：一腳接 D2，另一腳接 GND。

### 4.5 繼電器一響，風扇不轉、搖桿也失靈

- **現象**：車速到 40 km/h 時聽到繼電器「喀」一聲，但風扇沒有轉，之後搖桿就控制不了車子。
- **推測原因**：繼電器合上的瞬間 5V 被拉垮，Arduino 跟著重新啟動。可能是 5V 和 GND 短路，或風扇太耗電、USB 供電不夠。Arduino 重新啟動後忘記油門正按著，電腦端卻以為 W 一直按著，**油門就卡住了**，看起來像搖桿失靈。
- **已做的保護**：
  - Arduino 每次啟動都會送出 `#ready`。
  - 電腦端程式收到後，會放開所有按著的鍵，並印出「Arduino 已啟動」。如果這行一直重複出現，就代表電源被拉垮。
- **待檢查**：
  1. 有沒有短路：繼電器的 NO、COM 不能碰到 GND；風扇的 + 和 − 不能插在麵包板同一排。
  2. 把風扇直接接 5V／GND 測試，看 USB 供電夠不夠。不夠的話，改用電池盒或行動電源單獨供電給風扇。

---

## 目前的接線

| 元件 | 接法 | 作用 |
|---|---|---|
| 搖桿模組 | +5V → 5V、GND → GND、VRx → A0、VRy → A1（SW 不接） | 前後 = W／S，左右 = A／D |
| 手煞車按鈕（四腳） | 對角兩腳：一腳 → D2、另一腳 → GND | Space 手煞車 |
| 繼電器模組 | `+` → 5V、`-` → GND、`S` → D5 | 控制風扇開關 |
| 繼電器開關端 | COM → 5V、NO → 風扇 +（NC 不接） | 合上時風扇通電 |
| 風扇 | + → 繼電器 NO、− → GND | 模擬吹風 |
| 紅燈 LED | D12 → 220Ω → LED 長腳，短腳 → GND | 通過檢查點亮 1 秒 |

詳細接線圖和調整方式見 [`arduino/README.md`](arduino/README.md)。

## 使用步驟

1. 關掉電腦的藍牙，避免 COM3 撞號。
2. 用 Arduino IDE 上傳 `arduino/racing_serial_uno/racing_serial_uno.ino` 到 Uno（COM3），然後關掉序列埠監控視窗。
3. 執行電腦端程式：

   ```bash
   python "D:\Arduino物理課\arduino\racing_serial_uno\serial_to_keys.py" COM3
   ```

4. 遊戲會自動打開。點一下遊戲畫面，按「開始駕駛」。
5. 結束時在 PowerShell 按 `Ctrl+C`。

## 目前進度

| 項目 | 狀態 |
|---|---|
| 搖桿控制方向（WASD） | 程式完成，已上傳到 Arduino |
| 手煞車按鈕 | 程式完成，已上傳到 Arduino |
| 遊戲傳送車速、檢查點 | 完成，已用模擬序列埠測試，車速有正確傳出 |
| 繼電器控制風扇 | 繼電器有動作，**風扇還不會轉**，待檢查接線和供電 |
| 紅燈提示檢查點 | 程式完成，還沒在實機上確認 |
| Arduino 重新啟動時自動放開按鍵 | 完成，已確認 Arduino 會送出 `#ready` |

## 今天的 repo 變動

```
arduino_gwcl-/
├── arduino/
│   ├── README.md                     # 接線圖、使用方式、問題排除
│   └── racing_serial_uno/
│       ├── racing_serial_uno.ino     # Arduino Uno 程式
│       └── serial_to_keys.py         # 電腦端程式
├── car-racing-1/
│   └── index.html                    # 賽車遊戲（加上傳送車速、檢查點）
├── 物理課exp1.md
└── 物理課exp2.md                      # 本紀錄
```
