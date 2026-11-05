# Built as a static library only: one copy of the Lua runtime per server, and
# no Lua 5.1 DLL import/export macros to manage on Windows.
vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

# The Ubuntu archive carries the unmodified upstream tarball (same SHA as
# lua.org), as a mirror for when lua.org is unreachable.
vcpkg_download_distfile(ARCHIVE
    URLS
        "https://www.lua.org/ftp/lua-${VERSION}.tar.gz"
        "https://archive.ubuntu.com/ubuntu/pool/main/l/lua5.1/lua5.1_${VERSION}.orig.tar.gz"
    FILENAME "lua-${VERSION}.tar.gz"
    SHA512 0142fefcbd13afcd9b201403592aa60620011cc8e8559d4d2db2f92739d18186860989f48caa45830ff4f99bfc7483287fd3ff3a16d4dec928e2767ce4d542a9
)

vcpkg_extract_source_archive(SOURCE_PATH ARCHIVE "${ARCHIVE}")

file(COPY "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt" DESTINATION "${SOURCE_PATH}")

vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME unofficial-lua51)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/COPYRIGHT")
