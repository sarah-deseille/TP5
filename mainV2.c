#include <stdio.h> 
#include <string.h>
#include <assert.h>
#include <math.h>
#include "elt.h"

#define MAXCARS 128 //code ASCII (tab nomme data)


typedef struct {
    unsigned int nbElt; // nombre d'éléments (lettres) - nb effectif de nœuds dans le tas
    unsigned char tree[MAXCARS]; //minimier - tab qui contient les codes ASCII des lettres - table décrivant l’organisation des nœuds dans le tas
    int data[2*MAXCARS-1]; //de 0 à 127 contient les occurances des lettres - de 128 à 255 contient les noeuds créés - nombres d'occurrences des nœuds du tas
    int huffmanTree[2*MAXCARS-1]; //arbre de codage de Huffman
} T_indirectHeap;

typedef struct {
	unsigned int nbElt;
	unsigned int nbMaxElt;
	T_elt * tree;	
} T_heap;

//gcc main.c elt.c -o main

#include "include/traces.h" 
#include "include/check.h" 

#include "elt.h"

#define HEAP_ALLOCATION_OFFSET 5

#define iPARENT(i) 			(i-1)/2
#define iLCHILD(i) 			(2*i)+1
#define iRCHILD(i) 			(2*i)+2
#define iLASTINTERNAL(n)	n/2 -1
#define isINTERNAL(i,n) 	(2*i<(n-1))
#define isLEAF(i,n) 			(2*i>=(n-1))
#define isINTREE(i,n)		(i<n)
#define isROOT(i)				(i==0)
#define nbINTERNALS(n) 		n/2
#define nbLEAVES(n) 			ceil((double)n/2)
#define VALP(pHeap, i)		pHeap->tree[i]		
#define VAL(heap, i)			heap.tree[i]		

T_heap * newHeap(unsigned int nbMaxElt);
void freeHeap(T_heap *p); 
T_heap * initHeap(T_elt t[], int n) ;
void showHeap(T_heap *p);
void showHeap_rec(T_heap *p, int root, int indent); 

void swap(T_heap *p, int i, int j);


void siftDown(T_heap *p, int k);
T_elt getMin(const T_heap *p);
T_elt removeMin(T_heap *p);
void buildHeapV2(T_heap * p); 

void heapSortV2(T_heap *p);

void initIndirectHeap(T_indirectHeap * Mi);
int lettreToCode(char lettre);
T_indirectHeap analyserDocument(char * D);
void afficherData(T_indirectHeap * Mi);
void minimiserTree(T_indirectHeap * Mi);
void siftdownMinimier(T_indirectHeap *p);
//int eltCmp(T_elt e1, T_elt e2, T_indirectHeap * Mi)
void swap2(T_indirectHeap *p, int i, int j);
T_elt removeMin2(T_indirectHeap *p);
void afficherHuffmanTree(T_indirectHeap * Mi);
void remplissageHuffmanTree(T_indirectHeap * Mi, int C1, int C2, int Ni);
void showHeap2(T_indirectHeap *p);
void ajouternoeud(T_indirectHeap *p, int k);


int main(){

    //test pour la fonction analyserDocument
    T_indirectHeap Mi = analyserDocument("ABRADACABRA");
    afficherData(&Mi); //affiche le tableau des occurences des lettres


    //T_heap * minimier = NULL; 
	
	// int ex4[] = { 7, 15, 1, 9, 15, 10, 11, 4, 18, 6 }; //10
	//minimier=initHeap(ex4,10);
    int indice = 128; 
    while(Mi.nbElt > 1){ 
	    siftdownMinimier(&Mi);
        if (indice==128){
            showHeap2(&Mi);
            afficherHuffmanTree(&Mi);
            afficherData(&Mi);
        }
        T_elt C1 = removeMin2(&Mi); //recup C1
        printf("affichage du tree après avoir retirer C1\n");
        showHeap2(&Mi);
	   // siftdownMinimier(&Mi);        
        T_elt C2 = removeMin2(&Mi); //recup C2
        printf("affichage du tree après avoir retirer C2\n");
        showHeap2(&Mi);
        printf("\n\n");
        //Mi.tree[Mi.nbElt] = indice; //ajout de Ni
        //Mi.nbElt++;//création du nouv noeud
        remplissageData(&Mi, C1, C2, indice); //remplissage de data
        ajouternoeud(&Mi, indice);
        remplissageHuffmanTree(&Mi, C1, C2, indice);
        indice++;
        printf("\n\n");
        printf("Mi.nbElt : %d\n", Mi.nbElt);
        afficherData(&Mi);
        afficherHuffmanTree(&Mi);
        showHeap2(&Mi);
    }
    hasherLettre(&Mi);



    //showHeap(Mi->tree);
    /*
    buildHeapV2(minimier);
    showHeap(minimier);

    //tester la fonction removeMin
    T_elt min = removeMin(minimier);
    printf("Min : %s\n", toString(min));
    showHeap(minimier);*/
  return 0;
}

