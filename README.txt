
compilation
gcc main.c -lSDL -lSDL_image -o pceconvert


example :
pceconvert myimage.png

option :
-noalpha (if your image has no alpha)
-palette (only palette)
-paletteall (only palette but 1 pixel = 1 colors so square 16x16 pixel)
-bin (give .spr and .pal)
-loadpalette (read palette extern,by default read palette.png or add second argument , your image should be 16x1 pixel)
-bg
-spr


