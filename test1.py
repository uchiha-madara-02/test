# ====================================================================
# BƯỚC 1: IMPORT CÁC THƯ VIỆN HỆ THỐNG CƠ BẢN ĐỂ CHẠY HÀM KIỂM TRA
# ====================================================================
import os
import sys
import subprocess
import platform
import importlib

# ====================================================================
# BƯỚC 2: HỆ THỐNG KIỂM TRA VÀ TỰ ĐỘNG CÀI ĐẶT
# ====================================================================
def kiem_tra_thu_vien_python():
    """Tự động tìm và cài đặt các thư viện Python bị thiếu."""
    print("\n[*] Đang kiểm tra các thư viện Python cần thiết...")
    
    # Đã bỏ 'python-nmap' vì chúng ta dùng subprocess để gọi Nmap trực tiếp (Nhanh hơn!)
    thu_vien_can_thiet = {
        'psutil': 'psutil',
        'requests': 'requests',
        'scapy': 'scapy',
        'pyarmor': 'pyarmor'
    }
    
    thu_vien_thieu = []
    for module_name, pip_name in thu_vien_can_thiet.items():
        try:
            importlib.import_module(module_name)
        except ImportError:
            thu_vien_thieu.append(pip_name)
            
    if thu_vien_thieu:
        print(f"[-] Hệ thống đang thiếu các thư viện: {', '.join(thu_vien_thieu)}")
        chon = input("[?] Bạn có muốn tự động tải và cài đặt tất cả không? [y/n]: ").strip().lower()
        
        if chon == 'y':
            for pip_name in thu_vien_thieu:
                print(f"    -> Đang tải {pip_name}...")
                try:
                    subprocess.check_call([sys.executable, "-m", "pip", "install", pip_name])
                    print(f"    [v] Đã cài xong {pip_name}!")
                except subprocess.CalledProcessError:
                    print(f"    [!] Lỗi khi cài đặt {pip_name}. Vui lòng kiểm tra lại mạng.")
                    sys.exit(1)
            print("[v] Hoàn tất cài đặt các thư viện Python!")
        else:
            print("[!] Công cụ cần các thư viện này để hoạt động. Đang thoát chương trình...")
            sys.exit(1)
    else:
        print("[v] Mọi thư viện Python đã đầy đủ!")

def kiem_tra_va_cai_dat_nmap_loi():
    """Kiểm tra Nmap lõi, tự động sửa lỗi PATH trên Windows nếu tìm thấy tệp exe."""
    print("\n[*] Đang kiểm tra phần mềm Nmap lõi trên hệ thống...")
    try:
        subprocess.run(['nmap', '-V'], stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=True)
        print("[v] Tuyệt vời! Nmap lõi đã sẵn sàng hoạt động.")
        return True
    except FileNotFoundError:
        pass 
        
    os_name = platform.system()
    if os_name == "Windows":
        thu_muc_nmap = r"C:\Program Files (x86)\Nmap"
        file_exe = os.path.join(thu_muc_nmap, "nmap.exe")
        if os.path.exists(file_exe):
            print("    -> Đã tìm thấy Nmap bị ẩn trong ổ C! Đang tự động kết nối (Add to PATH)...")
            os.environ["PATH"] += os.pathsep + thu_muc_nmap
            print("    [v] Đã sửa lỗi đường dẫn thành công. Nmap lõi đã sẵn sàng!")
            return True

    print("[-] Không tìm thấy Nmap lõi trên thiết bị của bạn.")
    chon = input("[?] Bạn có muốn tự động tải và cài đặt Nmap lõi không? [y/n]: ").strip().lower()
    if chon != 'y':
        print("    [!] Vui lòng tự tải file .exe từ https://nmap.org/download.html")
        return False

    print(f"[+] Đang tiến hành cài đặt Nmap lõi cho hệ điều hành: {os_name}...")
    try:
        if os_name == "Windows":
            print("    -> Đang gọi 'winget' để tải Nmap ngầm...")
            subprocess.run(['winget', 'install', 'Insecure.Nmap', '-e', '--silent'], check=True)
            os.environ["PATH"] += os.pathsep + r"C:\Program Files (x86)\Nmap"
            print("    [v] Đã cài đặt xong Nmap trên Windows!")
        elif os_name == "Linux":
            if os.path.exists('/data/data/com.termux'):
                subprocess.run(['pkg', 'install', 'nmap', '-y'], check=True)
            else:
                subprocess.run(['sudo', 'apt-get', 'install', 'nmap', '-y'], check=True)
            print("    [v] Đã cài đặt xong Nmap trên Linux/Android!")
        return True
    except Exception as e:
        print(f"    [-] Tự động cài đặt thất bại: {e}")
        return False

