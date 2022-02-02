#! /bin/sh

VERSION_MAJOR=3
VERSION_PCIE_BOARD_VERSION=22
VERSION_MINOR=0
VERSION_STR=${VERSION_MAJOR}.${VERSION_PCIE_BOARD_VERSION}-${VERSION_MINOR}
PACKAGENAME=escam
PKG_DIR=${PACKAGENAME}_${VERSION_STR}
MODULENAME=lscpcie

# clean up
rm -rf escam_*

# bin file
if [ -f ../build-escam-Desktop-Release/build/escam ]
then
    mkdir -p ${PKG_DIR}/usr/bin
    cp ../build-escam-Desktop-Release/build/escam ${PKG_DIR}/usr/bin
elif [ -f ../escam/build/escam ]
then
    mkdir -p ${PKG_DIR}/usr/bin
    cp ../escam/build/escam ${PKG_DIR}/usr/bin
else
    echo "Escam binary not found. Please compile escam before running this script."
    return
fi


# kernel module
mkdir -p ${PKG_DIR}/usr/lib/modules-load.d
echo "# Load ${MODULENAME}.ko at boot\n${MODULENAME}" > ${PKG_DIR}/usr/lib/modules-load.d/${MODULENAME}.conf
mkdir -p ${PKG_DIR}/usr/src/${MODULENAME}-${VERSION_STR}/src
cp ../linux-driver/kernelspace/*.h ${PKG_DIR}/usr/src/${MODULENAME}-${VERSION_STR}/src
cp ../linux-driver/kernelspace/*.c ${PKG_DIR}/usr/src/${MODULENAME}-${VERSION_STR}/src
cp ../linux-driver/kernelspace/Makefile ${PKG_DIR}/usr/src/${MODULENAME}-${VERSION_STR}/src
echo "MAKE='make -C src/'\n\
CLEAN='make -C src/ clean'\n\
BUILT_MODULE_NAME=${MODULENAME}\n\
BUILT_MODULE_LOCATION=src/\n\
PACKAGE_NAME=${MODULENAME}\n\
PACKAGE_VERSION=${VERSION_STR}\n\
AUTOINSTALL='yes'\n\
DEST_MODULE_LOCATION=/kernel/drivers/pci\n\
REMAKE_INITRD=yes" > ${PKG_DIR}/usr/src/${MODULENAME}-${VERSION_STR}/dkms.conf

# .deb stuff
mkdir -p ${PKG_DIR}/DEBIAN
echo "Package: ${PACKAGENAME}\n\
Version: ${VERSION_STR}\n\
Section: base\n\
Priority: optional\n\
Architecture: amd64\n\
Depends:\n\
Maintainer: Stresing <info@stresing.de>\n\
Description: Software for line scan camera of Stresing\n\
 Enticklungsbuero Stresing\n\
 stresing.de" > ${PKG_DIR}/DEBIAN/control

dpkg-deb --build ${PKG_DIR}
