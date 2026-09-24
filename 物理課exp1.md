# 物理課 exp1 紀錄

- 日期：2026-09-24
- 專案 repo：<https://github.com/okeik/arduino_gwcl-.git>
- 組員：`（待填）`、`（待填）`、`（待填）`、`（待填）`

> 標示「待填」的地方是還沒確認的細節，請組員補上。

---

## 1. 下載並安裝 Arduino IDE

- 從 Arduino 官網（<https://www.arduino.cc/en/software>）下載 Arduino IDE 並安裝。
- 版本：`（待填）`
- 使用的板子型號：`（待填，例如 Arduino Leonardo / Uno）`

## 2. 賽車遊戲

- 遊戲名稱：**RALLY / RAW 2 — 拉力動力學實驗場**
- 線上網址：<https://jack72299947229994-stack.github.io/car-racing-1/>
- 原始碼：<https://github.com/jack72299947229994-stack/car-racing-1>
- 本 repo 中的副本：[`car-racing-1/index.html`](car-racing-1/index.html)

遊戲特色（依程式內容整理）：

- 單一 HTML 檔，瀏覽器直接開啟即可執行（建議用桌面版 Chrome）。
- 以 3D 引擎模擬車輛的剛體動力學，包含輪胎摩擦、滑移、懸吊彈簧壓縮、輪速等物理量。
- 畫面提供遙測資訊（FPS、力、滑移等），可以用來觀察物理量的變化。
- 操作方式：
  - 鍵盤：方向鍵可操作，另有油門／煞車、手煞車、升降檔、切換視角等按鍵。
  - 遊戲手把／方向盤：透過瀏覽器的 **Gamepad API** 讀取，只支援電腦能辨識成遊戲手把的裝置。

## 3. 連接 Arduino

- 連接方式：USB 連接電腦。
- 目的：`（待填，例如把 Arduino 做成方向盤／控制器來操控賽車遊戲）`
- 注意：遊戲只支援可被 Gamepad API 辨識的裝置。Arduino 要讓電腦當成遊戲手把，才能直接操控遊戲（例如用 Leonardo／Micro 這類可模擬 USB HID 的板子）。
- 使用的感測器／元件：`（待填）`
- 接線方式：`（待填）`
- Arduino 程式：`（待填，之後可放進本 repo）`

## 4. 建立四人共用的 GitHub

- 建立 repo：<https://github.com/okeik/arduino_gwcl-.git>
- 將四位組員加為協作者（Collaborators），大家都可以上傳修改。
- 電腦上的專案資料夾 `D:\Arduino物理課` 已和這個 repo 連接：
  1. `git init -b main`，建立本機 git repo。
  2. `git remote add origin https://github.com/okeik/arduino_gwcl-.git`，連接 GitHub。
  3. 把賽車遊戲程式碼複製到 `car-racing-1/`，commit 後 `git push` 上傳。
- 之後的工作流程：開始前先 `git pull` 抓最新版本 → 修改 → `git commit` → `git push`。

---

## 目前 repo 結構

```
arduino_gwcl-/
├── car-racing-1/
│   └── index.html      # 賽車遊戲（RALLY / RAW 2）
└── 物理課exp1.md        # 本紀錄
```
