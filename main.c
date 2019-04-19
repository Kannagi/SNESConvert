#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>


#ifdef __MINGW32__
#undef main
#endif


void snes_convert(SDL_Surface *image,char *address,char *addresspal,int *option,int num);

int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_VIDEO);


    SDL_Surface *image,*copy;
    int n = 1,i,ok = 0;

    int option[10];
    char address[500],addresspal[500],str[50];

    for(i = 0; i < 10;i++)
		option[i] = 0;

    strcpy(addresspal,"palette.png");
    address[0] = 0;

    for(i = 1; i < argc;i++)
    {
        if(argv[i][0] == '-')
        {
            if(strcmp(argv[i],"-2bpp") == 0) option[0] = 4;
            if(strcmp(argv[i],"-4bpp") == 0) option[0] = 16;
            if(strcmp(argv[i],"-8bpp") == 0) option[0] = 256;
            if(strcmp(argv[i],"-noalpha") == 0) option[1] = 1;
            if(strcmp(argv[i],"-palette") == 0) option[2] = 1;
            if(strcmp(argv[i],"-paletteall") == 0) option[2] = 3;
            if(strcmp(argv[i],"-mode7") == 0) option[2] = 2;
            if(strcmp(argv[i],"-asm") == 0) option[3] = 1;
			if(strcmp(argv[i],"-map") == 0) option[4] = 1;
            ok = 0;

            if(strcmp(argv[i],"-loadpalette") == 0)
            {
                option[2] = 4;
                ok = 1;
            }


        }else
        {
            if(ok == 0) strcpy(address,argv[i]);
            if(ok == 1) strcpy(addresspal,argv[i]);
            ok = 0;
        }
    }

    if(address[0] == 0)
    {
        printf("Enter a picture format .png,pcx,bmp, or jpg\n");
        printf("bpp -4 or -16 for choose palette \n");
        printf("Exemple :\nsnesconvert -4 myimage or snesconvert myimage (snesconvert choose auto -4 or -16)\n");
        return 0;
    }

    image = IMG_Load(address);
    if(image == NULL)
    {
        printf("Image is not valide\n");
        return 0;
    }

    copy = SDL_CreateRGBSurface(0,image->w,image->h,24,0,0,0,0);

    SDL_Rect rect;
    SDL_BlitSurface(image,NULL,copy,NULL);

	if(option[4] == 1)
	{
		SDL_FreeSurface(copy);
		copy = SDL_CreateRGBSurface(0,128,64,24,0,0,0,0);
		rect.w = 128;
		rect.h = 64;
		for(i = 0; i < 7;i++)
		{
			if(i < 4)
			{
				rect.x = 0;
				rect.y = i*64;
			}else
			{
				rect.x = 128;
				rect.y = (i-4)*64;

				if(i == 6)
				{
					rect.h = 128;
					SDL_FreeSurface(copy);
					copy = SDL_CreateRGBSurface(0,128,128,24,0,0,0,0);
				}
			}


			SDL_BlitSurface(image,&rect,copy,NULL);
			snes_convert(copy,address,addresspal,option,i);
			sprintf(str,"test_%d.bmp",i);
			SDL_SaveBMP(copy,str);
		}
	}else
		snes_convert(copy,address,addresspal,option,0);



    //SDL_SaveBMP(image,"test.bmp");
    SDL_FreeSurface(copy);
    SDL_FreeSurface(image);
    SDL_Quit();

    return 0;

}

int load_palette(SDL_Surface *image,unsigned char *palette,int noalpha)
{
	int i,l;
    unsigned char *pixel = image->pixels;
    int taille = image->w*image->h*image->format->BytesPerPixel;
	unsigned char r,g,b;
	int n = 3 ,black = 0,pal;

	for(i = 0;i < 0x300;i++)
		palette[i] = 0;

	for(i = 0;i < taille;i += image->format->BytesPerPixel)
	{
		r = pixel[i+0];
		g = pixel[i+1];
		b = pixel[i+2];

		pal = 1;
		for(l = 0;l < 0x300;l+=3)
		{
			if(palette[l+0] == r && palette[l+1] == g && palette[l+2] == b)
			{
				pal = 0;
				break;
			}
		}

		if(pal == 1)
		{
			if( r == 0xFF && g == 0x00 && b == 0xFF)
			{
				palette[0] = r;
				palette[1] = g;
				palette[2] = b;
			}else
			{
				palette[n+0] = r;
				palette[n+1] = g;
				palette[n+2] = b;

				n +=3;
			}

			//printf("%d %d : %x %x %x\n",(i/3)%image->w,(i/3)/image->w,r,g,b);

			if(n >= 0x300) return n;
		}else
		{
			if(black == 0 && r == 0 && g == 0 && b == 0)
			{
				palette[n+0] = r;
				palette[n+1] = g;
				palette[n+2] = b;
				//printf("%d %d : %x %x %x\n",(i/3)%image->w,(i/3)/image->w,r,g,b);

				n +=3;
				black = 1;
				if(n >= 0x300) return n;
			}
		}


	}

    return n;
}

