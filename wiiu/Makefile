# Minimal Makefile scaffold for building Sonic 3 A.I.R. for Wii U (devkitPPC / WUT)
# Adjust paths and source lists to match your tree and devkit installation.

CXX := $(shell command -v powerpc-eabi-g++ 2>/dev/null || echo powerpc-eabi-g++)
CC := $(shell command -v powerpc-eabi-gcc 2>/dev/null || echo powerpc-eabi-gcc)
AR := $(shell command -v powerpc-eabi-ar 2>/dev/null || echo powerpc-eabi-ar)
STRIP := $(shell command -v powerpc-eabi-strip 2>/dev/null || echo powerpc-eabi-strip)

# Root of repository
ROOT := $(abspath ..)

INCLUDES := -I$(ROOT)/sonic3air-wiiu -I$(ROOT)/sonic3air-wiiu/librmx/source -I$(ROOT)/sonic3air-wiiu/Oxygen \
			-I$(ROOT)/sonic3air-wiiu/framework/external/imgui -I$(ROOT)/sonic3air-wiiu/framework/external/imgui/imgui \
			-I$(ROOT)/sonic3air-wiiu/framework/external/ogg-vorbis/libvorbis/include -I$(ROOT)/sonic3air-wiiu/framework/external/ogg-vorbis/libogg/include

# Force sensible flags for the cross compiler and ignore environment CXXFLAGS
# Define __WIIU__ so PlatformDefinitions.h picks up `PLATFORM_WIIU` correctly
override CXXFLAGS := -O2 -mcpu=750 -Wall -fno-exceptions -fno-rtti -DPLATFORM_WIIU -D__WIIU__
LDFLAGS := -Wl,-T,wut_linker_script.ld

# Libraries typically required for WUT / GX2. Adjust as necessary for your toolchain.
LIBS := -lwhb -lgx2 -lcoreinit -lpowerenv -lproc_ui -lwhbfs -lwhbpad

SRCS := $(shell find $(ROOT)/sonic3air-wiiu/librmx/source $(ROOT)/sonic3air-wiiu/Oxygen -name '*.cpp' 2>/dev/null)
OBJS := $(patsubst %.cpp,%.o,$(SRCS))

TARGET := s3air_wiiu.elf

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)
	$(STRIP) --strip-all $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