# CHẠY KIỂM TRA NGAY LẬP TỨC
kiem_tra_thu_vien_python()
kiem_tra_va_cai_dat_nmap_loi()
print("\n" + "="*50)
print(" [v] HỆ THỐNG KHỞI TẠO THÀNH CÔNG! ĐANG VÀO TOOL...")
print("="*50 + "\n")

# ====================================================================
# BƯỚC 3: SAU KHI CHẮC CHẮN MÁY ĐÃ CÀI ĐỦ, BẮT ĐẦU IMPORT PHẦN CÒN LẠI
# ====================================================================
# 3.1: Các thư viện có sẵn của Python (Không bao giờ lỗi)
import string
import hashlib
import time
import logging
import datetime
import threading

# 3.2: Các thư viện tải ngoài (Chắc chắn đã có vì hàm Bước 2 đã lo)
import psutil
import requests

# 3.3: Cấu hình Scapy đặc thù
from scapy.config import conf
# Fix lỗi Layer 2 trên Windows
conf.L3socket = conf.L3socket6 
logging.getLogger("scapy.runtime").setLevel(logging.ERROR)
from scapy.all import sniff, IP, TCP, Raw

# ==========================================
# CẤU HÌNH TOÀN CỤC
# ==========================================
VIRUSTOTAL_API_KEY = "729c0ebecf59e8d515fcc0d5fb9c8d282c00082a0f4e9a196282e721186f0a85"
LOG_FILE = "security_toolkit.log"
OS_SYSTEM = platform.system() # Tự động nhận diện: 'Windows' hoặc 'Linux' (Android)

def write_log(message):
    time_now = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    log_entry = f"[{time_now}] {message}"
    print(log_entry)
    try:
        with open(LOG_FILE, "a", encoding="utf-8") as f:
            f.write(log_entry + "\n")
    except Exception:
        pass

# ==========================================
# MODULE 1: VIRUSTOTAL & KIỂM TRA MÃ BĂM
# ==========================================
def get_file_hash(file_path):
    sha256 = hashlib.sha256()
    try:
        with open(file_path, "rb") as f:
            for block in iter(lambda: f.read(4096), b""):
                sha256.update(block)
        return sha256.hexdigest()
    except Exception:
        return None

def check_virustotal(file_hash):
    if VIRUSTOTAL_API_KEY == "NHAP_API_KEY_CUA_BAN_VAO_DAY":
        return "[Bỏ qua] Chưa có API Key"
    url = f"https://www.virustotal.com/api/v3/files/{file_hash}"
    headers = {"x-apikey": VIRUSTOTAL_API_KEY}
    try:
        res = requests.get(url, headers=headers)
        if res.status_code == 200:
            data = res.json()
            stats = data['data']['attributes']['last_analysis_stats']
            malicious = stats.get('malicious', 0)
            
            if malicious > 0:
                results = data['data']['attributes']['last_analysis_results']
                virus_names = []
                for av_engine, av_result in results.items():
                    if av_result['category'] == 'malicious' and av_result['result']:
                        virus_names.append(av_result['result'])
                ten_virus = virus_names[0] if virus_names else "Mã độc không xác định"
                return f"NGUY HIỂM ({malicious} AV phát hiện) - Loại: {ten_virus}"
            return "An toàn"
            
        return "Chưa có dữ liệu" if res.status_code == 404 else f"Lỗi API ({res.status_code})"
    except Exception as e:
        return f"Lỗi mạng: {e}"
# ==========================================
# MODULE 1: VIRUSTOTAL & KIỂM TRA MÃ BĂM
# ==========================================
# ==========================================
# MODULE 2: QUÉT TỆP VÀ ỨNG DỤNG (THÔNG MINH & TỐI ƯU)
# ==========================================
def get_all_drives():
    """Tự động tìm tất cả các ổ đĩa trên hệ thống."""
    drives = []
    if OS_SYSTEM == "Windows":
        for letter in string.ascii_uppercase:
            drive_path = f"{letter}:\\"
            if os.path.exists(drive_path):
                drives.append(drive_path)
    else:
        paths_to_check = ['/sdcard/Download', '/storage/emulated/0/Download', '/tmp']
        for p in paths_to_check:
            if os.path.exists(p):
                drives.append(p)
    return drives

