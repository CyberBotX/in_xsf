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
	vbam/gba/Sound.h)
set(SOURCES
	vbam/apu/Blip_Buffer.cpp
	vbam/apu/Gb_Apu.cpp
	vbam/apu/Gb_Oscs.cpp
	vbam/apu/Multi_Buffer.cpp
	vbam/gba/bios.cpp
	vbam/gba/GBA.cpp
	vbam/gba/GBA-arm.cpp
	vbam/gba/GBA-thumb.cpp
	vbam/gba/Globals.cpp
	vbam/gba/Sound.cpp
	XSFConfig_GSF.cpp
	XSFPlayer_GSF.cpp)

add_library(in_gsf SHARED ${HEADERS} ${SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Header Files" FILES ${HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Source Files" FILES ${SOURCES})
target_link_libraries(in_gsf
	in_xsf_framework)
