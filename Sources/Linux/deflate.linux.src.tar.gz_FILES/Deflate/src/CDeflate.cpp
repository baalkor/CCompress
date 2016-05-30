#include "CDeflate.h"

//Données de test pour la fonction AutoTest()
unsigned char	test_alpha[] = "test test test test";	//Chaîne de caractères a compresser
int				test_alen = 19;							//Longeurs de la chaîne 
unsigned char	test_comp[] = {0x2b, 0x49, 0x2d, 0x2e, 0x51, 0x40, 0x25, 0x00};
int				test_clen = 8;

//*********************************************************
//*                      Comp & CompAlp                   *
//*********************************************************
//* Comparaisons pour le tris des tableau                 *
//*********************************************************
int Comp(const void *a, const void *b)
{
	return ((SAlp*)a)->Nb - ((SAlp*)b)->Nb;
}

int CompAlp(const void *a, const void *b)
{
	return ((SAlp*)a)->Pos - ((SAlp*)b)->Pos;
}

//*********************************************************
//*                      CDeflate                         *
//*********************************************************
//* Constructeur de la class CDeflate                     *
//* Initialise les deux arbres de Huffman pour les        *
//* distances et les littéraux/longueurs.                 *
//*********************************************************

CDeflate::CDeflate()
{
	int cpt;

	//Remplisage du tableau de correspondance des symoles de longueurs
	LenSymb[0] = 3;
	LenSymb[1] = 4;
	LenSymb[2] = 5;
	LenSymb[3] = 6;
	LenSymb[4] = 7;
	LenSymb[5] = 8;
	LenSymb[6] = 9;
	LenSymb[7] = 10;
	LenSymb[8] = 11;
	LenSymb[9] = 13;
	LenSymb[10] = 15;
	LenSymb[11] = 17;
	LenSymb[12] = 19;
	LenSymb[13] = 23;
	LenSymb[14] = 27;
	LenSymb[15] = 31;
	LenSymb[16] = 35;
	LenSymb[17] = 43;
	LenSymb[18] = 51;
	LenSymb[19] = 59;
	LenSymb[20] = 67;
	LenSymb[21] = 83;
	LenSymb[22] = 99;
	LenSymb[23] = 115;
	LenSymb[24] = 131;
	LenSymb[25] = 163;
	LenSymb[26] = 195;
	LenSymb[27] = 227;
	LenSymb[28] = 258;

	//Remplisage du tableau de correspondance des symoles de distance
	DistSymb[0] = 1;
	DistSymb[1] = 2;
	DistSymb[2] = 3;
	DistSymb[3] = 4;
	DistSymb[4] = 5;
	DistSymb[5] = 7;
	DistSymb[6] = 9;
	DistSymb[7] = 13;
	DistSymb[8] = 17;
	DistSymb[9] = 25;
	DistSymb[10] = 33;
	DistSymb[11] = 49;
	DistSymb[12] = 65;
	DistSymb[13] = 97;
	DistSymb[14] = 129;
	DistSymb[15] = 193;
	DistSymb[16] = 257;
	DistSymb[17] = 385;
	DistSymb[18] = 513;
	DistSymb[19] = 769;
	DistSymb[20] = 1025;
	DistSymb[21] = 1537;
	DistSymb[22] = 2049;
	DistSymb[23] = 3073;
	DistSymb[24] = 4097;
	DistSymb[25] = 6145;
	DistSymb[26] = 8193;
	DistSymb[27] = 12289;
	DistSymb[28] = 16385;
	DistSymb[29] = 24577;

	//Remplisage du tableau de l'ordre de stockage des codes de longueur pour l'encodage dynamique
	CLeOrder[0] = 16;
	CLeOrder[1] = 17;
	CLeOrder[2] = 18;
	CLeOrder[3] = 0;
	CLeOrder[4] = 8;
	CLeOrder[5] = 7;
	CLeOrder[6] = 9;
	CLeOrder[7] = 6;
	CLeOrder[8] = 10;
	CLeOrder[9] = 5;
	CLeOrder[10] = 11;
	CLeOrder[11] = 4;
	CLeOrder[12] = 12;
	CLeOrder[13] = 3;
	CLeOrder[14] = 13;
	CLeOrder[15] = 2;
	CLeOrder[16] = 14;
	CLeOrder[17] = 1;
	CLeOrder[18] = 15;

	SLLTree = new SHuffTree;
	SDTree  = new SHuffTree;

	SLLTree->Tree = new SHuffman[287];
	SDTree->Tree = new SHuffman[31];

	for (cpt=0;cpt<=15;cpt++)
		SLLTree->Nb[cpt] = 0;

	for (cpt=0;cpt<=143;cpt++)
		SLLTree->Tree[cpt].Len = 8;

	for (cpt=144;cpt<=255;cpt++)
		SLLTree->Tree[cpt].Len = 9;

	for (cpt=256;cpt<=279;cpt++)
		SLLTree->Tree[cpt].Len = 7;

	for (cpt=280;cpt<=287;cpt++)
		SLLTree->Tree[cpt].Len = 8;

	Huffman(SLLTree, 287);

	for (cpt=0;cpt<=5;cpt++)
		SDTree->Nb[cpt] = 0;

	for (cpt=0;cpt<=31;cpt++)
		SDTree->Tree[cpt].Len = 5;

	Huffman(SDTree, 31);

	BufferIn = new unsigned char[BUFFER_LEN];
	BufferInLen = 0;
	BufferOut = new unsigned char[BUFFER_LEN];
	BufferOutLen = 0;
    BufferOutTotLen = BUFFER_LEN;

	State.CompType = COMP_UNDEF;
	State.InBloc = false;
	State.OutPos = 0;
	State.UnCompLen = 0;
	State.BitsBuff = 0;
	State.BitsBuffLen = 0;
	State.LastBloc = false;
	State.BytesRead = 0;
	State.CellNum = 0;
	State.NbBigLit = 0;
	State.BlocNum = 0;

	for(cpt=0; cpt<MAX_CELLS; cpt++)
		TmpBloc[cpt] = NULL;

	for(cpt=0; cpt<MAX_BLOCS; cpt++)
		Blocs[cpt] = NULL;
}


