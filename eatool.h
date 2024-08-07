/**
 * Author     : 0xb4db01
 * Date       : 07/08/2024
 * 
 * Description: Header file for EATool.exe, a tool to read/write Extended
 * Attributes to files on a Windows NTFS filesystem.
 *
 * Credits    :
 * - reenz0h (@SEKTOR7net)
 * - Regin / ZeroAccess / Adam "Hexacorn"
 */

#pragma once

#include <windows.h>
#include <winternl.h>

/************************************
 * Windows API functions definition *
 ************************************/
typedef NTSTATUS (NTAPI * ZwQueryEaFile_t)(
	HANDLE           FileHandle,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID            Buffer,
	ULONG            Length,
	BOOLEAN          ReturnSingleEntry,
	PVOID            EaList,
	ULONG            EaListLength,
	PULONG           EaIndex,
	BOOLEAN          RestartScan
);

typedef NTSTATUS (NTAPI * ZwSetEaFile_t)(
	HANDLE           FileHandle,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID            Buffer,
	ULONG            Length
);

ZwQueryEaFile_t pZwQueryEaFile	= NULL;
ZwSetEaFile_t pZwSetEaFile		= NULL;

/*
 * Some errors, list is incomplete so I guess this is a TODO for the future.
 */
#define STATUS_NO_EAS_ON_FILE 			0xC0000052
#define STATUS_EAS_NOT_SUPPORTED 		0xC000004F
#define STATUS_INSUFFICIENT_RESOURCES	0xC000009A
#define STATUS_EA_LIST_INCONSISTENT		0x80000014
#define FILE_NEED_EA					0x00000080
#define STATUS_BUFFER_TOO_SMALL         0x00000023


/**************
 * PROTOTYPES *
 **************/
int win32API_init();
HANDLE open_read(const char *filename);
HANDLE open_read_write(const char *filename);
char *data_read(HANDLE filename);
int EA_read(HANDLE src, char * buf, size_t buf_size);
char *EA_get_value_raw(char *buff, size_t buff_size, const char *eaname);
FILE_FULL_EA_INFORMATION *EA_get_header(char *buff);
int EA_write(HANDLE src, char * EAname, char * EAvalue, size_t EAvalue_size);
int EA_print_data(char * c, size_t size);
int EA_print(char * buff, size_t buff_size, const char *eaname);

/****************************
 * FUNCTIONS IMPLEMENTATION *
 ****************************/

/*
 * Resolve needed win 32 API functions
 */
int win32API_init()
{
    pZwQueryEaFile = (ZwQueryEaFile_t) GetProcAddress(
            GetModuleHandle("ntdll.dll"), 
            "ZwQueryEaFile");

    pZwSetEaFile = (ZwSetEaFile_t) GetProcAddress(
            GetModuleHandle("ntdll.dll"), 
            "ZwSetEaFile");

    if (pZwQueryEaFile == NULL || pZwSetEaFile == NULL) {
        printf("[!] Could not resolve Zw API calls.\n");

        return -1;		
    }

    return 0;
}

/*
 * Open a file in read-only mode
 */
HANDLE open_read(const char *filename)
{
    HANDLE file_handle = CreateFile(
            filename, 
            GENERIC_READ, 
            0, 
            NULL, 
            OPEN_EXISTING, 
            0, 
            NULL);

	if (filename == INVALID_HANDLE_VALUE) {
		printf("[!] Could not open file: %s\n", file_handle);

        exit(-1);
	}

    return file_handle;
}

/*
 * Open a file in read-write mode
 */
HANDLE open_read_write(const char *filename)
{
    HANDLE file_handle = CreateFile(
            filename, 
            GENERIC_READ | GENERIC_WRITE, 
            0, 
            NULL, 
            OPEN_EXISTING, 
            0, 
            NULL);

	if (filename == INVALID_HANDLE_VALUE) {
		printf("[!] Could not open file: %s\n", file_handle);

        exit(-1);
	}

    return file_handle;
}

/*
 * Read data from a file. Maximum read bytes is set to 65535 bytes as this is
 * mainly used to read data from a file and store it as an EA value, which has
 * such size limitation.
 * See: https://wiki.archlinux.org/title/Extended_attributes#Support
 */
char *data_read(HANDLE hFile)
{
    LPVOID *lpBuffer = (LPVOID *)malloc(sizeof(char) * 65535);
    DWORD lpNumberOfBytesRead;

    if(ReadFile(hFile, lpBuffer, 65536, &lpNumberOfBytesRead, NULL))
    {
        printf("[EATOOL]::Read bytes from file: %d\n", lpNumberOfBytesRead);
    } else
    {
        printf("[EATOOL]::Error: data read %d\n", GetLastError());

        exit(-1);
    }

    return (char *)lpBuffer;
}

/* 
 *Read in all EA attributes. Note that here param buf is OUT.
 */
int EA_read(HANDLE src, OUT char *buf, size_t buf_size)
{
	IO_STATUS_BLOCK IoStatusBlock;
	
    NTSTATUS status = pZwQueryEaFile(
            src, 
            &IoStatusBlock, 
            (PFILE_FULL_EA_INFORMATION) buf, 
            buf_size, 
            FALSE, 
            NULL, 
            0, 
            NULL, 
            TRUE);

	if (status == STATUS_NO_EAS_ON_FILE) {
		printf("[!] There's no EA attribute on the file\n");

        exit(0);
	}

	if (status != 0) {
		printf("[!] Could not read file's EA records (%x)\n", status);

		return -1;		
	}	

	return 0;
}

/*
 * Get a *raw* value from an extended attribute specified with ename.
 * Pararmeter buff is the buffer returned by a EA_read() call.
 */
