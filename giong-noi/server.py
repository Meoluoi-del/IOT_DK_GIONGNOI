"""Local voice-command bridge. Python 3.10+, standard library only."""
import getpass
import json
import os
from pathlib import Path
import re
import unicodedata
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.error import HTTPError, URLError
from urllib.parse import urlencode
from urllib.request import urlopen

ROOT = Path(__file__).resolve().parent
HOST = "127.0.0.1"
PORT = 8000
TOKEN = ""
BLYNK_HOST = "sgp1.blynk.cloud"


def parse_command(text):
    """Accept exactly one supported command; reject negation and mixed commands."""
    if not isinstance(text, str) or len(text) > 200:
        raise ValueError("Hãy nhập hoặc nói một câu lệnh ngắn.")
    normalized = unicodedata.normalize("NFD", text.lower().replace("đ", "d"))
    normalized = "".join(c for c in normalized if unicodedata.category(c) != "Mn")
    normalized = " ".join(re.sub(r"[^a-z0-9\s]", " ", normalized).split())
    commands = {
        "bat den": ("v3", "1", "bật đèn"),
        "tat den": ("v3", "0", "tắt đèn"),
        "bat quat": ("v4", "1", "bật quạt"),
        "tat quat": ("v4", "0", "tắt quạt"),
    }
    if normalized not in commands:
        raise ValueError("Chỉ hỗ trợ: bật đèn, tắt đèn, bật quạt, tắt quạt. Mỗi lần một lệnh.")
    return commands[normalized]


def send_command(text):
    pin, value, label = parse_command(text)
    online_query = urlencode({"token": TOKEN})
    online_url = f"https://{BLYNK_HOST}/external/api/isHardwareConnected?{online_query}"
    with urlopen(online_url, timeout=10) as response:
        if response.status != 200 or response.read(128).strip().lower() != b"true":
            raise RuntimeError("ESP32 đang Offline trên Blynk. Hãy chạy mô phỏng Wokwi rồi thử lại.")
    query = urlencode({"token": TOKEN, pin: value})
    # HTTPS validates the server certificate using Python's trusted CA store.
    # Do not log this URL: it contains the device token.
    url = f"https://{BLYNK_HOST}/external/api/update?{query}"
    with urlopen(url, timeout=10) as response:
        if response.status != 200:
            raise RuntimeError("Blynk chưa chấp nhận lệnh.")
        response.read(4096)
    return f"Blynk đã nhận lệnh {label}. Kiểm tra LED/relay trong Wokwi để xác nhận thiết bị thực hiện."


class Handler(BaseHTTPRequestHandler):
    def log_message(self, format, *args):
        pass

    def respond(self, status, payload, content_type="application/json; charset=utf-8"):
        body = json.dumps(payload, ensure_ascii=False).encode("utf-8") if isinstance(payload, dict) else payload
        self.send_response(status)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-store")
        self.send_header("X-Content-Type-Options", "nosniff")
        self.end_headers()
        self.wfile.write(body)

    def host_allowed(self):
        return self.headers.get("Host") in {f"{HOST}:{PORT}", f"localhost:{PORT}"}

    def do_GET(self):
        if not self.host_allowed():
            return self.respond(403, {"message": "Host không hợp lệ."})
        if self.path not in {"/", "/index.html"}:
            return self.respond(404, {"message": "Không tìm thấy trang."})
        self.respond(200, (ROOT / "index.html").read_bytes(), "text/html; charset=utf-8")

    def do_POST(self):
        origin = self.headers.get("Origin")
        allowed_origins = {f"http://{HOST}:{PORT}", f"http://localhost:{PORT}"}
        if not self.host_allowed() or origin not in allowed_origins:
            return self.respond(403, {"message": "Chỉ nhận lệnh từ trang web cục bộ."})
        if self.path != "/command":
            return self.respond(404, {"message": "Không tìm thấy đường dẫn."})
        if self.headers.get_content_type() != "application/json":
            return self.respond(415, {"message": "Yêu cầu phải là JSON."})
        try:
            length = int(self.headers.get("Content-Length", "0"))
            if not 0 < length <= 2048:
                raise ValueError("Dữ liệu gửi không hợp lệ.")
            payload = json.loads(self.rfile.read(length))
            if not isinstance(payload, dict):
                raise ValueError("Dữ liệu gửi không hợp lệ.")
            message = send_command(payload.get("text", ""))
            self.respond(200, {"message": message})
        except (ValueError, UnicodeDecodeError) as exc:
            self.respond(400, {"message": str(exc)})
        except HTTPError as exc:
            self.respond(502, {"message": f"Blynk trả lỗi HTTP {exc.code}. Kiểm tra token, hostname và V3/V4."})
        except RuntimeError as exc:
            self.respond(502, {"message": str(exc)})
        except (URLError, TimeoutError, OSError):
            self.respond(502, {"message": "Không xác nhận được kết quả gửi. Kiểm tra Internet, Blynk và trạng thái Wokwi trước khi thử lại."})


def main():
    global TOKEN, BLYNK_HOST
    TOKEN = os.environ.get("BLYNK_AUTH_TOKEN", "").strip() or getpass.getpass("Device Auth Token (an khi nhap): ").strip()
    BLYNK_HOST = os.environ.get("BLYNK_SERVER", "").strip() or input("Blynk hostname [sgp1.blynk.cloud]: ").strip() or "sgp1.blynk.cloud"
    if not TOKEN:
        raise SystemExit("Can Device Auth Token de gui lenh.")
    if not re.fullmatch(r"(?:[a-z0-9-]+\.)?blynk\.cloud", BLYNK_HOST):
        raise SystemExit("Hostname phai la blynk.cloud hoac hostname vung *.blynk.cloud.")
    print(f"Mo http://{HOST}:{PORT} bang Chrome. Ctrl+C de dung.")
    with ThreadingHTTPServer((HOST, PORT), Handler) as server:
        try:
            server.serve_forever()
        except KeyboardInterrupt:
            pass


if __name__ == "__main__":
    main()
