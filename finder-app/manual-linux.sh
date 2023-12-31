#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.1.10
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}
mkdir -p ${OUTDIR}/rootfs
ROOT_FS=${OUTDIR}/rootfs

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    # TODO [Done]: Add your kernel build steps here
    # 1. Clear old build, remove .config
    make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- mrproper

    # 2. Make def .config
    make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- defconfig

    # 3. Build the kernel
    make -j8 ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- all
fi

echo "Adding the Image in outdir"
cp ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ${OUTDIR}

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# TODO [Done]: Create necessary base directories
mkdir rootfs
cd rootfs

mkdir bin dev etc home lib lib64 proc sbin sys tmp usr var
mkdir usr/bin usr/lib usr/sbin
mkdir var/log

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO [Done]:  Configure busybox
    make defconfig
else
    cd busybox
    make distclean
    make defconfig
fi

# TODO [Done]: Make and install busybox
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install
mkdir bin
cp busybox bin/

echo "Library dependencies"
${CROSS_COMPILE}readelf -a bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a bin/busybox | grep "Shared library"

# TODO [Done]: Add library dependencies to rootfs
cp ${FINDER_APP_DIR}/ld-linux-aarch64.so.1 $ROOT_FS/lib
cp ${FINDER_APP_DIR}/libm.so.6 $ROOT_FS/lib64
cp ${FINDER_APP_DIR}/libresolv.so.2 $ROOT_FS/lib64
cp ${FINDER_APP_DIR}/libc.so.6 $ROOT_FS/lib64

# TODO [Done]: Make device nodes
cd "$OUTDIR"
sudo mknod -m 666 rootfs/dev/null c 1 3
sudo mknod -m 666 rootfs/dev/tty c 5 1

# TODO [Done]: Clean and build the writer utility
cd ${FINDER_APP_DIR}
if [ -e "./writer" ]; then
    make clean
fi
make CROSS_COMPILE=aarch64-none-linux-gnu-

# TODO [Done]: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
mkdir $ROOT_FS/home/conf
mkdir $ROOT_FS/conf

cp writer $ROOT_FS/home
cp finder.sh $ROOT_FS/home
cp finder-test.sh $ROOT_FS/home
cp conf/username.txt $ROOT_FS/home/conf/username.txt
cp conf/assignment.txt $ROOT_FS/conf/assignment.txt
cp autorun-qemu.sh $ROOT_FS/home

# TODO [Done]: Chown the root directory
sudo chown -R root:root $ROOT_FS/../rootfs

# TODO: Create initramfs.cpio.gz
cd $OUTDIR/rootfs
find . | cpio -H newc -ov --owner root:root > $OUTDIR/initramfs.cpio
gzip -f $OUTDIR/initramfs.cpio

rm -rf $OUTDIR/busybox/bin
