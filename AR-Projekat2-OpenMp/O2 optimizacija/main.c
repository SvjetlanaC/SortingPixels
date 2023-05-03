#include <stdio.h>
#include <stdlib.h>
#pragma pack (1)
#include <string.h>
#include <time.h>
#include <omp.h>

typedef struct {
    unsigned char R;
    unsigned char G;
    unsigned char B;
}COLOR;

typedef struct
{
	unsigned short file_type;
	unsigned int file_size;
	unsigned short reserved1;
	unsigned short reserved2;
	unsigned int offset;
}BMP_FILE_HEADER;

typedef struct
{
	unsigned int size;
	long width;
	long height;
	unsigned short planes;
	unsigned short bit_per_px;
	unsigned int compression;
	unsigned int img_size;
	long x_px_m;
	long y_px_m;
	unsigned int clr_used;
	unsigned int clr_important;
}BMP_INFO; //zaglavlje slike

void sortColumns(BMP_INFO info,unsigned char *buffer,int i)
{
    COLOR t;
    int p=0,l;
    if(info.width % 4 != 0)
        p = 4 - (info.width % 4);
    for(int k=0; k<info.width * 3; k+=3)
        {
            t.B=buffer[i*info.width*3 + i*p + k];
            t.G=buffer[i*info.width*3 + i*p + k + 1];
            t.R=buffer[i*info.width*3 + i*p + k + 2];

            long s=0.3*t.R + 0.59*t.G + 0.11*t.B;

            for(l=k; l>0 && s < (0.3*buffer[i*info.width*3 + i*p + l-1]+0.59*buffer[i*info.width*3 + i*p + l-2]+0.11*buffer[i*info.width*3 + i*p +l-3]);l-=3)
            {
                buffer[i*info.width*3 + i*p + l]=buffer[i*info.width*3 + i*p + l-3];
                buffer[i*info.width*3 + i*p + l+1]=buffer[i*info.width*3 + i*p + l-2];
                buffer[i*info.width*3 + i*p + l+2]=buffer[i*info.width*3 + i*p + l-1];
            }
            buffer[i*info.width*3 + i*p + l]=t.B;
            buffer[i*info.width*3 + i*p + l+1]=t.G;
            buffer[i*info.width*3 + i*p + l+2]=t.R;
        }
}

void sort(BMP_INFO info, unsigned char *buffer)
{
    long i;
    int pivot=info.height/2;
    #pragma omp parallel sections
    {

        #pragma omp section
        {
            for(i=0; i < pivot-1; i++)
            {
                sortColumns(info,buffer,i);
            }
        }

        #pragma omp section
        {
            for(int i=pivot-1; i<info.height;i++)
            {
                sortColumns(info,buffer,i);
            }
        }
    }
}

int main(int argc, char *argv[])
{
	FILE *in, *out;
	clock_t start,end,total;
	if (argc < 3)
		printf("Nedovoljan broj argumenata!");
	else
	{
		if ((in = fopen(argv[1], "rb")) != NULL)
		{
			BMP_FILE_HEADER h;
			BMP_INFO info;
			unsigned char* buffer;

			int  size;
			fread(&h, sizeof(BMP_FILE_HEADER), 1, in);
			fread(&info, sizeof(BMP_INFO), 1, in);
			if (h.file_type != 0x4D42)
				printf("Nepoznat tip fajla!");
			else if (info.compression != 0)
				printf("Fajl je kompresovan!");
			else if (info.bit_per_px != 24)
				printf("Fajl nije 24-bitni bmp!");
			else
			{
				size = h.file_size - h.offset; //velicina bafera
				buffer = (unsigned char*)calloc(size, 1);

				fread(buffer,1, size, in);

                start=clock();
                sort(info,buffer);
                end=clock();

                total = (double)(end - start) / CLOCKS_PER_SEC * 1000;
                printf("Vrijeme izvrsavanja sa OpenMP optimizacijom: %ds%dms \n",total/1000,total%1000);


				if ((out = fopen(argv[2], "wb")) != NULL)
				{
					fwrite(&h, sizeof(BMP_FILE_HEADER), 1, out);
					fwrite(&info, sizeof(BMP_INFO), 1, out);
					fwrite(buffer, 1, size, out);
					fclose(out);
				}
				else
					printf("Greska prilikom otvaranja izlazne datoteke!");
				free(buffer);
			}
			fclose(in);
		}
		else
			printf("Greska prilikom otvaranja ulazne datoteke!");

	}
	FILE *info;
    if((info = fopen("info.txt", "a")) != NULL)
    {
        fprintf(info,"Vrijeme izvrsavanja sa OpenMP optimizacijom: %ds%dms \n",total/1000,total%1000);
    }
    else
        printf("Neuspjesno otvaranje datoteke sa rezultatima mjerenja!\n");
    fclose(info);
	return 0;
}