def local_heuristic_scan(file_path):
    """Công cụ quét nội bộ với bộ từ điển mở rộng."""
    suspicious_signatures = [
        b"WScript.Shell",           
        b"powershell.exe -nop",     
        b"vssadmin delete shadows", 
        b"CreateRemoteThread",      
        b"VirtualAllocEx",          
        b"stratum+tcp://",          
        b"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
        b"FromBase64String",        # Thường dùng để giải mã virus ẩn
        b"Invoke-Expression",       # Lệnh chạy mã độc của Powershell
        b"Net.WebClient"            # Lệnh lén tải file từ internet
    ]
    
    score = 0
    detected_sigs = []
    
    try:
        with open(file_path, "rb") as f:
            content = f.read(2 * 1024 * 1024).lower() 
            
        for sig in suspicious_signatures:
            if sig.lower() in content:
                score += 1
                detected_sigs.append(sig.decode('utf-8', errors='ignore'))
                
        if score >= 2:
            return f"NGUY HIỂM NỘI BỘ: Tệp có hành vi rủi ro cao -> {detected_sigs}"
        elif score == 1:
            return f"Đáng ngờ (Mức thấp): Chứa lệnh -> {detected_sigs}"
        else:
            return "An toàn (Không thấy dấu hiệu độc hại)"
    except Exception as e:
        return f"Không thể đọc tệp để quét: {e}"

def scan_hidden_and_suspicious(root_directory):
    write_log(f"\n[+] Đang quét tệp ẩn nguy hiểm tại phân vùng: {root_directory}")
    suspicious_exts = ['.exe', '.bat', '.vbs', '.ps1', '.sh', '.apk']
    threat_count = 0
    
    # DANH SÁCH BỎ QUA: Bỏ qua thư mục hệ thống để tránh bị treo tool
    skip_dirs = ['$RECYCLE.BIN', 'WINDOWS', 'PROGRAM FILES', 'PROGRAM FILES (X86)']
    
    for root, dirs, files in os.walk(root_directory):
        # Tối ưu hóa: Bỏ qua các thư mục trong skip_dirs
        dirs[:] = [d for d in dirs if d.upper() not in skip_dirs]
        
        for file in files:
            try:
                is_hidden = file.startswith('.') or (OS_SYSTEM == "Windows" and file.startswith('$'))
                if is_hidden:
                    ext = os.path.splitext(file)[1].lower()
                    if ext in suspicious_exts:
                        threat_count += 1
                        path = os.path.join(root, file)
                        write_log(f"    [!!!] Tệp ẩn: {path}")
                        
                        local_result = local_heuristic_scan(path)
                        write_log(f"          -> Quét Cục Bộ: {local_result}")
                        
                        file_hash = get_file_hash(path)
                        vt_result = check_virustotal(file_hash)
                        write_log(f"          -> Quét VirusTotal: {vt_result}")
                        
                        if VIRUSTOTAL_API_KEY != "NHAP_API_KEY_CUA_BAN_VAO_DAY": 
                            time.sleep(15)
            except Exception:
                continue
    write_log(f"    -> Đã quét xong {root_directory}. Phát hiện {threat_count} tệp đáng ngờ.")