//*********************************************************
//*                      ~CDeflate                        *
//*********************************************************
//* Destructeur de la class, suprime les buffers et libère*
//* la mémoire.                                           *
//*********************************************************

CDeflate::~CDeflate()
{
	delete(BufferIn);
	delete(BufferOut);
}

//*********************************************************
//*                      AutoTest                         *
//*********************************************************
//* Fonction permattant de tester le fonctionnement       *
//* correct de la classe.                                 *
//* Cette fonction compresse et décompresse et compare    *
//* le résultat et retourne une erreur si la chaine ne    * 
//* coresspond pas.                                       *
//*********************************************************

int	 CDeflate::AutoTest()
{
     //Zone mémoire ou les données on été compreséés
	unsigned char*	Comp;
	unsigned char*	DeComp;

    //Compression des données définie dans la variable TEST_ALPHA
	Comp = Compress(test_alpha,test_alen);

    //Si la compression échoue alors retourne 1
    if(memcmp(Comp, test_comp, test_clen) != 0)
       return 1;
    else {
         
         //Décompresse la zone mémoire
         DeComp = DeCompress(Comp,test_clen);
         //Si le résultatde la comparaison des deux chaine est différents retourn 2
         if (memcmp(DeComp,test_alpha, test_clen) != 0)
            return 1;
    }
    
    //Supression des pointeurs
    delete(Comp);
	delete (DeComp);
	
	//Test passé sans erreur retourne 0
    return 0;
}

//*********************************************************
//*                      Compress                         *
//*********************************************************
//* Compression des données en mémoire                    *
//*********************************************************

unsigned char* CDeflate::Compress(unsigned char* Data, unsigned long Len)
{
	BufferIn = Data;
	BufferInLen = Len;

	//Compresse les données du buffer
	Deflate();

	//Finalise la compression
	Final();

	return BufferOut;
}

//*********************************************************
//*                      DeCompress                       *
//*********************************************************
//* Décompression des données en mémoire                  *
//*********************************************************

unsigned char* CDeflate::DeCompress(unsigned char* Data, unsigned long Len)
{
	BufferIn = Data;
	BufferInLen = Len;

	//Compresse les données du buffer
	Inflate();

	//Renvoit le buffer de sortie
	return BufferOut;
}

//*********************************************************
//*                      CompressFile                     *
//*********************************************************
//* Compresse le fichier In et écrit dans Out             *
//*********************************************************

int CDeflate::CompressFile(FILE* In, FILE* Out)
{
	unsigned long	LenTmp;		//Longueur restente a lire
	
	//Lit la taille du fichier
	//Met le curseur de lecture à la fin
	//Si le cursuer ne peut être positioné, erreur du fichier
	if(fseek(In, 0, SEEK_END) != 0)
		return 1;
	//Lit la taille
	LenTmp = ftell(In);
	//Remet le curseur au début pour la lecture
	fseek(In, 0, SEEK_SET);

	BufferIn = new unsigned char[LenTmp];
	//Copie les Données dans tampon
	fread(BufferIn, sizeof(unsigned char), LenTmp, In);

	BufferInLen = LenTmp;

	//Compresse les données du buffer
	Deflate();

	//Finalise la compression
	Final();
	//Écrit les données dans le fichier de sortie
	fwrite(BufferOut, sizeof(char), BufferOutLen, Out);

	return 0;
}

//*********************************************************
//*                      DeCompressFile                   *
//*********************************************************
//* Decompresse le fichier In et écrit dans Out           *
//*********************************************************

int CDeflate::DeCompressFile(FILE* In, FILE* Out)
{
	unsigned long	LenTmp;		//Longueur restente a lire
	
	//Lit la taille du fichier
	//Met le curseur de lecture à la fin
	//Si le cursuer ne peut être positioné, erreur du fichier
	if(fseek(In, 0, SEEK_END) != 0)
		return 1;
	//Lit la taille
	LenTmp = ftell(In);
	//Remet le curseur au début pour la lecture
	fseek(In, 0, SEEK_SET);

	BufferIn = new unsigned char[LenTmp];

	//Copie les Données dans tampon
	fread(BufferIn, sizeof(unsigned char), LenTmp, In);

	BufferInLen = LenTmp;

	//Decompresse les données du buffer
	Inflate();

	//Écrit les données dans le fichier de sortie
	fwrite(BufferOut, sizeof(char), BufferOutLen, Out);

	return 0;
}

//*********************************************************
//*                      GetFinalLen                      *
//*********************************************************
//* Renvoit la longueur de données de BufferOut           *
//*********************************************************

unsigned long CDeflate::GetFinalLen()
{
	//Renvoit la longueur des données
	return BufferOutLen;
}

//*********************************************************
//*                      Inflate                          *
//*********************************************************
//* Decompression, fonction de bas niveau                 *
//*********************************************************

