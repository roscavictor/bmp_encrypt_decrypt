#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
typedef struct
{
	unsigned  char r;
	unsigned char g;
	unsigned char b;
}pixel;
uint32_t xorshift32(uint32_t n)
{

	uint32_t   x = n;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;

	return x;
}
union uni
{
	int val;
	unsigned char r;
	unsigned char g;
	unsigned char b;
};
pixel* Lin(char* imgpath)  //transforming image into 1d array
{

	unsigned char RGB[3];
	int i, j, x, padding;
	FILE* f;
	f = fopen(imgpath, "rb");
	unsigned  int dim, h, w;
	fseek(f, 2, SEEK_SET);
	fread(&dim, sizeof(unsigned int), 1, f);


	fseek(f, 18, SEEK_SET);
	fread(&w, sizeof(unsigned int), 1, f);
	fread(&h, sizeof(unsigned int), 1, f);  //read width and height
	fseek(f, 54, SEEK_SET);

	if (w % 4 != 0)
		padding = 4 - (3 * w) % 4;
	else
		padding = 0;
	pixel* v;
	v = (pixel*)malloc(h * w * sizeof(pixel));
	int k = 0;
	for (i = 0; i < h; i++)
	{
		for (j = 0; j < w; j++)
		{
			fread(RGB, 3, 1, f);
			v[k].r = RGB[2];
			v[k].g = RGB[1];
			v[k].b = RGB[0];
			k++;                 //linearising the pixel array (transforming it into a 1d array from 2d)
		}
		fseek(f, padding, SEEK_CUR);
	}
	fclose(f);

	return v;
}
void save(pixel* v, char* imgsource, char* destination)
{
	int i, j, x, padding;
	FILE* f = fopen("peppers.bmp", "rb");
	FILE* fout = fopen(destination, "wb+");
	unsigned  int dim, h, w;
	fseek(f, 2, SEEK_SET);
	fread(&dim, sizeof(unsigned int), 1, f);


	fseek(f, 18, SEEK_SET);
	fread(&w, sizeof(unsigned int), 1, f);
	fread(&h, sizeof(unsigned int), 1, f);
	rewind(f);
	unsigned char z;
	for (i = 0; i < 54; i++)
	{
		fread(&z, 1, 1, f);
		fwrite(&z, 1, 1, fout);
	}
	if (w % 4 != 0)
		padding = 4 - (3 * w) % 4;
	else
		padding = 0;

	int m = 0;

	for (i = 0; i < h; i++)
	{
		for (j = 0; j < w; j++)
		{
			fwrite(&v[m].b, 1, 1, fout);


			fwrite(&v[m].g, 1, 1, fout);

			fwrite(&v[m].r, 1, 1, fout);

			m++;
		}
		fseek(fout, padding, SEEK_CUR);
	}
	fclose(fout);
}
void swappixel(pixel* a, pixel* b)
{
	pixel aux = *a;
	*a = *b;
	*b = aux;
}
void swap(int* a, int* b)
{
	int aux = *a;
	*a = *b;
	*b = aux;
}
void crypt(char* imgsource, char* destination, char* keypath)
{
	int i, j, padding;
	FILE* fin = fopen(imgsource, "rb");
	FILE* fout = fopen(destination, "wb+");
	FILE* f = fopen(keypath, "r");

	unsigned  int dim, h, w;
	fseek(fin, 2, SEEK_SET);
	fread(&dim, sizeof(unsigned int), 1, fin);
	fseek(fin, 18, SEEK_SET);
	fread(&w, sizeof(unsigned int), 1, fin);
	fread(&h, sizeof(unsigned int), 1, fin);
	rewind(fin);
	unsigned char n;
	for (i = 0; i < 54; i++)
	{
		fread(&n, 1, 1, fin);
		fwrite(&n, 1, 1, fout);
	}
	if (w % 4 != 0)
		padding = 4 - (3 * w) % 4;
	else
		padding = 0;
	pixel* v;
	v = (pixel*)malloc(h * w * sizeof(pixel));
	v = Lin(imgsource);
	unsigned int* r;
	r = malloc((2 * h * w) * sizeof(unsigned int));
	int key;
	fscanf(f, "%d", &key);
	r[0] = key;   //key in first element, which helps us generate random numbers
	for (i = 1; i < 2 * h * w - 1; i++) //generate random numbers
	{
		r[i] = xorshift32(r[i - 1]);
	}
	unsigned int* perm;
	perm = malloc(h * w * sizeof(unsigned int));
	for (i = 0; i < h * w; i++)
	{
		perm[i] = i;
	}

	int t;
	int m = 0;
	for (i = h * w - 1; i > 0; i--)
	{
		t = r[m] % (i + 1);   //random number
		swap(&perm[t], &perm[i]);   // shuffle
		m++;
	}



	pixel* vprim;
	vprim = (pixel*)malloc(h * w * sizeof(pixel));
	for (i = 0; i < h * w; i++)
	{
		vprim[perm[i]].b = v[i].b;
		vprim[perm[i]].g = v[i].g;
		vprim[perm[i]].r = v[i].r;
	}
	union uni x;
	int sv;
	fscanf(f, "%d", &sv);
	fclose(f);
	x.val = sv;
	union uni y;
	y.val = r[h * w];

	pixel* vsecond;
	vsecond = (pixel*)malloc(h * w * sizeof(pixel));
	vsecond[0].b = x.b ^ vprim[0].b ^ y.b; //xor the elements
	vsecond[0].g = x.g ^ vprim[0].g ^ y.g;
	vsecond[0].r = x.r ^ vprim[0].r ^ y.r;
	union uni z;
	for (i = 1; i < h * w; i++)    //xor the other elements
	{
		z.val = r[h * w + i];
		vsecond[i].b = vsecond[i - 1].b ^ vprim[i].b ^ z.b;
		vsecond[i].g = vsecond[i - 1].g ^ vprim[i].g ^ z.g;
		vsecond[i].r = vsecond[i - 1].r ^ vprim[i].r ^ z.r;
	}


	save(vsecond, imgsource, destination);  //

}
void decrypt(char* imgsource, char* destination, char* keypath)
{
	int i, j, padding;
	FILE* fin = fopen(imgsource, "rb");
	FILE* fout = fopen(destination, "wb+");
	FILE* f = fopen(keypath, "r");



	unsigned  int dim, h, w;
	fseek(fin, 2, SEEK_SET);
	fread(&dim, sizeof(unsigned int), 1, fin);
	fseek(fin, 18, SEEK_SET);
	fread(&w, sizeof(unsigned int), 1, fin);
	fread(&h, sizeof(unsigned int), 1, fin);
	rewind(fin);
	unsigned char n;
	for (i = 0; i < 54; i++)
	{
		fread(&n, 1, 1, fin);
		fwrite(&n, 1, 1, fout);
	}
	if (w % 4 != 0)
		padding = 4 - (3 * w) % 4;
	else
		padding = 0;
	pixel* v;
	v = (pixel*)malloc(h * w * sizeof(pixel));
	v = Lin(imgsource);
	unsigned int* r;
	r = malloc((2 * h * w) * sizeof(unsigned int));
	int key;
	fscanf(f, "%d", &key);
	r[0] = key;
	for (i = 1; i < 2 * h * w - 1; i++) //generating random numbers
	{
		r[i] = xorshift32(r[i - 1]);
	}
	unsigned int* perm;
	perm = malloc(h * w * sizeof(unsigned int));
	for (i = 0; i < h * w; i++)
	{
		perm[i] = i;
	}

	int t;
	int m = 0;
	for (i = h * w - 1; i >= 0; i--)
	{
		t = r[m] % (i + 1);   //random number
		swap(&perm[t], &perm[i]);   // shuffle
		m++;
	}
	unsigned int* perminv;
	perminv = (unsigned int*)malloc(h * w * sizeof(unsigned int));
	for (i = 0; i < h * w; i++)   //inverse of permutation
	{
		perminv[perm[i]] = i;
	}
	union uni x;
	int sv;
	fscanf(f, "%d", &sv);
	fclose(f);
	x.val = sv;
	union uni y;
	y.val = r[h * w];

	pixel* vsecond;
	vsecond = (pixel*)malloc(h * w * sizeof(pixel));
	vsecond[0].b = x.b ^ v[0].b ^ y.b;
	vsecond[0].g = x.g ^ v[0].g ^ y.g;
	vsecond[0].r = x.r ^ v[0].r ^ y.r;
	union uni z;
	for (i = 1; i < h * w; i++) //xor-ing elements
	{
		z.val = r[h * w + i];
		vsecond[i].b = v[i - 1].b ^ v[i].b ^ z.b;
		vsecond[i].g = v[i - 1].g ^ v[i].g ^ z.g;
		vsecond[i].r = v[i - 1].r ^ v[i].r ^ z.r;
	}
	pixel* vprim;
	vprim = (pixel*)malloc(h * w * sizeof(pixel));
	for (i = 0; i < h * w; i++)
	{
		vprim[perminv[i]].b = vsecond[i].b;
		vprim[perminv[i]].g = vsecond[i].g;
		vprim[perminv[i]].r = vsecond[i].r;
	}



	save(vprim, imgsource, destination); //
}
void chisquared(char* imgsource)
{
	int i, j, padding;
	FILE* fin = fopen(imgsource, "rb");
	unsigned char RGB[3];
	unsigned  int dim, h, w;
	fseek(fin, 2, SEEK_SET);
	fread(&dim, sizeof(unsigned int), 1, fin);
	fseek(fin, 18, SEEK_SET);
	fread(&w, sizeof(unsigned int), 1, fin);
	fread(&h, sizeof(unsigned int), 1, fin);
	fseek(fin, 54, SEEK_SET);
	int r[256], g[256], b[256];
	for (i = 0; i < 256; i++)
	{
		r[i] = 0;
		g[i] = 0;
		b[i] = 0;
	}
	if (w % 4 != 0)
		padding = 4 - (3 * w) % 4;
	else
		padding = 0;
	int k = 0;
	for (i = 0; i < h; i++)
	{
		for (j = 0; j < w; j++)
		{
			fread(RGB, 3, 1, fin);
			r[RGB[2]]++;
			g[RGB[1]]++;
			b[RGB[0]]++;

		}
		fseek(fin, padding, SEEK_CUR);
	}
	double Sr = 0, Sg = 0, Sb = 0;
	double form = (h * w) / 256;
	for (i = 0; i < 256; i++)
	{
		Sr = Sr + ((r[i] - form) * (r[i] - form)) / (form);   //sum formula
		Sg = Sg + ((g[i] - form) * (g[i] - form)) / (form);
		Sb = Sb + ((b[i] - form) * (b[i] - form)) / (form);
	}


	printf("R= %.2f G=%.2f B=%.2f", Sr, Sg, Sb);
	fclose(fin);
}
int main()
{

	int i, x, y;
	FILE* fin, * fout;
	char name[101];
	printf("image name?\n");
	scanf("%s", name);
	fin = fopen(name, "rb");
	char key[101];
	printf("key location?");
	scanf("%s", key);
	printf("where to save crypted image?\n");
	char savedcrypt[101];
	scanf("%s", savedcrypt);
	crypt(name, savedcrypt, key);
	printf("where to save decrypt?\n");
	char saveddecrypt[101];
	scanf("%s", saveddecrypt);
	decrypt(savedcrypt, saveddecrypt, key);
	chisquared(name);
	printf("\n Chi squared of  crypted image:");
	chisquared(savedcrypt);
	return 0;
}