def scan_apps_and_processes():
    """Quét MỌI tiến trình đáng ngờ (Có dùng Whitelist để tối ưu)."""
    write_log(f"\n[+] Đang quét các Ứng dụng/Tiến trình đang hoạt động...")
    
    # DANH SÁCH TRẮNG: Không quét các lõi hệ thống này để đỡ tốn API VirusTotal
    safe_windows_procs = [
        'svchost.exe', 'explorer.exe', 'system', 'registry', 'smss.exe', 
        'csrss.exe', 'wininit.exe', 'services.exe', 'lsass.exe', 'winlogon.exe', 
        'fontdrvhost.exe', 'dwm.exe', 'spoolsv.exe', 'taskhostw.exe', 'SearchUI.exe'
    ]
    
    count_scanned = 0
    if OS_SYSTEM == "Windows":
        try:
            for proc in psutil.process_iter(['name', 'exe']):
                try:
                    exe_path, name = proc.info['exe'], proc.info['name']
                    # Bỏ qua nếu là tiến trình an toàn của Windows
                    if name and name.lower() in safe_windows_procs:
                        continue
                        
                    if exe_path and os.path.exists(exe_path) and name.endswith('.exe'):
                        h = get_file_hash(exe_path)
                        write_log(f"    [*] Quét tiến trình ngoài: {name} -> {check_virustotal(h)}")
                        count_scanned += 1
                        
                        if VIRUSTOTAL_API_KEY != "NHAP_API_KEY_CUA_BAN_VAO_DAY": 
                            time.sleep(15)
                except (psutil.AccessDenied, psutil.NoSuchProcess):
                    pass
            write_log(f"    -> Đã quét {count_scanned} tiến trình bên ngoài (Đã bỏ qua các tiến trình lõi).")
        except Exception as e:
            write_log(f"    [Lỗi] Không thể quét Tiến trình: {e}")
            
    elif OS_SYSTEM == "Linux": # Android
        try:
            output = subprocess.check_output(['pm', 'list', 'packages', '-f']).decode('utf-8')
            # Lấy 10 app trên Android thôi để đỡ lâu
            for line in output.strip().split('\n')[:10]:
                apk_path = line.split('=')[0].replace('package:', '')
                app_name = line.split('=')[-1]
                if os.path.exists(apk_path):
                    h = get_file_hash(apk_path)
                    write_log(f"    [*] Quét App: {app_name} -> {check_virustotal(h)}")
                    if VIRUSTOTAL_API_KEY != "NHAP_API_KEY_CUA_BAN_VAO_DAY": time.sleep(15)
        except Exception as e:
            write_log(f"    [Lỗi] Không thể quét APK: {e}")

# ==========================================
# MODULE 3: PHẦN CỨNG (CAM/MIC) & HỆ THỐNG
# ==========================================
def check_win_device_active(device_type):
    """Quét cả ứng dụng Desktop và ứng dụng Windows Store."""
    active_apps = []
    try:
        import winreg
        # 1. Nhánh NonPackaged (Các phần mềm tải từ web như Chrome, Zalo, OBS...)
        reg_path_non_packaged = rf"SOFTWARE\Microsoft\Windows\CurrentVersion\CapabilityAccessManager\ConsentStore\{device_type}\NonPackaged"
        
        def check_registry_key(path):
            try:
                with winreg.OpenKey(winreg.HKEY_CURRENT_USER, path) as key:
                    count = winreg.QueryInfoKey(key)[0]
                    for i in range(count):
                        app = winreg.EnumKey(key, i)
                        try:
                            with winreg.OpenKey(key, app) as app_key:
                                start, _ = winreg.QueryValueEx(app_key, "LastUsedTimeStart")
                                stop, _ = winreg.QueryValueEx(app_key, "LastUsedTimeStop")
                                
                                # --- ĐÃ SỬA LỖI LOGIC TẠI ĐÂY ---
                                # Nếu thời điểm bật LỚN HƠN thời điểm tắt -> Đang dùng!
                                if start > stop:
                                    # Cắt bớt đường dẫn loằng ngoằng, chỉ lấy tên tệp .exe cho đẹp
                                    app_name = app.split('#')[-1] if '#' in app else app
                                    active_apps.append(app_name)
                        except Exception:
                            pass
            except Exception:
                pass

        # Quét nhánh 1
        check_registry_key(reg_path_non_packaged)
        
        # 2. Nhánh Packaged (Các ứng dụng cài từ Microsoft Store như Camera mặc định)
        reg_path_packaged = rf"SOFTWARE\Microsoft\Windows\CurrentVersion\CapabilityAccessManager\ConsentStore\{device_type}"
        check_registry_key(reg_path_packaged)
        
    except Exception:
        pass
        
    # Loại bỏ tên trùng lặp (nếu có) và trả về kết quả
    return list(set(active_apps))

