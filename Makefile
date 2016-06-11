# Note: It is important to make sure you include the <bsd.kmod.mk> makefile after declaring the KMOD and SRCS variables.

# Declare Name of kernel module
KMOD    =  ssd_filter

# Enumerate Source files for kernel module
SRCS    =  device_if.h bus_if.h opt_cam.h ssd_filter_cam.c

#INC=-I/usr/share/mk/
# Include kernel module makefile
.include <bsd.kmod.mk>
