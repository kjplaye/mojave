all: _mojave.so

_mojave.so: _mojave.c
	gcc -O3 _mojave.c -o _mojave.so -fPIC -shared -I/usr/include/SDL2 -lSDL2 -lm -lSDL2_ttf

clean:
	rm -f _mojave.so
