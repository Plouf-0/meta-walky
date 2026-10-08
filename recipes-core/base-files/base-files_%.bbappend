FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://profile-walky-boot"

do_install:append() {
    install -d ${D}${sysconfdir}/skel
    install -m 0644 ${WORKDIR}/profile-walky-boot ${D}${sysconfdir}/skel/.profile
}

FILES:${PN} += "${sysconfdir}/skel/.profile"
