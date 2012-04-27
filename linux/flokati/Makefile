#inspired by http://manoftoday.wordpress.com/2007/10/11/writing-and-compiling-a-simple-program-for-openwrt/
#and the Makefile from ifenslave

include $(TOPDIR)/rules.mk

PKG_NAME:=flokati
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/flokati
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Flokati-Bridge and Flokati-Database
  VERSION:=1.0
  DEPENDS:=+ser2net libjson
endef

define Package/flokati/description
 Bridge between Flokati hardware and Flokati's IP-multicast group. Also includes a Flokati database cache.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/flokati/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/flokati-bridge $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/flokati-db $(1)/bin/
	
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/openwrt-init.sh $(1)/etc/init.d/flokati
	
	$(INSTALL_DIR) $(1)/etc/flokati/db
endef

define Package/flokati/postinst
	#!/bin/sh
	# check if we are on real system
	if [ -z "$${IPKG_INSTROOT}" ]; then
			echo "Enabling rc.d symlink for flokati"
			/etc/init.d/flokati enable
	fi
	exit 0
endef

define Package/flokati/prerm
	#!/bin/sh
	# check if we are on real system
	if [ -z "$${IPKG_INSTROOT}" ]; then
			echo "Removing rc.d symlink for flokati"
			/etc/init.d/flokati disable
	fi
	exit 0
endef

$(eval $(call BuildPackage,flokati))