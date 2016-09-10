#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>


#ifdef __MINGW32__
#undef main
#endif

void pce_convert(SDL_Surface *image,char *adresse,int noalpha,int mode,char *adressepal,int bin,int type);

int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Surface *image,*copy;
    int n = 1,noalpha = 0,i,mode = 0,ok = 0,bin = 0,type = 0;
    char adresse[500],adressepal[500];

    strcpy(adressepal,"palette.png");
    adresse[0] = 0;

    for(i = 1; i < argc;i++)
    {
        if(argv[i][0] == '-')
        {
            if(strcmp(argv[i],"-noalpha") == 0) noalpha = 1;
            if(strcmp(argv[i],"-palette") == 0) mode = 1;
            if(strcmp(argv[i],"-paletteall") == 0) mode = 3;
            if(strcmp(argv[i],"-bin") == 0) bin = 1;
            if(strcmp(argv[i],"-bg") == 0) type = 1;
            if(strcmp(argv[i],"-spr") == 0) type = 0;

            ok = 0;

            if(strcmp(argv[i],"-loadpalette") == 0)
            {
                mode = 4;
                ok = 1;
            }


        }else
        {
            if(ok == 0) strcpy(adresse,argv[i]);
            if(ok == 1) strcpy(adressepal,argv[i]);
            ok = 0;
        }
    }

    if(adresse[0] == 0)
    {
        printf("Enter a picture format .png,pcx,bmp\n");
        printf("Exemple :\npceconvert -4 myimage \n");
        return;
    }

    image = IMG_Load(adresse);
    if(image == NULL)
    {
        printf("Image is not valide\n");
        return;
    }

    copy = SDL_CreateRGBSurface(0,image->w,image->h,24,0,0,0,0);
    SDL_BlitSurface(image,NULL,copy,NULL);

    pce_convert(copy,adresse,noalpha,mode,adressepal,bin,type);

    SDL_FreeSurface(copy);
    SDL_FreeSurface(image);
    SDL_Quit();

    return 0;

}

int load_palette(SDL_Surface *image,unsigned char *palette)
{
    int i,l;
    unsigned char *pixel = image->pixels;
    int taille = image->w*image->h*image->format->BytesPerPixel;
    int n = 0,ok,first = 0;

    for(i = 0;i < 768;i+=3)
    {
    	palette[i+0] = 1;
    	palette[i+1] = 2;
    	palette[i+2] = 0;
    }


    for(i = 0;i < taille;i += image->format->BytesPerPixel)
    {
        ok = 0;
        for(l = 0;l < 768;l+=3)
        {
            if(palette[l+0] == pixel[i+0] && palette[l+1] == pixel[i+1] && palette[l+2] == pixel[i+2])
            {
                ok = 0;
                break;
            }else
            {
                ok = 1;
            }
        }

        if(ok == 1)
        {
            palette[n+0] = pixel[i+0];
            palette[n+1] = pixel[i+1];
            palette[n+2] = pixel[i+2];
            n +=3;
            if(n > 768) break;
        }
    }

    return n;
}

int load_paletteext(unsigned char *palette,char *adressepal)
{
    int i;
    SDL_Surface *image,*copy;
    image = IMG_Load(adressepal);

    if(image == NULL)
    {
        printf("Image is not valide\n");
        return;
    }
    for(i = 0;i < 768;i+=3)
    {
    	palette[i+0] = 1;
    	palette[i+1] = 2;
    	palette[i+2] = 0;
    }

    copy = SDL_CreateRGBSurface(0,image->w,image->h,24,0,0,0,0);
    SDL_BlitSurface(image,NULL,copy,NULL);
    int n = 0;
    unsigned char *pixel = copy->pixels;

    for(i = 0;i < image->w*copy->format->BytesPerPixel;i += copy->format->BytesPerPixel)
    {
        palette[i+0] = pixel[i+0];
        palette[i+1] = pixel[i+1];
        palette[i+2] = pixel[i+2];
        n+=3;
    }
	SDL_FreeSurface(copy);
	SDL_FreeSurface(image);


	return n;

}

