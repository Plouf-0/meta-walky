require recipes-core/images/core-image-base.bb

IMAGE_FEATURES += "ssh-server-openssh"

IMAGE_INSTALL:append = " weston"
IMAGE_INSTALL:append = " weston-init"
IMAGE_INSTALL:append = " qtbase"
IMAGE_INSTALL:append = " qtdeclarative"
#pour tests
IMAGE_INSTALL:append = " weston-examples"
IMAGE_INSTALL:append = " walky-hmi-controller"

inherit extrausers

EXTRA_USERS_PARAMS = "\
    useradd -d /home/walky -G video,input,audio,render,tty -s /bin/sh -m walky; \
"
