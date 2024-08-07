# EATool

## Description

EATool is a tool to read/write Extended Attributes to files on a Windows NTFS filesystem.

I wrote this tool during the Sektor7 Advanced Malware Development course for I felt the need of a tool to debug my RTO code during the exercises and also to have some reusable code for future projects.

Finally, I thought it would be nice to have an independent tool for interacting with Extended Attributes in general, maybe one day it can turn in handy.

## Compile

EATool is written in C and meant to be cross-compiled with MingW.

Once you have MingW installed you can run make. Following, EATool's Makefile

```
CC=x86_64-w64-mingw32-g++

all:
	$(CC) $(CINC) eatool.cpp -o eatool.exe

clean:
	rm -rf *.exe
```

## Usage

Running eatool.exe without parameters, or with -h, will display the help message:

```
C:\Users\gibbons> .\eatool.exe
[EATOOL]::Help
Options:
        -r -f <file>: reads all EA from <file>
        -r -n <name> -f <file>: reads EA <name> from <file>

        -w -n <name> -c <content> -f <file>: writes EA <name> with <content> in <file>
        -w -n <name> -i <content file> -f <file>: writes EA <name> with data from <content file> in <file>
```

eatool.exe on a file with no EAs:

```
PS C:\Users\gibbons> .\eatool.exe -r -f .\Desktop\pippo.txt
[!] There's no EA attribute on the file
```

Parameters -w -n -c and -f will create a new EA and display it:

```
PS C:\Users\gibbons> .\eatool.exe -w -n ROTOBUDDAX -c "MINCHIA, RADIO CYBERNET!" -f .\Desktop\pippo.txt
        00000000:  00 00 00 00 00 0a 18 00  52 4f 54 4f 42 55 44 44     ........ROTOBUDD
        00000010:  41 58 00 4d 49 4e 43 48  49 41 2c 20 52 41 44 49     AX.MINCHIA, RADI
        00000020:  4f 20 43 59 42 45 52 4e  45 54 21 00 00 00 00         O CYBERNET!....

        - EA_next_entry_offset = 0
        - EaFlags              = 0
        - EaNameLen            = a
        - EaValueLength        = 24
        - EA Name              = ROTOBUDDAX
        - EAValue:

        00000000:  4d 49 4e 43 48 49 41 2c  20 52 41 44 49 4f 20 43     MINCHIA, RADIO C
        00000010:  59 42 45 52 4e 45 54 21                              YBERNET!

PS C:\Users\gibbons> .\eatool.exe -w -n FUCO -c "SUCA IL FUCO" -f .\Desktop\pippo.txt
        00000000:  00 00 00 00 00 04 0c 00  46 55 43 4f 00 53 55 43     ........FUCO.SUC
        00000010:  41 20 49 4c 20 46 55 43  4f 00 00 00 00               A IL FUCO....

        - EA_next_entry_offset = 0
        - EaFlags              = 0
        - EaNameLen            = 4
        - EaValueLength        = 12
        - EA Name              = FUCO
        - EAValue:

        00000000:  53 55 43 41 20 49 4c 20  46 55 43 4f                 SUCA IL FUCO
```

Parameters -r and -f will read *all* EAs in the file:

```
 PS C:\Users\gibbons> .\eatool.exe -r -f .\Desktop\pippo.txt
        00000000:  2c 00 00 00 00 0a 18 00  52 4f 54 4f 42 55 44 44     ,.......ROTOBUDD
        00000010:  41 58 00 4d 49 4e 43 48  49 41 2c 20 52 41 44 49     AX.MINCHIA, RADI
        00000020:  4f 20 43 59 42 45 52 4e  45 54 21 00 00 00 00         O CYBERNET!....

        - EA_next_entry_offset = 0x2c
        - EaFlags              = 0
        - EaNameLen            = a
        - EaValueLength        = 24
        - EA Name              = ROTOBUDDAX
        - EAValue:

        00000000:  4d 49 4e 43 48 49 41 2c  20 52 41 44 49 4f 20 43     MINCHIA, RADIO C
        00000010:  59 42 45 52 4e 45 54 21                              YBERNET!

        00000000:  00 00 00 00 00 04 0c 00  46 55 43 4f 00 53 55 43     ........FUCO.SUC
        00000010:  41 20 49 4c 20 46 55 43  4f 00 00 00 00               A IL FUCO....

        - EA_next_entry_offset = 0
        - EaFlags              = 0
        - EaNameLen            = 4
        - EaValueLength        = 12
        - EA Name              = FUCO
        - EAValue:

        00000000:  53 55 43 41 20 49 4c 20  46 55 43 4f                 SUCA IL FUCO
```

Parameters -r -n and -f will read a single attribute specified with -n:

```
PS C:\Users\gibbons> .\eatool.exe -r -n ROTOBUDDAX -f .\Desktop\pippo.txt
        00000000:  2c 00 00 00 00 0a 18 00  52 4f 54 4f 42 55 44 44     ,.......ROTOBUDD
        00000010:  41 58 00 4d 49 4e 43 48  49 41 2c 20 52 41 44 49     AX.MINCHIA, RADI
        00000020:  4f 20 43 59 42 45 52 4e  45 54 21 00 00 00 00         O CYBERNET!....

        - EA_next_entry_offset = 0x2c
        - EaFlags              = 0
        - EaNameLen            = a
        - EaValueLength        = 24
        - EA Name              = ROTOBUDDAX
        - EAValue:

        00000000:  4d 49 4e 43 48 49 41 2c  20 52 41 44 49 4f 20 43     MINCHIA, RADIO C
        00000010:  59 42 45 52 4e 45 54 21                              YBERNET!
```

You can also write the content of a file as an EA value. But beware with echo and UNICODE!

```
PS C:\Users\gibbons> .\eatool.exe -w -n "DSFDSAF" -i .\Desktop\test.txt -f .\Desktop\pippo.txt
[EATOOL]::Read bytes from file: 18
        00000000:  00 00 00 00 00 07 12 00  44 53 46 44 53 41 46 00     ........DSFDSAF.
        00000010:  64 66 73 61 66 64 73 61  66 73 61 0d 0a 64 66 73     dfsafdsafsa..dfs
        00000020:  61 66 00 00 00 00                                    af....

        - EA_next_entry_offset = 0
        - EaFlags              = 0
        - EaNameLen            = 7
        - EaValueLength        = 18
        - EA Name              = DSFDSAF
        - EAValue:

        00000000:  64 66 73 61 66 64 73 61  66 73 61 0d 0a 64 66 73     dfsafdsafsa..dfs
        00000010:  61 66                                                af
```
