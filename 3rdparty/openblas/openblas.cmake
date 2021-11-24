include(ExternalProject)

set(OPENBLAS_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/openblas)

if(LINUX_AARCH64 OR APPLE_AARCH64)
    set(OPENBLAS_TARGET "ARMV8")
else()
    set(OPENBLAS_TARGET "NEHALEM")
endif()

set(OPENBLAS_INCLUDE_DIR "${OPENBLAS_INSTALL_PREFIX}/include/") # The "/"" is critical, see open3d_import_3rdparty_library.
set(OPENBLAS_LIB_DIR "${OPENBLAS_INSTALL_PREFIX}/lib")
set(OPENBLAS_LIBRARIES openblas)  # Extends to libopenblas.a automatically.

ExternalProject_Add(
    ext_openblas
    PREFIX openblas
    URL https://github.com/xianyi/OpenBLAS/archive/refs/tags/v0.3.10.tar.gz
    URL_HASH SHA256=0484d275f87e9b8641ff2eecaa9df2830cbe276ac79ad80494822721de6e1693
    DOWNLOAD_DIR "${OPEN3D_THIRD_PARTY_DOWNLOAD_DIR}/openblas"
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND $(MAKE) TARGET=${OPENBLAS_TARGET} NO_SHARED=1 LIBNAME=CUSTOM_LIB_NAME
    BUILD_IN_SOURCE True
    INSTALL_COMMAND $(MAKE) install PREFIX=${OPENBLAS_INSTALL_PREFIX} NO_SHARED=1 LIBNAME=CUSTOM_LIB_NAME
    COMMAND ${CMAKE_COMMAND} -E rename ${OPENBLAS_LIB_DIR}/CUSTOM_LIB_NAME ${OPENBLAS_LIB_DIR}/libopenblas.a
    BUILD_BYPRODUCTS ${OPENBLAS_LIB_DIR}/libopenblas.a
)

message(STATUS "OPENBLAS_INCLUDE_DIR: ${OPENBLAS_INCLUDE_DIR}")
message(STATUS "OPENBLAS_LIB_DIR ${OPENBLAS_LIB_DIR}")
message(STATUS "OPENBLAS_LIBRARIES: ${OPENBLAS_LIBRARIES}")
