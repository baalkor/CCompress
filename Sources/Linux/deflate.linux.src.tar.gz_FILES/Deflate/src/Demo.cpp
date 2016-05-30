#include "CDeflate.h"
#include <string.h>

/*************************************
*     programme de d�monstration     *
**************************************
*R�cup�res les argument              *
*Affectation des variables bool��m   *
*Appel des fonctions compress, etc.. *
*************************************/

int main(int argc, char *argv[])
{
	unsigned char*	Res;	//R�sultat
	unsigned int	i;
    FILE *ptrOpenFichier; //pointeur sur le fichier a ouvrir
    FILE *ptrWriteFichier; //Pointeur sur le fichier a cr�er.
    
    pCDeflate	Def;
    //***************************
    //pour des raisons de performances, lors du parcours des arguments,
    //le programme affecte des valeurs bool��en au op�rations demand�e..
    //Et apr�s le parcours de la boucle, celui-ci effectue les op�rations
    //****************************
    int Compteur;
    bool Compression = false;
    bool Decompression = false;
    bool Autotest = false;
    bool Aide = false;
    bool File = false;
    
	//Cr�er la class
	Def = new CDeflate;
    
    //Si aucun arguments n'est pass� affiche l'aide
    if (argc < 2) 
       Aide = true;
    else {
   
         for (Compteur=1;Compteur<argc;Compteur++){
             //Si commande -f ou f trouv�e alors 
             
             if ((memcmp(argv[Compteur], "-f", 2) == 0) || (memcmp(argv[Compteur], "f", 2) == 0))
              //Compression de fichier a vrai
              File = true;
             //Compression
             if ((memcmp(argv[Compteur], "-c", 2) == 0) || (memcmp(argv[Compteur], "c", 2) == 0)) 
              //Compression du fichier d'entr�e
              Compression = true;                  
             
              //D�compression du fichier donn� en param�tre
             if ((memcmp(argv[Compteur], "-d", 2) == 0) || (memcmp(argv[Compteur], "d", 2) == 0)) 
              Decompression = true;                      
        
              //Affiche l'aide
             if ((memcmp(argv[Compteur], "-h", 2) == 0) || (memcmp(argv[Compteur], "h", 2) == 0))
                Aide = true;
             //Appel de la proc�dure Autotest
             if ((memcmp(argv[Compteur], "-t", 2) == 0) || (memcmp(argv[Compteur], "t", 2) == 0))
                Autotest = true;
        }
     }
       
    if (Compression){
                     
       if (File){
          //Ouverture du fichier en lecture en binaire 
                      
          if (!(ptrOpenFichier = fopen(argv[argc - 2], "rb")))
             //Si erreur lors de l'ouverture affiche le message
             printf("Erreur aucun fichier trouve!");
          else 
          { 
             //creation du fichier compress�
             if (! (ptrWriteFichier = fopen(argv[argc - 1],"wb")) )
                //Si erreur lors de la cr�ation (probl�me de droits par ex) alors
                printf("Erreur lors de la cr�ation du fichier a compresser");
             else 
             {
                  //Sinon compresse le fichier
                  Def->CompressFile(ptrOpenFichier,ptrWriteFichier);
                  fclose(ptrOpenFichier);
                  fclose(ptrWriteFichier);
             }
          }
          
       } 
       else 
       {
          //Compression d'un de la chaine de caract�re suivant la commande -c (sans le f)
          
          Res = Def->Compress((unsigned char*)argv[argc - 2], strlen(argv[argc - 2]));
		  for(i=0; i<Def->GetFinalLen(); i++)
			printf("0x%02x ", *(Res+i));
       }
    }
    //D�compression d'un fichier ou d'une chaine de caract�re
    if (Decompression)
    {
       if (File)
       {
          //Si erreur lors de l'ouverute du fichier.
          if (! (ptrOpenFichier = fopen(argv[argc - 2], "rb")) )
             printf("erreur durant la phase d'ouverture du fichier..");
          else 
          {
               //Si erreur lors de la cr�ation du fichier d�compress�
           if (! (ptrWriteFichier = fopen(argv[argc - 1], "wb")) )
              printf("Erreur durant la c�ration du fichier d�compresser");
           else 
           {
               //D�compression du fichier
               Def->DeCompressFile(ptrOpenFichier,ptrWriteFichier);
               //Fermeture des fichiers
               fclose(ptrWriteFichier);
               fclose(ptrOpenFichier);
           }
         }
 
       }
       else 
       {
          //Si d�compression sans f alors d�compresse la cha�ne se trouvant a la fin de la commande
          Res = Def->DeCompress((unsigned char*)argv[argc - 2], strlen(argv[argc - 2]));   
		  for(i=0; i<Def->GetFinalLen(); i++)
			printf("0x%02x ", *(Res+i));
       }        
    }  
      
    if (Autotest)
    {
		if(Def->AutoTest() == 0)
			printf("Test pass� sans erreurs\n");
        else 
			printf("Erreur rencontr� durant la phases de test!\n");
         
    }
    
    if (Aide)
    {
       printf("\n-c pour compresser\n-d pour decompresser\n");
       printf("-t pour tester le programme\n-f pour compresser/decompresser un fichier\n");
    }
    
	return 0;

}