void showHeap(T_heap *p) {

	assert(p!=NULL);
	printf("Affichage du tas (nbElt : %d)\n",p->nbElt);
	showHeap_rec(p,0,0); 
	
}

void showHeap_rec(T_heap *p, int root, int indent) {
	// affiche récursivement en mode pseudo graphique 
	// t : tas
	// n : taille du tas
	// root : indice de la racine du sous-arbre à afficher
	// indent : décalage à appliquer lors de l'affichage 

	if (isINTREE(root,p->nbElt)) { //si root < n donc si root est dans le tas
		showHeap_rec(p,iRCHILD(root),indent+8);//on affiche le fils droit
		printf("%*s%s\n",indent,"",toString(VALP(p,root)));//on affiche la racine
		showHeap_rec(p,iLCHILD(root),indent+8);//on affiche le fils gauche
	}

}


/**************************************************************************************
*** Nom : T_heap * newHeap(unsigned int nbMaxElt)
*** Description : Créer un nouveau tas 
**************************************************************************************/
T_heap * newHeap(unsigned int nbMaxElt){

	T_heap * pAux = NULL; // tas vide
	
	if(nbMaxElt > 0){ // si le tas doit contenir au moins un élément
       pAux = (T_heap *) malloc(sizeof(T_heap));//on alloue de la mémoire pour le tas
       
	   if(pAux != NULL) {//si l'allocation a réussi
           pAux->nbElt = 0; //le tas est vide
           pAux->nbMaxElt = nbMaxElt;//on initialise le nombre maximal d'éléments
           pAux->tree = (T_elt *) malloc(sizeof(T_elt) * nbMaxElt);//on alloue de la mémoire pour le tableau d'éléments

           if(pAux->tree == NULL){ //si l'allocation a échoué
               free(pAux); //on libère la mémoire allouée pour le tas
               pAux = NULL; 
            }
        }
    }
	return pAux; 

}

/**************************************************************************************
*** Nom : void freeHeap(T_heap *p)
*** Description : Libérer l'espace réservé pour le tas
****************************************************************************************/
void freeHeap(T_heap *p) {

	if(p != NULL) {//si le tas existe
		if(p->tree != NULL) {//si le tableau d'éléments existe
			free(p->tree);//on libère la mémoire allouée pour le tableau d'éléments
		}
		free(p);//on libère la mémoire allouée pour le tas
	}

}


/****************************************************************************************
*** Nom : T_heap * initHeap(T_elt t[], int n)
*** Description : Permet d'initialiser un tas 
*****************************************************************************************/

T_heap * initHeap(T_elt t[], int n) {
	T_heap *pAux = newHeap(n + HEAP_ALLOCATION_OFFSET); //création d'un tas 
	
	if(pAux != NULL) { //si l'allocation a réussi
       pAux->nbElt = n; //on initialise le nombre d'éléments
       memcpy(pAux->tree, t, sizeof(T_elt) * n); //on copie les éléments du tableau t dans le tableau d'éléments du tas
   	}

	return pAux; 
}

/**************************************************************************************************
*** Nom : void swap(T_heap *p, int i, int j)67
*** Description : Echange de deux valeurs du tas
****************************************************************************************************/
void swap(T_heap *p, int i, int j) {

	T_elt aux; //variable auxiliaire pour l'échange
	aux = VALP(p,i); //on stocke la valeur de l'élément i dans aux

	VALP(p,i) = VALP(p,j); //on affecte la valeur de l'élément j à l'élément i
	VALP(p,j) = aux; //on affecte la valeur de aux à l'élément j

}


/***********************************************************************************************************
*** Nom : T_elt getMin(const T_heap *p)
*** Description : permet d’extraire l’élément min du tas
******************************************************************************************************/

