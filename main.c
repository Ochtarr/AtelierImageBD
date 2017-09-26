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
#define NB_IMAGE_TOTAL 500

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

byte** couleursToNDG(rgb8** imgSource, int nrl, int nrh, int ncl, int nch);
byte** filtreMoyenneur(byte** imgSource, int nrl, int nrh, int ncl, int nch);
int** gradientH(byte** imgSource, int nrl, int nrh, int ncl, int nch);
int** gradientV(byte** imgSource, int nrl, int nrh, int ncl, int nch);
int** normeGradient(int** gradientH, int** gradientV, int nrl, int nrh, int ncl, int nch);
float moyNormeGradient(int** gradient, int nrl, int nrh, int ncl, int nch);
byte** detectionContours(int** normeG, int seuil, int nrl, int nrh, int ncl, int nch);
int* histogrammeNDG(byte** imgSource, int nrl, int nrh, int ncl, int nch);
int** histogrammeCouleurs(rgb8** imgSource, int nrl, int nrh, int ncl, int nch);
int distanceEucliHistogrammesNDG(int* histoNDG1, int* histoNDG2);
int* distanceEucliHistogrammesC(int** histoCouleur1, int** histoCouleur2);
float distanceBatHistogrammes(int* histo1, int* histo2);
float* getProportionCouleur(int** histoCouleur);

/*
	traitement de toutes les images et enregistrement des infos dans DB_Images/results/x.txt
*/
void traitement();

/*
	isColor = 1 si l'image est en couleur, 0 sinon
*/
int isColor(rgb8 ** image, int nrl, int nrh, int ncl, int nch);

/*---------------------- 
	MAIN 
------------------------*/
int main(void)
{

	printf("Debut programme\n");

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

		printf("*\n");
		mIsColor = isColor(Image, nrl, nrh, ncl, nch);	
		printf("*\n");
		byte **ImageNDG = couleursToNDG(Image, nrl, nrh, ncl, nch);
		printf("*\n");
		int** histoCouleurs = histogrammeCouleurs(Image, nrl, nrh, ncl, nch);
		printf("*\n");
		int** gradH = gradientH(ImageNDG, nrl, nrh, ncl, nch);
		printf("*\n");
		int** gradV = gradientV(ImageNDG, nrl, nrh, ncl, nch);
		printf("*\n");
		int** normeGrad = normeGradient(gradH, gradV, nrl, nrh, ncl, nch);
		printf("*\n");
		valMoyNormeGradient = moyNormeGradient( normeGrad, nrh, nrl, nch, ncl);
		printf("*\n");
		
		fprintf(fichier, "name = %s\n", nomImage);
		fprintf(fichier, "isColor = %d\n\n", mIsColor);
		fprintf(fichier, "valMoyNormeGradient = %f\n", valMoyNormeGradient);

		fprintf(fichier, "histo_r :");
		for (j=0; j<256; j++)
			fprintf(fichier, " %d", histoCouleurs[0][i]);

		fprintf(fichier, "\nhisto_g :");
		for (j=0; j<256; j++)
			fprintf(fichier, " %d", histoCouleurs[1][i]);

		fprintf(fichier, "\nhisto_b");
		for (j=0; j<256; j++)
			fprintf(fichier, " %d", histoCouleurs[2][i]);

		/*FIN traitement image -> fichier*/
		
		//Free
		fclose(fichier);
		free_imatrix(histoCouleurs, nrl,nrh,ncl,nch);
		free_imatrix(gradH, nrl,nrh,ncl,nch);
		free_imatrix(gradV, nrl,nrh,ncl,nch);
		free_imatrix(normeGrad, nrl,nrh,ncl,nch);
		free_bmatrix(ImageNDG, nrl,nrh,ncl,nch);
		free_rgb8matrix(Image, nrl,nrh,ncl,nch);
	}
}

int isColor(rgb8 ** image, int nrl, int nrh, int ncl, int nch)
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

byte** couleursToNDG(rgb8** imgSource, int nrl, int nrh, int ncl, int nch){
	int i,j;
	byte **R;
	R = bmatrix(nrl,nrh,ncl,nch);

	for (i = nrl; i <= nrh; i++)
	{
		for (j = ncl; j <= nch; j++)
		{
			R[i][j] = (imgSource[i][j].r +  imgSource[i][j].g + imgSource[i][j].b)/3;
		}
	}
	return R;
}

