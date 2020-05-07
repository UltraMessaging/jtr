# lbm.sh - this script should be "sourced".
# 1. Replace the RELPATH and PLATFORM lines with your own values.
# 2. Create lbm.lic with your license information.

RELPATH=$HOME/UMP_6.12.1
PLATFORM=Linux-glibc-2.17-x86_64

# UM license key file: lbm.lic
if [ -f lbm.lic ];  then  :
  LBM_LICENSE_FILENAME=lbm.lic
  export LBM_LICENSE_FILENAME
else  :
  echo "lbm.sh: 'lbm.lic' not found"  >&2
fi

LBM_BASE=$RELPATH
TARGET_PLATFORM=$PLATFORM
LBM_PLATFORM=$LBM_BASE/$TARGET_PLATFORM
LBM_INCLUDE=$LBM_PLATFORM/include
export LBM_BASE TARGET_PLATFORM LBM_INCLUDE LBM_PLATFORM

LD_LIBRARY_PATH=$LBM_PLATFORM/lib
PATH="$LBM_PLATFORM/bin:$PATH"
export LD_LIBRARY_PATH PATH