int CDeflate::Inflate()
{
unsigned short LenTemp;	//Longueur pour les données non compressées
unsigned int Val;		//Valeur décodée
unsigned int len;		//Longueur de répétition
unsigned int dist;		//Distance de répétition
unsigned int NbLitCodes;//Nombre de codes dans l’arbre des littéraux
unsigned int NbDstCodes;//Nombre de codes dans l’arbre des distances
unsigned int NbCLeCodes;//Nombre de codes dans l’arbre des codes de longeurs de l’arbre des littéraux
unsigned int i;
unsigned int LenTab[319];//Tableau temporaire pour les longueurs de codes dynamiques

	BufferOut = new unsigned char[BUFFER_LEN];
	BufferOutTotLen = BUFFER_LEN;
	BufferOutLen = 0;

	//Rement le nombre de bytes lue dans le buffer à 0
	State.BytesRead = 0;

	while(State.BytesRead < BufferInLen)
	{
		//Aucun bloc n'est en cours, un nouveau débute
		if(!State.InBloc)
		{
			State.LastBloc = GetBits(1);	//Premier bit, marqeur de derrnier bloc
			State.CompType = GetBits(2);	//Deux bits, type de compression (00, 01, 10)
			State.InBloc = true;
		}

		//Le Bloc n'est pas compressé
		if(State.CompType == COMP_00)
		{
			memcpy(&LenTemp, BufferIn+State.BytesRead, 2);
			State.BytesRead += 4;

			//Copie dans la sortie
			for(i=0; i<LenTemp; i++)
			{
				Out(*(BufferIn+State.BytesRead));
				State.BytesRead ++;
			}
		}
		else
		{
			if(State.CompType == COMP_10)
			{
				NbLitCodes = GetBits(5) + 257;	//Nombre de litteraux
				NbDstCodes = GetBits(5) + 1;	//Nombre de code de distance
				NbCLeCodes = GetBits(4) + 4;	//Nombre de code de longueur

				//Création des arbres
				DCLTree = new SHuffTree;
				DCLTree->Tree = new SHuffman[20];

				DLLTree = new SHuffTree;
				DLLTree->Tree = new SHuffman[287];

				DDTree = new SHuffTree;
				DDTree->Tree = new SHuffman[31];

				//Lit les codes de longueur des codes de longueur (non, il n'y a pas de faute de frappe)
				for(i=0; i<NbCLeCodes; i++)
					DCLTree->Tree[CLeOrder[i]].Len = GetBits(3);
				for(i=i; i<20; i++)
					DCLTree->Tree[CLeOrder[i]].Len = 0;

				Huffman(DCLTree, 19);

				i = 0;
				while(i<NbLitCodes+NbDstCodes)
				{
					Val = Decode(DCLTree);
					//Il s'agit d'une longueur de code
					if(Val < 16)
					{
						LenTab[i] = Val;
						i ++;
					}
					else
					{ //Une répétition a lieu
						len = 0;		//Par défaut répète des 0
						if(Val == 16)	//répétition de 3 à 6 fois la derrnière longueur
						{
							len = LenTab[i-1];	//Derrnière valeur, celle à répéter
							Val = GetBits(2) +3;//Complément au nombre de répétitions
						}
						else 
							if(Val == 17)		//Répétition de 0, de 3 à 10 fois
								Val = GetBits(3) +3;//Complément au nombre de répétitions
							else				//Répétition de 0, de 11 à 138 fois
								Val = GetBits(7) +11;//Complément au nombre de répétitions
						while(Val--)
						{	//Écrit la répétition
							LenTab[i] = len;
							i ++;
						}
					}
				}

				//Copie les longueurs dans les arbres
				for(i=0; i<NbLitCodes; i++)
					DLLTree->Tree[i].Len = LenTab[i];
				for(i=i; i<(NbDstCodes+NbLitCodes); i++)
					DDTree->Tree[i-NbLitCodes].Len = LenTab[i];

				//Calcule les codes des symboles
				Huffman(DLLTree, NbLitCodes);
				Huffman(DDTree, NbDstCodes);

				//Définit les arbres à utiliser
				State.TreeL = DLLTree;
				State.TreeD = DDTree;

				//Supprime l'arbre des longueurs de code
				delete(DCLTree->Tree);
				delete(DCLTree);
			}
			else
			{	//Compression statique, pointe sur les arbres statiques
				State.TreeL = SLLTree;
				State.TreeD = SDTree;
			}
			do
			{
				//Décode le prochain symbole
				Val = Decode(State.TreeL);
				
				//Si la valeur est un littéral
				if(Val < 256)
					Out(Val);

				//Si Val est une longueur
				if(Val > 256)
				{
					//Décode la longueur
					len = LenSymb[Val-257];
					//Ajoute le complément si il y a
					len += GetBits(LenExtra(Val));

					//Décode la distance
					Val = Decode(State.TreeD);
					dist = DistSymb[Val];
					//Ajoute le complément si il y a
					dist += GetBits(DistExtra(Val));

					while(len--)
						Out(*((BufferOut+BufferOutLen)-dist));
				}
			}while((Val!=256)&&(State.BytesRead < BufferInLen));

			//Le bloc est finit, remet les arbres à 0
			if(Val==256)
			{
				if(State.CompType == COMP_10)
				{
					delete(State.TreeL->Tree);
					delete(State.TreeL);
					delete(State.TreeD->Tree);
					delete(State.TreeD);
				}

				State.TreeD = NULL;
				State.TreeL = NULL;

				State.InBloc = false;
			}
		}
	}

	return 0;
}

//*********************************************************
//*                      Deflate                          *
//*********************************************************
//* Compression, fonction de bas niveau                   *
//*********************************************************

int CDeflate::Deflate()
{
	unsigned int	Len =0;		//Longueur trouvée
	unsigned int	Dist =0;	//Position trouvée
	unsigned char*	Pos;		//Position actuelle de la recherche dans le buffer
	unsigned char*	PosGen;		//Position général dans le buffer, valeur recherchée
	unsigned int	i, j =0;	//Compteurs

	PosGen = BufferIn;	//Position générale = début de BufferIn

	//Tant qu'on est pas à la fin du buffer de données
	while(PosGen < (BufferIn + BufferInLen))
	{	
		if(PosGen-1 < BufferIn)
			Pos = PosGen;
		else
			Pos = PosGen-1;

		Len = 0;
		Dist = 0;

		//Boucle de recherche
		for(i=1; i<=32768; i++)
		{
			j = 0;	//Compteur de longueur

			//Tant que les characères se répète
			while( (*(PosGen+j) == *(Pos+j)) && (PosGen != BufferIn) )
			{
				j ++;	//Incrémentation de la longueur
				//Si on arrive à la fin des données, recherche terminée pour le moment
				if((PosGen+j) > (BufferIn+BufferInLen)) {break;}
				//Si on sort du buffer dans le quel on recherche, on arrête
				if(Pos+j > (BufferIn+BufferInLen)) {break;}
				if(j==258)
					break;
			}
			//Si la longueur trouvé est plus grande que celle trouvée avant on l'enregistre
			if(j>Len)
			{
				Dist = i;
				Len = j;
			}

			//Si Le pointeur de recherche dépasse le début des données actuelles
			if((Pos-1) < (BufferIn))
				break;
			else
				Pos --;
		}

		//Longueur de répétition trop courte
		if(Len<3)
		{
			Dist = 0;
			Len = 0;
		}

		if (State.CompType == COMP_UNDEF)//Si mode de compression non défini
			State.CompType = COMP_COMP;//Met type de compression a Données compresséés.

		if (State.CompType == COMP_00)
		{
			if ((Len == 0) && (BufferOutLen < 32768))
			{
				Out(*(PosGen));
			}
			else 
			{     
				DefFinaliseBloc();
		        	
				//passe en mode compressé
				State.CompType = COMP_COMP;
				State.CellNum --;

				//Nouvuea bloc
				TmpBloc[State.CellNum] = new BlocCell;
				TmpBloc[State.CellNum]->Lit = false;
		        
				//Copie des longeurs et disatnces dans le tableau TmpBloc
				TmpBloc[State.CellNum]->LitLen = Len;
				TmpBloc[State.CellNum]->Dist = Dist;
			} 
		}
		else	//Compression (01 ou 10)
		{
			//Si len et Dist sont a zéro il s'agit d'un littéral
			if ((Len == 0) || (Dist == 0))
			{
				if(TmpBloc[State.CellNum] != NULL)
					delete(TmpBloc[State.CellNum]);

				TmpBloc[State.CellNum] = new BlocCell;

				//Affecte lui la valeur vrai car c'est un littéral
				TmpBloc[State.CellNum]->Lit = true;
				//Copie du littéral dans le tabealue temporaire.
				TmpBloc[State.CellNum]->LitLen = *PosGen;  

				//Si littéral codé sur 9 bits
				if ((TmpBloc[State.CellNum]->LitLen >= 144) && (TmpBloc[State.CellNum]->LitLen <= 255))
					State.NbBigLit++;
				//Si le nombre de grand littéraux consécutif > 17
				if (State.NbBigLit > 17)
				{
					DefFinaliseBloc();

					//Passe en mode non compressé
					State.CompType = COMP_00;
					State.NbBigLit = 0;
				}
			}
			else
			{
				//Copie de la longeur et des disantces dans le tableau temporaire
				TmpBloc[State.CellNum] = new BlocCell;
				TmpBloc[State.CellNum]->Lit = false;
				TmpBloc[State.CellNum]->LitLen = Len;
				TmpBloc[State.CellNum]->Dist = Dist;
			}
			State.NbBigLit = 0;
		}

		if(State.CompType != COMP_00)
			State.CellNum ++;
		PosGen ++;
		if(Len>0)
			PosGen += Len-1;
	}

	DefFinaliseBloc();

	return 0;
}

