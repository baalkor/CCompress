#include <stdio.h>				//Utilisé pour l'accès aux fichiers
#include <stdlib.h>				//Utilisé pour la fonction qsort()
#include <memory.h>				//memcpy() et memset()

#define	BUFFER_LEN	4194304		//Taille des buffers (4mb)
#define MAX_BLOCS	1048576		//Nombre de bloc pour l'initialisation du tableau des blocs
#define MAX_CELLS	524288		//Nombre max de cellules dans un bloc. Une cellule = 33bit Si toutes les cellules sont plaines le tableau occupe 16'896Kbyte
								//Le nombre de blocs et de cellules permet de créer un fichier de maximum 512Go
#define	MAX_CODE_LEN	15		//Longueur maximale d'un code de huffman en bit

#define COMP_00		0x0000		//Non compressé
#define COMP_01		0x0001		//Compression statique
#define COMP_10		0x0002		//Compression dynamique
#define	COMP_COMP	0x00AA		//Compressé, mais type non choisis (n'apparaît pas dans les données)
#define COMP_UNDEF	0x00FF		//Compression non définie pour le moment (n'apparaît pas dans les données)

#define LAST_BLOC	0x0004		//Marque de fin de bloc (1xx où xx est le type de compression)

typedef struct SAlp				//Alphabet
{
	unsigned short	Pos;		//Position dans l'alphabet
	unsigned short	Nb;			//Nombre d'occurance
} SAlp, *pSAlp;

typedef struct SHuffman			//Code de huffman pour l'arbre
{
	unsigned short	Code;		//Code de huffman, sur 16bit
	unsigned short	Len;		//Longueur du code de huffman dans Code
} SHuffman, *pSHuffman;

typedef struct SHuffTree		//Arbre de huffman
{
	pSHuffman	Tree;			//Pointeur sur les codes
	int			Nb[MAX_CODE_LEN];//Tableau qui contien le nombre de code pour chaque longueur
	unsigned int	*Symb;		//Symboles classé par leur codes, utilise plus de ram mais facilite la relcture
} SHuffTree, *pSHuffTree;

typedef struct BlocCell			//Cellule dans le tableau du bloc, avant la décision dynamique ou statique
{
	bool			Lit;		//Vrais = littéral | Faux = longeur/distance
	unsigned short	LitLen;		//Litteral ou longueur (0-285)
	int				LLComp;		//Nombre de bit du complément à la longueur
	unsigned short	LenComp;	//Complément à la longueur
	unsigned short	Dist;		//Distance (Max 32768)
	int				LDComp;		//Nombre de bit du complément à la distance
	unsigned short	DistComp;	//Complément à la distance
} BlocCell, *pBlocCell;

struct SState					//Status de la decompression/compression
{
	bool			InBloc;		//Un bloc de données est en cours
	unsigned long   BlocNum;    //Numéro du bloc en cours, Blocs[n]
	unsigned long   CellNum;    //Numéro de la cellule du tableau temporaire
	short           NbBigLit;   //Le nombre de littéraux de 9 bits dans une série consécutive de littéraux
	unsigned short	CompType;	//Type de compression du bloc en cours
	bool			LastBloc;	//Derrnier bloc
	unsigned long	OutPos;		//Position dans le buffer de sortie BufferOut
	unsigned long	UnCompLen;	//Taille de données non compressée restante dans le bloc (si non compressé)
	int				BitsBuff;	//Contien les bits non encors utilisés (de 0 à 7 bits)
	int				BitsBuffLen;//Nombre de bits contenu dans BitsBuff, dépend du contexte /!\Nombre de bits, PAS byte/!\ /
	unsigned long	BytesRead;	//Nombre d'octets lus
	pSHuffTree		TreeL;		//Pointeur sur l'arbre de huffman pour les littéraux/longueurs
	pSHuffTree		TreeD;		//Pointeur sur l'arbre de huffman pour les distances
};

