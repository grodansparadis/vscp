include $(TOPDIR)/rules.mk

PKG_NAME:=evscpd
PKG_RELEASE:=1
PATH_VSCP:=/home/ice/work/pic/can/vscp.git
PKG_SOURCES:=$(PATH_VSCP)/src/vscp/daemon/linux/carambola
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=utils
	CATEGORY:=Utilities
	DEPENDS:=+libwebsockets +libwxbase
	TITLE:=$(PKG_NAME)
endef

MAKE_FLAGS += \
	CFLAGS="$(TARGET_CFLAGS) -g -O2 -fPIC -pthread" \
	DLFLAGS=" -g -shared -fPIC -Wl,--no-undefined" \
	CXXFLAGS="$(TARGET_CXXFLAGS) -fno-builtin -fno-rtti -nostdinc++ -g -O2  -D__LINUX__ -fPIC" \
	CPPFLAGS="$(TARGET_CPPFLAGS) -I$(STAGING_DIR)/usr/include/uClibc++ -I$(LINUX_DIR)/include -I$(STAGING_DIR)/usr/lib/wx/include/mipsel-openwrt-linux-base-unicode-release-2.8 -I$(STAGING_DIR)/usr/include/wx-2.8 -I$(PATH_VSCP)/src/vscp/daemon/linux -I$(PATH_VSCP)/src/common -I$(PATH_VSCP)/src/vscp/common -Iinclude -DwxUSE_GUI=0" \
	LDFLAGS="$(TARGET_LDFLAGS) $(LDFLAGS)" \
	LIBS="$(TARGET_LIBS) -lwebsockets -lz -lm -lcrypto -lpthread -ldl -lwx_baseu_xml-2.8-mipsel-openwrt-linux -lwx_baseu_net-2.8-mipsel-openwrt-linux -lwx_baseu-2.8-mipsel-openwrt-linux" \
	DESTDIR="$(PKG_INSTALL_DIR)"

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) $(PKG_SOURCES)/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) $(MAKE_FLAGS);
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_NAME) $(1)/bin/
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/socketcandrv.so $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/etc/vscp
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/vscpd.conf $(1)/etc/vscp
	$(INSTALL_DIR) $(1)/var/www/htdocs/vscp/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/www/*.html $(1)/var/www/htdocs/vscp/
	$(INSTALL_DIR) $(1)/var/www/htdocs/vscp/images/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/www/images/*.png $(1)/var/www/htdocs/vscp/images/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))

