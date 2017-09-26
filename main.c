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
float* getProportionCouleur(int* histoR, int* histoV, int* histoB);



int main(void)
{

	printf("Debut programme\n");

	//déclaration image
	rgb8 **I;
	long nrh,nrl,nch,ncl;

	//binarisation d'une image
	I = LoadPPM_rgb8matrix("Images/arbre1.ppm",&nrl,&nrh,&ncl,&nch);	
	byte **IBinarisee = c(I,nrl,nrh,ncl,nch);
	SavePGM_bmatrix(IBinarisee,nrl,nrh,ncl,nch,"Images/binarisees/arbre1.pgm"); 
	free_bmatrix(IBinarisee,nrl,nrh,ncl,nch);
	free_rgb8matrix(I,nrl,nrh,ncl,nch);

	printf("Fin du programme\n");
	return 0;
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
	int **imgReturned = imatrix(nrl,nrh,ncl,nch);

	for (i = nrl; i <= nrh; i++)
	{
		for (j = ncl; j <= nch; j++)
		{
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
	int* histoNDG = (int) malloc(sizeof(int)*256);
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
	int** histoCouleur = (int) malloc(sizeof(int*)*3);
	histoCouleur[0] = (int) malloc(sizeof(int)*256);
	histoCouleur[1] = (int) malloc(sizeof(int)*256);
	histoCouleur[2] = (int) malloc(sizeof(int)*256);

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

