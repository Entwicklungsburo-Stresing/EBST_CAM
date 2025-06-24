#!/usr/bin/env sh

VERSION_MAJOR=$(grep "#define VERSION_MAJOR_ESCAM" ../version.h | grep -o '[0-9]\+')
VERSION_PCIE_BOARD_VERSION=$(grep "#define VERSION_PCIE_BOARD_VERSION" ../version.h | grep -o '[0-9]\+')
VERSION_MINOR=$(grep "#define VERSION_MINOR" ../version.h | grep -o '[0-9]\+')
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

# so file
if [ -f ../ESLSCDLL/libESLSCDLL.so ]
then
    mkdir -p ${PKG_DIR}/usr/lib
    cp ../ESLSCDLL/libESLSCDLL.so ${PKG_DIR}/usr/lib
else
    echo "libESLSCDLL.so not found. Please compile libESLSCDLL.so before running this script."
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

# .deb stuff: control
mkdir -p ${PKG_DIR}/DEBIAN
echo "Package: ${PACKAGENAME}\n\
Version: ${VERSION_STR}\n\
Section: base\n\
Priority: optional\n\
Architecture: amd64\n\
Depends: dkms, linux-headers-generic\n\
Recommends: libqt6charts6, libhdf5-dev\n\
Maintainer: Stresing <info@stresing.de>\n\
Description: Software for line scan camera of Stresing\n\
 Enticklungsbuero Stresing\n\
 stresing.de" > ${PKG_DIR}/DEBIAN/control

# .deb stuff: postinst
echo "#! /bin/sh\n\
dkms build -m lscpcie -v ${VERSION_STR}\n\
dkms install -m lscpcie -v ${VERSION_STR}" > ${PKG_DIR}/DEBIAN/postinst
chmod +x ${PKG_DIR}/DEBIAN/postinst

# .deb stuff: prerm
echo "#! /bin/sh\n\
dkms uninstall -m lscpcie -v ${VERSION_STR}\n\
dkms remove -m lscpcie -v ${VERSION_STR} --all" > ${PKG_DIR}/DEBIAN/prerm
chmod +x ${PKG_DIR}/DEBIAN/prerm

# udev rule
mkdir -p ${PKG_DIR}/etc/udev/rules.d
echo "SUBSYSTEM==\"lscpcie\", MODE=\"0666\"" > ${PKG_DIR}/etc/udev/rules.d/98-stresing-lscpcie.rules

dpkg-deb --root-owner-group --build ${PKG_DIR}