def check_hardware_and_system():
    write_log("\n[+] Tình trạng Hệ thống & Phần cứng")
    
    # 1. CPU & RAM (Tích hợp phát hiện Đào tiền ảo)
    cpu_usage = psutil.cpu_percent(interval=1) # Đợi 1 giây để lấy chỉ số CPU chính xác
    ram_usage = psutil.virtual_memory().percent
    write_log(f"    -> CPU: {cpu_usage}% | RAM: {ram_usage}%")
    
    if cpu_usage > 85:
        write_log("    [!!!] CẢNH BÁO BẤT THƯỜNG: CPU đang hoạt động quá tải!")
        write_log("          Có khả năng máy bạn đang bị nhiễm mã độc Đào Tiền Ảo (Miner).")
    
    # 2. Network Active
    try:
        conns = [c for c in psutil.net_connections() if c.status == 'ESTABLISHED']
        write_log(f"    -> Kết nối mạng đang mở: {len(conns)}")
    except Exception:
        write_log("    [!] Không có quyền xem IP kết nối mạng.")

    # 3. Camera / Mic
    if OS_SYSTEM == "Windows":
        active_cams = check_win_device_active("webcam")
        if active_cams:
            write_log(f"    [!!!] CẢNH BÁO: Ứng dụng đang quay lén Camera: {active_cams}")
        else:
            write_log("    -> Camera: An toàn.")
            
        active_mics = check_win_device_active("microphone")
        if active_mics:
            write_log(f"    [!!!] CẢNH BÁO: Ứng dụng đang nghe lén Micro: {active_mics}")
        else:
            write_log("    -> Microphone: An toàn.")
            
    elif OS_SYSTEM == "Linux":
        try:
            cam_out = subprocess.check_output(['dumpsys', 'media.camera']).decode('utf-8')
            if "active: false" not in cam_out.lower() and "No cameras in use" not in cam_out:
                write_log("    [!!!] CẢNH BÁO: Camera Android có thể đang bật!")
            mic_out = subprocess.check_output(['dumpsys', 'audio']).decode('utf-8')
            if "active_record" in mic_out.lower():
                write_log("    [!!!] CẢNH BÁO: Microphone Android đang ghi âm!")
        except Exception:
            write_log("    [!] Thiếu quyền Root để dùng dumpsys.")

# ==========================================
# MODULE 4: GIÁM SÁT MẠNG (PACKET SNIFFING)
# ==========================================
def process_packet(packet):
    if packet.haslayer(IP) and packet.haslayer(TCP) and packet.haslayer(Raw):
        ip_dst = packet[IP].dst
        port_dst = packet[TCP].dport
        try:
            payload = packet[Raw].load.decode('utf-8', errors='ignore')
            if port_dst == 80:
                write_log(f"    [HTTP] Dữ liệu không mã hóa tới {ip_dst}")
            for word in ["password", "login", "admin", "token", "location"]:
                if word in payload.lower():
                    write_log(f"    [RÒ RỈ] Phát hiện từ khóa '{word}' gửi tới {ip_dst}")
        except Exception:
            pass

# ==========================================
# MODULE 4: GIÁM SÁT MẠNG VÀ BẮT GÓI TIN TỰ ĐỘNG
# ==========================================

def process_packet(packet):
    """Hàm phân tích gói tin của Scapy."""
    if packet.haslayer(IP) and packet.haslayer(TCP) and packet.haslayer(Raw):
        ip_dst = packet[IP].dst
        port_dst = packet[TCP].dport
        try:
            payload = packet[Raw].load.decode('utf-8', errors='ignore')
            if port_dst == 80:
                write_log(f"    [SCAPY] CẢNH BÁO: Dữ liệu không mã hóa đang gửi tới {ip_dst}")
            for word in ["password", "login", "admin", "token", "location"]:
                if word in payload.lower():
                    write_log(f"    [SCAPY] RÒ RỈ DỮ LIỆU: Bắt được từ '{word}' gửi tới {ip_dst}")
        except Exception:
            # Nếu payload bị nén (như .pyz) hoặc mã hóa, nó sẽ nhảy vào đây
            # Scapy không giải mã được nội dung, nhưng luồng kết nối bên dưới sẽ "chỉ điểm" kẻ gửi!
            pass

def scapy_worker(interface):
    """Luồng 1: Chạy Scapy nghe lén thụ động."""
    write_log(f"\n[+] [SCAPY] Đang bắt các luồng dữ liệu mạng trên: {interface if interface else 'Tự động'}")
    try:
        sniff(iface=interface, prn=process_packet, store=False)
    except Exception as e:
        write_log(f"\n[-] [SCAPY] Lỗi: {e}")

