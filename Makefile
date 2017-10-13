CXX=		gcc
CXXFLAGS=	-g
PROGRAMS=	myftpd

all:		myftpd

myftpd:		myftpd.c
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f $(PROGRAMS)
