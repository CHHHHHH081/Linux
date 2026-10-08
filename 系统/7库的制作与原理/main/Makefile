libfile.a:file1.o file2.o
	ar -rc $@ $^
file1.o:file1.c
	gcc -c $<
file2.o:file2.c
	gcc -c $<

.PHONY:output
output:
	mkdir -p lib/include
	mkdir -p lib/mylib
	cp -f *.h ./lib/include
	cp -f *.a ./lib/mylib
	tar czf lib.tgz lib

.PHONY:clean
clean:
	rm -rf lib *.o lib.tgz libfile.a