//*********************************************************
//*                      DefFinaliseBloc                  *
//*********************************************************
//* Finalise le bloc en cour et en créer un nouveau       *
//*********************************************************

int	CDeflate::DefFinaliseBloc()
{
unsigned int i;
unsigned short	TempLen;
SAlp			TabAlp[288]; //Tableau dans lequelle sont stocké les nombre de littéeaux par taille
SAlp			TabAlpD[31];
unsigned long	Somme;

	if (State.CompType == COMP_00)
	{
		Blocs[State.BlocNum] = new unsigned char[BufferOutLen];
		BlocsLen[State.BlocNum] = BufferOutLen;

		/*****************************************
		Écriture de la longeur du bloc et du complément a 1 du bloc.
		*****************************************/

		//Écrit le type de compression
		TempLen = 0;
		memcpy(Blocs[State.BlocNum], &TempLen, 1);

		//Écrtiture de la taille des données dans le bloc
		TempLen = BufferOutLen;
		memcpy(Blocs[State.BlocNum]+1, &TempLen, 2);

		//Complément a 1 de la taille totale
		TempLen = ~TempLen;
		memcpy(Blocs[State.BlocNum]+3, &TempLen, 2);

		//Copie les données depuis le buffer dans le buffer final
		memcpy(Blocs[State.BlocNum]+5, BufferOut, BufferOutLen);

		//Vidage du bloc de sortie 
		delete(BufferOut);
	}else	//Compressé
	{
		//Initialise le tableau de l'alphabet
		for (i=0;i<=287;i++)
		{
			TabAlp[i].Pos = i;
			TabAlp[i].Nb = 0;
		}
		for (i=0;i<=30;i++)
		{
			TabAlpD[i].Pos = i;
			TabAlpD[i].Nb = 0;
		}

		//Parcour les cellules
		for (i=0;i<State.CellNum;i++)
		{
			//Code et complément de longueur
			if(!TmpBloc[i]->Lit)
			{
				GetLenVal(TmpBloc[i]->LitLen, TmpBloc[i]);
				GetDistVal(TmpBloc[i]->Dist, TmpBloc[i]);
				TabAlpD[TmpBloc[i]->Dist].Nb ++;
			}
			TabAlp[TmpBloc[i]->LitLen].Nb ++;			
		}

		Somme = 0;
		//Parcours du tableau
		for (i=0;i<=287;i++)
		{
			//Si il n'y a pas d'occurence pour cette longueur
			if (TabAlp[i].Nb == 0)
			{
			  //Nombre de bits a encoder pour les littéraux
			  if ((TabAlp[i].Nb >= 0) && (TabAlp[i].Nb <= 143))
				  Somme += 8;
			  if ( (TabAlp[i].Nb >= 144) && (TabAlp[i].Nb <= 255))
				  Somme += 9;
			  if ( (TabAlp[i].Nb >= 256) && (TabAlp[i].Nb <= 279))
				  Somme += 7;
			  if ( (TabAlp[i].Nb >= 280) && (TabAlp[i].Nb <= 287))
				  Somme += 8;
			}
		}
		/*Si la somme d'encodeage des litteraux est plus grande que 406 
		(additionner les longueurs des codes non utilisés)*/

		if ((Somme > 406) && (State.CellNum > 100) && (State.BlocNum < 0))
		{
			State.CompType = COMP_10;

			//Créer les arbres pour les longueurs et les distances
			State.TreeL = new SHuffTree;
			State.TreeL->Tree = new SHuffman;
			CreateTree(TabAlp, 287, State.TreeL);

			State.TreeD = new SHuffTree;
			State.TreeD->Tree = new SHuffman;
			CreateTree(TabAlpD, 287, State.TreeD);
		}
		else
		{
			State.CompType = COMP_01;

			//Définit les arbres statiques
			State.TreeL = SLLTree;
			State.TreeD = SDTree;
		}

		PutBit(0, 1);				//Marqueur de fin de bloc
		PutBit(State.CompType, 2);  //Type de compression

		//Pour chaque bloc temporaire
		for (i=0;i<State.CellNum;i++)
		{
			//Si il s'agit d'un littéral
			if(TmpBloc[i]->Lit)
				//Encode le littéral et l'écrit dans le buffer de sortie
				PutCode(State.TreeL->Tree[TmpBloc[i]->LitLen].Code, State.TreeL->Tree[TmpBloc[i]->LitLen].Len);
			else
			{
				//Encode la longueur
				PutCode(State.TreeL->Tree[TmpBloc[i]->LitLen].Code, State.TreeL->Tree[TmpBloc[i]->LitLen].Len);
				//Si il y a un complément
				if(TmpBloc[i]->LLComp > 0)
					//Écrit le complément
					PutBit(TmpBloc[i]->LenComp, TmpBloc[i]->LLComp);

				//Encode la distance
				PutCode(State.TreeD->Tree[TmpBloc[i]->Dist].Code, State.TreeD->Tree[TmpBloc[i]->Dist].Len);
				//Si il y a un complément
				if(TmpBloc[i]->LDComp > 0)
					//Écrit le complément
					PutBit(TmpBloc[i]->DistComp, TmpBloc[i]->LDComp);
			}
		}
		//Marque la fin du bloc
		PutCode(State.TreeL->Tree[256].Code, State.TreeL->Tree[256].Len);
		//Vide le buffer de bit
		FlushBit();

		Blocs[State.BlocNum] = new unsigned char[BufferOutLen];
		memcpy(Blocs[State.BlocNum], BufferOut, BufferOutLen);
		BlocsLen[State.BlocNum] = BufferOutLen;

		delete(BufferOut);

		State.CellNum = 0;
	}
	//Nouveau bloc, remise à zéro
	for(i=0; i<State.CellNum; i++)
	{
		if(TmpBloc[i]!=NULL)
			delete(TmpBloc[i]);
		TmpBloc[i] = NULL;
	}

	//Nouveau bloc, remise à zéro
	State.BlocNum ++;
	BufferOut = NULL;
	BufferOutLen = 0;
	BufferOutTotLen = 0;

	return 0;
}

