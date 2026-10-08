SUMMARY = "Walky Home - menu principal et settings QML"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "git://github.com/Plouf-0/WalkyHome.git;protocol=https;branch=main"

SRCREV = "AUTOINC"

S = "${WORKDIR}/git"

DEPENDS = "qtbase qtdeclarative qtdeclarative-native"

inherit cmake qt6-cmake

EXTRA_OECMAKE = "-DQT_HOST_PATH_CMAKE_DIR=${STAGING_DIR_NATIVE}${prefix_native}/lib/cmake"

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${B}/appWalkyHome ${D}${bindir}/walky-home
}

FILES:${PN} += "${bindir}/walky-home"