byte** filtreMoyenneur(byte** imgSource, int nrl, int nrh, int ncl, int nch){
	int i,j;
	byte **imgReturned = bmatrix(nrl,nrh,ncl,nch);

	for (i = nrl; i <= nrh; i++)
	{
		for (j = ncl; j <= nch; j++)
		{
			//coin HG
			if( i == 0 && j == 0)
			{
				imgReturned[i][j] += (4*imgSource[i][j]+2*imgSource[i+1][j]+2*imgSource[i][j+1]+imgSource[i+1][j+1])/9;		
			}

			//coin HD
			if( i==nrh && j==0)
			{
				imgReturned[i][j] += (4*imgSource[i][j]+2*imgSource[i-1][j]+2*imgSource[i][j+1]+imgSource[i-1][j+1])/9;
			}
			
			//coin BG
			if( i==0 && j==nch)
			{
				imgReturned[i][j] += (4*imgSource[i][j]+2*imgSource[i+1][j]+2*imgSource[i][j-1]+imgSource[i+1][j-1])/9;	
			}
			//coin BD
			if( i==nrh && j==nch)
			{
				imgReturned[i][j] += (4*imgSource[i][j]+2*imgSource[i-1][j]+2*imgSource[i][j-1]+imgSource[i-1][j-1])/9;	
			}

			//cas classique
			imgReturned[i][j] += ( imgSource[i-1][j-1]+imgSource[i][j-1]+imgSource[i+1][j-1]+imgSource[i-1][j]+imgSource[i][j]+imgSource[i+1][j]+imgSource[i-1][j+1]+imgSource[i][j+1]+imgSource[i+1][j+1] )/9;

			
		}
	}
	return imgReturned;
}

int** gradientH(byte** imgSource, int nrl, int nrh, int ncl, int nch)
{
	int i,j;
	printf("gh1\n");
	int **imgReturned = imatrix(nrl,nrh,ncl,nch);
	printf("gh2\n");

	for (i = nrl; i <= nrh; i++)
	{
		for (j = ncl; j <= nch; j++)
		{
			printf("dbg : i:%d / j:%d", i, j);
			//coin HG
			if( i == 0 && j == 0)
			{
				imgReturned[i][j] += (-3*imgSource[i][j]+3*imgSource[i+1][j]+(-1*imgSource[i][j+1])+imgSource[i+1][j+1])/4;		
			}

			//coin HD
			if( i==nrh && j==0)
			{
				imgReturned[i][j] += (3*imgSource[i][j]+(-3)*imgSource[i-1][j]+imgSource[i][j+1]+(-imgSource[i-1][j+1]))/4;
			}
			
			//coin BG
			if( i==0 && j==nch)
			{
				imgReturned[i][j] += (-3*imgSource[i][j]+3*imgSource[i+1][j]-imgSource[i][j-1]+imgSource[i+1][j-1])/4;	
			}
			//coin BD
			if( i==nrh && j==nch)
			{
				imgReturned[i][j] += (3*imgSource[i][j]-3*imgSource[i-1][j]+imgSource[i][j-1]-imgSource[i-1][j-1])/4;	
			}

			//cas classique
			imgReturned[i][j] += ( -imgSource[i-1][j-1]+imgSource[i+1][j-1]-2*imgSource[i-1][j]+2*imgSource[i+1][j]-imgSource[i-1][j+1]+imgSource[i+1][j+1] )/4;

			
		}
	}
	return imgReturned;

}


int** gradientV(byte** imgSource, int nrl, int nrh, int ncl, int nch)
{
	int i,j;
	int **imgReturned = imatrix(nrl,nrh,ncl,nch);

	for (i = nrl; i <= nrh; i++)
	{
		for (j = ncl; j <= nch; j++)
		{
			//coin HG
			if( i == 0 && j == 0)
			{
				imgReturned[i][j] += (-3*imgSource[i][j]-imgSource[i+1][j]+3*imgSource[i][j+1]+imgSource[i+1][j+1])/4;		
			}

			//coin HD
			if( i==nrh && j==0)
			{
				imgReturned[i][j] += (-3*imgSource[i][j]-imgSource[i-1][j]+3*imgSource[i][j+1]+imgSource[i-1][j+1])/4;
			}
			
			//coin BG
			if( i==0 && j==nch)
			{
				imgReturned[i][j] += (3*imgSource[i][j]+imgSource[i+1][j]-3*imgSource[i][j-1]-imgSource[i+1][j-1])/4;	
			}
			//coin BD
			if( i==nrh && j==nch)
			{
				imgReturned[i][j] += (3*imgSource[i][j]+imgSource[i-1][j]-3*imgSource[i][j-1]-imgSource[i-1][j-1])/4;	
			}

			//cas classique
			imgReturned[i][j] += ( -imgSource[i-1][j-1]-2*imgSource[i][j-1]-imgSource[i+1][j-1]+imgSource[i-1][j+1]+2*imgSource[i][j+1]+imgSource[i+1][j+1] )/4;

		}
	}
	return imgReturned;
}


