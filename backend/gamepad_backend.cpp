#include <functional>
// Eine WebSocket-Bibliothek einbinden (z.B. Crow)
#include "crow_all.h" 


#include <iostream>
// #include <fcntl.h>
// #include <unistd.h>
// #include <sys/ioctl.h>
// #include <linux/i2c-dev.h>

// Hardware-Schnittstellen nach Betriebssystem trennen
#ifdef _WIN32
#  include <windows.h>
   // Hier eventuell Windows-Dummy-Typen oder Simulationen definieren
#else
#  include <sys/ioctl.h>
#  include <linux/i2c-dev.h>
#  include <fcntl.h>
#  include <unistd.h>
#endif

#include <cstdint>
#include <thread>
#include <chrono>

// Register-Konstanten für das Adafruit seesaw Framework
#define SEESAW_STATUS_BASE          0x00
#define SEESAW_STATUS_VERSION       0x02
#define SEESAW_GPIO_BASE            0x01
#define SEESAW_GPIO_DIRSET_BULK     0x02
#define SEESAW_GPIO_BULK            0x04
#define SEESAW_GPIO_PULLENSET       0x0B
#define SEESAW_ADC_BASE             0x09
#define SEESAW_ADC_CHANNEL_OFFSET   0x07

// Gamepad-Spezifische Pin-Masken (Produkt ID 5743)
#define BUTTON_X        6
#define BUTTON_Y        2
#define BUTTON_A        5
#define BUTTON_B        1
#define BUTTON_SELECT   0
#define BUTTON_START    16

const uint32_t BUTTON_MASK = (1UL << BUTTON_X) | (1UL << BUTTON_Y) | 
                             (1UL << BUTTON_A) | (1UL << BUTTON_B) | 
                             (1UL << BUTTON_SELECT) | (1UL << BUTTON_START);

class SeesawGamepad {
private:
    int i2c_fd = -1;
    uint8_t address;

    bool writeReg(uint8_t module, uint8_t reg, uint8_t *data, size_t len) {
        uint8_t buf[len + 2];
        buf[0] = module;
        buf[1] = reg;
        for (size_t i = 0; i < len; ++i) buf[i + 2] = data[i];
        return write(i2c_fd, buf, len + 2) == static_cast<ssize_t>(len + 2);
    }

    bool readReg(uint8_t module, uint8_t reg, uint8_t *data, size_t len, int delay_ms = 5) {
        uint8_t buf[2] = {module, reg};
        if (write(i2c_fd, buf, 2) != 2) return false;
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms)); // Nötig für seesaw MCUs
        return read(i2c_fd, data, len) == static_cast<ssize_t>(len);
    }

public:
    SeesawGamepad(uint8_t addr = 0x50) : address(addr) {} // Standardadresse ist 0x50

    ~SeesawGamepad() { if (i2c_fd >= 0) close(i2c_fd); }

    bool init(const char* i2c_device) {
        i2c_fd = open(i2c_device, O_RDWR);
        if (i2c_fd < 0) return false;
        if (ioctl(i2c_fd, I2C_SLAVE, address) < 0) return false;

        // 1. Firmware-Version verifizieren
        uint32_t version = 0;
        if (!readReg(SEESAW_STATUS_BASE, SEESAW_STATUS_VERSION, reinterpret_cast<uint8_t*>(&version), 4)) return false;
        
        // 2. Buttons als Input konfigurieren
        uint8_t mask_buf[4];
        uint32_t rev_mask = __builtin_bswap32(BUTTON_MASK); // Big-Endian Konvertierung für I2C
        std::memcpy(mask_buf, &rev_mask, 4);
        writeReg(SEESAW_GPIO_BASE, SEESAW_GPIO_DIRSET_BULK, mask_buf, 4);
        
        // 3. Interne Pullup-Widerstände für Buttons aktivieren
        writeReg(SEESAW_GPIO_BASE, SEESAW_GPIO_PULLENSET, mask_buf, 4);
        writeReg(SEESAW_GPIO_BASE, SEESAW_GPIO_BULK, mask_buf, 4); // High setzen

        return true;
    }

    void readInputs(uint32_t &buttons, int &x, int &y) {
        // Buttons lesen (Aktive-Low, 0 bedeutet gedrückt)
        uint32_t raw_gpio = 0;
        if (readReg(SEESAW_GPIO_BASE, SEESAW_GPIO_BULK, reinterpret_cast<uint8_t*>(&raw_gpio), 4)) {
            raw_gpio = __builtin_bswap32(raw_gpio);
            buttons = ~raw_gpio & BUTTON_MASK; // Invertieren, sodass 1 = Gedrückt
        }

        // Joystick-Achsen (ADC Kanäle 14 und 15 beim seesaw Gamepad)
        uint16_t adc_x = 0, adc_y = 0;
        if (readReg(SEESAW_ADC_BASE, SEESAW_ADC_CHANNEL_OFFSET + 14, reinterpret_cast<uint8_t*>(&adc_x), 2, 1)) {
            x = __builtin_bswap16(adc_x);
        }
        if (readReg(SEESAW_ADC_BASE, SEESAW_ADC_CHANNEL_OFFSET + 15, reinterpret_cast<uint8_t*>(&adc_y), 2, 1)) {
            y = __builtin_bswap16(adc_y);
        }
    }
};

