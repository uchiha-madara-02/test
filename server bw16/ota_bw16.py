import socket
import os
import sys
import time

# Địa chỉ mặc định của mạch BW16 khi phát WiFi
TARGET_IP = "192.168.1.1" 
TARGET_PORT = 8082
CHUNK_SIZE = 1024  

def draw_progress_bar(percent, sent, total):
    bar_length = 30
    filled = int(bar_length * percent // 100)
    bar = '█' * filled + '-' * (bar_length - filled)
    sys.stdout.write(f"\r[|{bar}| {percent}%] {sent}/{total} B")
    sys.stdout.flush()

def upload_firmware(filepath):
    if not os.path.exists(filepath):
        print(f"Lỗi: Không tìm thấy tệp '{filepath}'")
        return

    filesize = os.path.getsize(filepath)
    print("\n" + "="*40)
    print(" CÔNG CỤ NẠP OTA BW16 (TERMUX)")
    print("="*40)
    print(f"File: {os.path.basename(filepath)}")
    print(f"Size: {filesize} bytes")
    print(f"Đang kết nối tới BW16 ({TARGET_IP})...")

    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(10)
        sock.connect((TARGET_IP, TARGET_PORT))
        print("-> Đã kết nối! Đang bơm dữ liệu...\n")
    except Exception as e:
        print(f"Lỗi kết nối: {e}")
        print("Hãy kiểm tra xem điện thoại đã bắt đúng WiFi BW16 chưa!")
        return

    sent_bytes = 0
    try:
        with open(filepath, "rb") as f:
            while True:
                chunk = f.read(CHUNK_SIZE)
                if not chunk: break
                
                sock.sendall(chunk)
                sent_bytes += len(chunk)
                percent = int((sent_bytes / filesize) * 100)
                draw_progress_bar(percent, sent_bytes, filesize)
                
                # Cực kỳ quan trọng: Delay 10ms để chip BW16 không bị tràn RAM
                time.sleep(0.01) 

        print("\n\n-> Nạp hoàn tất! Mạch đang khởi động lại.")
    except Exception as e:
        print(f"\n\nLỗi truyền tải: {e}")
    finally:
        sock.close()

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Cách dùng: python up_ota.py <ten_file.bin>")
    else:
        upload_firmware(sys.argv[1])