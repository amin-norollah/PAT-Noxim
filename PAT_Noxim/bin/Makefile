TARGET_ARCH = linux
CC     = g++
OPT    = -O3
DEBUG  = -g
OTHER  = # -Wall -Wno-deprecated
CFLAGS = $(OPT)

VPATH = ../src:.
MODULE = noxim
SRCS = ./NoximNoC.cpp ./NoximRouter.cpp ./NoximProcessingElement.cpp ./NoximBuffer.cpp \
	./NoximStats.cpp ./NoximGlobalStats.cpp ./NoximGlobalRoutingTable.cpp \
	./NoximLocalRoutingTable.cpp ./NoximGlobalTrafficTable.cpp ./NoximReservationTable.cpp ./NoximVCState.cpp \
	./NoximPower.cpp ./NoximCmdLineParser.cpp ./NoximMain.cpp ./NoximThermal_IF.cpp ./NoximVLink.cpp \
        ./NoximLog.cpp
	
SRCS_C = ./Hotspot_util.c ./Hotspot_temperature_grid.c ./Hotspot_temperature_block.c ./Hotspot_temperature.c \
        ./Hotspot_shape.c ./Hotspot_RCutil.c ./Hotspot_npe.c ./Hotspot_package.c ./Hotspot_flp.c 
		
		
ROUTER_MODEL = ./ORION_router.c ./ORION_arbiter.c ./ORION_crossbar.c
POWER_MODEL	 = ./ORION_router_power.c
LINK_MODEL   = ./ORION_link.c
CLOCK_MODEL  = ./ORION_clock.c
AREA_MODEL   = ./ORION_router_area.c
ARRAY_MODEL  = ./ORION_array_l.c ./ORION_array_m.c ./ORION_cam.c
ALU_MODEL    = ./ORION_ALU.c
OTHER_MODEL  = ./ORION_misc.c ./ORION_permu.c
STATIC_MODEL = ./ORION_static.c
MISC_SRC     = ./ORION_util.c ./ORION_time.c

SRCS_C_ORION = $(ARRAY_MODEL) $(ROUTER_MODEL) $(POWER_MODEL)\
		$(ALU_MODEL) $(OTHER_MODEL) $(STATIC_MODEL) $(LINK_MODEL) $(CLOCK_MODEL) $(MISC_SRC) $(AREA_MODEL)


OBJS = $(SRCS:.cpp=.o)
OBJS_C = $(SRCS_C:.c=.o )
OBJS_C_ORION = $(SRCS_C_ORION:.c=.o )	

OBJS_A = $(OBJS)
OBJS_A += $(OBJS_C)
OBJS_A += $(OBJS_C_ORION)
include ./Makefile.defs
