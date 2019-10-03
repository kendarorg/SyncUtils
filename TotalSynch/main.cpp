#include <iostream>
#include <stdio.h>
#include <windows.h>
#include "CFSElement.h"
#include <vector>
#include "utils.h"

/*
@TODO
Impostazioni da command line
     1-Verifica o meno del contenuto
          1.1-Aggiornamento delle date in caso di contenuto identico
     2-Eliminazione della destinazione
     3-Aggiunta di files mancanti nella destinazione
Analisi->tira fuori un file con tutta la descrizione
     Se trova un file diverso segna tutta la struttura come diversa sino a radice-1

In caso di cose simili
     4-Salvataggio del crc sorgente e del crc destinazione,
          Verifica delle date e della lunghezza per controllare che sian corrispondenti
          Se
*/

using namespace std;



int main(int argv, char *argc[])
{
     if(false==parseCommandArgs(argv,argc)){
          printf("Problem with parameters!!!\n");
          return -1;
     }
     printf("SYN: %s->%s\n",intSourcePath,intDestPath);
     CFSElement srcElement(intSourcePath,(char*)"",true);
     CFSElement dstElement(intDestPath,(char*)"",true);
     verify(&srcElement,&dstElement);

     return 0;
}