int** normeGradient(int** gradientH, int** gradientV, int nrl, int nrh, int ncl, int nch) {
	int i,j;
	int **normeG = imatrix(nrl,nrh,ncl,nch);

	for (i = nrl; i <= nrh; i++)
	{
		for (j = ncl; j <= nch; j++)
		{
			normeG[i][j] = floor(sqrt(pow(gradientH[i][j],2)+pow(gradientV[i][j],2)));
		}
	}

	return normeG;
}

float moyNormeGradient(int** gradient, int nrl, int nrh, int ncl, int nch) {
	float nbElts = 0.0, somme =0.0;
	int i,j;
	for (i = nrl; i <= nrh; i++)
	{
		for (j = ncl; j <= nch; j++)
		{
			somme += gradient[i][j];
			nbElts++;	
		}
	}
	return somme / nbElts;
}

byte** detectionContours(int** normeG, int seuil, int nrl, int nrh, int ncl, int nch) {
	int i,j;
	byte **imgReturned = bmatrix(nrl,nrh,ncl,nch);

	for (i = nrl; i <= nrh; i++)
	{
		for (j = ncl; j <= nch; j++)
		{
			if(normeG[i][j] >= seuil)
				imgReturned[i][j] = 255;
			else
				imgReturned[i][j] = 0;
		}
	}
	return imgReturned;
}

int* histogrammeNDG(byte** imgSource, int nrl, int nrh, int ncl, int nch) {
	int* histoNDG = malloc(sizeof(int)*256);
	int i,j;
	for (i = nrl; i <= nrh; i++)
	{
		for (j = ncl; j <= nch; j++)
		{
			histoNDG[imgSource[i][j]]++;
		}
	}
	return histoNDG;
}

int** histogrammeCouleurs(rgb8** imgSource, int nrl, int nrh, int ncl, int nch) {
	int** histoCouleur = malloc(sizeof(int*)*3);
	histoCouleur[0] = malloc(sizeof(int)*256);
	histoCouleur[1] = malloc(sizeof(int)*256);
	histoCouleur[2] = malloc(sizeof(int)*256);

	int i,j;
	for (i = nrl; i <= nrh; i++)
	{
		for (j = ncl; j <= nch; j++)
		{
			histoCouleur[0][imgSource[i][j].r]++;
			histoCouleur[1][imgSource[i][j].g]++;
			histoCouleur[2][imgSource[i][j].b]++;
		}
	}
	return histoCouleur;
}

int distanceEucliHistogrammesNDG(int* histoNDG1, int* histoNDG2) {
	int i;
	int sommeDiff = 0;
	for(i=0;i<256;i++)
	{
		sommeDiff += abs(histoNDG1[i]-histoNDG2[i]);
	}
	return sommeDiff;
}

int* distanceEucliHistogrammesC(int** histoCouleur1, int** histoCouleur2) {
	int i;
	int* sommeDiff = (int*) malloc(3*sizeof(int));
	for(i=0;i<256;i++)
	{
		sommeDiff[0] += abs(histoCouleur1[0][i]-histoCouleur2[0][i]);
		sommeDiff[1] += abs(histoCouleur1[1][i]-histoCouleur2[1][i]);
		sommeDiff[2] += abs(histoCouleur1[2][i]-histoCouleur2[2][i]);
	}
	return sommeDiff;
}

float* getProportionCouleur(int** histoCouleur) {
	float* propCouleurs = malloc(sizeof(float)*3);
	int i=0;
	for(i=0;i<256;i++){
	}
	return propCouleurs;
}