//*********************************************************
//*                      Final                            *
//*********************************************************
//* Colle tous les blocs en-semble et renvoit le résultat *
//*********************************************************

int CDeflate::Final()
{
char	Code;
int		i;

	State.BlocNum --;

	//Marque le derrnier bloc comme final
	memcpy(&Code, Blocs[State.BlocNum], 1);
	Code ++;
	memcpy(Blocs[State.BlocNum], &Code, 1);

	//Joint
	delete(BufferOut);
	BufferOutTotLen = 0;
	for(i=0; i<=State.BlocNum; i++)
		BufferOutTotLen += BlocsLen[i];

	BufferOut = new unsigned char[BufferOutTotLen];
	for(i=0; i<=State.BlocNum; i++)
	{
		memcpy(BufferOut+BufferOutLen, Blocs[i], BlocsLen[i]);
		BufferOutLen += BlocsLen[i];
		delete(Blocs[i]);
	}

	return 0;
}

//*********************************************************
//*                      Huffman                          *
//*********************************************************
//* Tree	Pointeur sur un arbre de Huffman préalablement*
//*         remplis avec les longueures des codes.        *
//* NbCodes Nombre de codes dans le tableau Tree          *
//*                                                       *
//* Remplis le tableau Tree avec les codes de Huffman     *
//* correspondent à chaque longueur de code.              *
//*********************************************************

int CDeflate::Huffman(pSHuffTree Tree, int NbCodes)
{
	int i;				//Compteur de boucle
	int max_len = 0;	//Longueur du plus grand code en bits
	int MaxCode = 0;	//Définit la plus grande valeur code
	unsigned short Code;
	unsigned short Index;
	unsigned short CodeT[16];	//Tableau qui contien le premier code pour chaque longueur de code
	unsigned short IndexT[16];	//Tableau qui contien l'index de chaque longueur (le premier symbole)
	unsigned short Next_Code[16];	//Tableau qui contien le premier code pour chaque longueur de code permanent


	//Nouvelle méthode
	//Met tout les comptes à 0
	for (i = 0; i <= MAX_CODE_LEN; i++)
		Tree->Nb[i] = 0;
	//Remplis Tree->Nb[] avec le nombre de code de chaque longueur
	for (i = 0; i < NbCodes; i++)
		Tree->Nb[Tree->Tree[i].Len] ++;

	//Calcule le plus petit code de chaque longueur de code
	Next_Code[1] = 0;
	Code = 0;
	Index = 0;
	for (i = 1; i < MAX_CODE_LEN; i++)
	{
		Next_Code[i + 1] = Next_Code[i] + Tree->Nb[i];
		CodeT[i] = Code;
		Code += Tree->Nb[i];
		Code <<=1;
		IndexT[i] = Index;
		Index += Tree->Nb[i];
	}

	Tree->Symb = new unsigned int[(1<<MAX_CODE_LEN)-1];
	//Calcule le code de chaque symbole
	//Les symboles qui n'apparaisse pas (longueur du code de 0) n'ont pas de code.
	//Enregistre les codes et les symboles pour le décodage
	for (i=0;i<NbCodes;i++)
	{
		if (Tree->Tree[i].Len != 0)
		{
			Tree->Symb[Next_Code[Tree->Tree[i].Len]] = i;		//Code pour le décodage
			Tree->Tree[i].Code = (Next_Code[Tree->Tree[i].Len]+CodeT[Tree->Tree[i].Len])-IndexT[Tree->Tree[i].Len];	//Code
			Next_Code[Tree->Tree[i].Len]++;
			if(Tree->Tree[i].Code > MaxCode)
				MaxCode = Tree->Tree[i].Code;
		}else
			Tree->Tree[i].Code = 0;
	}
	return 0;
}

//*********************************************************
//*                      LenExtra                         *
//*********************************************************
//* Len		Longueur, entier entre 257 et 285             *
//*                                                       *
//* Renvoit le nombre de bit du complément en fonction    *
//* de la longueur reçue en paramètre.                    *
//*********************************************************

int CDeflate::LenExtra(int Len)
{

 if (((Len >= 257 && Len <=264) || Len == 285))
    return 0;
 if ((Len >= 265 && Len <=268))
    return 1;
 if ((Len >= 269 && Len <=272))
    return 2;
 if ((Len >= 273 && Len <=276))
    return 3;
 if ((Len >= 277 && Len <=280))
    return 4;
 if ((Len >= 281 && Len <=284))
    return 5;
      
	return -1;
}