def auto_connection_monitor():
    """Luồng 2 (MỚI): Tự động phát hiện mọi kết nối mạng từ máy bạn đi ra ngoài."""
    write_log("\n[+] [AUTO-MONITOR] Đang tự động theo dõi các ứng dụng gửi dữ liệu ra ngoài...")
    seen_connections = set()
    
    while True:
        try:
            # Lấy toàn bộ kết nối mạng hiện tại của máy
            for conn in psutil.net_connections(kind='inet'):
                # Chỉ bắt các kết nối đã thiết lập (ESTABLISHED) và có IP đích (raddr)
                if conn.status == 'ESTABLISHED' and conn.raddr:
                    ip_dich = conn.raddr.ip
                    port_dich = conn.raddr.port
                    
                    # Bỏ qua địa chỉ cục bộ (Localhost) để đỡ rác màn hình
                    if ip_dich == '127.0.0.1':
                        continue
                        
                    # Tạo ID nhận diện kết nối
                    conn_sig = f"{conn.laddr.port}-{ip_dich}-{port_dich}"
                    
                    # Nếu là kết nối mới tinh vừa bật lên
                    if conn_sig not in seen_connections:
                        seen_connections.add(conn_sig)
                        
                        # Truy tìm tên ứng dụng đang lén gửi dữ liệu
                        ten_app = "Không xác định (Cần chạy Tool bằng quyền Admin)"
                        try:
                            if conn.pid:
                                proc = psutil.Process(conn.pid)
                                ten_app = proc.name()
                        except (psutil.NoSuchProcess, psutil.AccessDenied):
                            pass
                            
                        write_log(f" [CẢNH BÁO KẾT NỐI] App: '{ten_app}' (PID:{conn.pid}) -> Đang gửi dữ liệu tới IP: {ip_dich}:{port_dich}")
                        
            time.sleep(2) # Cứ 2 giây quét lại một lần
        except Exception:
            pass

def nmap_worker(nmap_command):
    """Luồng 3: Chạy Nmap với các tham số do người dùng chọn."""
    lenh_chuoi = " ".join(nmap_command)
    write_log(f"\n[+] [NMAP] Đang thực thi lệnh: {lenh_chuoi}")
    try:
        process = subprocess.Popen(nmap_command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, encoding='utf-8', errors='ignore')
        for line in iter(process.stdout.readline, ''):
            clean_line = line.strip()
            if clean_line:
                write_log(f" [NMAP] {clean_line}")
        process.wait()
        write_log("\n[+] [NMAP] Đã hoàn tất quá trình quét!")
    except Exception as e:
        write_log(f"\n[-] [NMAP] Lỗi: {e}")

