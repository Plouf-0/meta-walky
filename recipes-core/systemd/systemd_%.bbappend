FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://autologin.conf"

do_install:append() {
    install -d ${D}${systemd_system_unitdir}/getty@tty1.service.d
    install -m 0644 ${WORKDIR}/autologin.conf ${D}${systemd_system_unitdir}/getty@tty1.service.d/autologin.conf
}

FILES:${PN} += "${systemd_system_unitdir}/getty@tty1.service.d/autologin.conf"
