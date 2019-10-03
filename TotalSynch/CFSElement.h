#ifndef CFSELEMENT_H
#define CFSELEMENT_H

#include <string.h>
#include <vector>
#include <stdio.h>
#include "utils.h"


class CFSElement
{
    public:
        void unlinkDir(char*from);
        void copyFile(char*from,char*to);
        void copyDir(char*from,char*to);
        std::vector<CFSElement*>childElements;

        /** Default constructor */
        CFSElement(char*rootPath,char*path,bool isDir=false);
        /** Default destructor */
        virtual ~CFSElement();
        /** Access m_path
         * \return The current value of m_path
         */
        char*GetPath() { return m_path; }
        /** Set m_path
         * \param val New value to set
         */
        void SetPath(char*val) { strcpy(m_path,val); }
        /** Access m_isDir
         * \return The current value of m_isDir
         */
        bool IsDir() { return m_isDir; }
        /** Set m_isDir
         * \param val New value to set
         */
        void SetIsDir(bool val=true) { m_isDir = val; }
        /** Access m_size
         * \return The current value of m_size
         */
        long GetSize() { return m_size; }
        /** Set m_size
         * \param val New value to set
         */
        void SetSize(long val) { m_size = val; }
        /** Access m_lastModTime
         * \return The current value of m_lastModTime
         */
        long GetLastModTime() { return m_lastModTime; }
        /** Set m_lastModTime
         * \param val New value to set
         */
        void SetLastModTime(long val) { m_lastModTime = val; }

        void loadChildren();
        bool containsElement(CFSElement*toBeContained,CFSElement**containedElement);
        void copyTo(CFSElement*whereShouldCopy,bool srcToDst=true);
        void cleanChildren();
        void getFullPath(char*fullPath){ sprintf(fullPath,"%s/%s",m_rootPath,m_path);};
        void removeElement(bool fromSrcToDst=true);
        bool analyzed;
    protected:
    private:

        char m_path[1024]; //!< Member variable "m_path"
        char m_rootPath[1024]; //!< Member variable "m_path"
        bool m_isDir; //!< Member variable "m_isDir"
        long m_size; //!< Member variable "m_size"
        long m_lastModTime; //!< Member variable "m_lastModTime"
};

#endif // CFSELEMENT_H
