set(VENDOR_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/vendor")
set(vendor_libraries vendor)

# s3.amazonaws.com/github/downloads/swganh/mmoserver
ExternalProject_Add(vendor
	DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}
	URL http://s3.amazonaws.com/github/downloads/swganh/mmoserver/mmoserver-deps-${mmoserver_VERSION_MAJOR}.${mmoserver_VERSION_MINOR}.${mmoserver_VERSION_PATCH}-win.tar.gz
	URL_MD5 5f4014f86752a59a304b834fb7cd4c6e
	SOURCE_DIR ${VENDOR_PREFIX}
	UPDATE_COMMAND ""
	PATCH_COMMAND ""
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
	INSTALL_COMMAND ""
)