//*********************************************************
//*                      DistExtra                        *
//*********************************************************
//* Dist	Distance, entier entre 0 et 29                *
//*                                                       *
//* Renvoit le nombre de bit du complément à la distance  *
//*********************************************************

int CDeflate::DistExtra(int Dist)
{

 if ((Dist >= 0) && (Dist <= 3))
    return 0;
 if (Dist == 4 || Dist == 5)
    return 1;
 if (Dist == 6 || Dist == 7)
    return 2;
 if (Dist == 8 || Dist == 9)
    return 3;
 if (Dist ==10 || Dist ==11)
    return 4;
 if (Dist ==12 || Dist ==13)
    return 5;
 if (Dist ==14 || Dist ==15)
    return 6;       
 if (Dist ==16 || Dist ==17)
    return 7;
 if (Dist ==18 || Dist ==19)
    return 8;
 if (Dist ==20 || Dist ==21)
    return 9;
 if (Dist ==22 || Dist ==23)
    return 10;
 if (Dist ==24 || Dist ==25)
   return 11;
 if (Dist ==26 || Dist ==27)
   return 12;
 if (Dist ==28 || Dist ==29)
   return 13;

	return -1;
}

//*********************************************************
//*                      GetLenVal                        *
//*********************************************************
//* Len		Longueur, entier entre 3 et 258               *
//*                                                       *
//* Renvoit la valeur a utiliser                          *
//*********************************************************

int CDeflate::GetLenVal(int Len, pBlocCell C)
{
	if(Len == 3)
	{
		C->LLComp = 0;
		C->LitLen = 257;
	}
	if(Len == 4)
	{
		C->LLComp = 0;
		C->LitLen = 258;
	}
	if(Len == 5)
	{
		C->LLComp = 0;
		C->LitLen = 259;
	}
	if(Len == 6)
	{
		C->LLComp = 0;
		C->LitLen = 260;
	}
	if(Len == 7)
	{
		C->LLComp = 0;
		C->LitLen = 261;
	}
	if(Len == 8)
	{
		C->LLComp = 0;
		C->LitLen = 262;
	}
	if(Len == 9)
	{
		C->LLComp = 0;
		C->LitLen = 263;
	}
	if(Len == 10)
	{
		C->LLComp = 0;
		C->LitLen = 264;
	}
	if(Len == 11 || Len == 12)
	{
		C->LLComp = 1;
		C->LenComp = Len-11;
		C->LitLen = 265;
	}
	if(Len == 13 || Len == 14)
	{
		C->LLComp = 1;
		C->LenComp = Len-13;
		C->LitLen = 266;
	}
	if(Len == 15 || Len == 16)
	{
		C->LLComp = 1;
		C->LenComp = Len-15;
		C->LitLen = 267;
	}
	if(Len == 17 || Len == 18)
	{
		C->LLComp = 1;
		C->LenComp = Len-17;
		C->LitLen = 268;
	}
	if((Len >= 19) && (Len <= 22))
	{
		C->LLComp = 2;
		C->LenComp = Len-19;
		C->LitLen = 269;
	}
	if((Len >= 23) && (Len <= 26))
	{
		C->LLComp = 2;
		C->LenComp = Len-23;
		C->LitLen = 270;
	}
	if((Len >= 27) && (Len <= 30))
	{
		C->LLComp = 2;
		C->LenComp = Len-27;
		C->LitLen = 271;
	}
	if((Len >= 31) && (Len <= 34))
	{
		C->LLComp = 2;
		C->LenComp = Len-31;
		C->LitLen = 272;
	}
	if((Len >= 35) && (Len <= 42))
	{
		C->LLComp = 3;
		C->LenComp = Len-35;
		C->LitLen = 273;
	}
	if((Len >= 43) && (Len <= 50))
	{
		C->LLComp = 3;
		C->LenComp = Len-43;
		C->LitLen = 274;
	}
	if((Len >= 51) && (Len <= 58))
	{
		C->LLComp = 3;
		C->LenComp = Len-51;
		C->LitLen = 275;
	}
	if((Len >= 59) && (Len <= 66))
	{
		C->LLComp = 3;
		C->LenComp = Len-59;
		C->LitLen = 276;
	}
	if((Len >= 67) && (Len <= 82))
	{
		C->LLComp = 4;
		C->LenComp = Len-67;
		C->LitLen = 277;
	}
	if((Len >= 83) && (Len <= 98))
	{
		C->LLComp = 4;
		C->LenComp = Len-83;
		C->LitLen = 278;
	}
	if((Len >= 99) && (Len <= 114))
	{
		C->LLComp = 4;
		C->LenComp = Len-99;
		C->LitLen = 279;
	}
	if((Len >= 115) && (Len <= 130))
	{
		C->LLComp = 4;
		C->LenComp = Len-115;
		C->LitLen = 280;
	}
	if((Len >= 131) && (Len <= 162))
	{
		C->LLComp = 5;
		C->LenComp = Len-131;
		C->LitLen = 281;
	}
	if((Len >= 163) && (Len <= 194))
	{
		C->LLComp = 5;
		C->LenComp = Len-163;
		C->LitLen = 282;
	}
	if((Len >= 195) && (Len <= 226))
	{
		C->LLComp = 5;
		C->LenComp = Len-195;
		C->LitLen = 283;
	}
	if((Len >= 227) && (Len <= 257))
	{
		C->LLComp = 5;
		C->LenComp = Len-227;
		C->LitLen = 284;
	}
	if(Len == 258)
	{
		C->LLComp = 0;
		C->LitLen = 285;
	}

	return 0;
}

//*********************************************************
//*                      GetDistVal                       *
//*********************************************************
//* Dist	Distance, entier entre 1 et 32768             *
//*                                                       *
//* Renvoit la valeur a utiliser                          *
//*********************************************************

