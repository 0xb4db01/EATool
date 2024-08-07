CC=x86_64-w64-mingw32-g++

all:
	$(CC) $(CINC) eatool.cpp -o eatool.exe

clean:
	rm -rf *.exe
