../in_xsf_framework.lib: $(patsubst %.cpp, %.obj, $(wildcard *.cpp)) $(patsubst %.c, %.obj, $(wildcard zlib/*.c))
	$(WINE) lib.exe /nologo /out:$@ $^

%.obj: %.c
	$(WINE) cl.exe /nologo /std:c11 /MT /DUNICODE /D_UNICODE /DNDEBUG /DWINAMP_PLUGIN /DUNICODE_INPUT_PLUGIN /O2 /EHsc /c /Fo$@ $<

%.obj: %.cpp $(wildcard *.h) GNUmakefile
	$(WINE) cl.exe /nologo /std:c++latest /MT /DUNICODE /D_UNICODE /DNDEBUG /DWINAMP_PLUGIN /DUNICODE_INPUT_PLUGIN /I zlib /O2 /EHsc /c /Fo$@ $<

clean:
	rm -f ../in_xsf_framework.lib *.obj zlib/*.obj winamp/*.obj

.PHONY: clean
