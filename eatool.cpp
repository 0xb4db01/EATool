/**
 * Author     : 0xb4db01
 * Date       : 07/08/2024
 * 
 * Description: EATool.exe, a tool to read/write Extended Attributes to files
 * on a Windows NTFS filesystem.
 * I wrote this tool during the Sektor7 Advanced Malware Development course for
 * I felt the need of a tool to debug my RTO code during the exercises and also
 * to have some reusable code for future projects.
 * Finally, I thought it would be nice to have an independent tool for
 * interacting with Extended Attributes in general, maybe one day it can turn
 * in handy.
 * All credits are in the header file.
 *
 */

#include <stdio.h>
#include <getopt.h>
#include <string.h>

#include "eatool.h"

void help()
{
    printf("[EATOOL]::Help\n");
    printf("Options:\n");

    printf("\t-r -f <file>: reads all EA from <file>\n");
    printf("\t-r -n <name> -f <file>: reads EA <name> from <file>\n");

    printf("\n");

    printf("\t-w -n <name> -c <content> -f <file>:\
 writes EA <name> with <content> in <file>\n");
    printf("\t-w -n <name> -i <content file> -f <file>:\
 writes EA <name> with data from <content file> in <file>\n");
}

int main(int argc, char *argv[]) {
    if(argc <= 1)
    {
        help();

        return -1;
    }

    if(win32API_init() == -1)
    {
        printf("[EATOOL]::Error: could not initialize win api pointers.\n");

        return -1;
    }

    int opt = -1;
    bool readfile = false;
    char *eaname = NULL;
    bool writefile = false;
    bool eacontent_from_file = false;
    char *eacontent = NULL;
    char *inputfile = NULL;
    char *filename = NULL;

    while((opt = getopt(argc, argv, "rwf:i:c:n:h")) != -1)
    {
        switch(opt)
        {
            case 'r':
                readfile = true;
            break;
            case 'w':
                writefile = true;
            break;
            case 'f':
                filename = optarg;
            break;
            case 'n':
                eaname = optarg;
            break;
            case 'c':
                eacontent = optarg;
            break;
            case 'i':
                eacontent_from_file = true;
                inputfile = optarg;
            break;
            case 'h':
                help();
            break;
            default:
                help();
        }
    }

    if(readfile && writefile)
    {
        printf("[EATOOL]::Error: can't choose both -r and -w arguments.\n");

        return -1;
    }

    if(readfile && filename == NULL)
    {
        printf("[EATOOL]::Error: must provide a file to read.\n");

        return -1;
    } else if(readfile && filename)
    {
        /*
         * If eaname is NULL, we read all entries, else we read the specified
         * one.
         */
        char buff[8192];
        HANDLE file_handle = open_read(filename);
        EA_read(file_handle, buff, 8192);

        if(!eaname)
        {
           EA_print(buff, 8192, NULL);
        } else
        {
           EA_print(buff, 8192, eaname);
        }

        return 0;
    }

    if(writefile && filename == NULL)
    {
        printf("[EATOOL]::Error: must provide a file to write.\n");

        return -1;
    } else if(writefile && filename)
    {
        if(!eaname)
        {
            printf("[EATOOL]::Error: must provide a name for EA.\n");

            return -1;
        } else
        {
            /*
             * EA content can be taken from a file with -i or from command line
             * with -c...
             */
            if(eacontent_from_file)
            {
                HANDLE input_file_handle = open_read(inputfile);

                eacontent = data_read(input_file_handle);
            } else
            {
                if(!eacontent)
                {
                    printf("[EATOOL]::Error: provide content.\n");

                    exit(-1);
                }
            }

            HANDLE file_handle = open_read_write(filename);

            EA_write(
                    file_handle, 
                    eaname, 
                    eacontent, 
                    strlen(eacontent));

            char buff[8192];

            EA_read(file_handle, buff, 8192);
            EA_print(buff, 8192, eaname);
        }

        return 0;
    }

	return 666;
}
