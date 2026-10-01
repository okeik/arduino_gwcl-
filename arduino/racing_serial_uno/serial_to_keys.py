"""
把 Arduino Uno 送來的 Serial 訊息轉成鍵盤按鍵（Windows）。

用法：
    pip install pyserial
    python serial_to_keys.py COM3

訊息格式：+w 按下 W、-w 放開 W；空白鍵寫成 sp。
用掃描碼送出按鍵，瀏覽器收到的 event.code 才會是 KeyW 等，遊戲才認得。
按 Ctrl+C 結束，結束時會放開所有按鍵。
"""

import ctypes
import sys
from ctypes import wintypes

import serial

# 鍵盤掃描碼（Set 1）
SCAN_CODES = {
    "w": 0x11, "a": 0x1E, "s": 0x1F, "d": 0x20,
    "q": 0x10, "e": 0x12, "x": 0x2D, "m": 0x32,
    "c": 0x2E, "t": 0x14, "r": 0x13, "p": 0x19,
    "sp": 0x39,
}

INPUT_KEYBOARD = 1
KEYEVENTF_KEYUP = 0x0002
KEYEVENTF_SCANCODE = 0x0008


class KEYBDINPUT(ctypes.Structure):
    _fields_ = [
        ("wVk", wintypes.WORD),
        ("wScan", wintypes.WORD),
        ("dwFlags", wintypes.DWORD),
        ("time", wintypes.DWORD),
        ("dwExtraInfo", ctypes.c_size_t),
    ]


class MOUSEINPUT(ctypes.Structure):
    _fields_ = [
        ("dx", wintypes.LONG),
        ("dy", wintypes.LONG),
        ("mouseData", wintypes.DWORD),
        ("dwFlags", wintypes.DWORD),
        ("time", wintypes.DWORD),
        ("dwExtraInfo", ctypes.c_size_t),
    ]


class INPUT(ctypes.Structure):
    class _U(ctypes.Union):
        _fields_ = [("ki", KEYBDINPUT), ("mi", MOUSEINPUT)]

    _anonymous_ = ("u",)
    _fields_ = [("type", wintypes.DWORD), ("u", _U)]


def send_key(scan, press):
    flags = KEYEVENTF_SCANCODE | (0 if press else KEYEVENTF_KEYUP)
    inp = INPUT(type=INPUT_KEYBOARD)
    inp.ki = KEYBDINPUT(0, scan, flags, 0, 0)
    ctypes.windll.user32.SendInput(1, ctypes.byref(inp), ctypes.sizeof(INPUT))


def main():
    port = sys.argv[1] if len(sys.argv) > 1 else "COM3"
    held = set()

    with serial.Serial(port, 115200, timeout=1) as ser:
        print(f"已連接 {port}，切到遊戲視窗即可操作。按 Ctrl+C 結束。")
        try:
            while True:
                line = ser.readline().decode(errors="ignore").strip()
                if len(line) < 2 or line[0] not in "+-":
                    continue
                key = line[1:]
                scan = SCAN_CODES.get(key)
                if scan is None:
                    continue
                press = line[0] == "+"
                send_key(scan, press)
                if press:
                    held.add(key)
                else:
                    held.discard(key)
                print(line)
        except KeyboardInterrupt:
            pass
        finally:
            for key in held:
                send_key(SCAN_CODES[key], False)


if __name__ == "__main__":
    main()
