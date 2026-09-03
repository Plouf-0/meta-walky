FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://weston.ini file://weston-user.conf"

do_install:append() {
    install -d ${D}${sysconfdir}/xdg/weston
    install -m 0644 ${WORKDIR}/weston.ini ${D}${sysconfdir}/xdg/weston/weston.ini

    install -d ${D}${systemd_system_unitdir}/weston.service.d
    install -m 0644 ${WORKDIR}/weston-user.conf ${D}${systemd_system_unitdir}/weston.service.d/weston-user.conf
}

SYSTEMD_AUTO_ENABLE:${PN} = "disable"
SYSTEMD_SERVICE:${PN} = "weston.service weston.socket"

FILES:${PN} += "${sysconfdir}/xdg/weston/weston.ini \
    ${systemd_system_unitdir}/weston.service.d/weston-user.conf \
"

# TODO: le weston.ini installé ici référence maintenant
# "modules=walky-hmi-controller.so" (ivi-shell) au lieu de kiosk-shell.
# Le paquet walky-hmi-controller doit donc être présent sur l'image et son
# .so accessible dans ${libexecdir}/weston avant que Weston ne démarre —
# géré via IMAGE_INSTALL:append dans walky-image.bb (voir plus bas), pas
# ici : ce bbappend ne fait qu'installer la config, pas le module lui-même.