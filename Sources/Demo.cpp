#include "CDeflate.h"
#include <string.h>

/*************************************
*     programme de démonstration     *
**************************************
*Récupères les argument              *
*Affectation des variables booléém   *
*Appel des fonctions compress, etc.. *
*************************************/

int main(int argc, char *argv[])
{
	unsigned char*	Res;	//Résultat
	unsigned int	i;
    FILE *ptrOpenFichier; //pointeur sur le fichier a ouvrir
    FILE *ptrWriteFichier; //Pointeur sur le fichier a créer.
    
    pCDeflate	Def;
    //***************************
    //pour des raisons de performances, lors du parcours des arguments,
    //le programme affecte des valeurs boolééen au opérations demandée..
    //Et après le parcours de la boucle, celui-ci effectue les opérations
    //****************************
    int Compteur;
    bool Compression = false;
    bool Decompression = false;
    bool Autotest = false;
    bool Aide = false;
    bool File = false;
    
	//Créer la class
	Def = new CDeflate;
    
    //Si aucun arguments n'est passé affiche l'aide
    if (argc < 2) 
       Aide = true;
    else {
   
         for (Compteur=1;Compteur<argc;Compteur++){
             //Si commande -f ou f trouvée alors 
             
             if ((memcmp(argv[Compteur], "-f", 2) == 0) || (memcmp(argv[Compteur], "f", 2) == 0))
              //Compression de fichier a vrai
              File = true;
             //Compression
             if ((memcmp(argv[Compteur], "-c", 2) == 0) || (memcmp(argv[Compteur], "c", 2) == 0)) 
              //Compression du fichier d'entrée
              Compression = true;                  
             
              //Décompression du fichier donné en paramètre
             if ((memcmp(argv[Compteur], "-d", 2) == 0) || (memcmp(argv[Compteur], "d", 2) == 0)) 
              Decompression = true;                      
        
              //Affiche l'aide
             if ((memcmp(argv[Compteur], "-h", 2) == 0) || (memcmp(argv[Compteur], "h", 2) == 0))
                Aide = true;
             //Appel de la procédure Autotest
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
             //creation du fichier compressé
             if (! (ptrWriteFichier = fopen(argv[argc - 1],"wb")) )
                //Si erreur lors de la création (problème de droits par ex) alors
                printf("Erreur lors de la création du fichier a compresser");
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
          //Compression d'un de la chaine de caractère suivant la commande -c (sans le f)
          
          Res = Def->Compress((unsigned char*)argv[argc - 2], strlen(argv[argc - 2]));
		  for(i=0; i<Def->GetFinalLen(); i++)
			printf("0x%02x ", *(Res+i));
       }
    }
    //Décompression d'un fichier ou d'une chaine de caractère
    if (Decompression)
    {
       if (File)
       {
          //Si erreur lors de l'ouverute du fichier.
          if (! (ptrOpenFichier = fopen(argv[argc - 2], "rb")) )
             printf("erreur durant la phase d'ouverture du fichier..");
          else 
          {
               //Si erreur lors de la création du fichier décompressé
           if (! (ptrWriteFichier = fopen(argv[argc - 1], "wb")) )
              printf("Erreur durant la cération du fichier décompresser");
           else 
           {
               //Décompression du fichier
               Def->DeCompressFile(ptrOpenFichier,ptrWriteFichier);
               //Fermeture des fichiers
               fclose(ptrWriteFichier);
               fclose(ptrOpenFichier);
           }
         }
 
       }
       else 
       {
          //Si décompression sans f alors décompresse la chaîne se trouvant a la fin de la commande
          Res = Def->DeCompress((unsigned char*)argv[argc - 2], strlen(argv[argc - 2]));   
		  for(i=0; i<Def->GetFinalLen(); i++)
			printf("0x%02x ", *(Res+i));
       }        
    }  
      
    if (Autotest)
    {
		if(Def->AutoTest() == 0)
			printf("Test passé sans erreurs\n");
        else 
			printf("Erreur rencontré durant la phases de test!\n");
         
    }
    
    if (Aide)
    {
       printf("\n-c pour compresser\n-d pour decompresser\n");
       printf("-t pour tester le programme\n-f pour compresser/decompresser un fichier\n");
    }
    
	return 0;

}