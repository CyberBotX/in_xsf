set(HEADERS
	SSEQPlayer/Channel.h
	SSEQPlayer/common.h
	SSEQPlayer/consts.h
	SSEQPlayer/FATSection.h
	SSEQPlayer/INFOEntry.h
	SSEQPlayer/INFOSection.h
	SSEQPlayer/NDSStdHeader.h
	SSEQPlayer/Player.h
	SSEQPlayer/SBNK.h
	SSEQPlayer/SDAT.h
	SSEQPlayer/SSEQ.h
	SSEQPlayer/SWAR.h
	SSEQPlayer/SWAV.h
	SSEQPlayer/SYMBSection.h
	SSEQPlayer/Track.h
	resource.h
	XSFConfig_NCSF.h
	XSFConfigDialog_NCSF.h
	XSFPlayer_NCSF.h)
set(RESOURCES
	in_ncsf.rc)
set(SOURCES
	SSEQPlayer/Channel.cpp
	SSEQPlayer/FATSection.cpp
	SSEQPlayer/INFOEntry.cpp
	SSEQPlayer/INFOSection.cpp
	SSEQPlayer/NDSStdHeader.cpp
	SSEQPlayer/Player.cpp
	SSEQPlayer/SBNK.cpp
	SSEQPlayer/SDAT.cpp
	SSEQPlayer/SSEQ.cpp
	SSEQPlayer/SWAR.cpp
	SSEQPlayer/SWAV.cpp
	SSEQPlayer/SYMBSection.cpp
	SSEQPlayer/Track.cpp
	XSFConfig_NCSF.cpp
	XSFConfigDialog_NCSF.cpp
	XSFPlayer_NCSF.cpp)

add_library(in_ncsf SHARED ${HEADERS} ${RESOURCES} ${SOURCES})
set_target_properties(in_ncsf PROPERTIES PREFIX "")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Header Files" FILES ${HEADERS})
source_group("Resource Files" FILES ${RESOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Source Files" FILES ${SOURCES})
target_compile_options(in_ncsf PUBLIC
	$<$<CXX_COMPILER_ID:MSVC>:/wd6011 /wd6385 /wd26819>)
target_link_libraries(in_ncsf
	in_xsf_framework)
