#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <stdio.h>
#include <windows.h>

#define IS_WINDOWS       (1)



#define SRC_PATH         "F:/Xampp"
#define DST_PATH         "H:/Xampp"
//Documents
//PortableApps
//Xampp


extern bool doRemoveFromDst;
extern bool doRemoveFromSrc;
extern bool doCopyNewFileFromSrc;
extern bool doCopyNewFileFromDst;
extern bool doUpdateNew;
extern bool doUpdateFromSrc;
extern bool doUpdateFromDst;
extern bool doOnlyDateAndSizeCheck;
extern int allowedSecondsLeak;

extern bool doOnlyAnalisys;

//extern bool doOverwriteOlder;



extern bool analyzeOnly();

extern  char intSourcePath[MAX_PATH];
extern  char intDestPath[MAX_PATH];

#define REMOVE_FROM_DST       (1)
#define REMOVE_FROM_SRC       (0)
#define COPY_NEW_FILES        (1)
#define OVERWRITE_OLDER       (1)

#define ANALYZE_ONLY          (1)

#if ANALYZE_ONLY
#undef REMOVE_FROM_DST
#undef COPY_NEW_FILES
#undef OVERWRITE_OLDER
#define REMOVE_FROM_DST       (0)
#define COPY_NEW_FILES        (0)
#define OVERWRITE_OLDER       (0)
#endif

#define BUFFER_READ_SZ        (1024*512)

class CFSElement;

extern void setupFileTime(char*from,char*to);
extern UINT fileTimeToUnixTime(FILETIME FileTime);
extern bool matchString( const char *wzString, const char *wzPattern );

extern void verify(CFSElement*srcElement,CFSElement*dstElement);
extern bool verifyFile(CFSElement*srcElement,CFSElement*dstElement);
extern bool compareComplete(FILE*src,FILE*dst);
extern bool comparePartial(FILE*src,FILE*dst,int sz);

extern void addPattern(char*patt);
extern bool allowedByBlackList(char* path,char*name);
extern bool parseCommandArgs(int argv, char *argc[]);

#endif // UTILS_H