char *EA_get_value_raw(char * buff, size_t buff_size, const char *eaname)
{
	char * p = NULL;
	char EA_name[256] = { 0 };
	char *pEA_name;

    FILE_FULL_EA_INFORMATION *EA_header = EA_get_header(buff);

	p = buff;

	while(TRUE) {
		pEA_name = (char *) (p + 8);
		memset(EA_name, 0, 256);
		memcpy(EA_name, pEA_name, EA_header->EaNameLength);

        if(strncmp(eaname, EA_name, 256) == 0)
        {
            if(EA_header->EaValueLength > 0)
            {
                char *ret = (char *)malloc(
                        sizeof(char) * EA_header->EaValueLength);
                
                snprintf(
                        ret, 
                        EA_header->EaValueLength + 1,
                        "%s", 
                        (p + 8 + EA_header->EaNameLength + 1));

                return ret;
            }
        }

        if(EA_header->NextEntryOffset == 0) break;

        p += EA_header->NextEntryOffset;

        EA_header = EA_get_header(p);
	}

	return 0;
}

FILE_FULL_EA_INFORMATION *EA_get_header(char *buff)
{
    return (FILE_FULL_EA_INFORMATION *)buff;
}

/*
 * Write a single EA attribute
 */
int EA_write(HANDLE src, char * EAname, char * EAvalue, size_t EAvalue_size)
{
    /*
     * We check if the name provided doesn't exceed 255 bytes
     */
    if(strlen(EAname) > 255)
    {
        printf("[EATOOL]::Error: EA Name is too long (max 255 bytes)\n");

        exit(-1);
    }

	IO_STATUS_BLOCK IoStatusBlock;
	FILE_FULL_EA_INFORMATION dumpEA = { 0 };

	/* 
     * Setup the EA information structure
     */
	dumpEA.NextEntryOffset = 0;
	dumpEA.Flags = 0;
	dumpEA.EaNameLength = (UCHAR) strlen(EAname);
	dumpEA.EaValueLength = (USHORT) EAvalue_size;

    strcpy_s(dumpEA.EaName, dumpEA.EaNameLength + 1, EAname);

	memcpy(
            dumpEA.EaName + dumpEA.EaNameLength + 1, 
            EAvalue, 
            dumpEA.EaValueLength);
	
	/*
     * Send it to the kernel
     */
	NTSTATUS status = pZwSetEaFile(
            src, 
            &IoStatusBlock, 
            (PVOID) &dumpEA, 
            sizeof(FILE_FULL_EA_INFORMATION)
                    + dumpEA.EaNameLength 
                    + 1 
                    + dumpEA.EaValueLength);

	if (status != 0) {
		printf("[!] Error writing EA (%x)\n", status);
		return -1;
	}

	return 0;
}

/*
 * Quick and dirty way to print data of unknown type
 */
int EA_print_data(char * c, size_t size)
{
    // 16-char line + 1 null byte
	char temp[17];
	int i = 0;

	printf("\t");

	while ( i < size) {
        // print offset of the bytes
		printf("%.8x:  ", i);

        // print hex bytes, 16 per line
		for (int j = 0 ; j < 16 ; j++) {
            // put extra space in the middle of the line
			if (j == 8) printf(" ");

            // print the current byte
			printf("%.2hhx ", *c);

            // if current byte is printable, save it as-is
            // otherwise, save it as a dot
			if (*c >= 32 && *c <= 126) temp[j] = *c;
			else temp[j] = '.';

            // if in the last line...
			if ( ++i >= size) {
                // add extra space between hex values and ascii string
				for ( int a = 0; a < (16 - j) * 2 - 1; a++)
					printf(" ");
				break;
			}

            // take the next byte
			c++;
		}

        // add the trailing null byte
		temp[16] = '\0';

        // and print the line as a string
		printf("%#20s\n\t", temp);

		memset(temp, 0, 16);			
	}

	printf("\n");
	
	return 0;
}

/*
 * Print the data returned from ZwQueryEaFile()
 */
int EA_print(char * buff, size_t buff_size, const char *eaname)
{
	char * p = NULL;
	char EA_name[256] = { 0 };
	char *pEA_name;

    FILE_FULL_EA_INFORMATION *EA_header = EA_get_header(buff);

	p = buff;

	while(TRUE) {
		pEA_name = (char *) (p + 8);
		memset(EA_name, 0, 256);
		memcpy(EA_name, pEA_name, EA_header->EaNameLength);

        if(!eaname || strncmp(eaname, EA_name, 256) == 0)
        {
            EA_print_data(
                    p, 
                    sizeof(FILE_FULL_EA_INFORMATION)
                            + EA_header->EaNameLength
                            + EA_header->EaValueLength
                            + 1);

            printf(
                    "\t- EA_next_entry_offset = %#lx\n", 
                    EA_header->NextEntryOffset);

            printf("\t- EaFlags              = %#x\n", EA_header->Flags);
            printf("\t- EaNameLen            = %x\n", EA_header->EaNameLength);

            printf(
                    "\t- EaValueLength        = %d\n", 
                    EA_header->EaValueLength);

            printf("\t- EA Name              = %s\n", EA_name);

            if(EA_header->EaValueLength > 0)
            {
                printf("\t- EAValue:\n\n");
                
                EA_print_data(
                        (p + 8 + EA_header->EaNameLength + 1),
                        EA_header->EaValueLength);
            }
            
            if(eaname) break;
        }

        if(EA_header->NextEntryOffset == 0) break;

        p += EA_header->NextEntryOffset;

        EA_header = EA_get_header(p);
	}

	return 0;
}
