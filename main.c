/*---------------------- 
	Librairies 
------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "def.h"
#include "nrio.h"
#include "nrarith.h"
#include "nralloc.h"

/*---------------------- 
    Variables globales 
------------------------*/

#define SEUIL 80
#define NB_IMAGE_TOTAL 50
#define NB_IMAGE_LUES_SIMULT 3

/*---------------------- 
	Structures 
------------------------*/

typedef struct s_mask
{
	int pixels[3][3];
} mask;

/*---------------------- 
	Prototypes 
------------------------*/
void etiquettage(byte** binary, rgb8 ** image, char nomImageResultat[255], long nrh, long nrl, long nch, long ncl);


int main(void)
{

	printf("Debut programme\n");

	printf("Fin du programme\n");
	return 0;
}


void etiquettage(byte** binary, rgb8 ** image, char nomImageResultat[255], long nrh, long nrl, long nch, long ncl)
{
	int** etiquette;
	int i,j,k;
	int maxEtiquette = 0;
	int nbEtiquette = 0;
	int nbRegions = 0;
	char nameF[255];
	
	etiquette = imatrix(nrl, nrh, ncl, nch);

	i = 0;
	while (nomImageResultat[i] != '.')
	{
		nameF[i] = nomImageResultat[i];
		i++;
	}
	nameF[i] = '.'; nameF[i+1] = 't'; nameF[i+2] = 'x'; nameF[i+3] = 't';

	for (i = nrl; i < nrh; i++)
	{
		for (j = ncl; j < nch; j++)
		{
			byte attC = binary[i][j];
			byte attA, attB;
			float eA, eB;

			if (i > nrl)
			{
				attA = binary[i - 1][j];
				eA = etiquette[i - 1][j];
			}
			else
			{
				attA = 0;
				eA = 0;
			}

			if (j > ncl)
			{
				attB = binary[i][j - 1];
				eB = etiquette[i][j - 1];
			}
			else
			{
				attB = 0;
				eB = 0;
			}

			if (attC == attA)
			{
				if (attC != attB)
					etiquette[i][j] = eA;
				else
				{
					etiquette[i][j] = eB;
					if (eA != eB)
					{
						int k, l;

						for (k = nrl; k <= i; k++)
						{
							for (l = ncl; l < nch; l++)
							{
								if (etiquette[k][l] == eA)
									etiquette[k][l] = eB;
							}
						}
					}
				}
			}
			else
			{
				if (attC == attB)
					etiquette[i][j] = eB;
				else
				{
					etiquette[i][j] = maxEtiquette;
					maxEtiquette++;
				}
			}
		}
	}

	FILE* fichier = NULL;
    fichier = fopen(nameF, "w+");
	
	for (k=0; k<maxEtiquette; k++)
	{
		int nbPixel = 0;
		float cX = 0;
		float cY = 0;
		int nX = 0;
		int nY = 0;

		for (i = nrl; i < nrh; i++)
		{
			for (j = ncl; j < nch; j++)
			{
				if (etiquette[i][j] == k)
				{
					cX += j;
					cY += i;
					nX++;
					nY++;
					nbPixel++;
				}
			}
		}

		float resEcartX = 0;
		float resEcartY = 0;
		float resEcartXY = 0;
		float direction = 0;

		float moyGris = 0;
		float moyR = 0;
		float moyV = 0;
		float moyB = 0;
		
		int countGris[255];
		int gris;
		int l;

		for (l=0; l<255; l++)
			countGris[l] = 0;

		for (i = nrl; i < nrh; i++)
		{
			for (j = ncl; j < nch; j++)
			{
				if (etiquette[i][j] == k)
				{
					resEcartX += (j - (float)(cX/nX))*(j - (float)(cX/nX));
					resEcartY += (i - (float)(cY/nY))*(i - (float)(cY/nY));
					resEcartXY += (j - (float)(cX/nX))*(i - (float)(cY/nY));

					moyGris += (image[i][j].r +  image[i][j].g + image[i][j].b);
					moyR += image[i][j].r;
					moyV += image[i][j].g;
					moyB += image[i][j].b;

					gris = moyGris/(3*nX);
					countGris[gris]++;
				}
			}
		}

		direction = 0.5 * atan((2*resEcartXY/nY)/((resEcartX/nX)-(resEcartY/nY)));

		if (nbPixel != 0)
		{
			fprintf(fichier, "\nNombre de pixel dans la région[%d] : %d\n", k, nbPixel);
			fprintf(fichier, "Coordonnéé centre gravité : %f , %f\n", (float)(cX/nX), (float)(cY/nY));
			
			fprintf(fichier, "Ecart type en X : %f\n", resEcartX/nX);
			fprintf(fichier, "Ecart type en Y : %f\n", resEcartY/nY);
			fprintf(fichier, "Ecart type en XY : %f\n", resEcartXY/nY);

			fprintf(fichier, "Direction principale : %f\n", direction*180/3.14);

			fprintf(fichier, "Moyenne de gris : %f\n", moyGris/(3*nY));
			fprintf(fichier, "Moyenne de rouge : %f\n", moyR/(3*nY));
			fprintf(fichier, "Moyenne de vert : %f\n", moyV/(3*nY));
			fprintf(fichier, "Moyenne de bleu : %f\n", moyB/(3*nY));

			//Histogramme pour chaque région
			fprintf(fichier, "Histogramme : \n");

			for (l=0; l<255; l++)
				fprintf(fichier, "%d ", countGris[l]);

			fprintf(fichier, "\n\n");

			nbRegions++;
		}
	}

	fprintf(fichier, "\nAu total, il y a %d régions.\n\n", nbRegions);

	for (i = nrl; i < nrh; i++)
	{
		for (j = ncl; j < nch; j++)
		{
			if (etiquette[i][j] != 0) //background
			{
				fprintf(fichier, "E : %d\n", etiquette[i][j]);
			}
		}
	}

	fclose(fichier);
}


