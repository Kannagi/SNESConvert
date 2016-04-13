#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>


#ifdef __MINGW32__
#undef main
#endif

void snes_convert(SDL_Surface *image,char *adresse,int force,int noalpha,int mode,char *adressepal);

int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Surface *image,*copy;
    int n = 1,force = 0,noalpha = 0,i,mode = 0,ok = 0;
    char adresse[500],adressepal[500];

    strcpy(adressepal,"palette.png");
    adresse[0] = 0;

    for(i = 1; i < argc;i++)
    {
        if(argv[i][0] == '-')
        {
            if(strcmp(argv[i],"-2bpp") == 0) force = 4;
            if(strcmp(argv[i],"-4bpp") == 0) force = 16;
            if(strcmp(argv[i],"-8bpp") == 0) force = 256;
            if(strcmp(argv[i],"-noalpha") == 0) noalpha = 1;
            if(strcmp(argv[i],"-palette") == 0) mode = 1;
            if(strcmp(argv[i],"-paletteall") == 0) mode = 3;
            if(strcmp(argv[i],"-mode7") == 0) mode = 2;

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
        printf("Enter a picture format .png,pcx,bmp, or jpg\n");
        printf("Force -4 or -16 for choose palette \n");
        printf("Exemple :\nsnesconvert -4 myimage or snesconvert myimage (snesconvert choose auto -4 or -16)\n");
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

    snes_convert(copy,adresse,force,noalpha,mode,adressepal);

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
    int n = 0,ok;
    //printf("%d %d = %d octet\n",image->w,image->h,taille);

    for(i = 0;i < 768;i++)
        palette[i] = 0;

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

void load_paletteext(unsigned char *palette,char *adressepal)
{
    int i;
    SDL_Surface *image,*copy;
    image = IMG_Load(adressepal);

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

int write_rom(FILE *file,SDL_Surface *image,unsigned char *pixel,unsigned char *palette,int npal,int noalpha,int type)
{
    int casex,casey;
    int tiles[64];
    int octet4[8];
    int i,l;
    int x,y,size = 0;
    char chaine[500];
    char casm1[500];
    char casm2[500];
    char casm3[500];
    char casm4[500];

    casex = 0;
    casey = 0;

    sprintf(chaine,"\n");
    fputs(chaine,file);


    while(1)
    {
        tri_palette(image,casex,casey,pixel,palette,tiles);

        if(type == 0) //2,4,8pbb
        {
            sprintf(casm1,"    .db ");
            sprintf(casm2,"    .db ");
            sprintf(casm3,"    .db ");
            sprintf(casm4,"    .db ");

            for(y = 0;y <8;y++)
            {
                octet4[0] = 0;
                octet4[1] = 0;
                octet4[2] = 0;
                octet4[3] = 0;

                octet4[4] = 0;
                octet4[5] = 0;
                octet4[6] = 0;
                octet4[7] = 0;

                for(x = 0;x < 8;x++)
                {
                    i = tiles[x + (y*8)] + noalpha;

                    if(i > npal-1) i = npal-1;
                    octet4[0] += ( (i>>0) & 0x01 ) << (7 - x);
                    octet4[1] += ( (i>>1) & 0x01 ) << (7 - x);
                    octet4[2] += ( (i>>2) & 0x01 ) << (7 - x);
                    octet4[3] += ( (i>>3) & 0x01 ) << (7 - x);

                    octet4[4] += ( (i>>4) & 0x01 ) << (7 - x);
                    octet4[5] += ( (i>>5) & 0x01 ) << (7 - x);
                    octet4[6] += ( (i>>6) & 0x01 ) << (7 - x);
                    octet4[7] += ( (i>>7) & 0x01 ) << (7 - x);
                }



                sprintf(chaine,"$%.2x,$%.2x,",octet4[0],octet4[1]);
                strcat(casm1,chaine);

                if(npal > 4)
                {
                    sprintf(chaine,"$%.2x,$%.2x,",octet4[2],octet4[3]);
                    strcat(casm2,chaine);
                }

                if(npal > 16)
                {
                    sprintf(chaine,"$%.2x,$%.2x,",octet4[4],octet4[5]);
                    strcat(casm3,chaine);

                    sprintf(chaine,"$%.2x,$%.2x,",octet4[6],octet4[7]);
                    strcat(casm4,chaine);
                }
            }

            i = strlen(casm1);
            casm1[i-1] = 0;
            fputs(casm1,file);
            fputs("\n",file);
            size += 16;

            if(npal > 4)
            {
                i = strlen(casm2);
                casm2[i-1] = 0;
                fputs(casm2,file);
                fputs("\n",file);
                size += 16;
            }

            if(npal > 16)
            {
                i = strlen(casm3);
                casm3[i-1] = 0;
                fputs(casm3,file);
                fputs("\n",file);

                i = strlen(casm4);
                casm4[i-1] = 0;
                fputs(casm4,file);
                fputs("\n",file);
                size += 32;
            }

        }else //8pbb mode 7
        {
            sprintf(casm1,"    .db");
            for(y = 0;y <8;y++)
            {
                for(x = 0;x < 8;x++)
                {
                    i = tiles[x + (y*8)] + noalpha;

                    sprintf(chaine," $%.2x,",i);
                    strcat(casm1,chaine);
                }
            }

            i = strlen(casm1);
            casm1[i-1] = 0;
            fputs(casm1,file);
            fputs("\n",file);
            size += 64;

        }


        casex += 8;
        if(casex+8 >image->w)
        {
            casex = 0;
            casey += 8;
        }

        if(casey+8 >image->h) break;
    }

    //ecriture palette
    fputs("\n",file);
    fputs("\n",file);
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

int write_pal(FILE *file,SDL_Surface *image,char *schaine,unsigned char *palette,unsigned char *pixel,int color,int mode,int taille)
{
    int i,n;
    int psize = 0;
    char chaine[100];
    unsigned char couleur;
    int octet4[4];

    sprintf(chaine,"pallette_%s:\n",schaine);
    fputs(chaine,file);
    sprintf(chaine,"    .db  ");
    fputs(chaine,file);

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

        if(i != 0) fputs(",",file);

        couleur = palette[n+2]/8;
        octet4[0] = couleur;

        couleur = palette[n+1]/8;
        octet4[0] += ( 0x07 & couleur) << 5;
        octet4[1] =  (0x18 & couleur) >> 3;

        couleur = palette[n+0]/8;
        octet4[1] += couleur << 2;

        sprintf(chaine,"$%.2x,$%.2x",octet4[0],octet4[1]);
        fputs(chaine,file);
        //printf("%s %d %d %d\n",chaine ,palette[n+0],palette[n+1],palette[n+2]);
        psize += 2;
    }

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


void snes_convert(SDL_Surface *image,char *adresse,int force,int noalpha,int mode,char *adressepal)
{
    FILE *file;
    int i,l,taille,color = 0,type = 0;
    int x,y,size = 0,psize = 0;
    char chaine[200],schaine[200];

    unsigned char palette[768];
    for(i = 0;i < 768;i++)
        palette[i] = 0;

    if(mode == 2) type = 1;

    unsigned char *pixel = image->pixels;

    taille = image->w*image->h*image->format->BytesPerPixel;

    color = load_palette(image,palette)/3;

    //-------------------------------
    if(mode == 4)
    {
        load_paletteext(palette,adressepal);
        mode = 0;
    }

    //-------------------------------

    printf("color : %d\n",color);
    if(force == 4) color = 4;
    if(force == 16) color = 16;
    if(force == 256) color = 256;

    //-------------------------------
    output_filename(adresse,schaine);

    sprintf(chaine,"%s.asm",schaine);
    file = fopen(chaine,"w");

    if(mode == 0 || mode == 2 || mode == 4)
        size = write_rom(file,image,pixel,palette,color,noalpha,type);

    psize = write_pal(file,image,schaine,palette,pixel,color,mode,taille);

    write_end(file,psize,size);
}

