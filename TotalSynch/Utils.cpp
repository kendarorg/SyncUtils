#include "Utils.h"
#include "CFSElement.h"
#include <vector>
#include <map>
#include <io.h>     // For access().
#include <sys/types.h>  // For stat().
#include <sys/stat.h>   // For stat().


//Variables for copy
char bigBufferSrc[BUFFER_READ_SZ];
char bigBufferDst[BUFFER_READ_SZ];

bool doRemoveFromDst=false;
bool doRemoveFromSrc=false;
bool doCopyNewFileFromSrc=false;
bool doCopyNewFileFromDst=false;
bool doUpdateNew=false;
bool doUpdateFromSrc=false;
bool doUpdateFromDst=false;
bool doOnlyAnalisys=false;
bool doOnlyDateAndSizeCheck=false;
int allowedSecondsLeak=10;

bool doOverwriteOlder=false;

char intSourcePath[MAX_PATH]= {0};
char intDestPath[MAX_PATH]= {0};



#define SECS_BETWEEN_EPOCHS     ((__int64)0x2B6109100LLU)
#define SECS_TO_100NS		  ((__int64)0x989680LLU)

bool DirectoryExists( const char* absolutePath ){

    if( _access( absolutePath, 0 ) == 0 ){

        struct stat status;
        stat( absolutePath, &status );

        return (status.st_mode & S_IFDIR) != 0;
    }
    return false;
}

UINT fileTimeToUnixTime(FILETIME FileTime)
{
     /* get the full win32 value, in 100ns */
     __int64 lUnixTime = ((__int64)FileTime.dwHighDateTime << 32) + FileTime.dwLowDateTime;

     /* convert to the Unix epoch */
     lUnixTime = lUnixTime-(SECS_BETWEEN_EPOCHS * SECS_TO_100NS);

     UINT UnixTime =(UINT)(lUnixTime/SECS_TO_100NS); /* now convert to seconds */

     return (UINT)UnixTime;
}

