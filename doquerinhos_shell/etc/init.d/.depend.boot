TARGETS = mountkernfs.sh hostname.sh mountdevsubfs.sh procps hwclock.sh checkroot.sh urandom checkfs.sh checkroot-bootclean.sh bootmisc.sh mountall.sh mountall-bootclean.sh mountnfs-bootclean.sh mountnfs.sh
INTERACTIVE = checkroot.sh checkfs.sh
mountdevsubfs.sh: mountkernfs.sh
procps: mountkernfs.sh
hwclock.sh: mountdevsubfs.sh
checkroot.sh: hwclock.sh mountdevsubfs.sh hostname.sh
urandom: hwclock.sh
checkfs.sh: checkroot.sh
checkroot-bootclean.sh: checkroot.sh
bootmisc.sh: checkroot-bootclean.sh mountall-bootclean.sh mountnfs-bootclean.sh
mountall.sh: checkfs.sh checkroot-bootclean.sh
mountall-bootclean.sh: mountall.sh
mountnfs-bootclean.sh: mountnfs.sh