void tri_palette(SDL_Surface *image,int casex,int casey,unsigned char *pixel,unsigned char *palette,int *tiles)
{
    int x,y,i,l;
    int n = 0;
    int r,v,b;


    for(y = casey;y < casey+8;y++)
    {
        for(x = casex;x < casex+8;x++)
        {
            i = (y*image->w*image->format->BytesPerPixel) + x*image->format->BytesPerPixel;
            r = pixel[i+0];
            v = pixel[i+1];
            b = pixel[i+2];

            for(l = 0;l < 768;l+=3)
            {
                if(palette[l+0] == r && palette[l+1] == v && palette[l+2] == b)
                    break;
            }

            tiles[n] = l/3;

            n++;
        }

        //printf("\n-------------------------------------------\n");
    }
}

int write_rom(FILE *file,SDL_Surface *image,unsigned char *pixel,unsigned char *palette,int noalpha,int binaire,int type)
{
    int casex,casey;
    int tiles[64];
    int i,l;
    int x,y,size = 0;
    char chaine[500];
    int bin[128];
	int bx,by,bn;

	int nl = 4,ns = 128,casez = 16;

	if(type == 1)
	{
		nl = 1;
		ns = 32;
		casez = 8;
	}

    casex = 0;
    casey = 0;


    sprintf(chaine,"\n");
    if(binaire == 0) fputs(chaine,file);



    while(1)
    {



        for(i = 0;i < 128;i++)
			bin[i] = 0;


		for(l = 0;l < nl;l++)
		{

			if(l == 1)
			{
				casex+= 8;
			}

			if(l == 2)
			{
				casey += 8;
			}

			if(l == 3)
			{
				casex -= 8;
			}

			tri_palette(image,casex,casey,pixel,palette,tiles);

			for(y = 0;y < 8;y++)
			{
				for(x = 0;x < 8;x++)
				{
					i = tiles[x + (y*8)] + noalpha;
					if(i > 15) i = 15;

					bx = x/8;
					by = y*2;
					bn = bx+by;


					if(type == 0)
					{

						if(l == 0) bn += 0x01;
						if(l == 2) bn += 0x10;
						if(l == 3) bn += 0x11;

						bin[bn+0x00] += ( (i&0x01)>>0 ) << (7 - x);
						bin[bn+0x20] += ( (i&0x02)>>1 ) << (7 - x);
						bin[bn+0x40] += ( (i&0x04)>>2 ) << (7 - x);
						bin[bn+0x60] += ( (i&0x08)>>3 ) << (7 - x);
					}else
					{
						bin[bn+0x00] += ( (i&0x01)>>0 ) << (7 - x);
						bin[bn+0x01] += ( (i&0x02)>>1 ) << (7 - x);
						bin[bn+0x10] += ( (i&0x04)>>2 ) << (7 - x);
						bin[bn+0x11] += ( (i&0x08)>>3 ) << (7 - x);
					}

					//if(i > npal-1) i = npal-1;

				}

			}

			if(l == 3)
			{
				casey -= 8;
			}

		}

		for(i = 0;i < ns;i++)
		{
			if(binaire == 0)
			{
				if(i%16 == 0) fputs("    .db ",file);

				if(i%16 != 15) sprintf(chaine,"$%.2x,",bin[i]);
				else sprintf(chaine,"$%.2x",bin[i]);

				fputs(chaine,file);

				if(i%16 == 15) fputs("\n",file);
			}else
			{
				fputc(bin[i],file);
			}

		}




		size += ns;

        casex += casez;
        if(casex+casez >image->w)
        {
            casex = 0;
            casey += casez;
        }

        if(casey+casez >image->h) break;

    }

    //ecriture palette
    if(binaire == 0) fputs("\n\n",file);
    else fclose(file);

    return size;
}

