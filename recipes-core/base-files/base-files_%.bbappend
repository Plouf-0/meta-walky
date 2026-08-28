FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://profile-walky-kiosk"

do_install:append() {
    install -d ${D}${sysconfdir}/skel
    install -m 0644 ${WORKDIR}/profile-walky-kiosk ${D}${sysconfdir}/skel/.profile
}

FILES:${PN} += "${sysconfdir}/skel/.profile"
