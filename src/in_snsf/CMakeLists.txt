set(HEADERS
	snes9x/65c816.h
	snes9x/apu/apu.h
	snes9x/apu/blargg_common.h
	snes9x/apu/blargg_config.h
	snes9x/apu/blargg_endian.h
	snes9x/apu/bspline_resampler.h
	snes9x/apu/hermite_resampler.h
	snes9x/apu/linear_resampler.h
	snes9x/apu/osculating_resampler.h
	snes9x/apu/resampler.h
	snes9x/apu/ring_buffer.h
	snes9x/apu/sinc_resampler.h
	snes9x/apu/SNES_SPC.h
	snes9x/apu/SPC_CPU.h
	snes9x/apu/SPC_DSP.h
	snes9x/cpuaddr.h
	snes9x/cpuexec.h
	snes9x/cpumacro.h
	snes9x/dma.h
	snes9x/getset.h
	snes9x/memmap.h
	snes9x/port.h
	snes9x/ppu.h
	snes9x/sdd1.h
	snes9x/snes9x.h
	XSFConfig_SNSF.h
	XSFConfigDialog_SNSF.h)
set(SOURCES
	snes9x/apu/apu.cpp
	snes9x/apu/SNES_SPC.cpp
	snes9x/apu/SNES_SPC_misc.cpp
	snes9x/apu/SPC_DSP.cpp
	snes9x/cpu.cpp
	snes9x/cpuexec.cpp
	snes9x/cpuops.cpp
	snes9x/dma.cpp
	snes9x/globals.cpp
	snes9x/memmap.cpp
	snes9x/ppu.cpp
	snes9x/sdd1.cpp
	XSFConfig_SNSF.cpp
	XSFConfigDialog_SNSF.cpp
	XSFPlayer_SNSF.cpp)

add_library(in_snsf SHARED ${HEADERS} ${SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Header Files" FILES ${HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Source Files" FILES ${SOURCES})
target_compile_options(in_snsf PUBLIC
	$<$<CXX_COMPILER_ID:MSVC>:/wd4127 /wd4245 /wd6385 /wd6386 /wd26453 /wd26495 /wd26812 /wd26819>)
target_link_libraries(in_snsf
	in_xsf_framework)
