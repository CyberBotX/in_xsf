UNAME:=	$(shell uname -s)

SRCDIR:=	$(dir $(abspath $(lastword $(MAKEFILE_LIST))))

ZLIB_SRCS:= $(sort $(wildcard $(SRCDIR)in_xsf_framework/zlib/*.c))
ZLIB_OBJS:=	$(subst $(SRCDIR),,$(ZLIB_SRCS:%.c=%.o))
FRAMEWORK_SRCS:=	$(sort $(wildcard $(SRCDIR)in_xsf_framework/*.cpp))
FRAMEWORK_OBJS:=	$(subst $(SRCDIR),,$(FRAMEWORK_SRCS:%.cpp=%.o))

in_2sf_SRCS:=	$(wildcard $(SRCDIR)in_2sf/*.cpp \
	$(SRCDIR)in_2sf/desmume/*.cpp \
	$(SRCDIR)in_2sf/desmume/*/*.cpp \
	$(SRCDIR)in_2sf/desmume/*/*/*.cpp \
	$(SRCDIR)in_2sf/desmume/*/*/*/*.cpp \
	$(SRCDIR)in_2sf/desmume/*/*/*/*/*.cpp \
	$(SRCDIR)in_2sf/spu/*.cpp)
in_gsf_SRCS:=	$(wildcard $(SRCDIR)in_gsf/*.cpp) $(wildcard $(SRCDIR)in_gsf/vbam/apu/*.cpp) $(wildcard $(SRCDIR)in_gsf/vbam/gba/*.cpp)
in_ncsf_SRCS:=	$(wildcard $(SRCDIR)in_ncsf/*.cpp) $(wildcard $(SRCDIR)in_ncsf/SSEQPlayer/*.cpp)
in_snsf_SRCS:=	$(wildcard $(SRCDIR)in_snsf/*.cpp) $(wildcard $(SRCDIR)in_snsf/snes9x/*.cpp) $(wildcard $(SRCDIR)in_snsf/snes9x/apu/*.cpp)

DLLS=	in_2sf in_gsf in_ncsf in_snsf
DLLS:=	$(sort $(addsuffix .dll,$(DLLS)))

COMPILER:=	$(shell $(CXX) -v 2>/dev/stdout)

MY_CPPFLAGS=	$(CPPFLAGS) -std=gnu++17 -DWINAMP_PLUGIN -DUNICODE_INPUT_PLUGIN -D_WINDOWS -I$(SRCDIR)in_xsf_framework -I$(SRCDIR)in_xsf_framework/winamp -I$(SRCDIR)in_xsf_framework/zlib -I$(SRCDIR)in_2sf/desmume
MY_CXXFLAGS=	$(CXXFLAGS) -std=gnu++17 -DWINAMP_PLUGIN -DUNICODE_INPUT_PLUGIN -D_WINDOWS -I$(SRCDIR)in_xsf_framework -I$(SRCDIR)in_xsf_framework/winamp -I$(SRCDIR)in_xsf_framework/zlib -pipe -Wall -Wctor-dtor-privacy -Wold-style-cast -Wextra -Wno-div-by-zero -Wfloat-equal -Wshadow -Winit-self -Wcast-qual -Wunreachable-code -Woverloaded-virtual -Wno-long-long -Wno-switch
MY_CFLAGS=	$(CFLAGS) -std=gnu17 -DWINAMP_PLUGIN -DUNICODE_INPUT_PLUGIN -D_WINDOWS -I$(SRCDIR)in_xsf_framework -I$(SRCDIR)in_xsf_framework/winamp -I$(SRCDIR)in_xsf_framework/zlib -pipe -Wall -Wextra -Wno-div-by-zero -Wfloat-equal -Wshadow -Winit-self -Wcast-qual -Wunreachable-code -Wno-long-long -Wno-switch
ifeq (,$(findstring clang,$(COMPILER)))
MY_CXXFLAGS+=	-Wlogical-op
endif
MY_LDFLAGS=	$(LDFLAGS)

DLL_SRCS_template=	$(1)_SRCS:=	$$(sort $$($(1)_SRCS))
DLL_OBJS_template=	$(1)_OBJS:=	$$(subst $(SRCDIR),,$$($(1)_SRCS:%.cpp=%.o))

$(foreach dll,$(DLLS),$(eval $(call DLL_SRCS_template,$(basename $(notdir $(dll))))))
$(foreach dll,$(DLLS),$(eval $(call DLL_OBJS_template,$(basename $(notdir $(dll))))))

SRCS:=	$(sort $(FRAMEWORK_SRCS) $(ZLIB_SRCS) $(foreach dll,$(DLLS),$($(basename $(notdir $(dll)))_SRCS)))
OBJS:=	$(sort $(FRAMEWORK_OBJS) $(ZLIB_OBJS) $(foreach dll,$(DLLS),$($(basename $(notdir $(dll)))_OBJS)))
DEPS:=	$(OBJS:%.o=%.d)

.PHONY: all debug clean

.SUFFIXES:
.SUFFIXES: .cpp .o .d .a .dll

all: $(DLLS)
debug: MY_CXXFLAGS+=	-g -D_DEBUG
debug: all

define DLL_template
$(1): $$(FRAMEWORK_OBJS) $$(ZLIB_OBJS) $$($$(basename $$(notdir $(1)))_OBJS)
	@echo "Linking $$@..."
	$$(CXX) -shared -fPIC $$(MY_CXXFLAGS) -o $$@ $$^ $$(MY_LDFLAGS)
endef
define SRC_template
ifeq ($(findstring .cpp,$(1)),.cpp)
$$(subst $(SRCDIR),,$(1:%.cpp=%.o)): $(1)
	@echo "Compiling $$<..."
	@$$(CXX) $$(MY_CXXFLAGS) -o $$(subst $(SRCDIR),,$$@) -c $$<
else
$$(subst $(SRCDIR),,$(1:%.c=%.o)): $(1)
	@echo "Compiling $$<..."
	@$$(CC) $$(MY_CFLAGS) -o $$(subst $(SRCDIR),,$$@) -c $$<
endif
endef
define DEP_template
ifeq ($(findstring .cpp,$(1)),.cpp)
$$(subst $(SRCDIR),,$(1:%.cpp=%.d)): $(1)
else
$$(subst $(SRCDIR),,$(1:%.c=%.d)): $(1)
endif
	@echo "Calculating depends for $$<..."
	-@mkdir -p $$(@D)
	@$$(CXX) $$(MY_CPPFLAGS) -MM -MF $$(subst $(SRCDIR),,$$@).tmp $$<
	@sed 's,$$(notdir $$*)\.o[ :]*,$$(subst /,\/,$$*).o $$(subst /,\/,$$@): ,g' < $$(subst $(SRCDIR),,$$@).tmp > $$(subst $(SRCDIR),,$$@)
	@rm $$(subst $(SRCDIR),,$$@).tmp
endef

$(foreach dll,$(DLLS),$(eval $(call DLL_template,$(dll))))
$(foreach src,$(SRCS),$(eval $(call SRC_template,$(src))))
$(foreach src,$(SRCS),$(eval $(call DEP_template,$(src))))

$(subst $(SRCDIR),,$(in_2sf_SRCS:%.cpp=%.d)): MY_CPPFLAGS+=	-msse
$(in_2sf_OBJS): MY_CXXFLAGS+=	-msse -DHAVE_LIBZ -I$(SRCDIR)/in_2sf/desmume

in_xsf_framework/zlib/%.o: MY_CFLAGS+= -Wno-implicit-fallthrough -Wno-cast-qual
in_2sf/desmume/%.o: MY_CXXFLAGS+=	-Wno-shadow -Wno-unused-parameter -Wno-unused-value -Wno-unused-variable -Wno-missing-field-initializers -Wno-unused-function -Wno-implicit-fallthrough -Wno-old-style-cast -Wno-deprecated-copy -Wno-class-memaccess -Wno-sign-compare -Wno-float-equal -Wno-unused-but-set-variable
ifneq (,$(findstring clang,$(COMPILER)))
in_2sf/desmume/%.o: MY_CXXFLAGS+=	-Wno-missing-braces
endif
in_snsf/snes9x/%.o: MY_CXXFLAGS+= -Wno-implicit-fallthrough -Wno-shift-negative-value
in_gsf/vbam/apu/Gb_Apu: MY_CXXFLAGS+=	-Wno-implicit-fallthrough
in_gsf/vbam/gba/GBA-arm.o: MY_CXXFLAGS+=	-Wno-unused-variable
ifeq (,$(findstring clang,$(COMPILER)))
in_gsf/vbam/gba/GBA-arm.o: MY_CXXFLAGS+=	-Wno-unused-but-set-variable
endif

clean:
	@echo "Cleaning OBJs and DLLs..."
	-@rm -f $(OBJS) $(DLLS) $(DEPS)

-include $(DEPS)
