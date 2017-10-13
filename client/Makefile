CXX=		gcc
CXXFLAGS=	-g
PROGRAMS=	myftp

all:		myftp

myftp:		myftp.c
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f $(PROGRAMS)