void setupFileTime(char*from,char*to)
{
     FILETIME ftCreate, ftAccess, ftWrite;
     FILETIME ftCreateOut, ftAccessOut, ftWriteOut;
     HANDLE hFileIn = CreateFile(from,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
     HANDLE hFileOut = CreateFile(to,FILE_WRITE_ATTRIBUTES|GENERIC_READ, FILE_SHARE_WRITE|FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
     GetFileTime(hFileIn, &ftCreate, &ftAccess, &ftWrite);
     GetFileTime(hFileOut, &ftCreateOut, &ftAccessOut, &ftWriteOut);
     if((ftWriteOut.dwLowDateTime!=ftWrite.dwLowDateTime)||
               (ftWriteOut.dwHighDateTime!=ftWrite.dwHighDateTime)) {
          SetFileTime (hFileOut, &ftCreate, &ftAccess, &ftWrite);
     }

     CloseHandle(hFileIn);
     CloseHandle(hFileOut);
}

bool comparePartial(FILE*src,FILE*dst,int sz)
{
     int blockSize=sz/5;
     int sizeCmp = blockSize>(BUFFER_READ_SZ*10)?1024:50;
     int counter=0;
     int sizeReadSrc=0;
     int sizeReadDst=0;
     fseek(src,0,SEEK_SET);
     fseek(dst,0,SEEK_SET);
     while(counter<5) {
          fseek(src,counter*blockSize,SEEK_SET);
          fseek(dst,counter*blockSize,SEEK_SET);
          sizeReadSrc=fread(bigBufferSrc,1, sizeCmp ,src);
          sizeReadDst=fread(bigBufferDst,1, sizeCmp ,dst);
          if(0!=memcmp(bigBufferDst,bigBufferSrc,sizeReadDst)) {
               return false;
          }
          counter++;
     }
     return true;
}

bool compareComplete(FILE*src,FILE*dst)
{
     fseek(src,0,SEEK_SET);
     fseek(dst,0,SEEK_SET);
     int sizeReadSrc=fread(bigBufferSrc,1, BUFFER_READ_SZ ,src);
     int sizeReadDst=fread(bigBufferDst,1, BUFFER_READ_SZ ,dst);
     while(sizeReadDst>0 && sizeReadSrc>0) {
          if(sizeReadDst!=sizeReadSrc) {
               return false;
          }
          if(0!=memcmp(bigBufferDst,bigBufferSrc,sizeReadDst)) {
               return false;
          }
          sizeReadSrc=fread(bigBufferSrc,1, BUFFER_READ_SZ ,src);
          sizeReadDst=fread(bigBufferDst,1, BUFFER_READ_SZ ,dst);
     }
     return true;
}

bool areFilesEquals(CFSElement*srcElementOri,CFSElement*dstElementOri)
{
     char tmpPathSrc[MAX_PATH];
     char tmpPathDst[MAX_PATH];

     CFSElement*srcElement=srcElementOri;
     CFSElement*dstElement = dstElementOri;

     srcElement->getFullPath(tmpPathSrc);
     if(false ==allowedByBlackList(tmpPathSrc,srcElement->GetPath())) {
          srcElement->analyzed=true;
          return true;
     }
     dstElement->getFullPath(tmpPathDst);

     if(doOnlyDateAndSizeCheck) {
          if((srcElement->GetLastModTime()>(dstElement->GetLastModTime()+allowedSecondsLeak))||
                    (srcElement->GetLastModTime()<(dstElement->GetLastModTime()-allowedSecondsLeak))) {
               return false;
          }

          if(srcElement->GetSize()!=dstElement->GetSize()) {
               return false;
          }
          return true;
     }

     FILE*src=NULL;
     FILE*dst=NULL;

     src=fopen(tmpPathSrc,"rb");
     dst=fopen(tmpPathDst,"rb");

     /*if(doOnlyDateAndSizeCheck) {
          if(false==comparePartial(src,dst,srcElement->GetSize())) {
               fclose(src);
               fclose(dst);
               return false;
          }
          fclose(src);
          fclose(dst);
          return true;
     }*/

     if(srcElement->GetSize()<(BUFFER_READ_SZ)) {
          if(false==compareComplete(src,dst)) {
               fclose(src);
               fclose(dst);
               return false;
          } else {
               fclose(src);
               fclose(dst);
               return true;
          }
     } else if(false==comparePartial(src,dst,srcElement->GetSize())) {
          fclose(src);
          fclose(dst);
          return false;
     }

     if(false==compareComplete(src,dst)) {
          fclose(src);
          fclose(dst);
          return false;
     }

     fclose(src);
     fclose(dst);


     return true;
}

static void removeUnavailableElements(CFSElement*srcElement,CFSElement*dstElement)
{
     CFSElement*el=NULL;
     CFSElement*elOther=NULL;
     unsigned int i=0;

     for(i=0; i<dstElement->childElements.size(); i++) {
          el = dstElement->childElements.at(i);
          if(el!=NULL) {
               if(false==srcElement->containsElement(el,&elOther)) {
                    dstElement->childElements[i]=NULL;
                    el->removeElement();
                    delete(el);
               }
          }
     }
}

static void verifyDirs(CFSElement*srcElement,CFSElement*dstElement)
{
     CFSElement*el=NULL;
     CFSElement*elOther=NULL;
     unsigned int i=0;

     for(i=0; i<srcElement->childElements.size(); i++) {
          el = srcElement->childElements.at(i);
          if(el!=NULL && !el->analyzed) {
               //Only for dirs
               if(el->IsDir()) {
                    //Check only if something is present...already managed the eventual copy
                    if(true==dstElement->containsElement(el,&elOther)) {
                         //If one is dir and not the other
                         if(el->IsDir()!=elOther->IsDir()) {
                              printf("ERROR MISMATCH 0x0001 !!! %s-%s\n",el->GetPath(),elOther->GetPath());
                              exit(0);
                              return;
                         } else {
                              if(elOther->analyzed) {
                                   el->analyzed=true;
                              } else {
                                   //Check if the element is a directory
                                   //el->getFullPath(dirPath);
                                   //If it's a directory should check and eventually copy
                                   verify(el,elOther);
                                   el->analyzed=true;
                                   elOther->analyzed=true;
                              }
                         }
                    }
               }
          }
     }
}

static void copyNewElements(CFSElement*srcElement,CFSElement*dstElement)
{
     CFSElement*el=NULL;
     CFSElement*elOther=NULL;
     unsigned int i=0;

     for(i=0; i<srcElement->childElements.size(); i++) {
          el = srcElement->childElements.at(i);
          if(el!=NULL) {
               //If not present the element on destination
               if(false==dstElement->containsElement(el,&elOther)) {
                    //Copy the element to the destination, does not matter what it is
                    el->copyTo(dstElement);
                    el->analyzed=true;
               }
          }
     }
}


static void updateModifiedFiles(CFSElement*srcElement,CFSElement*dstElement,bool updateBasedOnDate)
{
     CFSElement*el=NULL;
     CFSElement*elOther=NULL;
     unsigned int i=0;
     char tmpsrc[MAX_PATH]= {0};
     char tmpdst[MAX_PATH]= {0};
     bool areEquals=false;
//     CFSElement*tmpEl=NULL;


     for(i=0; i<srcElement->childElements.size(); i++) {
          areEquals=false;
          el = srcElement->childElements.at(i);
          if(el!=NULL && !el->analyzed) {
               //If not present the element on destination
               if(true==dstElement->containsElement(el,&elOther)) {
                    //If one is dir and not the other
                    if(el->IsDir()!=elOther->IsDir()) {
                         printf("ERROR MISMATCH 0x0002 !!! %s-%s\n",el->GetPath(),elOther->GetPath());
                         exit(-1);
                         return;
                    } else if(!el->IsDir()) {
                         el->getFullPath(tmpsrc);
                         elOther->getFullPath(tmpdst);
                         areEquals = areFilesEquals(el,elOther);
                         if(updateBasedOnDate) {
                              if(!areEquals) {
                                   if(el->GetLastModTime()>(elOther->GetLastModTime()+allowedSecondsLeak)) {
                                        if(!doOnlyAnalisys)el->copyTo(dstElement);
                                        elOther->SetLastModTime(el->GetLastModTime());
                                        elOther->SetSize(el->GetSize());
                                        if(!doOnlyAnalisys)setupFileTime(tmpsrc,tmpdst);
                                        el->analyzed=true;
                                        elOther->analyzed=true;
                                        if(doOnlyAnalisys)printf("DIF: %s\n",tmpdst);
                                   } else if(el->GetLastModTime()<(elOther->GetLastModTime()-allowedSecondsLeak)) {
                                        if(!doOnlyAnalisys)elOther->copyTo(srcElement);
                                        el->SetLastModTime(elOther->GetLastModTime());
                                        el->SetSize(elOther->GetSize());
                                        if(!doOnlyAnalisys)setupFileTime(tmpdst,tmpsrc);
                                        if(doOnlyAnalisys)printf("DIF: %s\n",tmpsrc);
                                        el->analyzed=true;
                                        elOther->analyzed=true;
                                   } else {
                                        printf("Conflict on %s-%s\n",tmpsrc,tmpdst);
                                        exit(-1);
                                        return;
                                   }
                              } else if(el->GetLastModTime()>(elOther->GetLastModTime()+allowedSecondsLeak)) {
                                   if(!doOnlyAnalisys)setupFileTime(tmpsrc,tmpdst);
                                   el->analyzed=true;
                                   elOther->analyzed=true;
                              } else if(elOther->GetLastModTime()>(el->GetLastModTime()-allowedSecondsLeak)) {
                                   if(!doOnlyAnalisys)setupFileTime(tmpdst,tmpsrc);
                                   el->analyzed=true;
                                   elOther->analyzed=true;
                              }
                         } else {
                              if(!areEquals) {

                                   if(!doOnlyAnalisys)el->copyTo(dstElement);
                                   elOther->SetLastModTime(el->GetLastModTime());
                                   elOther->SetSize(el->GetSize());
                                   if(!doOnlyAnalisys)setupFileTime(tmpsrc,tmpdst);
                                   if(doOnlyAnalisys)printf("DIF: %s\n",tmpdst);
                                   el->analyzed=true;
                                   elOther->analyzed=true;
                              }
                         }
                    }
               }
          }
     }
}


void verify(CFSElement*srcElement,CFSElement*dstElement)
{
     //Load child elements
     srcElement->loadChildren();

     dstElement->loadChildren();

     //Verify elements that are in dest but not in source and remove them
     if(doRemoveFromDst) {
          removeUnavailableElements(srcElement,dstElement);
     }

     //Verify elements that are in source but not in dest and remove them
     if(doRemoveFromSrc) {
          removeUnavailableElements(dstElement,srcElement);
     }

     //Copy elements from source to dest if not present in dest
     if(doCopyNewFileFromSrc) {
          copyNewElements(srcElement,dstElement);
     }

     //Copy elements from dst to src if not present in src
     if(doCopyNewFileFromDst) {
          copyNewElements(dstElement,srcElement);
     }

     //Check if have to do updates on files based on file times
     if(doUpdateNew) {
          updateModifiedFiles(srcElement,dstElement,true);
          updateModifiedFiles(dstElement,srcElement,true);
     } else if(doUpdateFromSrc) {
          updateModifiedFiles(srcElement,dstElement,false);
     } else if(doUpdateFromDst) {
          updateModifiedFiles(dstElement,srcElement,false);
     }

     verifyDirs(srcElement,dstElement);

     srcElement->cleanChildren();
     dstElement->cleanChildren();
}


bool matchString( const char *wzString, const char *wzPattern )
{
     switch (*wzPattern) {
     case '\0':
          return !*wzString;
     case '*':
          return matchString(wzString, wzPattern+1) ||
                 ( *wzString && matchString(wzString+1, wzPattern) );
     case '?':
          return *wzString &&
                 matchString(wzString+1, wzPattern+1);
     default:
          return (*wzPattern == *wzString) &&
                 matchString(wzString+1, wzPattern+1);
     }
}

std::vector <char*>patterns;

void addPattern(char*patt)
{
     char*tmp=NULL;
     for(unsigned int i=0; i<patterns.size(); i++) {
          tmp = patterns[i];
          if(tmp!=NULL) {
               if(strcmp(patt,tmp)==0) {
                    return;
               }
          }
     }
     tmp = (char*)malloc(strlen(patt)+1);
     strcpy(tmp,patt);
     patterns.push_back(tmp);
}

bool patternAllowed(char*path)
{
     char*tmp=NULL;
     for(unsigned int i=0; i<patterns.size(); i++) {
          tmp = patterns[i];
          if(tmp!=NULL) {
               if(true==matchString(path,tmp)) {
                    return false;
               }
          }
     }
     return true;
}

bool nameAllowed(char* path,char*name)
{
     return true;
}

//Avoid   *.tmp, *.tmp_, *.log, *.old, *.bak, *.pid
//True me
bool allowedByBlackList(char* path,char*name)
{
     return patternAllowed(path) && nameAllowed(path,name);
}

int toInt(char*from)
{
     if(strlen(from)==0 || strlen(from)>10) {
          return allowedSecondsLeak;
     }
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

void printHelp(int res)
{
     FILE*f=fopen("help.txt","rt");
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

bool parseCommandArgs(int argv, char *argc[])
{
     if(argv<3) {
          printHelp(0);
          return false;
     }
     char*tmp=NULL;
     for(int i=1; i<argv; i++) {
          tmp=argc[i];
          //sdblh
          //ftpov
          //acruw
          if(strlen(tmp)>=2) {
               if(tmp[0]=='-') {
                    switch(tmp[1]) {
                    case('h'):
                         printHelp(0);
                         break;
                    case('l'):
                         i++;
                         if(i>=argv) {
                              return false;
                         }
                         tmp=argc[i];
                         if(strlen(tmp)<3) {
                              return false;
                         }
                         //Number follow
                         allowedSecondsLeak=toInt(tmp);
                         break;
                    case('f'):
                         i++;
                         if(i>=argv) {
                              return false;
                         }
                         tmp=argc[i];
                         if(strlen(tmp)<3) {
                              return false;
                         }
                         //Source dir follow
                         strcpy(intSourcePath,tmp);
                         break;
                    case('t'):
                         i++;
                         if(i>=argv) {
                              return false;
                         }
                         tmp=argc[i];
                         if(strlen(tmp)<3) {
                              return false;
                         }
                         //Dest dir follow
                         strcpy(intDestPath,tmp);
                         break;
                    case('p'):
                         i++;
                         if(i>=argv) {
                              return false;
                         }
                         tmp=argc[i];
                         if(strlen(tmp)<3) {
                              return false;
                         }
                         //Pattern follow (between double brakets)
                         addPattern(tmp);
                         break;
                    case('o'):
                         for(unsigned int j=2; j<strlen(tmp); j++) {
                              switch(tmp[j]) {
                              case('v'):
                                   doOnlyDateAndSizeCheck=true;
                                   break;
                              case('a'):
                                   doOnlyAnalisys=true;
                                   break;
                              case('c'):
                                   if((j+1)<strlen(tmp)) {
                                        if(tmp[j+1]=='f') {
                                             doCopyNewFileFromSrc=true;
                                             doRemoveFromSrc=false;
                                             i++;
                                        } else if(tmp[j+1]=='t') {
                                             doCopyNewFileFromDst=true;
                                             doRemoveFromDst=false;
                                             i++;
                                        } else if(tmp[j+1]=='b') {
                                             doCopyNewFileFromSrc=true;
                                             doCopyNewFileFromDst=true;
                                             doRemoveFromDst=false;
                                             doRemoveFromSrc=false;
                                             i++;
                                        } else {
                                             doCopyNewFileFromSrc=true;
                                             doRemoveFromSrc=false;
                                        }
                                   } else {
                                        doCopyNewFileFromSrc=true;
                                        doRemoveFromSrc=false;
                                   }
                                   break;
                              case('r'):
                                   if((j+1)<strlen(tmp)) {
                                        if(tmp[j+1]=='f') {
                                             doRemoveFromSrc=true;
                                             doCopyNewFileFromSrc=false;
                                             i++;
                                        } else if(tmp[j+1]=='t') {
                                             doRemoveFromDst=true;
                                             doCopyNewFileFromDst=false;
                                             i++;
                                        } else if(tmp[j+1]=='b') {
                                             doRemoveFromSrc=true;
                                             doRemoveFromDst=true;
                                             doCopyNewFileFromSrc=false;
                                             doCopyNewFileFromDst=false;
                                             i++;
                                        } else {
                                             doRemoveFromDst=true;
                                             doCopyNewFileFromDst=false;
                                        }
                                   } else {
                                        doRemoveFromDst=true;
                                        doCopyNewFileFromDst=false;
                                   }
                                   break;
                              case('u'):
                                   doUpdateNew=true;
                                   doUpdateFromSrc=false;
                                   doUpdateFromDst=false;
                                   break;
                              case('w'):
                                   if((j+1)<strlen(tmp)) {
                                        if(tmp[j+1]=='f') {
                                             doUpdateFromSrc=true;
                                             doUpdateNew=false;
                                             i++;
                                        } else if(tmp[j+1]=='t') {
                                             doUpdateFromDst=true;
                                             doUpdateNew=false;
                                             i++;
                                        } else if(tmp[j+1]=='b') {
                                             i++;
                                             printf("Impossible Subparameter b for w in %s\n",tmp);
                                             exit(-1);
                                        } else {
                                             doUpdateFromSrc=true;
                                             doUpdateNew=false;
                                        }
                                   } else {
                                        doUpdateFromSrc=true;
                                        doUpdateNew=false;
                                   }
                                   break;

                              default:
                                   printf("Unknown Parameter %c in %s \n",tmp[j],tmp);
                                   exit(-1);
                                   break;
                              }
                         }
                         break;
                    default:
                         printf("Unknown Parameter %s\n",tmp);
                         exit(-1);
                         break;
                    }
               }
          }
     }
     if(strlen(intSourcePath)==0 ||strlen(intDestPath)==0) {
          printHelp(0);
          return false;
     }

     if(!DirectoryExists(intSourcePath)) {
          printf("Missing source dir %s\n",intSourcePath);
          return false;
     }

    if(!DirectoryExists(intDestPath)) {
          printf("Missing destination dir %s\n",intDestPath);
          return false;
     }

     return true;
}

bool analyzeOnly()
{
     if(doRemoveFromDst==false && doRemoveFromSrc==false &&
               doCopyNewFileFromSrc==false && doCopyNewFileFromDst==false &&
               doOverwriteOlder==false) {
          return true;
     }
     return false;
}


