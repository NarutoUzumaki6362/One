import random
import os
import time

def banner():
    os.system("clear" if os.name == "posix" else "cls")
    print("\033[1;35m" + r"""
╔════════════════════════════════════════════╗
║   🔥 HEX PAYLOAD GENERATOR — MB PERFECT  ║
║     Tu Jitna Maangega, Utna Hi Milega     ║
╚════════════════════════════════════════════╝
""" + "\033[0m")

def generate_payload_by_text_size(size_in_mb, output_file='spidy.txt'):
    target_size = size_in_mb * 1024 * 1024  # Final text file size in bytes
    written_size = 0

    print(f"\n\033[1;34m🚀 Target Payload Text Size: {size_in_mb}MB → {target_size} bytes\033[0m")

    with open(output_file, 'w') as f:
        while written_size < target_size:
            payload_bytes = [random.randint(0, 255) for _ in range(32)]
            payload = ''.join(f"\\x{b:02X}" for b in payload_bytes)
            line = f'"{payload}"\n'
            f.write(line)
            written_size += len(line)

            percent = int((written_size / target_size) * 100)
            bar = '█' * (percent // 5) + '-' * ((100 - percent) // 5)
            print(f"\033[1;33m⚙️  Progress: [{bar}] {percent}%\033[0m", end='\r')

    print(f"\n\033[1;32m✅ DONE! Final payload text size = {written_size} bytes ≈ {size_in_mb}MB\033[0m")
    print("\033[1;36m💥 File: spidy.txt is ready to inject pain!\033[0m\n")

if __name__ == "__main__":
    banner()
    try:
        mb = int(input("\033[1;36m🤖 Kitne MB ke HEX payloads chahiye? (text file size): \033[0m"))
        generate_payload_by_text_size(mb)
    except:
        print("\033[1;31m[❌] Bhai number daal... string nahi chalega.\033[0m")