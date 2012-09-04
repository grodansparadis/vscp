include $(TOPDIR)/rules.mk

PKG_NAME:=evscpd
PKG_RELEASE:=1
PATH_VSCP:=/usr/local/src/m2m
PKG_SOURCES:=$(PATH_VSCP)/src/vscp/daemon/linux/carambola
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=$(PKG_NAME)
endef

MAKE_FLAGS += \
	CFLAGS="$(TARGET_CFLAGS)" \
	CXXFLAGS="$(TARGET_CXXFLAGS) -fno-builtin -fno-rtti -nostdinc++" \
	CPPFLAGS="$(TARGET_CPPFLAGS) -I$(STAGING_DIR)/usr/include/uClibc++ -I$(LINUX_DIR)/include -I$(STAGING_DIR)/usr/lib/wx/include/mipsel-openwrt-linux-base-unicode-release-2.8 -I$(STAGING_DIR)/usr/include/wx-2.8 -I$(PATH_VSCP)/src/vscp/daemon/linux -I$(PATH_VSCP)/src/common -I$(PATH_VSCP)/src/vscp/common  -DwxUSE_GUI=0" \
	LDFLAGS="$(TARGET_LDFLAGS) $(LDFLAGS)" \
	LIBS="$(TARGET_LIBS) -lz -lm -lcrypto -lpthread -ldl -lwx_baseu_xml-2.8-mipsel-openwrt-linux -lwx_baseu_net-2.8-mipsel-openwrt-linux -lwx_baseu-2.8-mipsel-openwrt-linux" \
	DESTDIR="$(PKG_INSTALL_DIR)"

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) $(PKG_SOURCES)/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) $(MAKE_FLAGS);
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_NAME) $(1)/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))