int CDeflate::GetDistVal(int Dist, pBlocCell C)
{
	if (Dist == 1)
	{
		C->LDComp = 0;
		C->Dist = 0;
	}
	if (Dist == 2)
	{
		C->LDComp = 0;
		C->Dist = 1;
	}
	if (Dist == 3)
	{
		C->LDComp = 0;
		C->Dist = 2;
	}
	if (Dist == 4)
	{
		C->LDComp = 0;
		C->Dist = 3;
	}
	if (Dist == 5 || Dist == 6)
	{
		C->LDComp = 1;
		C->DistComp = Dist-5;
		C->Dist = 4;
	}
	if (Dist == 7 || Dist == 8)
	{
		C->LDComp = 1;
		C->DistComp = Dist-7;
		C->Dist = 5;
	}
	if ((Dist >= 9) && (Dist <= 12))
	{
		C->LDComp = 2;
		C->DistComp = Dist-9;
		C->Dist = 6;    
	}   
	if ((Dist >= 13) && (Dist <= 16))
	{
		C->LDComp = 2;
		C->DistComp = Dist-13;
		C->Dist = 7;
	}
	if ((Dist >= 17) && (Dist <= 24))
	{
		C->LDComp = 3;
		C->DistComp = Dist-17;
		C->Dist = 8;
	}
	if ((Dist >= 25) && (Dist <= 32))
	{
		C->LDComp = 3;
		C->DistComp = Dist-25;
		C->Dist = 9;
	}
	if ((Dist >= 33) && (Dist <= 48))
	{
		C->LDComp = 4;
		C->DistComp = Dist-33;
		C->Dist = 10;
	}
	if ((Dist >= 49) && (Dist <= 64))
	{
		C->LDComp = 4;
		C->DistComp = Dist-49;
		C->Dist = 11;
	}
	if ((Dist >= 65) && (Dist <= 96))
	{
		C->LDComp = 5;
		C->DistComp = Dist-65;
		C->Dist = 12;
	}
	if ((Dist >= 97) && (Dist <= 128))
	{
		C->LDComp = 5;
		C->DistComp = Dist-97;
		C->Dist = 13;
	}
	if ((Dist >= 129) && (Dist <= 192))
	{
		C->LDComp = 6;
		C->DistComp = Dist-129;
		C->Dist = 14;
	}
	if ((Dist >= 193) && (Dist <= 256))
	{
		C->LDComp = 6;
		C->DistComp = Dist-193;
		C->Dist = 15;
	}
	if ((Dist >= 257) && (Dist <= 384))
	{
		C->LDComp = 7;
		C->DistComp = Dist-257;
		C->Dist = 16;
	}
	if ((Dist >= 385) && (Dist <= 512))
	{
		C->LDComp = 7;
		C->DistComp = Dist-385;
		C->Dist = 17;
	}
	if ((Dist >= 513) && (Dist <= 768))
	{
		C->LDComp = 8;
		C->DistComp = Dist-513;
		C->Dist = 18;
	}
	if ((Dist >= 769) && (Dist <= 1024))
	{
		C->LDComp = 8;
		C->DistComp = Dist-769;
		C->Dist = 19;
	}
	if ((Dist >= 1025) && (Dist <= 1536))
	{
		C->LDComp = 9;
		C->DistComp = Dist-1025;
		C->Dist = 20;
	}
	if ((Dist >= 1537) && (Dist <= 2048))
	{
		C->LDComp = 9;
		C->DistComp = Dist-1537;
		C->Dist = 21;
	}
	if ((Dist >= 2049) && (Dist <= 3072))
	{
		C->LDComp = 10;
		C->DistComp = Dist-2049;
		C->Dist = 22;
	}
	if ((Dist >= 3073) && (Dist <= 4096))
	{
		C->LDComp = 10;
		C->DistComp = Dist-3073;
		C->Dist = 23;
	}
	if ((Dist >= 4097) && (Dist <= 6144))
	{
		C->LDComp = 11;
		C->DistComp = Dist-4097;
		C->Dist = 24;
	}
	if ((Dist >= 6145) && (Dist <= 8192))
	{
		C->LDComp = 11;
		C->DistComp = Dist-6145;
		C->Dist = 25;
	}
	if ((Dist >= 8193) && (Dist <= 12288))
	{
		C->LDComp = 12;
		C->DistComp = Dist-8193;
		C->Dist = 26;
	}
	if ((Dist >= 12289) && (Dist <= 16384))
	{
		C->LDComp = 12;
		C->DistComp = Dist-12289;
		C->Dist = 27;
	}
	if ((Dist >= 16385) && (Dist <= 24576))
	{
		C->LDComp = 13;
		C->DistComp = Dist-16385;
		C->Dist = 28;
	}
	if ((Dist >= 24577) && (Dist <= 32768))
	{
		C->LDComp = 13;
		C->DistComp = Dist-24577;
		C->Dist = 29;
	}

	return 0;
}

//*********************************************************
//*                      GetBits                          *
//*********************************************************
//* N		Nombre de bits demandés                       *
//*                                                       *
//* Lit et renvoit les N bits des données en entrée       *
//*********************************************************

int CDeflate::GetBits(int N)
{
	int buff;	//Buffer des bits

	//Charge les bits qui n'étais pas encors utilisés d'avant
	buff = State.BitsBuff;
	//Tant que le nombre de bits contenu dans le buffer est plus petit que le nombre demandé
	while(State.BitsBuffLen < N)
	{
		//Copie 8 bits dans buff
		buff |= (unsigned long)*(BufferIn+State.BytesRead) << State.BitsBuffLen;
		State.BytesRead ++;
		State.BitsBuffLen += 8;
	}

	//Met à jour le buffer de bits avec les bits non utilisés
	State.BitsBuff = (int)(buff >> N);
	State.BitsBuffLen -= N;

	//Renvoit les bits demandés
	return (int)(buff & ((1L << N) - 1));
}

//*********************************************************
//*                      Decode                           *
//*********************************************************
//* Décode le code de huffman qui suit dans les données   *
//* avec l'arbre courent                                  *
//*********************************************************

int CDeflate::Decode(pSHuffTree Tree)
{
	int len;			//Nombre de bits actuel dans le code
	int code;			//Longueur du code qui tente d'être décodé
	int first;			//Premier code pour la longueur len
	int count;			//Nombre de code pour cette longueur
	int index;			//Index du premier code la longueur len dans l'arbre de huffman

	code = 0;
	first = 0;
	index = 0;
	for(len = 1; len <= MAX_CODE_LEN; len++)
	{
		//Lit 1 bit du code
		code |= GetBits(1);
		count = Tree->Nb[len];
		//Si le code est de cette longueur alors retourne le symbole
		if (code < first + count)
			//return Tree->Tree[index+(code-first)].Code;
			return Tree->Symb[index+(code-first)];
		//Met à jour les informations pour la longueur suivante
		index += count;
		first += count;
		first <<= 1;
		code <<= 1;
	}
	//Erreur, code non trouvé
	return -1;
}

