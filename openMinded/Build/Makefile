SDK_PATH:=$(shell pwd)/../../ARDroneLib/
PC_TARGET=yes
USE_LINUX=yes

ifdef MYKONOS
   include $(ARDRONE_CUSTOM_CONFIG)
   include $(ARDRONE_BUILD_CONFIG)
else
   include $(SDK_PATH)/Soft/Build/custom.makefile
   include $(SDK_PATH)/Soft/Build/config.makefile
endif

ifeq "$(RELEASE_BUILD)" "yes"
   ARDRONE_TARGET_DIR=$(shell pwd)/../
else
   ARDRONE_TARGET_DIR=$(shell pwd)/../
endif

TARGET=smartfox

# Used directories
MSVMPACK_DIR= $(shell pwd)/../Sources/MSVMpack1.5/
PG_INC=/usr/include/postgresql/
PG_LIB=/usr/lib/postgresql/8.4/lib/
DB_LIB_DIR=$(shell pwd)/../Sources/Navdata/database/
BUILD_DIR= $(shell pwd) 
SRC_DIR:=$(shell pwd)/../Sources
# Define application source files
GENERIC_BINARIES_SOURCE_DIR:=$(SRC_DIR)

GENERIC_BINARIES_COMMON_SOURCE_FILES+=	\
   Navdata/navdata_analyse.c    	\
   Navdata/classifier.c    	\
   ardrone_move_cmd.c   		\
   disable_navdata_demo.c    		\
   utils.c		\
   Model/model.c	\
   Model/residue.c      \
   UI/gui.c  		\
   UI/displayMsg.c	\
   UI/configurePage.c	\
   UI/debugPage.c 	\
   UI/diagnosisPage.c 	\
   UI/ardrone_gui.c	\
   UserApp/user_app.c \
   UserApp/mission.c 
   

# ^ ######### USER APPLICATION ########  ^
# | add here the c files of your threads |

# Include directories 
GENERIC_INCLUDES+=			\
	$(SRC_DIR)			\
	$(DB_LIB_DIR) 			\
	$(LIB_DIR) 			\
	$(PG_INC) 			\
	$(SDK_PATH)/Soft/Common 	\
	$(SDK_PATH)/Soft/Lib 		\
	$(MSVMPACK_DIR)include/ 	\

GENERIC_TARGET_BINARIES_PREFIX=

GENERIC_TARGET_BINARIES_DIR=$(ARDRONE_TARGET_DIR)

GENERIC_BINARIES_SOURCE_ENTRYPOINTS+=			\
   smartfox_main.c

GENERIC_INCLUDES:=$(addprefix -I,$(GENERIC_INCLUDES))

# Included libraries management
GENERIC_LIB_PATHS=-L$(GENERIC_TARGET_BINARIES_DIR) -L$(PG_LIB) -L$(DB_LIB_DIR) -lopenMinded_db -L$(MSVMPACK_DIR)lib/
GENERIC_LIBS=-lpc_ardrone -lgtk-x11-2.0 -lrt -lpq -lmsvm -lm -ldl -lpthread

SDK_FLAGS+="USE_APP=yes"
SDK_FLAGS+="APP_ID=smartfox"

# Transmit values to the PARROT Library makefile
export GENERIC_CFLAGS
export GENERIC_LIBS
export GENERIC_LIB_PATHS
export GENERIC_INCLUDES
export GENERIC_BINARIES_SOURCE_DIR
export GENERIC_BINARIES_COMMON_SOURCE_FILES
export GENERIC_TARGET_BINARIES_PREFIX
export GENERIC_TARGET_BINARIES_DIR
export GENERIC_BINARIES_SOURCE_ENTRYPOINTS


LIB=-lmsvm -lm -ldl -lpthread

# Bug fix ...
export GENERIC_LIBRARY_SOURCE_DIR=$(GENERIC_BINARIES_SOURCE_DIR)

# Prevent from erasing files that are called like a makefile keyword
.PHONY: $(TARGET) build_libs build_openMinded_db_lib



all: build_openMinded_db_lib build_libs $(TARGET)

$(TARGET):
	@$(MAKE) -C $(SDK_PATH)/VP_SDK/Build $(TMP_SDK_FLAGS) $(SDK_FLAGS) $(MAKECMDGOALS) USE_LINUX=yes
	mv $(ARDRONE_TARGET_DIR)/smartfox_main $(TARGET)
	mv $(TARGET) $(ARDRONE_TARGET_DIR)/

$(MAKECMDGOALS): build_libs
	@$(MAKE) -C $(SDK_PATH)/VP_SDK/Build $(TMP_SDK_FLAGS) $(SDK_FLAGS) $(MAKECMDGOALS) USE_LINUX=yes

build_libs:
	@$(MAKE) -C $(SDK_PATH)/Soft/Build $(TMP_SDK_FLAGS) $(SDK_FLAGS)  $(MAKECMDGOALS) USE_LINUX=yes



# OpenMinded Database management library compilation
build_openMinded_db_lib:
	@(cd $(DB_LIB_DIR) && $(MAKE))
	@(cd $(BUILD_DIR)) 
#	@(cd $(MSVMPACK_DIR) && $(MAKE))

