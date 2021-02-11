../in_2sf.dll: $(patsubst %.cpp, %.obj, $(wildcard desmume/*.cpp desmume/*/*.cpp spu/*.cpp *.cpp)) ../in_xsf_framework.lib
	$(WINE) link.exe /nologo /dll /machine:x86 user32.lib libucrt.lib libvcruntime.lib libcmt.lib libcpmt.lib /out:$@ $^

%.obj: %.cpp $(wildcard desmume/*.h ../in_xsf_framework/*.h) GNUmakefile
	$(WINE) cl.exe /nologo /std:c++latest /MT /DUNICODE /D_UNICODE /DNDEBUG /O2 /EHsc /I ../in_xsf_framework /I ../in_xsf_framework/zlib /I spu /I desmume /c /Fo$@ $<

clean:
	rm -f ../in_2sf.dll ../in_2sf.exp ../in_2sf.lib *.obj desmume/*.obj desmume/*/*.obj