typedef class CDeflate
{
	public:
		CDeflate();				//Constructeur de la class
		~CDeflate();			//Destructeur de la class

		int		AutoTest();									//Test le bon fonctionnement de la class
		unsigned char*	Compress(unsigned char* Data, unsigned long Len);	//Compresse Data et renvoit un pointeur sur les données compressées
		unsigned char*	DeCompress(unsigned char* Data, unsigned long Len);	//Decompresse Data et renvoit un pointeur sur les données décompressées
		int		CompressFile(FILE* In, FILE* Out);			//Compresse le fichier In dans Out
		int		DeCompressFile(FILE* In, FILE* Out);		//Décompresse le fichier In dans Out
		unsigned long	GetFinalLen();						//Renvoit la taille des données compressées ou décompressés

	private:
		int		Inflate();									//Décompression, routine de bas niveau
		int		Deflate();									//Compression, routine de bas niveau
		int		DefFinaliseBloc();							//Finalise un bloc de deflate() et l'écrit dans le buffer de sortie
		int		Final();									//Finalisation de la compression (Deflate())

		int		Huffman(pSHuffTree Tree, int NbCodes);		//Recréer l'arbre 
		int		CreateTree(SAlp Vals[], int size, pSHuffTree Tree);//Créer un arbre de huffman balancé a redondance minimale
		int		DistExtra(int Dist);						//Renvoi le nombre de bits du complément pour Dist
		int		LenExtra(int Len);							//Renvoi le nombre de bits du complément pour Len
		int		GetDistVal(int Dist, pBlocCell C);			//Renvoit la valeur a utiliser
		int		GetLenVal(int Len, pBlocCell C);			//Renvoit la valeur a utiliser

		int		GetBits(int N);								//Renvoit les N bits depuis les données entrente
		int		Decode(pSHuffTree Tree);					//Décode le code de huffman qui suit dans les données avec l'arbre donné

		int		PutBit(unsigned short Code, int N);			//Écriture du Code de longueur N dans le buffer de sortie
		int		FlushBit();									//Vide le buffer d'écritre, fin du bloc.
		int		PutCode(unsigned short Code, int N);		//Envoit un code de huffman en sense inverse à PutBit()
		int		Out(char Val);								//Copie un char dans le buffer de sortie (BufferOut)

		SState	State;										//Status de compression/decompression

		bool	CompressMode;								//Mode de compression actuel (oui ou non)
		int		NbBigLit;									//Nombre de littéraux de 9 bits enregistré dans le bloc

		unsigned char*	BufferIn;							//Buffer d'entrée, données à compresser ou décompresser
		unsigned long BufferInLen;							//Taille de données dans BufferIn
        unsigned long BufferTotInLen;                       //Taille du buffer d'entrée
        
		unsigned char*	BufferOut;							//Buffer de Sortie, données non compressés ou données décompressées
		unsigned long BufferOutLen;							//Taille de données dans BufferOut
		unsigned long BufferOutTotLen;						//Taille du buffer de sortie

		unsigned char*	Blocs[MAX_BLOCS];					//Contien les blocs de données compressées
		unsigned long	BlocsLen[MAX_BLOCS];				//Taille individuelle des blocs dans Blocs[]
		int				NbBlocs;							//Nombre de codes enregistrés dans Blocs[]

		pBlocCell	TmpBloc[MAX_CELLS];						//Tableau temporaire pour les données d'un bloc compressé, avant de choisir le type de compression
		int			NbTmpBloc;								//Nombre de codes enregistrés dans TmpBloc[]

		char	CompType;									//Type de compression actuelle (COMP_xx)

		pSHuffTree	SLLTree;								//Arbre de huffman des littéraux/longueurs statique
		pSHuffTree	SDTree;									//Arbre de huffman des distances statique
		pSHuffTree	DCLTree;								//Arbre de huffman pour les codes de longueurs pour le décodage des arbres dynamiques
		pSHuffTree	DLLTree;								//Arbre de huffman des littéraux/longueurs dynamique
		pSHuffTree	DDTree;									//Arbre de huffman des distances dynamique

		short	LenSymb[29];								//Correspondance longueur/symbole
		short	DistSymb[30];								//Correspondance Distance/symbole
		short	CLeOrder[19];								//Ordre de stockage des codes de longueur pour l'encodage dynamique

} CDeflate, *pCDeflate;
