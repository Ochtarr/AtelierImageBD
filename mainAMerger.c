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

#define NB_IMAGE_TOTAL 500

/*
	traitement de toutes les images et enregistrement des infos dans DB_Images/results/x.txt
*/
void traitement();

/*
	isColor = 1 si l'image est en couleur, 0 sinon
*/
int isColor(rgb8 ** image, long nrh, long nrl, long nch, long ncl);

int main(void)
{
	printf("Debut programme!\n");

	traitement();

	printf("Fin du programme\n");
	return 0;
}

void traitement()
{
	int cmp;

	for( cmp=0; cmp<=(NB_IMAGE_TOTAL-1); cmp++)
	{
		//Initialisation des fichiers images à lire
		rgb8 **Image;
		int i,j;
		char nomImage[255];
		long nrh,nrl,nch,ncl;
		char nomImageResultat[255];
		char nameF[255];
		int mIsColor;
		int histo_r[256], histo_g[256], histo_b[256];
		float valMoyNormeGradient;

		sprintf(nomImageResultat, "DB_Images/results/%d.txt", cmp);
		sprintf(nomImage, "DB_Images/ppm/%d.ppm", cmp);

		//Lecture Image
		Image = LoadPPM_rgb8matrix(nomImage, &nrl,&nrh,&ncl,&nch);

		//Création du chemin/nom du fichier txt
		i = 0;
		while (nomImageResultat[i] != '.')
		{
			nameF[i] = nomImageResultat[i];
			i++;
		}
		nameF[i] = '.'; nameF[i+1] = 't'; nameF[i+2] = 'x'; nameF[i+3] = 't'; nameF[i+4] = '\0';

		FILE* fichier = NULL;
    	fichier = fopen(nameF, "w+");

		printf("Traitement de %s\n", nomImage);

		/*DEBUT traitement image -> fichier*/

		mIsColor = isColor(Image, nrh, nrl, nch, ncl);	
		//histo_r = funct_histoR;
		//histo_g = funct_histoR;
		//histo_b = funct_histoR;
		//valMoyNormeGradient = 

		fprintf(fichier, "name = %s\n", nomImage);
		fprintf(fichier, "isColor = %d\n\n", mIsColor);
		fprintf(fichier, "valMoyNormeGradient = %f\n", valMoyNormeGradient);

		fprintf(fichier, "histo_r :");
		for (j=0; j<256; j++)
			fprintf(fichier, " %d", histo_r[i]);

		fprintf(fichier, "\nhisto_g :");
		for (j=0; j<256; j++)
			fprintf(fichier, " %d", histo_g[i]);

		fprintf(fichier, "\nhisto_b");
		for (j=0; j<256; j++)
			fprintf(fichier, " %d", histo_b[i]);

		/*FIN traitement image -> fichier*/
		
		//Free
		fclose(fichier);
		free_rgb8matrix(Image, nrl,nrh,ncl,nch);
	}
}

int isColor(rgb8 ** image, long nrh, long nrl, long nch, long ncl)
{
	int i, j;
	for(i=0; i<nrh; i++)
	{
		for(j=0; j<nch; j++)
		{
			if ((image[i][j].r != image[i][j].g) && (image[i][j].r != image[i][j].b))
				return 0;
			
			//printf("%d %d %d, %d\n", image[i][j].r, image[i][j].g, image[i][j].b, result);
		}
	}

	return 1;	
}