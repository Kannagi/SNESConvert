
compilation
gcc main.c -lSDL -lSDL_image -o snesconvert


example :
snesconvert myimage.png

option :
-4 forced 2bpp(4colors)
-16 forced 4bpp(16colors)
-noalpha (if your image has no alpha)
-palette (only palette)
-paletteall (only palette but 1 pixel = 1 colors so square 16x16 pixel)
-mode7 (for mode7 data)
-loadpalette (read for palette : palette.png so 1x16 pixel)


