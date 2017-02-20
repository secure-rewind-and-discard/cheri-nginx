#!/usr/bin/env bash
set -e
CHERI_ROOT="/home/trasz/cheri"
CHERISDK="${CHERI_ROOT}/output/sdk256/bin"
CHERILDDIR="${CHERI_ROOT}/build/cheribsd-obj-256/mips.mips64/usr/home/trasz/cheri/cheribsd/tmp/usr/bin/"
CHERIBSD_SYSROOT="${CHERI_ROOT}/output/sdk256/sysroot"
INSTALL_DIR=${CHERI_ROOT}/output/rootfs256
READLINE_INCLUDE_DIR=${CHERIBSD_SYSROOT}/usr/include/edit/

# This needs to be done first.  Nginx doesn't support cross-building;
# if you try to tell it to use cheri compiler, it will try to execute
# the output binary and the build will fail.
sh auto/configure --with-debug

COMMON_FLAGS="-pipe --sysroot=${CHERIBSD_SYSROOT} -B${CHERILDDIR} -target cheri-unknown-freebsd -mabi=sandbox -msoft-float -mxgot -O0 -G0 -static -integrated-as"
COMPILE_FLAGS="${COMMON_FLAGS} -isystem ${READLINE_INCLUDE_DIR} -Wcheri-capability-misuse -Werror=implicit-function-declaration -Werror=format -Werror=undefined-internal"

export CFLAGS=${COMPILE_FLAGS}
export CFLAGS=${COMPILE_FLAGS}
export PATH=${CHERISDK}:${CHERILDDIR}:$PATH
export CC=${CHERISDK}/clang

sed -i '' "s!^CC .*!CC = ${CC}!;s!^CFLAGS .*!CFLAGS = ${CFLAGS}!;s!^CPP .*!CPP = ${CC} -E!;s!^LINK .*!LINK = ${CC} ${CFLAGS}!" objs/Makefile
make
#make install DESTDIR=${INSTALL_DIR}
#chmod -R a+rX ${INSTALL_DIR}

