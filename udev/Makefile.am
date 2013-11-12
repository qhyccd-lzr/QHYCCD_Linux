udev_rulesdir = /etc/udev/rules.d
udev_rules_DATA = 85-qhyccd.rules
EXTRA_DIST = $(udev_rules_DATA)

install-data-hook:
	sed -i 's|QHY_HEX_PATH|$(libdir)/libqhyccd/firmware|g' $(udev_rulesdir)/85-qhyccd.rules
