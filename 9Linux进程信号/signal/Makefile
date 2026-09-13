.PHONY:all
all:mykill testsig
mykill:mykill.cc
	g++ -o $@ $^
testsig:testsig.cc
	g++ -o $@ $^
.PHONY:clean
clean:
	rm -f mykill testsig