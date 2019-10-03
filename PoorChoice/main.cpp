#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

using namespace std;

#define MAX_LEN          (128)
#define ENV_MAX_LEN      (32)

static unsigned int maxAnswerLen=80;
static unsigned int maxRetry=3;
static char exitChar='$';
static char retryString[MAX_LEN]="Retry";
static char abortString[MAX_LEN]="Abort";
static char envVar[ENV_MAX_LEN]="CHOICE";

static int toInt(char*from)
{
     char cRes[10];
     memset(cRes,'0',10);
     int res=0;
     if(from[0]=='-') {
          cRes[res]=from[0];
          res++;
     }
     for(unsigned int i=0; i<strlen(from); i++) {
          if(from[i]>='0' && from[i]<='9') {
               cRes[res]=from[i];
               res++;
          }
     }
     return atoi(cRes);
}

static void printHelp(int res)
{
     FILE*f=fopen("helpChoice.txt","rt");
     if(f==NULL) {
          printf("Missing help file 'help.txt'\n");
     }
     char buffer[81]= {0};
     int icount=0;
     while(NULL!=fgets(buffer,81,f )) {
          if(buffer[0]!='#'){
               if(icount==23) {
                    printf("Press Enter to continue\n");
                    getchar();
                    icount=0;
               }
               printf("%s",buffer);
               icount++;
          }
     }
     fclose(f);
     exit(res);
}


static bool parseCommandArgs(int argv, char *argc[])
{
     if(argv==1) {
          printHelp(-1);
          return false;
     }

     char*tmp=NULL;
     for(int i=1; i<argv; i++) {
          tmp=argc[i];
          if(strlen(tmp)==2) {
               if(tmp[0]=='-') {
                    switch(tmp[1]) {
                    case('h'):
                         printHelp(0);
                         break;
                    case('l'):     //Max answer len
                         i++;
                         if(i>=argv) {
                              return false;
                         }
                         tmp=argc[i];
                         if(strlen(tmp)>2) {
                              return false;
                         }
                         //Number follow
                         if(toInt(tmp)<=0){
                              return false;
                         }
                         maxAnswerLen=toInt(tmp);
                         break;
                    case('r'):     //Max retries
                         i++;
                         if(i>=argv) {
                              return false;
                         }
                         tmp=argc[i];
                         if(strlen(tmp)>1) {
                              return false;
                         }
                         //Number follow
                         if(toInt(tmp)<=0){
                              return false;
                         }
                         maxRetry=toInt(tmp);
                         break;
                    case('s'):     //Retry string
                         i++;
                         if(i>=argv) {
                              return false;
                         }
                         tmp=argc[i];
                         if(strlen(tmp)>=MAX_LEN) {
                              return false;
                         }
                         //String follow
                         if(strlen(tmp)==0){
                              return false;
                         }
                         strcpy(retryString,tmp);
                         break;
                    case('a'):     //Abort string
                         i++;
                         if(i>=argv) {
                              return false;
                         }
                         tmp=argc[i];
                         if(strlen(tmp)>=MAX_LEN) {
                              return false;
                         }
                         //String follow
                         if(strlen(tmp)==0){
                              return false;
                         }
                         strcpy(abortString,tmp);
                         break;
                    case('e'):     //Env var name
                         i++;
                         if(i>=argv) {
                              return false;
                         }
                         tmp=argc[i];
                         if(strlen(tmp)>=ENV_MAX_LEN) {
                              return false;
                         }
                         //String follow
                         if(strlen(tmp)==0){
                              return false;
                         }
                         strcpy(envVar,tmp);
                         break;
                    case('x'):     //Exit char
                         i++;
                         if(i>=argv) {
                              return false;
                         }
                         tmp=argc[i];
                         if(strlen(tmp)!=1) {
                              return false;
                         }
                         //Char follow
                         exitChar=tmp[0];
                         break;
                    default:
                         printf("Unknown Parameter %s\n",tmp);
                         exit(-1);
                         break;
                    }
               }
          }
     }

     return true;
}


int main(int argv, char *argc[])
{

     int bytes_read;
     char *my_string;

     if(false==parseCommandArgs(argv,argc)){
          return -1;
     }
     /* These 2 lines are the heart of the program. */
     my_string = (char *) malloc (maxAnswerLen + 1);
     cin.getline (my_string, maxAnswerLen);
     bytes_read = cin.gcount();
     //printf("%s\n",my_string);
     while(bytes_read<=0){
          printf("%s\n",retryString);
          cin.getline (my_string, maxAnswerLen);
          bytes_read = cin.gcount();
     }
     if((strlen(my_string)==1)&&(my_string[0]==exitChar)){
          printf("%s\n",abortString);
          return -1;
     }
     FILE* f=fopen(envVar,"wt");
     if(f==NULL){
          return -1;
     }
     fprintf(f,"%s",my_string);
     fclose(f);
     return 0;
}
