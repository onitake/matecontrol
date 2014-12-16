TARGET = matecontrol

# Aversive sources
AVERSIVE_DIR = ../aversive

# Project sources
SRC = \
	main.c \
	dispatch.c

CFLAGS = -DDISPATCH_SIZE=128

########################################

-include .aversive_conf
include $(AVERSIVE_DIR)/mk/aversive_project.mk
