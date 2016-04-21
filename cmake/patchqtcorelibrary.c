/*******************************************************************************

Licensed to the OpenCOR team under one or more contributor license agreements.
See the NOTICE.txt file distributed with this work for additional information
regarding copyright ownership. The OpenCOR team licenses this file to you under
the Apache License, Version 2.0 (the "License"); you may not use this file
except in compliance with the License. You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the
specific language governing permissions and limitations under the License.

*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#define NEEDLE "qt_prfxpath="
#define NEEDLE_SIZE 12

int main(int argc, char **argv)
{
	FILE *file;
	size_t i, fileSize, filePosition;
	char *fileContents;

    // Make sure that we have the correct number of arguments

    if (argc != 2)
        return 1;

    // Open the Qt Core library and determine its size

    file = fopen(argv[1], "r+b");

    if (!file)
        return 1;

    fseek(file, 0, SEEK_END);

    fileSize = ftell(file);

    // Allocate enough memory to read the whole contents of the Qt Core library

	fileContents = (char *) malloc(fileSize+1);

	if (!fileContents) {
        fclose(file);

        return 1;
    }

	// Read the whole contents of the Qt Core library
    
	fseek(file, 0, SEEK_SET);

	if (!fread(fileContents, fileSize, 1, file)) {
        fclose(file);

        return 1;
    }

    // Look for our needle

    filePosition = -1;

    for (i = 0; i < fileSize; ++i) {
        if (fileContents[i] == NEEDLE[0]) {
            if (!strncmp(fileContents+i, NEEDLE, NEEDLE_SIZE)) {
                filePosition = i;

                break;
            }
        }
    }

    if (filePosition == -1) {
        fclose(file);

        return 1;
    }

    // Patch the Qt Core library

    filePosition += NEEDLE_SIZE;

    fileContents[filePosition] = '.';
    
    while (fileContents[++filePosition] != 0)
        fileContents[filePosition] = 0;

	fseek(file, 0, SEEK_SET);

	if (!fwrite(fileContents, fileSize, 1, file)) {
        fclose(file);
        
        return 1;
    }
    
	fclose(file);

	// Free the memory we previously allocated

	free(fileContents);

    return 0;
}
