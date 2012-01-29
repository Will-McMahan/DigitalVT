#ifndef _FILEHELPER_H_
#define _FILEHELPER_H_

#include <stdio.h>
#include <string>



int fileExists(std::string filename)
{
	FILE* file;
    if( file = fopen(filename.c_str(), "r") )
    {
        fclose(file);
        return 1;
    }

    return 0;
}

int permissionToOverwrite(std::string filename)
{
	char answer;
	
	printf("Do you want to overwrite the file: %s? (y/n) ", filename.c_str());

	while(1) {
		answer = getch();
		if (answer == 'y' || answer == 'Y') {
			printf("\n");
			return 1;
		}
		else if (answer == 'n' || answer == 'N') {
			printf("\n");
			return 0;
		}
		else {
			printf("\nInvalid answer: (y/n) ");
		}
	}
	return -1; 
}

#endif