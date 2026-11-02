# # sudo apt install libcrow-dev
# sudo apt update
# sudo apt install libasio-dev
# sudo apt install libcrow-dev
# g++ -O2 -Wall -std=c++17 gamepad_backend_2.cpp -o gamepad_backend
# sudo gamepad_backend
# git clone https://github.com/CrowCpp/Crow.git && sudo cp -r Crow/include/* /usr/local/include/

cmake_minimum_required(VERSION 3.10)
project(seesaw-gamepad VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(seesaw_backend gamepad_backend.cpp)

# Systemd Integration (optional, falls Autostart gewünscht)
install(TARGETS seesaw_backend DESTINATION ${CMAKE_INSTALL_BINDIR})
