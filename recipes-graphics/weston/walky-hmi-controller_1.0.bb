SUMMARY = "Walky custom ivi-shell HMI controller"
DESCRIPTION = "Minimal 2-layer overlay controller for Walky (app layer + \
overlay layer for keyboard/banner/status indicators), replacing the \
reference hmi-controller.so"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

DEPENDS = "weston wayland-protocols"
RDEPENDS:${PN} += "weston"

SRC_URI = "file://walky-hmi-controller.c \
           file://meson.build \
"

S = "${WORKDIR}"

inherit meson pkgconfig

FILES:${PN} += "${libdir}/weston/walky-hmi-controller.so"