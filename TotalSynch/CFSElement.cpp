#include "CFSElement.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <tchar.h>
#include <direct.h>
#include <fstream>


#define ALL_ATTS  (FA_DIREC | FA_ARCH)



using namespace std;
char myReadBuffer[BUFFER_READ_SZ];

void CFSElement::unlinkDir(char*from)
{
     char path[MAX_PATH];
     char srcPath[MAX_PATH];
     char curPath[MAX_PATH];
     sprintf( path, "%s\\*", from);
#if IS_WINDOWS
     WIN32_FIND_DATA fd;

     HANDLE hFind = FindFirstFile( path, &fd);
     if(hFind != INVALID_HANDLE_VALUE)	{
          do {
               //First copy files
               if(!(fd.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY)&&
                         !(fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
#ifdef UNICODE
                    tctoc(curPath,fd.cFileName);
#else
                    strcpy(curPath,fd.cFileName);
#endif
                    strcpy(srcPath,from);
                    strcat(srcPath,"/");
                    strcat(srcPath,curPath);

                    unlink(srcPath);
               }
          } while( FindNextFile( hFind, &fd));

          FindClose( hFind);
     }

     hFind = FindFirstFile( path, &fd);
     if(hFind != INVALID_HANDLE_VALUE)	{
          do {
               //First copy files
               if((fd.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY)&&
                         !(fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
#ifdef UNICODE
                    tctoc(curPath,fd.cFileName);
#else
                    strcpy(curPath,fd.cFileName);
#endif
                    int last=strlen(curPath);;
                    if( ((last==2)&&(curPath[0]!='.')&&(curPath[1]!='.'))||
                              ((last==1)&&(curPath[0]!='.'))||
                              (last>2)) {

                         strcpy(srcPath,from);
                         strcat(srcPath,"/");
                         strcat(srcPath,curPath);
                         unlinkDir(srcPath);
                    }
               }
          } while( FindNextFile( hFind, &fd));

          FindClose( hFind);
     }

#else     //IS_LINUX

     struct finddata   fd;
     unsigned int  res;

     //chdir(path);

     for (res = findfirst(path, &fd, ALL_ATTS); res == 0; res = findnext(&fd)) {
          if (!(fd.ff_attrib & FA_DIREC)&&!(fd.ff_attrib & FA_REPARSE)) {
               strcpy(curPath,fd.ff_name);
               strcpy(srcPath,from);
               strcat(srcPath,"/");
               strcat(srcPath,curPath);

               unlink(srcPath);

          }
     }

     for (res = findfirst(path, &fd, ALL_ATTS); res == 0; res = findnext(&fd)) {
          if ((fd.ff_attrib & FA_DIREC)&&!(fd.ff_attrib & FA_REPARSE)) {

               strcpy(curPath,fd.ff_name);
               int last=strlen(curPath);;
               if( ((last==2)&&(curPath[0]!='.')&&(curPath[1]!='.'))||
                         ((last==1)&&(curPath[0]!='.'))||
                         (last>2)) {

                    strcpy(srcPath,from);
                    strcat(srcPath,"/");
                    strcat(srcPath,curPath);
                    unlinkDir(srcPath);
               }

          }
     }
#endif
     rmdir(from);
}




void tctoc(char* strTo,TCHAR* strFrom)
{
     for(unsigned int i = 0; i < _tcslen(strFrom); i++)
          strTo[i] = (CHAR) strFrom[i];
}

CFSElement::CFSElement(char*rootPath,char*path,bool isDir)
{
     //ctor
     analyzed=false;
     strcpy(m_path,path);
     strcpy(m_rootPath,rootPath);
     SetLastModTime(0);
     SetIsDir(isDir);
}

CFSElement::~CFSElement()
{
     //dtor
     cleanChildren();
}

void CFSElement::loadChildren()
{
     char fullPath[MAX_PATH]= {0};
     strcpy(fullPath,m_rootPath);
     if(m_path[0]!=0) {
          strcat(fullPath,(char*)"/");
          strcat(fullPath,m_path);
     }
     char path[MAX_PATH];
     char curPath[MAX_PATH];

     sprintf( path, "%s\\*", fullPath);
     CHAR theRealPath[MAX_PATH];
#if IS_WINDOWS
     WIN32_FIND_DATA fd;
     HANDLE hFind = FindFirstFile( path, &fd);
     if(hFind != INVALID_HANDLE_VALUE)	{
          do {
               curPath[0]=0;
#ifdef UNICODE
               tctoc(curPath,fd.cFileName);
#else
               strcpy(curPath,fd.cFileName);
#endif
               if(m_path[0]!=0) {
                    sprintf(theRealPath,"%s/%s",m_path,curPath);
               } else {
                    sprintf(theRealPath,"%s",curPath);
               }
               CFSElement* fse= NULL;
               if((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                         !(fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
                    int last=strlen(curPath)-1;
                    if( ((last==2)&&(curPath[0]!='.')&&(curPath[1]!='.'))||
                              ((last==1)&&(curPath[0]!='.'))||
                              (last>2)) {
                         fse=new CFSElement(m_rootPath,theRealPath,true);
                    }
               } else if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
                    fse=new CFSElement(m_rootPath,theRealPath,false);
                    fse->SetSize((fd.nFileSizeHigh * (MAXDWORD+1)) + fd.nFileSizeLow);
                    fse->SetLastModTime(fileTimeToUnixTime(fd.ftLastWriteTime));
               }
               childElements.push_back(fse);
          } while( FindNextFile( hFind, &fd));

          FindClose( hFind);
     }
#else          //IS_LINUX
     Some crap;
#endif
}

bool CFSElement::containsElement(CFSElement*toBeContained,CFSElement**containedElement)
{
     CFSElement*curr=NULL;
     for(unsigned int i=0; i<childElements.size(); i++) {
          curr=childElements[i];
          if(curr!=NULL) {
               if(strcmp(curr->m_path,toBeContained->m_path)==0) {
                    *containedElement=curr;
                    return true;
               }
          }
     }
     return false;
}

void CFSElement::copyFile(char*from,char*to)
{
     if(false==allowedByBlackList(from,(char*)"")) {
          //printf("NOT ALLOWED %s\n",from);
          return;
     }
     int c;
     FILE *in,*out;
     CHAR newTo[MAX_PATH];
     sprintf(newTo,"%s.~",to);
     in = fopen( from, "rb" );
     out = fopen( to, "wb" );
     if(in==NULL || !in)     {
          //fprintf(stderr,"%s: No such file or directory\n",argv[1]);
     } else if(out==NULL || !out)     {
          //fprintf(stderr,"%s: No such file or directory\n",argv[2]);
     } else {
          while((c=fread(myReadBuffer,1,BUFFER_READ_SZ,in))>0) {
               fwrite(myReadBuffer,1,c,out);
          }
          fclose(in);
          fclose(out);
          setupFileTime(from,to);
     }
}

void CFSElement::copyDir(char*from,char*to)
{
     char destPath[MAX_PATH]= {0};
     char path[MAX_PATH];
     char srcPath[MAX_PATH];
     char curPath[MAX_PATH];

     WIN32_FIND_DATA fd;

     sprintf( path, "%s\\*", from);
     mkdir(to);

#if IS_WINDOWS
     HANDLE hFind = FindFirstFile( path, &fd);
     if(hFind != INVALID_HANDLE_VALUE)	{
          do {
               //First copy files
               if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)&&
                         !(fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
#ifdef UNICODE
                    tctoc(curPath,fd.cFileName);
#else
                    strcpy(curPath,fd.cFileName);
#endif
                    strcpy(srcPath,from);
                    strcat(srcPath,"/");
                    strcat(srcPath,curPath);

                    strcpy(destPath,to);
                    strcat(destPath,"/");
                    strcat(destPath,curPath);

                    copyFile(srcPath,destPath);
               }
          } while( FindNextFile( hFind, &fd));

          FindClose( hFind);
     }

     hFind = FindFirstFile( path, &fd);
     if(hFind != INVALID_HANDLE_VALUE)	{
          do {
               //First copy files
               if((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)&&
                         !(fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
#ifdef UNICODE
                    tctoc(curPath,fd.cFileName);
#else
                    strcpy(curPath,fd.cFileName);
#endif
                    int last=strlen(curPath);
                    if( ((last==2)&&(curPath[0]!='.')&&(curPath[1]!='.'))||
                              ((last==1)&&(curPath[0]!='.'))||
                              (last>2)) {
                         strcpy(srcPath,from);
                         strcat(srcPath,"/");
                         strcat(srcPath,curPath);

                         strcpy(destPath,to);
                         strcat(destPath,"/");
                         strcat(destPath,curPath);
                         copyDir(srcPath,destPath);
                    }
               }
          } while( FindNextFile( hFind, &fd));

          FindClose( hFind);
     }
#else          //IS_LINUX
     struct ffblk  fd;
     unsigned int  res;

     //chdir(path);

     for (res = findfirst(path, &fd, ALL_ATTS); res == 0; res = findnext(&fd)) {
          if (!(fd.ff_attrib & FA_DIREC)&&!(fd.ff_attrib & FA_REPARSE)) {
               strcpy(curPath,fd.ff_name);
               strcpy(srcPath,from);
               strcat(srcPath,"/");
               strcat(srcPath,curPath);

               strcpy(destPath,to);
               strcat(destPath,"/");
               strcat(destPath,curPath);

               copyFile(srcPath,destPath);
          }
     }

     for (res = findfirst(path, &fd, ALL_ATTS); res == 0; res = findnext(&fd)) {
          if ((fd.ff_attrib & FA_DIREC)&&!(fd.ff_attrib & FA_REPARSE)) {
               strcpy(curPath,fd.ff_name);
               int last=strlen(curPath);
               if( ((last==2)&&(curPath[0]!='.')&&(curPath[1]!='.'))||
                         ((last==1)&&(curPath[0]!='.'))||
                         (last>2)) {

                    strcpy(srcPath,from);
                    strcat(srcPath,"/");
                    strcat(srcPath,curPath);

                    strcpy(destPath,to);
                    strcat(destPath,"/");
                    strcat(destPath,curPath);
                    copyDir(srcPath,destPath);
               }
          }
     }
#endif
}

void CFSElement::copyTo(CFSElement*whereShouldCopy,bool srcToDst)
{
     bool iShouldCopy=false;
     CHAR from[MAX_PATH];
     CHAR to[MAX_PATH];
     getFullPath(from);
     strcpy(to,whereShouldCopy->m_rootPath);
     strcat(to,"/");
     strcat(to,m_path);
     if(doCopyNewFileFromSrc && srcToDst) {
          iShouldCopy=true;
     } else if(doCopyNewFileFromDst && !srcToDst) {
          iShouldCopy=true;
     }

     if(doOnlyAnalisys) {
          iShouldCopy=false;
     }
     if(iShouldCopy) {
          if(IsDir()) {
               printf("CPD: %s\n",from);
               copyDir(from,to);
          } else {
               copyFile(from,to);
          }
     } else if(doOnlyAnalisys) {
          if(IsDir()) {
               printf("CPD: %s\n",from);
          } else {
               if(allowedByBlackList(from,(char*)"")) {
                    printf("CPF: %s\n",from);
               }
          }
     }
}

void CFSElement::cleanChildren()
{
     unsigned int i=0;
     for(i=0; i<childElements.size(); i++) {
          CFSElement*el = childElements.at(i);
          if(NULL!=el) {
               delete(el);
               childElements[i]=NULL;
          }
     }
     childElements.clear();
}

void CFSElement::removeElement(bool fromSrcToDst)
{
     bool doremove=false;
     CHAR perc[MAX_PATH];
     getFullPath(perc);
     if(doRemoveFromDst && fromSrcToDst) {
          doremove=true;
     } else if(doRemoveFromSrc && !fromSrcToDst) {
          doremove=true;
     }

     if(doOnlyAnalisys) {
          doremove=false;
     }
     if(doremove) {
          if(false==IsDir()) {
               unlink(perc);
          } else {
               printf("RMD: %s\n",perc);
               unlinkDir(perc);
          }
     } else {
          if(false==IsDir()) {
               printf("RMF: %s\n",perc);
          } else {
               printf("RMD: %s\n",perc);
          }
     }
}