T_elt getMin(const T_heap *p){


   return p->tree[0];
}
   


//prototype : 
/*huffman(D)
  Mi = analyserDocument(D)	// Comptage des occurrences
  Ht = initHuffmanTree() // Initialisation de l’arbre de codage
  transformerEnMinimierV2(Mi) // Réorganisation du minimier
  Pour i = 1 jusque n-1 			
 C1=extraireMin(Mi) // Extraire et réorganiser
 C2=extraireMin(Mi) // Extraire et réorganiser
 AjouterNoeud(Ht,Ni) // Ajout dans l’arbre de codage
 insererMI(Mi,Ni,VAL(C1)+VAL(C2)) // Insérer et réorganiser
*/

/*légende : 
Mi : minimier indirect
Ht : arbre de codage de Huffman
C1, C2 : caractères extraits
Ni : noeud inséré*/

//fonction pour init T_indirectHeap
void initIndirectHeap(T_indirectHeap * Mi) {
    Mi->nbElt = 0; //initialisation du nombre d'éléments
    for (int i = 0; i < MAXCARS*2-1; i++) { //initialisation du tableau data de 0 à 128 soit du nb d'occurances des lettres
        Mi->data[i] = 0;
        Mi->huffmanTree[i] = -256; //initialisation de l'arbre de codage de Huffman
    }
    for (int i = 0; i < MAXCARS; i++) Mi->tree[i] = 0; //initialisation du minimier
}

//fonction pour transformer une lettre en son code ascii
//param : lettre
//return : code ascii
int lettreToCode(char lettre) {
    return (int) lettre;
}

//fonction qui compte les occurences des lettres
//param : chaine de caractères
T_indirectHeap analyserDocument(char * D) {
    T_indirectHeap Mi;
    int i;
    initIndirectHeap(&Mi); //initialisation
    //je parcours la chaine de caractères
    for (i = 0; i < strlen(D); i++) {
        //je récupère la lettre et je l'a converti en code ASCII
        int code = lettreToCode(D[i]);
        //j'incrémente le nombre d'occurences de la lettre à la position code
        Mi.data[code]++;     
    }
    //parcourir data afin de remplir tree
    for (i = 0; i < MAXCARS; i++) {
        //dans data, il n'y a qu'une seule fois A avec un nb d'occurences de 5 par exemple
        if (Mi.data[i] != 0) { //si le nombre d'occurences est différent de 0
            //ajoute dans tree l'indice i
            Mi.tree[Mi.nbElt] = i; //j'ajoute dans tree, à l'indice trouvé dans Mi.nbElt la lettre
            Mi.nbElt++; //si j'ajoute qqc dans tree, le nombre d'éléments augmente
        }
    }
    return Mi;
}


void afficherData(T_indirectHeap * Mi) {
    int i;
    for (i = 0; i < (MAXCARS*2)-1; i++) {
        if (Mi->data[i] != 0) { //si le nombre d'occurences est différent de 0
            printf("Lettre : %c, Occurences : %d\n", (char) i, Mi->data[i]); //j'affiche la lettre et le nombre d'occurences
        }
    }
}

//fonction initHuffmanTree
T_heap * initHuffmanTree() {
    //j'init la var globale huffmanTree
    //TODO
    return NULL;
}

void buildHeapV2(T_heap * p){
    //transformer le tableau en minimier
    for (int i = iLASTINTERNAL(p->nbElt); i >= 0; i--) {
        siftDown(p, i);
    }
}


T_elt removeMin(T_heap *p) {
    T_elt aux; 
    aux = p->tree[0];
    p->tree[0] = p->tree[p->nbElt-1];
    p->nbElt--;
    siftDown(p,0);
    return aux; 
}


void siftDown(T_heap *p, int k) {

    int i = k;
    while (isINTERNAL(i,p->nbElt) && eltCmp(VALP(p,i),VALP(p,iLCHILD(i)))>0) {
        int j = iLCHILD(i);
        if (isINTREE(iRCHILD(i),p->nbElt) && eltCmp(VALP(p,iRCHILD(i)),VALP(p,j))<=0) {
            j = iRCHILD(i);
        }
        swap(p,i,j);
        i = j;
    }
}

