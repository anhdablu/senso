#include "crow.h"
#include <vector>
#include <mutex>
#include <algorithm>
#include <iostream>
#include <thread>
#include <string>

// Linux Hardware-Header für I2C
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

std::vector<crow::websocket::connection*> clients;
std::mutex clients_mutex;

// I2C-Konfiguration passend zu Ihrem i2c-stub und Python-Skript
const char* I2C_DEVICE = "/dev/i2c-1";
const int SLAVE_ADDRESS = 0x20;

void i2cStubReader() {
    int file_i2c;
    
    std::cout << "[C++ I2C] Thread wurde gestartet..." << std::endl << std::flush;
    
    // 1. I2C Bus öffnen
    if ((file_i2c = open(I2C_DEVICE, O_RDWR)) < 0) {
        // Sichern durch std::flush, damit es SOFORT gedruckt wird
        std::cerr << "[C++ I2C] SCHWERWIEGENDER FEHLER: Kann " << I2C_DEVICE << " nicht öffnen!" << std::endl << std::flush;
        return;
    }
    
    // 2. Verbindung zur virtuellen Adresse herstellen
    if (ioctl(file_i2c, I2C_SLAVE, SLAVE_ADDRESS) < 0) {
        std::cerr << "[C++ I2C] SCHWERWIEGENDER FEHLER: ioctl Adresse 0x" << std::hex << SLAVE_ADDRESS << " verweigert!" << std::endl << std::flush;
        close(file_i2c);
        return;
    }

    // Dieser Log MUSS erscheinen, sobald das Backend startet!
    std::cout << "[C++ I2C] ERFOLG: Überwache " << I2C_DEVICE << " auf Adresse 0x" << std::hex << SLAVE_ADDRESS << "..." << std::endl << std::flush;

    unsigned char last_value = 0x00;

    // Strukturen für den nativen SMBus-Lesezugriff vorbereiten
    union i2c_smbus_data smbus_data;
    struct i2c_smbus_ioctl_data ioctl_data;
    
    ioctl_data.read_write = 1; // 1 steht für READ
    ioctl_data.command = 0x00; // Wir lesen aus Register 0x00
    ioctl_data.size = 2;       // 2 steht für I2C_SMBUS_BYTE_DATA
    ioctl_data.data = &smbus_data;

    while (true) {
        // Wir nutzen ioctl mit I2C_SMBUS, um exakt das Register 0x00 auszulesen
        if (ioctl(file_i2c, I2C_SMBUS, &ioctl_data) >= 0) {
            unsigned char current_value = smbus_data.byte;

            // Loggt JEDE Änderung, auch wenn es 0x00 ist (erleichtert das Debuggen!)
            if (current_value != last_value) {
                std::string command = "";

                if (current_value == 0x01) command = "start";
                else if (current_value == 0x02) command = "blue";
                else if (current_value == 0x03) command = "green";
                else if (current_value == 0x04) command = "red";
                else if (current_value == 0x05) command = "yellow";

                if (!command.empty()) {
                    std::cout << "[C++ I2C] SIGNAL ERKANNT: 0x" << std::hex << (int)current_value 
                              << " -> Sende an Web: " << command << " (Clients: " << clients.size() << ")" << std::endl << std::flush;
                    
                    std::lock_guard<std::mutex> lock(clients_mutex);
                    for (auto* client : clients) {
                        client->send_text(command);
                    }
                } else if (current_value == 0x00) {
                    std::cout << "[C++ I2C] Button losgelassen (0x00)" << std::endl << std::flush;
                }
                
                last_value = current_value;
            }
        } else {
            // Falls der ioctl-Aufruf fehlschlägt, loggen wir das sofort
            std::cerr << "[C++ I2C] Fehler beim SMBus-Read-ioctl!" << std::endl << std::flush;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}


int main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/gamepad")
        .websocket(&app)
        .onopen([&](crow::websocket::connection& conn) {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.push_back(&conn);
            CROW_LOG_INFO << "Frontend per WebSocket verbunden.";
        })
        .onclose([&](crow::websocket::connection& conn, const std::string& /*reason*/, uint16_t /*status*/) {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.erase(std::remove(clients.begin(), clients.end(), &conn), clients.end());
            CROW_LOG_INFO << "Frontend getrennt.";
        });

    // STARTET DEN HARDWARE-LESE-THREAD IM HINTERGRUND
    std::thread hwThread(i2cStubReader);
    hwThread.detach(); 

    // Startet den Crow-Server
    app.port(8080).multithreaded().run();
}
