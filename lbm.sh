# lbm.sh
# 1. Replace the RELPATH and PLATFORM lines with your own values.
# 2. Create lbm.lic with your license information:
# export LBM_LICENSE_INFO="..."

RELPATH=$HOME/UMP_6.12.1
PLATFORM=Linux-glibc-2.17-x86_64

# Source the file that defines env var for license key.
export LBM_LICENSE_FILENAME
LBM_LICENSE_FILENAME=lbm.lic

export LBM_BASE TARGET_PLATFORM LBM_PLATFORM
LBM_BASE=$RELPATH
TARGET_PLATFORM=$PLATFORM
LBM_PLATFORM=$LBM_BASE/$TARGET_PLATFORM

export LD_LIBRARY_PATH
LD_LIBRARY_PATH=$LBM_PLATFORM/lib

export LBM_INCLUDE
LBM_INCLUDE=$LBM_PLATFORM/include

export PATH
PATH="$LBM_PLATFORM/bin:$PATH"