void minimiserTree(T_indirectHeap * Mi) {
    for (int i = iLASTINTERNAL(Mi->nbElt); i >= 0; i--) {
        siftDown(Mi, i);
    }
}

/*int eltCmp(T_elt e1, T_elt e2, T_indirectHeap * Mi) {
    return Mi->data[e1] - Mi->data[e2];
}*/


void siftdownMinimier(T_indirectHeap *p){
    // int i = 0;
    // while(isINTERNAL(k,p->nbElt)){
    //     i = iLCHILD(k);
    //     if(i+1 < p->nbElt && eltCmp(p->data[VALP(p,i+1)],p->data[VALP(p,i)] < 0)){
    //         i++;
    //     }
    //     if(eltCmp(VALP(p,k),VALP(p,i))<=0){
    //         break;
    //     }
    //     swap(p,k,i);
    //     k=i;
    // }

    // //trier le code ascii des lettres de tree en utilisant le nombre d'occurancesqui se trouve à la position du code ascii de data
    for (int i = 1; i < p->nbElt; i++) {//commence 1 car racine a pas de parent
        while ((i > 0) && (p->data[p->tree[i]] < p->data[p->tree[iPARENT(i)]])) {
            swap2(p, i, iPARENT(i));
            i = iPARENT(i);
        }    
    }
}

void swap2(T_indirectHeap *p, int i, int j) {
    int aux; //variable auxiliaire pour l'échange
    aux = p->tree[i]; //on stocke la valeur de l'élément i dans aux
    p->tree[i] = p->tree[j]; //on affecte la valeur de l'élément j à l'élément i
    p->tree[j] = aux; //on affecte la valeur de aux à l'élément j
}

T_elt removeMin2(T_indirectHeap *p) {
    T_elt aux; 
    aux = p->tree[0];
    p->tree[0] = p->tree[p->nbElt-1];
    p->nbElt--;
    siftdownMinimier(p);
    return aux; 
}

void showHeap2(T_indirectHeap *p) {
    assert(p!=NULL);
    printf("Affichage du tas (nbElt : %d)\n",p->nbElt);
    for (int i = 0; i < p->nbElt; i++) {
        printf("%d ", p->tree[i]);
    }
    printf("\n");
}

void remplissageData(T_indirectHeap * Mi, int C1, int C2, int Ni) {
    Mi->data[Ni] = Mi->data[C1] + Mi->data[C2];
}

void remplissageHuffmanTree(T_indirectHeap * Mi, int C1, int C2, int Ni) {
    Mi->huffmanTree[C1] = 0-Ni;
    Mi->huffmanTree[C2] = Ni;
}

void afficherHuffmanTree(T_indirectHeap * Mi){
    int i;
    for (i = 0; i < (MAXCARS*2)-1; i++) {
        if (Mi->huffmanTree[i] != -256) { //si le nombre d'occurences est différent de 0
            printf("racine : %d, fils g/d: %d", i, Mi->huffmanTree[i]); //j'affiche la lettre et le nombre d'occurences
        }
    }
}

void hasherLettre(T_indirectHeap * Mi){

    //on part de tree[0] pour remonter tout le SAG et SAB en ajoutant 0 quand on va à gauche et 0 quand on va à droite
    //SAG = sous arbre-gauche
    //SAD = sous arbre-droit

    int racine = Mi->tree[0];
    int feuille = 0; //curseur qui se déplace
    int aux; 
    while (feuille<128){
        while (Mi->data[feuille]==0 && feuille < 128) feuille++;
        if (feuille==128) return;
        printf("feuille %d \n", feuille);
        printf ("lettre: %c\n", Mi->tree[feuille]);
        getchar();
        aux=feuille;
        feuille++;
        while (aux!=racine){
            if (Mi->huffmanTree[aux]>0){
                printf ("1");
                aux=Mi->huffmanTree[aux];
            }
            else{
                printf ("0");
                aux=absolute(Mi->huffmanTree[aux]);
            }
        }
        printf ("\n");
    }
}

int absolute(int k){
    if (k<0) return -k;
    else return k;
}

void ajouternoeud(T_indirectHeap *p, int k){
//but : ajouter le noeud créé à tree, k = indice noeud créé 
    p->tree[p->nbElt] = k; //ajout de Ni
    p->nbElt++;//création du nouv noeud
    //swap2(p,k,0);
    siftdownMinimier(p);
}