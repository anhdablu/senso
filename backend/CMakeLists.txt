# g++ -O2 -Wall -std=c++17 gamepad_backend.cpp -o gamepad_backend

cmake_minimum_required(VERSION 3.10)
project(seesaw-gamepad VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(seesaw_backend gamepad_backend.cpp)

# Systemd Integration (optional, falls Autostart gewünscht)
install(TARGETS seesaw_backend DESTINATION ${CMAKE_INSTALL_BINDIR})
