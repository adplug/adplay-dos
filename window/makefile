all: txtgfx.obj window.obj .symbolic

txtgfx.obj: txtgfx.c txtgfx.h
	wcc386 txtgfx.c

window.obj: window.cpp window.h
	wpp386 window.cpp

clean: .symbolic
	del *.obj
