../in_ncsf.dll: $(patsubst %.cpp, %.obj, $(wildcard SSEQPlayer/*.cpp *.cpp)) ../in_xsf_framework.lib
	$(WINE) link.exe /nologo /dll /machine:x86 user32.lib libucrt.lib libvcruntime.lib libcmt.lib libcpmt.lib /out:$@ $^

%.obj: %.cpp $(wildcard SSEQPlayer/*.h ../in_xsf_framework/*.h) GNUmakefile
	$(WINE) cl.exe /nologo /std:c++latest /MT /DUNICODE /D_UNICODE /DNDEBUG /O2 /EHsc /I ../in_xsf_framework /I ../in_xsf_framework/zlib /I SSEQPlayer /c /Fo$@ $<

clean:
	rm -f ../in_ncsf.dll ../in_ncsf.exp ../in_ncsf.lib *.obj SSEQPlayer/*.obj
