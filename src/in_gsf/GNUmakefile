../in_gsf.dll: $(patsubst %.cpp, %.obj, $(wildcard vbam/*/*.cpp *.cpp)) ../in_xsf_framework.lib
	$(WINE) link.exe /nologo /dll /machine:x86 user32.lib libucrt.lib libvcruntime.lib libcmt.lib libcpmt.lib /out:$@ $^

%.obj: %.cpp $(wildcard vbam/*/*.h ../in_xsf_framework/*.h) GNUmakefile
	$(WINE) cl.exe /nologo /std:c++latest /MT /DUNICODE /D_UNICODE /DNDEBUG /O2 /EHsc /I ../in_xsf_framework /I ../in_xsf_framework/zlib /c /Fo$@ $<

clean:
	rm -f ../in_gsf.dll ../in_gsf.exp ../in_gsf.lib *.obj vbam/*/*.obj

.PHONY: clean
