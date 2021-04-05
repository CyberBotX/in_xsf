set(HEADERS
	desmume/arm_jit.h
	desmume/armcpu.h
	desmume/bios.h
	desmume/cp15.h
	desmume/emufile.h
	desmume/FIFO.h
	desmume/firmware.h
	desmume/instruction_attributes.h
	desmume/instruction_tabdef.inc
	desmume/instructions.h
	desmume/matrix.h
	desmume/mc.h
	desmume/mem.h
	desmume/metaspu/metaspu.h
	desmume/MMU.h
	desmume/MMU_timing.h
	desmume/NDSSystem.h
	desmume/PACKED.h
	desmume/PACKED_END.h
	desmume/readwrite.h
	desmume/registers.h
	desmume/slot1.h
	desmume/SPU.h
	desmume/thumb_tabdef.inc
	desmume/types.h
	desmume/utils/AsmJit/apibegin.h
	desmume/utils/AsmJit/apiend.h
	desmume/utils/AsmJit/AsmJit.h
	desmume/utils/AsmJit/base.h
	desmume/utils/AsmJit/base/assembler.h
	desmume/utils/AsmJit/base/codegen.h
	desmume/utils/AsmJit/base/compiler.h
	desmume/utils/AsmJit/base/constpool.h
	desmume/utils/AsmJit/base/containers.h
	desmume/utils/AsmJit/base/context_p.h
	desmume/utils/AsmJit/base/cpuinfo.h
	desmume/utils/AsmJit/base/cputicks.h
	desmume/utils/AsmJit/base/error.h
	desmume/utils/AsmJit/base/globals.h
	desmume/utils/AsmJit/base/intutil.h
	desmume/utils/AsmJit/base/lock.h
	desmume/utils/AsmJit/base/logger.h
	desmume/utils/AsmJit/base/operand.h
	desmume/utils/AsmJit/base/runtime.h
	desmume/utils/AsmJit/base/string.h
	desmume/utils/AsmJit/base/vectypes.h
	desmume/utils/AsmJit/base/vmem.h
	desmume/utils/AsmJit/base/zone.h
	desmume/utils/AsmJit/build.h
	desmume/utils/AsmJit/Config.h
	desmume/utils/AsmJit/host.h
	desmume/utils/AsmJit/x86.h
	desmume/utils/AsmJit/x86/x86assembler.h
	desmume/utils/AsmJit/x86/x86compiler.h
	desmume/utils/AsmJit/x86/x86context_p.h
	desmume/utils/AsmJit/x86/x86cpuinfo.h
	desmume/utils/AsmJit/x86/x86inst.h
	desmume/utils/AsmJit/x86/x86operand.h
	desmume/utils/AsmJit/x86/x86scheduler_p.h
	desmume/utils/bits.h
	desmume/version.h
	spu/adpcmdecoder.h
	spu/interpolator.h
	spu/samplecache.h
	spu/sampledata.h
	XSFConfig_2SF.h
	XSFConfigDialog_2SF.h)
set(DESMUME_SOURCES
	desmume/addons/slot1_retail.cpp
	desmume/arm_instructions.cpp
	desmume/arm_jit.cpp
	desmume/armcpu.cpp
	desmume/bios.cpp
	desmume/cp15.cpp
	desmume/emufile.cpp
	desmume/FIFO.cpp
	desmume/firmware.cpp
	desmume/mc.cpp
	desmume/metaspu/metaspu.cpp
	desmume/MMU.cpp
	desmume/NDSSystem.cpp
	desmume/readwrite.cpp
	desmume/slot1.cpp
	desmume/SPU.cpp
	desmume/thumb_instructions.cpp
	desmume/utils/AsmJit/base/assembler.cpp
	desmume/utils/AsmJit/base/codegen.cpp
	desmume/utils/AsmJit/base/compiler.cpp
	desmume/utils/AsmJit/base/constpool.cpp
	desmume/utils/AsmJit/base/containers.cpp
	desmume/utils/AsmJit/base/context.cpp
	desmume/utils/AsmJit/base/cpuinfo.cpp
	desmume/utils/AsmJit/base/cputicks.cpp
	desmume/utils/AsmJit/base/error.cpp
	desmume/utils/AsmJit/base/globals.cpp
	desmume/utils/AsmJit/base/logger.cpp
	desmume/utils/AsmJit/base/operand.cpp
	desmume/utils/AsmJit/base/runtime.cpp
	desmume/utils/AsmJit/base/string.cpp
	desmume/utils/AsmJit/base/vmem.cpp
	desmume/utils/AsmJit/base/zone.cpp
	desmume/utils/AsmJit/x86/x86assembler.cpp
	desmume/utils/AsmJit/x86/x86compiler.cpp
	desmume/utils/AsmJit/x86/x86context.cpp
	desmume/utils/AsmJit/x86/x86cpuinfo.cpp
	desmume/utils/AsmJit/x86/x86inst.cpp
	desmume/utils/AsmJit/x86/x86operand.cpp
	desmume/utils/AsmJit/x86/x86operand_regs.cpp
	desmume/utils/AsmJit/x86/x86scheduler.cpp
	desmume/version.cpp
	spu/adpcmdecoder.cpp
	spu/interpolator.cpp
	spu/samplecache.cpp
	spu/sampledata.cpp)
set(SOURCES
	${DESMUME_SOURCES}
	XSFConfig_2SF.cpp
	XSFConfigDialog_2SF.cpp
	XSFPlayer_2SF.cpp)

set(DESMUME_COMPILE_OPTIONS
	$<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:-Wno-shadow -Wno-unused-parameter -Wno-unused-value -Wno-unused-variable -Wno-missing-field-initializers -Wno-unused-function -Wno-implicit-fallthrough -Wno-old-style-cast -Wno-deprecated-copy
		-Wno-sign-compare -Wno-float-equal -Wno-missing-braces>
	$<$<CXX_COMPILER_ID:GNU>:-Wno-class-memaccess -Wno-unused-but-set-variable>
	$<$<CXX_COMPILER_ID:Clang>:-Wno-array-bounds>)

set_source_files_properties(${DESMUME_SOURCES} PROPERTIES
	COMPILE_OPTIONS "${DESMUME_COMPILE_OPTIONS}")

add_library(in_2sf SHARED ${HEADERS} ${SOURCES})
set_target_properties(in_2sf PROPERTIES PREFIX "")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Header Files" FILES ${HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Source Files" FILES ${SOURCES})
target_compile_options(in_2sf PUBLIC
	$<$<CXX_COMPILER_ID:MSVC>:/wd4018 /wd4100 /wd4146 /wd4127 /wd4189 /wd4201 /wd4245 /wd4456 /wd4459 /wd4505 /wd4701 /wd4703 /wd4706 /wd4996 /wd6001 /wd6011 /wd6297 /wd6308 /wd6385 /wd6386 /wd26454 /wd26495 /wd26812 /wd26819 /wd28112>)
target_link_libraries(in_2sf
	in_xsf_framework)