//*********************************************************
//*                      Out                              *
//*********************************************************
//* Copie un char dans le buffer de sortie (BufferOut)    *
//*********************************************************

int CDeflate::Out(char Val)
{
unsigned char*	temp;	//Pointeur temporaire

	//Si le buffer est trop petit
	if(BufferOutTotLen < BufferOutLen+1)
	{
		if(BufferOut != NULL)
		{
			temp = BufferOut;
			BufferOut = new unsigned char[BufferOutTotLen+BUFFER_LEN];
			memcpy(BufferOut, temp, BufferOutLen);
			BufferOutTotLen += BUFFER_LEN;
			delete(temp);
		}
		else
		{
			BufferOut = new unsigned char[BUFFER_LEN];
			BufferOutTotLen = BUFFER_LEN;
		}
	}

	memcpy(BufferOut+BufferOutLen, &Val, 1);
	BufferOutLen ++;

	return 0;
}

//*********************************************************
//*                      PutBit                           *
//*********************************************************
//*Écriture du Code de longueur N dans le buffer de sortie*
//*********************************************************

int CDeflate::PutBit(unsigned short Code, int N)
{
	State.BitsBuff |= Code<<State.BitsBuffLen;
	State.BitsBuffLen += N;

	while(State.BitsBuffLen >= 8)
	{
		Out(State.BitsBuff & 0xFF);
		State.BitsBuffLen -= 8;
		State.BitsBuff >>= 8;
	}

	return 0;
}

//*********************************************************
//*                      PutCode                          *
//*********************************************************
//* Envoit un code de huffman en sense inverse à PutBit() *
//*********************************************************

int CDeflate::PutCode(unsigned short Code, int N)
{
unsigned short Mask;
unsigned short Dec;

	//Masque pour prendre le bit de gauche
	Mask = 0x01<<(N-1);
	Dec = N-1;

	while(N>0)
	{	//Lit de bit de gauche et l'envoit à PutBit()
		PutBit((Code&Mask)>>Dec, 1);
		Code <<=1;
		N --;
	}

	return 0;
}

//*********************************************************
//*                      FlushBit                         *
//*********************************************************
//* Vide le buffer d'écritre, fin du bloc.                *
//*********************************************************

int CDeflate::FlushBit()
{
	//Si il reste de données dans le buffer
	if(State.BitsBuffLen != 0)
	{	//Tant que le buffer n'est pas plein, remplis avec des 0
		while(State.BitsBuffLen < 8)
		{
			State.BitsBuff |= 0<<State.BitsBuffLen;
			State.BitsBuffLen ++;
		}

		//Ecrit le derrnier octet
		Out(State.BitsBuff & 0xFF);
		State.BitsBuffLen = 0;
		State.BitsBuff  = 0;
	}

	return 0;
}

//*********************************************************
//*                      CreateTree                       *
//*********************************************************
//* Créer l'arbre de huffman optimal pour l'alphabet reçu *
//*********************************************************

int CDeflate::CreateTree(SAlp Vals[], int size, pSHuffTree Tree)
{
int first;
int	r;
int	l;
int	i;
int j;
int k;
int m;
int	reb;

	//Trie les valeurs par ordre croissant de fréquances
	qsort(Vals, size, sizeof(SAlp), Comp);

	//Élimine les symboles avec un nombre de 0 de l'arbre
	first = 0;
	while(first<size && Vals[first].Nb == 0)
		first++;

	//Si il n'y a que deux symboles dans l'alphabet
	if(first == size-1)
	{
		Vals[first].Nb = 1;
		return 0;
	}

	//Définit les paires, créer l'arbre virtuel
	//En réalité aucun arbre n'est créé, il est imaginé
	Vals[first].Nb += Vals[first+1].Nb;
	r = first;
	l = first+2;
	for(i=(first+1); i<(size-1); i++)
	{
		//Selectionne le premier item pour l'union par paire
		if(l >= size || Vals[r].Nb < Vals[l].Nb)
		{
			 Vals[i].Nb = Vals[r].Nb;
			 Vals[r].Nb = i;
			 r ++;
		}
		else
		{
			 Vals[i].Nb = Vals[l].Nb;
			 l ++;
		}

		//Ajout l'item suivant
		if(l >= size || (r < i && Vals[r].Nb < Vals[l].Nb))
		{
			Vals[i].Nb += Vals[r].Nb;
			Vals[r].Nb = i;
			r ++;
		}
		else
		{
			Vals[i].Nb += Vals[l].Nb;
			l ++;
		}
	}

    //Deuxième étape, définit les longueurs de codes temporaires.
	//Traite l'arbre en ordre inverse
	Vals[size - 2].Nb = 0;
	for (i = (size-3); i >= first; i--)
		Vals[i].Nb = Vals[Vals[i].Nb].Nb + 1;

	//Étape finale, définit les longueurs de codes finales
	//L'arbre est traité dans l'ordre normal
	reb = 0;
	j = 1;
	k = 0;
	m = 0;
	r = size -2;
	l = size -1;

	while(j > 0)
	{ 
		while(r >= first && Vals[r].Nb == k)
		{
			m ++;
			r --;
		}

		while(j > m)
		{
			Vals[l].Nb = k;
			l --;
			j --;
			//Le code est trop long, l'arbre devra être rebalancé
			if (k > MAX_CODE_LEN)
				reb++;
		}
		j = 2*m;
		k ++;
		m = 0;
	}

	//Trie les valeurs dans l'ordre de la taille des codes
	//Uniquement pour les codes participant à l'arbre i.e. nombre > 0
	qsort(&Vals[first], size-first, sizeof(SAlp), Comp);

	//Copie les longueurs des codes dans l'arbre
	for(i=0; i<size; i++)
		Tree->Tree[Vals[i].Pos].Len = Vals[i].Nb;

	//Créer les codes
	Huffman(Tree, size);

	return 0;
}
