all: tag.obj

EXTRA_CFLAGS=-std=gnu99 -O0
EXTRA_CXXFLAGS=-std=c++11 -O0
ifdef ASLR
EXTRA_CFLAGS+= -fPIC -fPIE
EXTRA_CXXFLAGS+= -fPIC -fPIE
endif
ifdef DEBUG
EXTRA_CFLAGS+= -g
EXTRA_CXXFLAGS+= -g
endif

tag: tag.c Makefile
	gcc -o tag tag.c $(EXTRA_CFLAGS)
tag.obj: tag
	objdump -d tag > tag.obj
clean: 
	rm -f tag *.o core *.obj
clean_pipes:
	rm -f /tmp/parent_to_child.pipe /tmp/child_to_parent.pipe
