"""
Arduino Uno 和賽車遊戲之間的橋樑（Windows）。

1. 把 Arduino 送來的 Serial 訊息轉成鍵盤按鍵（搖桿 → WASD、手煞車按鈕 → Space）。
2. 在 http://localhost:8000/telemetry 接收遊戲送來的車速、檢查點並轉給 Arduino
   （風扇模擬吹風、紅燈提示通過檢查點）。

用法：
    pip install pyserial
    python serial_to_keys.py COM3          # 開啟 GitHub Pages 上的遊戲
    python serial_to_keys.py COM3 --local  # 沒有網路時，改開 http://localhost:8000/ 的本機遊戲

Arduino → 電腦：+w 按下 W、-w 放開 W；空白鍵寫成 sp。
電腦 → Arduino：S<車速 km/h>，例如 S87；C 代表通過檢查點。
用掃描碼送出按鍵，瀏覽器收到的 event.code 才會是 KeyW 等，遊戲才認得。
按 Ctrl+C 結束，結束時會放開所有按鍵並關掉風扇。
"""

import ctypes
import sys
import threading
import webbrowser
from ctypes import wintypes
from functools import partial
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import parse_qs, urlparse

import serial

HTTP_PORT = 8000
GAME_DIR = Path(__file__).resolve().parents[2] / "car-racing-1"
PUBLISHED_GAME_URL = "https://okeik.github.io/arduino_gwcl-/car-racing-1/"
LOCAL_GAME_URL = f"http://localhost:{HTTP_PORT}/"

# 鍵盤掃描碼（Set 1）
SCAN_CODES = {
    "w": 0x11, "a": 0x1E, "s": 0x1F, "d": 0x20,
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


class GameHandler(SimpleHTTPRequestHandler):
    """提供本機遊戲網頁，並把 /telemetry 收到的車速、檢查點轉給 Arduino。"""

    def __init__(self, *args, arduino, **kwargs):
        self.arduino = arduino
        super().__init__(*args, **kwargs)

    def end_headers(self):
        # 允許 GitHub Pages 上的遊戲連到本機
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Private-Network", "true")
        super().end_headers()

    def do_OPTIONS(self):
        self.send_response(204)
        self.send_header("Access-Control-Allow-Methods", "GET")
        self.end_headers()

    def do_GET(self):
        url = urlparse(self.path)
        if url.path != "/telemetry":
            super().do_GET()
            return

        query = parse_qs(url.query)
        try:
            speed = max(0, int(query.get("speed", ["0"])[0]))
        except ValueError:
            speed = 0
        self.arduino.write(f"S{speed}")
        if query.get("cp", ["0"])[0] == "1":
            self.arduino.write("C")
            print("通過檢查點 → 紅燈")

        self.send_response(204)
        self.end_headers()

    def log_message(self, format, *args):
        pass


class Arduino:
    def __init__(self, ser):
        self.ser = ser
        self.lock = threading.Lock()

    def write(self, line):
        with self.lock:
            self.ser.write(f"{line}\n".encode())


def main():
    args = [arg for arg in sys.argv[1:] if not arg.startswith("--")]
    port = args[0] if args else "COM3"
    game_url = LOCAL_GAME_URL if "--local" in sys.argv else PUBLISHED_GAME_URL
    held = set()

    with serial.Serial(port, 115200, timeout=1) as ser:
        arduino = Arduino(ser)
        handler = partial(GameHandler, arduino=arduino, directory=str(GAME_DIR))
        server = ThreadingHTTPServer(("127.0.0.1", HTTP_PORT), handler)
        threading.Thread(target=server.serve_forever, daemon=True).start()

        print(f"已連接 {port}，遊戲網址：{game_url}")
        print("點一下遊戲畫面即可操作。按 Ctrl+C 結束。")
        webbrowser.open(game_url)

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
            arduino.write("S0")
            server.shutdown()


if __name__ == "__main__":
    main()
