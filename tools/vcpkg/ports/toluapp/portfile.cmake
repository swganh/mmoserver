vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

# The upstream site (codenix.com) is gone; Debian and Ubuntu keep the 1.0.93
# source. Only the runtime library is built, not the tolua++ generator.
vcpkg_download_distfile(ARCHIVE
    URLS
        "https://deb.debian.org/debian/pool/main/t/tolua++/tolua++_${VERSION}+repack.orig.tar.gz"
        "https://archive.ubuntu.com/ubuntu/pool/universe/t/tolua++/tolua++_${VERSION}+repack.orig.tar.gz"
    FILENAME "tolua++_${VERSION}+repack.orig.tar.gz"
    SHA512 97e6c33bb46533ac22be293d0535481908f27bdad4dbb6317dc38d2388102b09134b0e62daed9ac29a33699eda9099d965d467bea3b8874e7a8db634d6b7f9df
)

vcpkg_extract_source_archive(SOURCE_PATH ARCHIVE "${ARCHIVE}")

file(COPY "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt" DESTINATION "${SOURCE_PATH}")

vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME unofficial-toluapp)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/COPYRIGHT")
