ifneq ($(BUILD),DEBUG)
	ifneq ($(BUILD),RELEASE)
		BUILD = DEBUG
	endif
endif

ifeq ($(BUILD),DEBUG)
	GDBFLAG = -g
endif

#ARCH_TYPE = x86#$(shell uname -m)
#ARCH_TYPE = $(shell uname -m)
