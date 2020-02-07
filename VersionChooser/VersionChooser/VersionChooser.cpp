// VersionChooser.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include<string>
#include<iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include "dirent.h"
#include <stdio.h>

#include <sys/types.h>
#include <errno.h>

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <fileapi.h>

#define MAX_VERSIONS 1000
#define MAX_VERSION_LEN 100
#define CHAR_SIZE sizeof(char)

bool isDir(const char* name)
{
	DIR* directory = opendir(name);
	if (NULL != directory) {
		closedir(directory);
		return true;
	}
	return false;
}


int listVersions(char* root, char* versions, char* dirs, char* selectedVersion) {
	int counter = -1;
	DIR* dir;
	if (NULL != (dir = opendir(root))) {

		struct dirent* ent;
		while (NULL != (ent = readdir(dir)) && MAX_VERSIONS > counter) {
			if ('.' == ent->d_name[0]) {
				continue;
			}
			size_t rootPathLen = strlen(root);
			size_t dirNameLen = strlen(ent->d_name);
			char fullPath[MAX_PATH];
			if (NULL != fullPath) {
				strcpy(fullPath, root);
				strcat(fullPath, "\\");
				strcat(fullPath, ent->d_name);
				if (isDir(fullPath)) {
					counter++;
					size_t fullPathLen = strlen(fullPath);
					strcpy(versions + MAX_VERSION_LEN * CHAR_SIZE * counter, ent->d_name);
					strcpy(dirs + MAX_VERSION_LEN * CHAR_SIZE * counter, fullPath);
					strcpy(selectedVersion, ent->d_name);
				}
			}
		}

		closedir(dir);
	}
	return counter;
}

void chooseVersion(char* versions, char* selectedVersion, int counter) {
	printf("Choose the version (Enter for default):\n");
	for (int i = 0; i <= counter; i++) {
		printf("%d: %s\n", i, versions + MAX_VERSION_LEN * CHAR_SIZE * i);
	}
	char selectedItem[255];
	fgets(selectedItem, 255, stdin);
	if (0 < strlen(selectedItem)) {
		int selectedItemIndex = atoi(selectedItem);

		strcpy(selectedVersion, versions + MAX_VERSION_LEN * CHAR_SIZE * selectedItemIndex);
	}
}

int main(int argc, char* argv[])
{
	int returnValue = 0;
	//VersionChooser "D:\Programs" "clojure" "TempFileName"
	//printf("Current dir: %s\n", argv[1]);
	char* programsPath = argv[1];
	char* programName = argv[2];
	char* tempPath = argv[3];
	char expectedVersion[MAX_VERSION_LEN];
	expectedVersion[0] = 0;

	if (argc == 5) {
		strcpy(expectedVersion, argv[4]);
	}

	char rootPath[MAX_PATH];
	if (NULL != rootPath) {
		strcpy(rootPath, programsPath);
		strcat(rootPath + strlen(programsPath), "\\");
		strcat(rootPath + strlen(programsPath) + 1, programName);
	}
	char selectedVersion[255];
	char* versions = (char*)malloc(MAX_VERSIONS * MAX_VERSION_LEN * CHAR_SIZE);
	char* dirs = (char*)malloc(MAX_VERSIONS * MAX_PATH * CHAR_SIZE);
	if (NULL != versions && NULL != dirs) {
		if (strlen(expectedVersion) > 0) {
			strcpy(selectedVersion, expectedVersion);
		}else {
			int counter = listVersions(rootPath, versions, dirs, selectedVersion);
			if (counter > 0) {
				chooseVersion(versions, selectedVersion, counter);
			}
		}
		printf("Selected Version: %s\n", selectedVersion);

		//Write file with
		FILE* settingsFile = fopen(tempPath, "w");
		fprintf(settingsFile, "set %s_VERSION=%s\n", programName, selectedVersion);
		fclose(settingsFile);
	}
	else {
		perror("");
		returnValue = EXIT_FAILURE;
	}

	free(dirs);
	free(versions);
	return returnValue;
}