class GamepadWorker {
private:
    std::thread worker_thread;
    bool running = false;
    std::function<void(const std::string&)> event_callback;

    // Speichert den letzten Zustand, um Button-Flanken (Pressed-Events) zu erkennen
    uint32_t last_buttons = 0;
    bool last_left = false, last_right = false, last_up = false, last_down = false;

public:
    void start(const std::string& device, std::function<void(const std::string&)> callback) {
        event_callback = callback;
        running = true;
        worker_thread = std::thread([this, device]() {
            SeesawGamepad gamepad(0x50);
            if (!gamepad.init(device.c_str())) return;

            while (running) {
                uint32_t buttons = 0;
                int x = 0, y = 0;
                gamepad.readInputs(buttons, x, y);

                // --- 1. Physische Buttons auswerten (A, B, X, Y, START) ---
                if ((buttons & (1UL << BUTTON_A)) && !(last_buttons & (1UL << BUTTON_A))) event_callback("yellow");
                if ((buttons & (1UL << BUTTON_B)) && !(last_buttons & (1UL << BUTTON_B))) event_callback("green");
                if ((buttons & (1UL << BUTTON_X)) && !(last_buttons & (1UL << BUTTON_X))) event_callback("blue");
                if ((buttons & (1UL << BUTTON_Y)) && !(last_buttons & (1UL << BUTTON_Y))) event_callback("red");
                if ((buttons & (1UL << BUTTON_START)) && !(last_buttons & (1UL << BUTTON_START))) event_callback("start");
                last_buttons = buttons;

                // --- 2. Analog-Stick als Steuerkreuz-Ersatz auswerten (Schwellenwert ca. 300 von 1023) ---
                bool current_left  = (x < 200);
                bool current_right = (x > 800);
                bool current_up    = (y < 200);
                bool current_down  = (y > 800);

                if (current_left  && !last_left)  event_callback("blue");   // Oben-Links
                if (current_right && !last_right) event_callback("green");  // Oben-Rechts
                if (current_down  && !last_down)  event_callback("yellow"); // Unten-Rechts
                if (current_up    && !last_up)    event_callback("red");    // Unten-Links

                last_left = current_left; last_right = current_right;
                last_up = current_up; last_down = current_down;

                std::this_thread::sleep_for(std::chrono::milliseconds(20)); // I2C Polling Rate
            }
        });
    }

    void stop() {
        running = false;
        if (worker_thread.joinable()) worker_thread.join();
    }
};

int main() {
    crow::SimpleApp app;
    GamepadWorker gamepad;
    
    // Globale Liste aller verbundenen WebSockets (Frontends)
    std::vector<crow::websocket::connection*> clients;

    // Starte den isolierten I2C-Thread. Er broadcastet Events an alle Frontends
    gamepad.start("/dev/i2c-1", [&](const std::string& event) {
        for (auto client : clients) {
            client->send_text(event);
        }
    });

    // WebSocket-Endpunkt für das React Frontend
    CROW_ROUTE(app, "/gamepad")
        .websocket()
        .onopen([&](crow::websocket::connection& conn) { clients.push_back(&conn); })
        .onclose([&](crow::websocket::connection& conn, const std::string&) {
            clients.erase(std::remove(clients.begin(), clients.end(), &conn), clients.end());
        });

    // --- HIER KÖNNEN IHRE ANDEREN BACKEND TASKS KOEXISTIEREN ---
    CROW_ROUTE(app, "/api/other-task")([](){ return "Backend macht noch andere Dinge!"; });

    app.port(8080).multithreaded().run(); // Startet den asynchronen Netzwerk-Server
    gamepad.stop();
    return 0;
}
