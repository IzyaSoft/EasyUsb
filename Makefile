# EasyUsb project Makefile, v 1.0
# ********************************0. INCLUDE SECTION***********************************
# include Makefile.extra
# *****************************END OF INCLUDE SECTION**********************************
# ********************************1. VARIABLES SECTION*********************************
# 1 C/C++ Compiler
CXX = g++
CXXFLAGS = -g -O2 -Wall
# 2. Compiler And Linker Keys (man gcc)
PROJECT_NAME = libEasyUsb
LIB_NAME = $(PROJECT_NAME).so
LIB_VERSION_OPTION = 1.0
LIB_BUILD_DIRECTORY = .
LIB_INSTALL_DIR = /usr/lib
LIB_INCLUDE_DIR = /usr/include/$(PROJECT_NAME)

LIB_COMPILE_OPTION = -fPIC
LIB_LINK_OPTION = -shared

LANG_OPTION = -std=c++11
# 3. Linking libraries
LIBPATH = -L. -L.. -L/lib64/ -L/lib/ -L/usr/lib64 -L/usr/lib
LIBS = -lusb-1.0
# -lc -lrt
# -lrt -lpthread
# 4. DEFINITIONS (PREPROCESSOR DEFINE)
ROOT_DIR := ./src
DEFS := -D__GXX_EXPERIMENTAL_CXX0X__
# 5. INCLUDES PATH
INCLUDES := -I$(ROOT_DIR)/ -I/usr/include/libusb-1.0
# 6. SOURCES FILES
# Directories with sources
DIRS = $(ROOT_DIR)
SRC_DIRS := $(addprefix / , $(DIRS))
# Obtaining source files list
C_SRC_FILES := $(foreach sdir, $(SRC_DIRS), $(wildcard $(sdir)/*.c))
CPP_SRC_FILES := $(foreach sdir, $(SRC_DIRS), $(wildcard $(sdir)/*.cpp))
# 7. OBJECTS FILE NAME
C_OBJFILES = $(C_SRC_FILES:.c=.o)
CPP_OBJFILES = $(CPP_SRC_FILES:.cpp=.o) $(C_OBJFILES)
# $(SRCFILES:.cpp=.o)
# 8. TARGETS OR RESULTING OBJ-FILE
# LIB_VERSION := 1.0
CPP_SHARED_LIB = $(LIB_BUILD_DIRECTORY)/$(LIB_NAME).$(LIB_VERSION_OPTION)
DEFAULT_TARGET = shared-lib
# *****************************END OF VARIABLES SECTION********************************
# ****************************2. BUILDING TARGETS SECTION******************************
# BY DEFAULT BUILDS first target (make without parameters)
# TO BUILD SPECIFIC TARGETS TYPE make "target name" (without quotation marks)
# each target MUST BE WRITTEN AS: Dependencies Tab(press tab key) Command
# MAKEFILE SPECIAL MACROSES (STARTS WITH $):
# $@ name of target 
# $? list of dependancies before that macro
# $^ list of dependancies which independent of wheather they met before or after
# $+ similar to $^ but doesn't exclude dublicates
# $< first dependancy

# PHONY TARGET ARE TARGETS WITHOUT OUTPUT FILES
.PHONY: depend clean finish create-build-dir copy-include install

all: clean create-build-dir $(DEFAULT_TARGET) finish

shared-lib: clean create-build-dir $(CPP_SHARED_LIB) copy-include finish

create-build-dir:
	@ -mkdir -p $(LIB_BUILD_DIRECTORY)
	
$(CPP_SHARED_LIB):$(C_OBJFILES) $(CPP_OBJFILES)
	$(CXX) $(LIB_LINK_OPTION) $(LIBPATH) $(LIBS) -o $(CPP_SHARED_LIB) $(CPP_OBJFILES)
	@ -ln -s $(CPP_SHARED_LIB) $(LIB_NAME)
	
install:
	@ -cp $(CPP_SHARED_LIB) $(LIB_INSTALL_DIR)
	@ -ln -s $(LIB_INSTALL_DIR)/$(LIB_NAME).$(LIB_VERSION_OPTION) $(LIB_INSTALL_DIR)/$(LIB_NAME)
	@ -mkdir $(LIB_INCLUDE_DIR)
	@ -cp -R $(LIB_BUILD_DIRECTORY)/include $(LIB_INCLUDE_DIR)

uninstall:
	@ -rm $(LIB_INSTALL_DIR)/$(LIB_NAME)
	@ -rm $(LIB_INSTALL_DIR)/$(LIB_NAME).$(LIB_VERSION_OPTION)
	@ -rm -rf $(LIB_INCLUDE_DIR)

# These are the suffix replacement rules
%.o : %.c
	$(CXX) -c $(CXXFLAGS) $(LANG_OPTION) $(LIB_COMPILE_OPTION) $(DEFS) $(INCLUDES) $< -o $@

%.o : %.cpp
	$(CXX) -c $(CXXFLAGS) $(LANG_OPTION)  $(LIB_COMPILE_OPTION) $(DEFS) $(INCLUDES) $< -o $@

clean:
	@ -rm -f $(CPP_OBJFILES)
	@ -rm -f $(LIB_NAME)
	@ -rm -f $(CPP_SHARED_LIB)
	@ -rm -rf $(LIB_BUILD_DIRECTORY)/include

copy-include:
	@ mkdir -p $(LIB_BUILD_DIRECTORY)/include
	@ find $(ROOT_DIR) -name *.h -exec cp {} $(LIB_BUILD_DIRECTORY)/include \;
#@ cp -R $(ROOT_DIR)/*.h $(LIB_BUILD_DIRECTORY)/include

#remove intermediate obj files
finish:
	@ -rm $(CPP_OBJFILES)

depend: $(C_SRC_FILES) $(CPP_SRC_FILES)
	makedepend $(INCLUDES) $^

# make depend needs this line
# ****************************END OF BUILDING TARGETS SECTION**************************

