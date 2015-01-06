TARGET = matecontrol

# Aversive sources
AVERSIVE_DIR = ../aversive

# Project sources
SRC = \
	main.c \
	dispatch.c \
	led.c

CFLAGS = -DDISPATCH_QUEUE_LENGTH=32

########################################

-include .aversive_conf
include $(AVERSIVE_DIR)/mk/aversive_project.mk
