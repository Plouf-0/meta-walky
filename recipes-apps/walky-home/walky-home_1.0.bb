SUMMARY = "Walky Home - menu principal et settings QML"
LICENSE = "MIT"

SRC_URI = "git://github.com/Plouf-0/WalkyHome.git;protocol=ssh;branch=main"

SRCREV = "AUTOINC"

S = "${WORKDIR}/git"

DEPENDS = "qtbase qtdeclarative"

inherit cmake qt6-cmake

EXTRA_OECMAKE = ""

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${B}/walky-home ${D}${bindir}/walky-home
}

FILES:${PN} += "${bindir}/walky-home"