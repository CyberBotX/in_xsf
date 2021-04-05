set(HEADERS
	vbam/apu/blargg_common.h
	vbam/apu/blargg_config.h
	vbam/apu/Blip_Buffer.h
	vbam/apu/Gb_Apu.h
	vbam/apu/Gb_Oscs.h
	vbam/apu/Multi_Buffer.h
	vbam/common/Port.h
	vbam/common/SoundDriver.h
	vbam/gba/bios.h
	vbam/gba/GBA.h
	vbam/gba/GBAcpu.h
	vbam/gba/GBAinline.h
	vbam/gba/Globals.h
	vbam/gba/Sound.h
	XSFConfig_GSF.h
	XSFConfigDialog_GSF.h)
set(VBAM_SOURCES
	vbam/apu/Blip_Buffer.cpp
	vbam/apu/Gb_Apu.cpp
	vbam/apu/Gb_Oscs.cpp
	vbam/apu/Multi_Buffer.cpp
	vbam/gba/bios.cpp
	vbam/gba/GBA.cpp
	vbam/gba/GBA-arm.cpp
	vbam/gba/GBA-thumb.cpp
	vbam/gba/Globals.cpp
	vbam/gba/Sound.cpp)
set(SOURCES
	${VBAM_SOURCES}
	XSFConfig_GSF.cpp
	XSFConfigDialog_GSF.cpp
	XSFPlayer_GSF.cpp)

set(VBAM_COMPILE_OPTIONS
	$<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:-Wno-unused-variable -Wno-implicit-fallthrough>
	$<$<CXX_COMPILER_ID:GNU>:-Wno-unused-but-set-variable>)

set_source_files_properties(${VBAM_SOURCES} PROPERTIES
	COMPILE_OPTIONS "${VBAM_COMPILE_OPTIONS}")

add_library(in_gsf SHARED ${HEADERS} ${SOURCES})
set_target_properties(in_gsf PROPERTIES PREFIX "")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Header Files" FILES ${HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Source Files" FILES ${SOURCES})
target_compile_options(in_gsf PUBLIC
	$<$<CXX_COMPILER_ID:MSVC>:/wd4127 /wd4189 /wd26495 /wd26812>)
target_link_libraries(in_gsf
	in_xsf_framework)