void output_filename(char *adresse,char *schaine)
{
    int l = 0;
    int i = 0;
    while(adresse[i] != 0 && adresse[i] != '.' )
    {
        schaine[l] = adresse[i];
        l++;

        if(adresse[i] == '/' || adresse[i] == '\\') l = 0;
        i++;
    }
    schaine[l] = 0;
}

int write_pal(FILE *file,SDL_Surface *image,char *schaine,unsigned char *palette,unsigned char *pixel,int color,int mode,int taille,int bin)
{
    int i,n;
    int psize = 0;
    char chaine[100];
    int couleur;
    int octet4[4];

	if(bin == 0)
	{
		sprintf(chaine,"pallette_%s:\n",schaine);
		fputs(chaine,file);
		sprintf(chaine,"    .dw  ");
		fputs(chaine,file);
	}


    if(mode == 3)
    {
        n = 0;
        for(i = 0;i < taille;i += image->format->BytesPerPixel)
        {
            palette[n+0] = pixel[i+0];
            palette[n+1] = pixel[i+1];
            palette[n+2] = pixel[i+2];
            n +=3;
            if(n > 768) break;
        }
        color = n/3;
    }


    for(i = 0;i < color;i++)
    {
        n = i*3;

        octet4[0] = palette[n+0]>>5;
        octet4[1] = palette[n+1]>>5;
		octet4[2] = palette[n+2]>>5;

		couleur = (octet4[1]<<6) + (octet4[2]<<3) +octet4[0];






        if(bin == 0)
		{
			sprintf(chaine,"$%.4x",couleur);
			fputs(chaine,file);

			if(i != color-1) fputs(",",file);
		}else
		{
			fputc(couleur&0xFF,file);
			fputc( (couleur&0xFF)>>8 ,file);
		}



        //printf("%s %d %d %d , %.2x %.2x %.2x\n",chaine ,palette[n+0],palette[n+1],palette[n+2],octet4[0],octet4[1],octet4[2]);

        psize += 2;
    }


    if(bin == 1) fclose(file);

    return psize;
}

void write_end(FILE *file,int psize,int size)
{
    char chaine[200];

    fputs("\n",file);
    fputs("\n",file);
    fputs("\n",file);

    sprintf(chaine,";palette size octet : %d ,hexa $%.4x",psize,psize);
    fputs(chaine,file);
    fputs("\n",file);
    sprintf(chaine,";size octet : %d ,hexa $%.4x",size,size);
    fputs(chaine,file);

    fputs("\n",file);


    fclose(file);
}


void pce_convert(SDL_Surface *image,char *adresse,int noalpha,int mode,char *adressepal,int bin,int type)
{
    FILE *file;
    int i,l,taille,color = 0;
    int x,y,size = 0,psize = 0;
    char chaine[200],schaine[200];

    unsigned char palette[768];
    for(i = 0;i < 768;i++)
        palette[i] = 0;


    unsigned char *pixel = image->pixels;

    taille = image->w*image->h*image->format->BytesPerPixel;

    color = load_palette(image,palette)/3;

    //-------------------------------
    if(mode == 4)
    {
        color = load_paletteext(palette,adressepal)/3;
        mode = 0;
    }

    //-------------------------------

    printf("color : %d\n",color);

    //-------------------------------
    output_filename(adresse,schaine);

    if(bin == 0)
    {
    	sprintf(chaine,"%s.asm",schaine);
    	file = fopen(chaine,"w");
    }else
    {
    	sprintf(chaine,"%s.spr",schaine);
    	file = fopen(chaine,"wb");
    }

    if(mode == 0 || mode == 4)
        size = write_rom(file,image,pixel,palette,noalpha,bin,type);


	if(bin == 1)
    {
    	sprintf(chaine,"%s.pal",schaine);
    	file = fopen(chaine,"wb");
    }

    psize = write_pal(file,image,schaine,palette,pixel,color,mode,taille,bin);

    if(bin == 0) write_end(file,psize,size);
}