void load_paletteext(unsigned char *palette,char *addresspal)
{
    int i;
    SDL_Surface *image,*copy;
    image = IMG_Load(addresspal);

    if(image == NULL)
    {
        printf("Image is not valide\n");
        return;
    }
    for(i = 0;i < 768;i++)
        palette[i] = 0;

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
}

void tri_palette(SDL_Surface *image,int blocx,int blocy,unsigned char *pixel,unsigned char *palette,int *tiles)
{
    int x,y,i,l;
    int n = 0;
    int r,v,b;


    for(y = blocy;y < blocy+8;y++)
    {
        for(x = blocx;x < blocx+8;x++)
        {
            i = (y*image->w*image->format->BytesPerPixel) + x*image->format->BytesPerPixel;
            r = pixel[i+0];
            v = pixel[i+1];
            b = pixel[i+2];

            for(l = 0;l < 0x300;l+=3)
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

int write_rom(FILE *file,SDL_Surface *image,unsigned char *pixel,unsigned char *palette,int npal,int type,int bin)
{
    int blocx,blocy;
    int tiles[64];
    int snespixel[8];
    int i,l;
    int x,y,size = 0;
    char str[500];
    char sasm1[500];
    char sasm2[500];
    char sasm3[500];
    char sasm4[500];
    short binpixel[32];
    char *cbinpixel = (char*)binpixel;

    blocx = 0;
    blocy = 0;

    if(bin == 0) fputs("\n",file);

    while(1)
    {
        tri_palette(image,blocx,blocy,pixel,palette,tiles);

        if(type == 0) //2,4,8 bpp
        {
        	if(bin == 0)
			{
				sprintf(sasm1,"    .db ");
				sprintf(sasm2,"    .db ");
				sprintf(sasm3,"    .db ");
				sprintf(sasm4,"    .db ");
			}

            for(y = 0;y <8;y++)
            {
            	for(i = 0;i < 8;i++)
					snespixel[i] = 0;

                for(x = 0;x < 8;x++)
                {
                    i = tiles[x + (y*8)];

                    if(i > npal-1) i = 1+i%(npal-1);
                    snespixel[0] += ( (i>>0) & 0x01 ) << (7 - x);
                    snespixel[1] += ( (i>>1) & 0x01 ) << (7 - x);
                    snespixel[2] += ( (i>>2) & 0x01 ) << (7 - x);
                    snespixel[3] += ( (i>>3) & 0x01 ) << (7 - x);

                    snespixel[4] += ( (i>>4) & 0x01 ) << (7 - x);
                    snespixel[5] += ( (i>>5) & 0x01 ) << (7 - x);
                    snespixel[6] += ( (i>>6) & 0x01 ) << (7 - x);
                    snespixel[7] += ( (i>>7) & 0x01 ) << (7 - x);
                }

				if(bin == 0)
				{
					sprintf(str,"$%.2x,$%.2x,",snespixel[0],snespixel[1]);
					strcat(sasm1,str);

					if(npal > 4)
					{
						sprintf(str,"$%.2x,$%.2x,",snespixel[2],snespixel[3]);
						strcat(sasm2,str);
					}

					if(npal > 16)
					{
						sprintf(str,"$%.2x,$%.2x,",snespixel[4],snespixel[5]);
						strcat(sasm3,str);

						sprintf(str,"$%.2x,$%.2x,",snespixel[6],snespixel[7]);
						strcat(sasm4,str);
					}
				}else
				{
					binpixel[y]    = snespixel[0] + (snespixel[1]<<8);
					binpixel[y+8]  = snespixel[2] + (snespixel[3]<<8);
					binpixel[y+16] = snespixel[4] + (snespixel[5]<<8);
					binpixel[y+24] = snespixel[6] + (snespixel[7]<<8);
				}
            }

            if(bin == 0)
			{
				i = strlen(sasm1);
				sasm1[i-1] = 0;
				fputs(sasm1,file);
				fputs("\n",file);
				size += 16;

				if(npal > 4)
				{
					i = strlen(sasm2);
					sasm2[i-1] = 0;
					fputs(sasm2,file);
					fputs("\n",file);
					size += 16;
				}

				if(npal > 16)
				{
					i = strlen(sasm3);
					sasm3[i-1] = 0;
					fputs(sasm3,file);
					fputs("\n",file);

					i = strlen(sasm4);
					sasm4[i-1] = 0;
					fputs(sasm4,file);
					fputs("\n",file);
					size += 32;
				}
			}else
			{
				fwrite(binpixel,2 ,8,file);
				if(npal > 4)  fwrite(&binpixel[8],2 ,8,file);
				if(npal > 16) fwrite(&binpixel[16],2 ,16,file);
			}



        }else //8pbb mode 7
        {
            if(bin == 0) sprintf(sasm1,"    .db");
            for(y = 0;y <8;y++)
            {
                for(x = 0;x < 8;x++)
                {
                    i = tiles[x + (y*8)];

                    if(bin == 0)
					{
						sprintf(str," $%.2x,",i);
						strcat(sasm1,str);
					}else
					{
						cbinpixel[x + (y*8)] =  i;
					}
                }
            }

            if(bin == 0)
			{
				i = strlen(sasm1);
				sasm1[i-1] = 0;
				fputs(sasm1,file);
				fputs("\n",file);
			}else
			{
				fwrite(cbinpixel,1 ,64,file);
			}

            size += 64;

        }


        blocx += 8;
        if(blocx+8 >image->w)
        {
            blocx = 0;
            blocy += 8;
        }

        if(blocy+8 >image->h) break;
    }

	if(bin == 0)
	{
		fputs("\n",file);
		fputs("\n",file);
	}
    return size;
}

void output_filename(char *address,char *str)
{
    int l = 0;
    int i = 0;
    while(address[i] != 0 && address[i] != '.' )
    {
        str[l] = address[i];
        l++;

        if(address[i] == '/' || address[i] == '\\') l = 0;
        i++;
    }
    str[l] = 0;
}

int write_pal(FILE *file,SDL_Surface *image,char *sstr,unsigned char *palette,unsigned char *pixel,int ncolor,int mode,int taille,int bin)
{
    int i,n;
    int psize = 0;
    char str[100];
    unsigned char color;
    int snespixel[4];

	if(bin == 0)
	{
		sprintf(str,"pallette_%s:\n",sstr);
		fputs(str,file);
		sprintf(str,"    .db  ");
		fputs(str,file);
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
            if(n >= 0x300) break;
        }
        color = n/3;
    }


    for(i = 0;i < ncolor;i++)
    {
        n = i*3;

        if( (bin == 0) && (i != 0) )fputs(",",file);

        color = palette[n+2]/8;
        snespixel[0] = color;

        color = palette[n+1]/8;
        snespixel[0] += ( 0x07 & color) << 5;
        snespixel[1] =  (0x18 & color) >> 3;

        color = palette[n+0]/8;
        snespixel[1] += color << 2;

		if(bin == 0)
		{
			sprintf(str,"$%.2x,$%.2x",snespixel[0],snespixel[1]);
			fputs(str,file);
		}else
		{
			fputc(snespixel[0],file);
			fputc(snespixel[1],file);
		}
        psize += 2;
    }

    return psize;
}

void write_end(FILE *file,int psize,int size)
{
    char str[200];

    fputs("\n",file);
    fputs("\n",file);
    fputs("\n",file);

    sprintf(str,";palette size octet : %d ,hexa $%.4x",psize,psize);
    fputs(str,file);
    fputs("\n",file);
    sprintf(str,";size octet : %d ,hexa $%.4x",size,size);
    fputs(str,file);

    fputs("\n",file);


    fclose(file);
}


void snes_convert(SDL_Surface *image,char *address,char *addresspal,int *option,int num)
{
    FILE *file;
    int i,l,taille,ncolor = 0,type = 0;
    int x,y,size = 0,psize = 0;
    char str[200],sstr[200];

    unsigned char palette[0x300];
    for(i = 0;i < 0x300;i++)
        palette[i] = 0;

	int mode = option[2];
	int bpp  = option[0];

    if(mode == 2) type = 1;

    unsigned char *pixel = image->pixels;

    taille = image->w*image->h*image->format->BytesPerPixel;

    ncolor = load_palette(image,palette,option[1])/3;

    //-------------------------------
    if(mode == 4)
    {
        load_paletteext(palette,addresspal);
        mode = 0;
    }

    //-------------------------------

    printf("color : %d\n",ncolor);
    if(bpp == 4) ncolor = 4;
    if(bpp == 16) ncolor = 16;
    if(bpp == 256) ncolor = 256;

    //-------------------------------
    output_filename(address,sstr);
	if(option[4] == 1)
	{
		if(option[3] == 1) sprintf(str,"%s_%d.asm",sstr,num);
		else sprintf(str,"%s_%d.spr",sstr,num);
	}else
	{
		if(option[3] == 1) sprintf(str,"%s.asm",sstr);
		else sprintf(str,"%s.spr",sstr);
	}
    if(option[3] == 1) file = fopen(str,"w");
    else file = fopen(str,"wb");

    if(mode == 0 || mode == 2 || mode == 4)
        size = write_rom(file,image,pixel,palette,ncolor,type,!option[3]);

	if(option[3] == 0)
	{
		fclose(file);
		sprintf(str,"%s.pal",sstr);
		if(option[4] == 1) sprintf(str,"%s_%d.pal",sstr,num);
		file = fopen(str,"wb");
	}

    psize = write_pal(file,image,sstr,palette,pixel,ncolor,mode,taille,!option[3]);

    if(option[3] == 1)
		write_end(file,psize,size);
	else
		fclose(file);
}