def start_network_monitor():
    """Menu chính của Giám sát mạng"""
    print("\n" + "="*60)
    print(" 🌐 CHẾ ĐỘ GIÁM SÁT VÀ TẤN CÔNG MẠNG (MULTITHREADING) 🌐")
    print("="*60)
    print(" [Tự động - Không cần nhập IP]")
    print("  5. GIÁM SÁT TỰ ĐỘNG - Theo dõi trực tiếp app nào đang gửi dữ liệu ra ngoài.")
    print("\n [Chủ động - Vũ khí Nmap]")
    print("  1. Quét Nhanh (Fast Scan).")
    print("  2. Quét Dịch Vụ & HĐH.")
    print("  3. Quét Tìm Thiết Bị (Ping Sweep) trong mạng Wi-Fi.")
    print("  4. Quét FULL Tính Năng Nmap.")
    print("  0. Thoát chế độ giám sát mạng.")
    print("-" * 60)
    
    choice = input("[?] Chọn chế độ (0-5): ").strip()
    if choice == '0': return
    
    nmap_command = []
    use_auto_monitor = False

    if choice == '5':
        use_auto_monitor = True
    elif choice in ['1', '2', '3', '4']:
        if choice == '3':
            target_ip = input("[?] Nhập Dải IP mạng WiFi của bạn (VD: 192.168.1.0/24): ").strip()
            nmap_command = ["nmap", "-sn", target_ip]
        else:
            target_ip = input("[?] Nhập IP/Domain mục tiêu (VD: 192.168.1.5 hoặc google.com): ").strip()
            if choice == '1': nmap_command = ["nmap", "-F", "-T4", target_ip]
            elif choice == '2': nmap_command = ["nmap", "-sV", "-O", "-T4", target_ip]
            elif choice == '4': nmap_command = ["nmap", "-p-", "-A", "-T4", "-vv", "--packet-trace", target_ip]

    interface = None if platform.system() == "Windows" else "wlan0"
    write_log("\n[!] HỆ THỐNG BẮT ĐẦU CHẠY... (Nhấn Ctrl + C để DỪNG TOÀN BỘ)")

    threads = []

    # 1. Luôn chạy Scapy để bắt dữ liệu
    t_scapy = threading.Thread(target=scapy_worker, args=(interface,))
    t_scapy.daemon = True
    threads.append(t_scapy)

    # 2. Nếu chọn số 5, chạy luồng Auto Monitor
    if use_auto_monitor:
        t_auto = threading.Thread(target=auto_connection_monitor)
        t_auto.daemon = True
        threads.append(t_auto)

    # 3. Nếu chọn 1,2,3,4, chạy Nmap
    if nmap_command:
        t_nmap = threading.Thread(target=nmap_worker, args=(nmap_command,))
        t_nmap.daemon = True
        threads.append(t_nmap)

    for t in threads: t.start()

    try:
        while True: time.sleep(1)
    except KeyboardInterrupt:
        write_log("\n[STOP] Đang đóng các tiến trình mạng và quay về Menu chính...")
        time.sleep(1)

def print_banner():
    # Mã màu đỏ rực rỡ cho Madara
    red = "\033[1;31m"
    reset = "\033[0m"
    
    banner = rf"""
    {red}
    ███╗   ███╗ █████╗ ██████╗  █████╗ ██████╗  █████╗ 
    ████╗ ████║██╔══██╗██╔══██╗██╔══██╗██╔══██╗██╔══██╗
    ██╔████╔██║███████║██║  ██║███████║██████╔╝███████║
    ██║╚██╔╝██║██╔══██║██║  ██║██╔══██║██╔══██╗██╔══██║
    ██║ ╚═╝ ██║██║  ██║██████╔╝██║  ██║██║  ██║██║  ██║
    ╚═╝     ╚═╝╚═╝  ╚═╝╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝
    >>  Uchiha Madara - The Ultimate Security Toolkit  <<
    {reset}
    """
    # Lệnh xóa màn hình để Banner luôn nằm trên cùng
    os.system('cls' if os.name == 'nt' else 'clear')
    print(banner)

# ==========================================
# CHƯƠNG TRÌNH CHÍNH & MENU
# ==========================================
def main_menu():
    while True:
        print_banner()
        print("\n" + "="*50)
        print(f" 🛡️ ULTIMATE SECURITY TOOLKIT (Đang chạy trên {OS_SYSTEM}) 🛡️")
        print("="*50)
        print(" 1. Quét Hệ thống (File ẩn, Tiến trình, Cam/Mic trên TẤT CẢ ổ đĩa)")
        print(" 2. Giám sát Mạng trực tiếp (Wireshark Mode)")
        print(" 3. Chạy Tất cả & Ghi Log")
        print(" 4. Thoát")
        print("="*50)
        
        choice = input("Chọn chức năng (1-4): ").strip()
        
        if choice == '1':
            check_hardware_and_system()
            scan_apps_and_processes()
            # Tự động lấy danh sách ổ đĩa và quét toàn bộ
            drives = get_all_drives()
            for drive in drives:
                scan_hidden_and_suspicious(drive)
                
        elif choice == '2':
            start_network_monitor()
            
        elif choice == '3':
            check_hardware_and_system()
            scan_apps_and_processes()
            # Tự động lấy danh sách ổ đĩa và quét toàn bộ
            drives = get_all_drives()
            for drive in drives:
                scan_hidden_and_suspicious(drive)
            start_network_monitor() # Sẽ chạy vô hạn cho đến khi bấm Ctrl+C
            
        elif choice == '4':
            print("Đã thoát chương trình. Tạm biệt!")
            break
        else:
            print("Lựa chọn không hợp lệ!")

if __name__ == "__main__":
    main_menu()