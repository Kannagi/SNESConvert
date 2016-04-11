#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>


#ifdef __MINGW32__
#undef main
#endif

void snes_convert(SDL_Surface *image,char *adresse,int force,int noalpha,int mode);

int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Surface *image,*copy;
    int n = 1,force = 0,noalpha = 0,i,mode = 0;
    char adresse[1000];
    adresse[0] = 0;

    for(i = 1; i < argc;i++)
    {
        if(argv[i][0] == '-')
        {
            if(strcmp(argv[i],"-4") == 0) force = 4;
            if(strcmp(argv[i],"-16") == 0) force = 16;
            if(strcmp(argv[i],"-noalpha") == 0) noalpha = 1;
            if(strcmp(argv[i],"-palette") == 0) mode = 1;
            if(strcmp(argv[i],"-paletteall") == 0) mode = 3;
            if(strcmp(argv[i],"-mode7") == 0) mode = 2;
            if(strcmp(argv[i],"-loadpalette") == 0) mode = 4;

        }else
        {
            strcpy(adresse,argv[i]);
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

    snes_convert(copy,adresse,force,noalpha,mode);

    SDL_FreeSurface(copy);
    SDL_FreeSurface(image);
    SDL_Quit();

    return 0;

}

void snes_convert(SDL_Surface *image,char *adresse,int force,int noalpha,int mode)
{
    int i,l,taille,npal = 0;
    unsigned char palette[768];
    for(i = 0;i < 768;i++)
        palette[i] = 0;


    unsigned char *pixel = image->pixels;

    taille = image->w*image->h*image->format->BytesPerPixel;

    //printf("%d %d = %d octet\n",image->w,image->h,taille);
    int n = 0,ok;

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

    //-------------------------------
    SDL_Surface *image2,*copy;
    if(mode == 4)
    {

        image2 = IMG_Load("palette.png");
        if(image2 == NULL)
        {
            printf("Image is not valide\n");
            return;
        }
        for(i = 0;i < 768;i++)
            palette[i] = 0;

        copy = SDL_CreateRGBSurface(0,image2->w,image2->h,24,0,0,0,0);
        SDL_BlitSurface(image2,NULL,copy,NULL);
        n = 0;
        unsigned char *pixel2 = copy->pixels;

        for(i = 0;i < image2->w*copy->format->BytesPerPixel;i += copy->format->BytesPerPixel)
        {
            palette[i+0] = pixel2[i+0];
            palette[i+1] = pixel2[i+1];
            palette[i+2] = pixel2[i+2];
            n+=3;
        }

        mode = 0;
    }

    //-------------------------------

    printf("color : %d\n",n/3);
    npal = n/3;
    if(force == 4) npal = 4;
    if(force == 16) npal = 16;

    int x,y,size = 0;
    int casex,casey;

    casex = 0;
    casey = 0;

    char casm1[10000];
    char casm2[1500];
    char chaine[1000],schaine[1000];
    unsigned char couleur;

    i = 0;
    while(adresse[i] != 0 && adresse[i] != '.' )
    {
        schaine[i] = adresse[i];
        i++;
    }
    schaine[i] = 0;

    int tiles[64],r,v,b;
    int octet4[4];

    FILE *file;
    sprintf(chaine,"%s.asm",schaine);
    file = fopen(chaine,"w");

    sprintf(chaine,"\n");
    fputs(chaine,file);


    while(mode != 1 && mode < 3)
    {
        n = 0;
        for(y = casey;y < casey+8;y++)
        {
            for(x = casex;x < casex+8;x++)
            {
                i = (y*image->w*image->format->BytesPerPixel) + x*image->format->BytesPerPixel;
                r = pixel[i+0];
                v = pixel[i+1];
                b = pixel[i+2];
                //printf("i %d x:%d y:%d %d %d %d / ",i/4,x,y,r,v,b);

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

        if(mode == 0)
        {
            sprintf(casm1,"    .db");
            sprintf(casm2,"    .db");

            for(y = 0;y <8;y++)
            {
                octet4[0] = 0;
                octet4[1] = 0;
                octet4[2] = 0;
                octet4[3] = 0;

                for(x = 0;x < 8;x++)
                {
                    i = tiles[x + (y*8)] + noalpha;

                    if(i > 15) i = 15;
                    octet4[0] += ( (i>>0) & 0x01 ) << -(x - 7);
                    octet4[1] += ( (i>>1) & 0x01 ) << -(x - 7);
                    octet4[2] += ( (i>>2) & 0x01 ) << -(x - 7);
                    octet4[3] += ( (i>>3) & 0x01 ) << -(x - 7);
                }



                sprintf(chaine," $%.2x, $%.2x,",octet4[0],octet4[1]);
                strcat(casm1,chaine);

                if(npal > 4)
                {
                    sprintf(chaine," $%.2x, $%.2x,",octet4[2],octet4[3]);
                    strcat(casm2,chaine);
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

        }else
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

    //------------------------
    l = 0;
    i = 0;
    while(adresse[i] != 0 && adresse[i] != '.' )
    {
        schaine[l] = adresse[i];
        l++;

        if(adresse[i] == '/' || adresse[i] == '\\') l = 0;
        i++;
    }
    schaine[l] = 0;
    //------------------------

    sprintf(chaine,"pallette_%s:\n",schaine);
    fputs(chaine,file);
    sprintf(casm1,"    .db");

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
        npal = n/3;
    }

    int psize = 0;
    for(i = 0;i < npal;i++)
    {
        n = i*3;

        couleur = palette[n+2]/8;
        octet4[0] = couleur;

        couleur = palette[n+1]/8;
        octet4[0] += ( 0x07 & couleur) << 5;
        octet4[1] =  (0x18 & couleur) >> 3;

        couleur = palette[n+0]/8;
        octet4[1] += couleur << 2;

        sprintf(chaine," $%.2x, $%.2x,",octet4[0],octet4[1]);

        //printf("%s %d %d %d\n",chaine ,palette[n+0],palette[n+1],palette[n+2]);
        strcat(casm1,chaine);
        psize += 2;
    }

    i = strlen(casm1);
    casm1[i-1] = 0;
    fputs(casm1,file);
    fputs("\n",file);
    fputs("\n",file);
    fputs("\n",file);

    sprintf(chaine,";palette size octet : %d ,hexa $%4x",psize,psize);
    fputs(chaine,file);
    fputs("\n",file);
    fputs("\n",file);
    sprintf(chaine,";size octet : %d ,hexa $%4x",size,size);
    fputs(chaine,file);

    fputs("\n",file);


    fclose(file);
}

