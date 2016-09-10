
compilation
gcc main.c -lSDL -lSDL_image -o snesconvert


example :
snesconvert myimage.png

option :
-2bpp forced 4colors
-4bpp forced 16colors
-8bpp forced 256colors
-noalpha (if your image has no alpha)
-palette (only palette)
-paletteall (only palette but 1 pixel = 1 colors so square 16x16 pixel)
-mode7 (for mode7 8bpp)
-loadpalette (read palette extern,by default read palette.png or add second argument , your image should be 4/16/256x1 pixel)


