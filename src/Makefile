#SOURCES=	IN_XSF IN_2SF IN_GSF IN_SNSF IN_NCSF TMP
SOURCES=	IN_XSF IN_2SF IN_GSF IN_SNSF IN_NCSF

IN_XSF_SRCS =	./in_xsf.cpp $(shell find . \! -name . -prune \( -iname \*.cpp -o -iname \*.c \) \! -iname tmp.cpp \! -iname XSFConfig_\*.cpp \! -iname XSFPlayer_\*.cpp \! -iname in_\*sf.cpp)
IN_2SF_SRCS =	./XSFConfig_2SF.cpp ./XSFConfig_Winamp.cpp ./XSFPlayer_2SF.cpp $(shell find ./desmume \( -iname \*.cpp -o -iname \*.c \) \! -iname cpu_detect_x86_win.cpp \! -iname debug.cpp)
IN_GSF_SRCS =	./XSFConfig_GSF.cpp ./XSFConfig_Winamp.cpp ./XSFPlayer_GSF.cpp $(shell find ./vbam \( -iname \*.cpp -o -iname \*.c \))
IN_SNSF_SRCS =	./XSFConfig_SNSF.cpp ./XSFConfig_Winamp.cpp ./XSFPlayer_SNSF.cpp $(shell find ./snes9x \( -iname \*.cpp -o -iname \*.c \))
IN_NCSF_SRCS =	./XSFConfig_NCSF.cpp ./XSFConfig_Winamp.cpp ./XSFPlayer_NCSF.cpp $(shell find ./SSEQPlayer \( -iname \*.cpp -o -iname \*.c \))
#TMP_SRCS =	./tmp.cpp

$(foreach SOURCE,$(SOURCES),$(eval $(SOURCE)_OBJS = $(addprefix obj/,$(patsubst %.cpp,%.o,$($(SOURCE)_SRCS:.c=.o)))))
$(foreach SOURCE,$(SOURCES),$(eval ALL_OBJS += $($(SOURCE)_OBJS)))

#DLLS = in_2sf.dll in_gsf.dll in_snsf.dll in_ncsf.dll tmp.exe
DLLS = in_2sf.dll in_gsf.dll in_snsf.dll in_ncsf.dll

$(foreach FILE,$(ALL_OBJS),$(shell mkdir -p $(dir $(FILE))))

CXX ?=	g++
CFLAGS ?=	-pipe -pedantic -Wall -Wextra -Winit-self -Wfloat-equal -Wshadow -Wcast-qual -Wlogical-op -Wunreachable-code -Wabi -Wno-unused-parameter -Wno-variadic-macros -Wno-strict-aliasing
CXXFLAGS ?=	-Woverloaded-virtual

obj/%.o : %.c
	@echo "Compiling $^..."
	@$(CC) -std=gnu99 $(CFLAGS) -Wc++-compat -DWINAMP_PLUGIN -D_WINDOWS -I. $(EXTRA_INCLUDES) -I/c/Program\ Files/Winamp\ SDK -o $@ -c $^

obj/%.o : %.cpp
	@echo "Compiling $^..."
	@$(CXX) -std=gnu++98 $(CFLAGS) $(CXXFLAGS) -DWINAMP_PLUGIN -D_WINDOWS -I. $(EXTRA_INCLUDES) -I/c/Program\ Files/Winamp\ SDK -o $@ -c $^

.PHONY: clean all profile debug optimized

all: $(DLLS)
profile: CFLAGS += -g -pg
profile: all
debug: CFLAGS += -g
debug: all
optimized: CFLAGS += -O3
optimized: all

$(IN_XSF_OBJS): CXXFLAGS += -Weffc++
$(IN_2SF_OBJS): EXTRA_INCLUDES = -Idesmume
$(IN_2SF_OBJS): CFLAGS += -DHAVE_LIBZ
in_2sf.dll: $(IN_XSF_OBJS) $(IN_2SF_OBJS)
	@echo "Linking $@..."
	@$(CXX) $(CFLAGS) -static-libgcc -static-libstdc++ -shared -o $@ $^ -lz -lgdi32
$(IN_GSF_OBJS): EXTRA_INCLUDES = -Ivbam
$(IN_GSF_OBJS): CFLAGS += -DNO_DEBUGGER -DFINAL_VERSION
in_gsf.dll: $(IN_XSF_OBJS) $(IN_GSF_OBJS)
	@echo "Linking $@..."
	@$(CXX) $(CFLAGS) -static-libgcc -static-libstdc++ -shared -o $@ $^ -lz -lgdi32
$(IN_SNSF_OBJS): EXTRA_INCLUDES = -Isnes9x
in_snsf.dll: $(IN_XSF_OBJS) $(IN_SNSF_OBJS)
	@echo "Linking $@..."
	@$(CXX) $(CFLAGS) -static-libgcc -static-libstdc++ -shared -o $@ $^ -lz -lgdi32
$(IN_NCSF_OBJS): EXTRA_INCLUDES = -ISSEQPlayer
in_snsf.dll: $(IN_XSF_OBJS) $(IN_NCSF_OBJS)
	@echo "Linking $@..."
	@$(CXX) $(CFLAGS) -static-libgcc -static-libstdc++ -shared -o $@ $^ -lz -lgdi32
tmp.exe: $(IN_XSF_OBJS) $(IN_2SF_OBJS) $(TMP_OBJS)
	@echo "Linking $@..."
	@$(CXX) $(CFLAGS) -static-libgcc -static-libstdc++ -o $@ $^ -lz -lgdi32

clean:
	@echo "Cleaning DLLs and OBJs..."
	@-rm -f $(DLLS) $(ALL_OBJS)
