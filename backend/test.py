import os
import fcntl
import time
import ctypes

# Konfiguration basierend auf Ihren verifizierten Daten
I2C_BUS_DEV = "/dev/i2c-1"  # Die von i2c-stub erzeugte Datei
SLAVE_ADDRESS = 0x20        # Die chip_addr aus Ihrem modprobe

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

def write_to_stub_register(register, value, name):
    print(f"[STUB-WRITER] Simuliere: {name} -> Schreibe Wert 0x{value:02X} in Register 0x{register:02X}")
    
    try:
        # 1. Datei öffnen
        fd = os.open(I2C_BUS_DEV, os.O_RDWR)
        
        # 2. Slave Adresse setzen
        fcntl.ioctl(fd, I2C_SLAVE, SLAVE_ADDRESS)
        
        # 3. SMBus-Strukturen für den Kernel vorbereiten
        smbus_data = union_i2c_smbus_data()
        smbus_data.byte = value
        
        ioctl_data = struct_i2c_smbus_ioctl_data()
        ioctl_data.read_write = I2C_SMBUS_WRITE
        ioctl_data.command = register  # Das Ziel-Register im Speicher
        ioctl_data.size = I2C_SMBUS_BYTE_DATA
        ioctl_data.data = ctypes.pointer(smbus_data)
        
        # 4. Den offiziellen SMBus-Schreibbefehl an den Kernel senden
        fcntl.ioctl(fd, I2C_SMBUS, ioctl_data)
        
        os.close(fd)
    except Exception as e:
        print(f"Fehler beim Schreiben in den i2c-stub: {e}")

def run_simulation():
    print("Starte I2C-Stub Simulation auf /dev/i2c-1...")
    
    # 1. Start-Signal (z.B. Register 0x00, Wert 0x01)
    write_to_stub_register(0x00, 0x01, "START-BUTTON")
    time.sleep(3.0) # Warten auf React-Startsequenz
    
    # 2. Die 4 Farb-Buttons nacheinander simulieren
    buttons = [
        (0x02, "BLUE"),
        (0x03, "GREEN"),
        (0x04, "RED"),
        (0x05, "YELLOW")
    ]
    
    for val, name in buttons:
        write_to_stub_register(0x00, val, name)
        time.sleep(1.0)
        
        # Button wieder loslassen (Wert auf 0x00 setzen)
        write_to_stub_register(0x00, 0x00, "RELEASE")
        time.sleep(0.3)

if __name__ == "__main__":
    run_simulation()
