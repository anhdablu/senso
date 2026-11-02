# sudo modprobe i2c-dev
# sudo modprobe i2c-stub chip_addr=32
# sudo rmmod i2c-stub
# sudo -E python3 write_stub_interactive.py
import os
import fcntl
import time
import ctypes
import sys

# Konfiguration (identisch mit Ihrem C++ Backend)
I2C_BUS_DEV = "/dev/i2c-1"
SLAVE_ADDRESS = 0x20

# Linux Kernel I2C-Definitionen für IOCTL
I2C_SLAVE = 0x0703
I2C_SMBUS = 0x0720
I2C_SMBUS_WRITE = 0
I2C_SMBUS_BYTE_DATA = 2

class union_i2c_smbus_data(ctypes.Union):
    _fields_ = [("byte", ctypes.c_uint8),
                ("word", ctypes.c_uint16),
                ("block", ctypes.c_uint8 * 34)]

class struct_i2c_smbus_ioctl_data(ctypes.Structure):
    _fields_ = [("read_write", ctypes.c_uint8),
                ("command", ctypes.c_uint8),
                ("size", ctypes.c_uint32),
                ("data", ctypes.POINTER(union_i2c_smbus_data))]

def write_to_stub_register(register, value):
    try:
        fd = os.open(I2C_BUS_DEV, os.O_RDWR)
        fcntl.ioctl(fd, I2C_SLAVE, SLAVE_ADDRESS)
        
        smbus_data = union_i2c_smbus_data()
        smbus_data.byte = value
        
        ioctl_data = struct_i2c_smbus_ioctl_data()
        ioctl_data.read_write = I2C_SMBUS_WRITE
        ioctl_data.command = register
        ioctl_data.size = I2C_SMBUS_BYTE_DATA
        ioctl_data.data = ctypes.pointer(smbus_data)
        
        fcntl.ioctl(fd, I2C_SMBUS, ioctl_data)
        os.close(fd)
    except Exception as e:
        print(f"\n[FEHLER] Konnte nicht in i2c-stub schreiben: {e}")

def print_menu():
    print("\n" + "="*50)
    print(" INTERAKTIVER I2C-GAMEPAD-SIMULATOR")
    print(" Tippen Sie eine Taste + Enter zum Senden:")
    print("="*50)
    print("  s -> START / AGAIN  (0x01)")
    print("  b -> BLUE           (0x02)")
    print("  g -> GREEN          (0x03)")
    print("  r -> RED            (0x04)")
    print("  y -> YELLOW         (0x05)")
    print("  q -> Beenden")
    print("="*50)

def main():
    if not os.path.exists(I2C_BUS_DEV):
        print(f"[FEHLER] {I2C_BUS_DEV} existiert nicht! Ist i2c-stub geladen?")
        sys.exit(1)

    print_menu()

    while True:
        try:
            user_input = input("Eingabe > ").strip().lower()
            
            if user_input == 'q':
                print("Simulator beendet.")
                break
                
            # Zuweisung von Taste zu I2C-Byte-Wert
            mapping = {
                's': (0x01, "START"),
                'b': (0x02, "BLUE"),
                'g': (0x03, "GREEN"),
                'r': (0x04, "RED"),
                'y': (0x05, "YELLOW")
            }
            
            if user_input in mapping:
                byte_value, name = mapping[user_input]
                print(f"-> Sende {name} (0x{byte_value:02X})...")
                
                # 1. Taste drücken (Wert in Register 0x00 schreiben)
                write_to_stub_register(0x00, byte_value)
                
                # 2. Kurze Verzögerung und Taste automatisch wieder loslassen (0x00)
                # Das verhindert, dass der Button im C++ Backend "dauerhaft gedrückt" bleibt.
                time.sleep(0.15)
                write_to_stub_register(0x00, 0x00)
            else:
                print("Ungültige Taste! Bitte nutzen Sie s, b, g, r, y oder q.")
                
        except KeyboardInterrupt:
            print("\nSimulator abgebrochen.")
            break

if __name__ == "__main__":
    main()
