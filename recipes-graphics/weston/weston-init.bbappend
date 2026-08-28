FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://weston.ini file://weston-user.conf"

do_install:append() {
    install -d ${D}${sysconfdir}/xdg/weston
    install -m 0644 ${WORKDIR}/weston.ini ${D}${sysconfdir}/xdg/weston/weston.ini

    install -d ${D}${systemd_system_unitdir}/weston.service.d
    install -m 0644 ${WORKDIR}/weston-user.conf ${D}${systemd_system_unitdir}/weston.service.d/weston-user.conf
}

FILES:${PN} += "${sysconfdir}/xdg/weston/weston.ini \
    ${systemd_system_unitdir}/weston.service.d/weston-user.conf \
"

# weston.service et weston.socket sont désactivés : le lancement de Weston
# passe par /etc/skel/.profile (exec weston --socket=wayland-0), pas par
# le service systemd. On garde quand même l'installation de weston-user.conf
# ci-dessus (inoffensive tant que le service est disabled) au cas où on
# voudrait un jour retester l'approche "service systemd" plutôt que .profile.
SYSTEMD_AUTO_ENABLE:${PN} = "disable"
SYSTEMD_SERVICE:${PN} = "weston.service weston.socket"
