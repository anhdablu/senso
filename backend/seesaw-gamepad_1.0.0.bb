SUMMARY = "Backend driver for Adafruit Seesaw I2C Gamepad MiniPad"
DESCRIPTION = "Reads I2C inputs from Adafruit Seesaw and provides backend data."
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

# Falls der Code lokal im selben Verzeichnis liegt (oder nutzen Sie git://...)
SRC_URI = "file://gamepad_backend.cpp \
           file://CMakeLists.txt"

S = "${WORKDIR}"

inherit cmake systemd

# Falls Sie das Backend als Systemd-Service beim Booten starten wollen:
SYSTEMD_SERVICE:${PN} = "seesaw-gamepad.service"
SYSTEMD_AUTO_ENABLE:${PN} = "enable"

# Stellen Sie sicher, dass Laufzeitabhängigkeiten und I2C-Tools vorhanden sind
RDEPENDS:${PN} += "i2c-tools"

do_install:append() {
    # Wenn Sie einen systemd Service nutzen, installieren Sie ihn hier:
    # install -d ${D}${systemd_system_unitdir}
    # install -m 0644 ${WORKDIR}/seesaw-gamepad.service ${D}${systemd_system_unitdir}
}
