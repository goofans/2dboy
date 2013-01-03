all: tinyxml.a

CXX=g++

CFLAGS:=-DTIXML_USE_STL -m32 -march=i686 -O2 -g $(CFLAGS)
CXXFLAGS=$(CFLAGS)

%.o: ../%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f *.o

distclean: clean
	rm -f tinyxml.a

tinyxml.a: tinyxml.o tinyxmlerror.o tinyxmlparser.o
	ar crs $@ $^

.PHONY: all clean distclean

