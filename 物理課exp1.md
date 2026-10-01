# 物理課 exp1 紀錄

- 日期：2026-09-24
- 專案 repo：<https://github.com/okeik/arduino_gwcl-.git>
- 組員：`（待填）`、`（待填）`、`（待填）`、`（待填）`

> 標示「待填」的地方是還沒確認的細節，請組員補上。

---

## 1. 下載並安裝 Arduino IDE

- 從 Arduino 官網（<https://www.arduino.cc/en/software>）下載 Arduino IDE 並安裝。
- 版本：`（待填）`
- 使用的板子型號：Arduino Uno

## 2. 賽車遊戲

- 遊戲名稱：**RALLY / RAW 2 — 拉力動力學實驗場**
- 我們發布的網址（GitHub Pages）：<https://okeik.github.io/arduino_gwcl-/car-racing-1/>
- 原作者：<https://github.com/jack72299947229994-stack/car-racing-1>
- 本 repo 中的程式：[`car-racing-1/index.html`](car-racing-1/index.html)（已加上傳送車速、檢查點給 Arduino 的功能）

遊戲特色（依程式內容整理）：

- 單一 HTML 檔，瀏覽器直接開啟即可執行（建議用桌面版 Chrome）。
- 以 3D 引擎模擬車輛的剛體動力學，包含輪胎摩擦、滑移、懸吊彈簧壓縮、輪速等物理量。
- 畫面提供遙測資訊（FPS、力、滑移等），可以用來觀察物理量的變化。
- 操作方式：
  - 鍵盤：方向鍵可操作，另有油門／煞車、手煞車、升降檔、切換視角等按鍵。
  - 遊戲手把／方向盤：透過瀏覽器的 **Gamepad API** 讀取，只支援電腦能辨識成遊戲手把的裝置。

## 3. 連接 Arduino

- 使用板子：Arduino Uno，透過 USB 連接電腦（COM3）。
- 輸入：搖桿取代 WASD，一顆按鈕當手煞車（Space）；其他功能維持用電腦鍵盤。
- 輸出：
  - 風扇轉速隨遊戲車速變化，模擬吹風的感覺（5 km/h 以下停止，150 km/h 全速）。
  - 通過檢查點時紅燈亮 1 秒。
- 做法：Uno 不能直接模擬 USB 鍵盤，所以電腦執行 Python 程式 `serial_to_keys.py` 當橋樑：
  - Arduino → 電腦：搖桿／按鈕的「按下／放開」訊息，轉成鍵盤按鍵。
  - 遊戲 → 電腦 → Arduino：車速和檢查點，控制風扇和紅燈。
- 使用的元件：搖桿模組 ×1、按鈕 ×1、5V 小風扇 ×1、NPN 電晶體 ×1、二極體 1N4007 ×1、紅色 LED ×1、電阻 1kΩ／220Ω 各 1、麵包板、杜邦線。
- 接線方式與使用說明：見 [`arduino/README.md`](arduino/README.md)。
- 程式：[`arduino/racing_serial_uno/`](arduino/racing_serial_uno/)

## 4. 建立四人共用的 GitHub

- 建立 repo：<https://github.com/okeik/arduino_gwcl-.git>
- 將四位組員加為協作者（Collaborators），大家都可以上傳修改。
- 電腦上的專案資料夾 `D:\Arduino物理課` 已和這個 repo 連接：
  1. `git init -b main`，建立本機 git repo。
  2. `git remote add origin https://github.com/okeik/arduino_gwcl-.git`，連接 GitHub。
  3. 把賽車遊戲程式碼複製到 `car-racing-1/`，commit 後 `git push` 上傳。
- 開啟 GitHub Pages（從 `main` 分支發布），遊戲網址為 <https://okeik.github.io/arduino_gwcl-/car-racing-1/>。
- 之後的工作流程：開始前先 `git pull` 抓最新版本 → 修改 → `git commit` → `git push`。

---

## 目前 repo 結構

```
arduino_gwcl-/
├── arduino/
│   ├── README.md                 # 接線與使用說明
│   └── racing_serial_uno/        # Uno 程式 + serial_to_keys.py
├── car-racing-1/
│   └── index.html                # 賽車遊戲（RALLY / RAW 2）
└── 物理課exp1.md                  # 本紀錄
```
