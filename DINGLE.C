#include <os2.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compress.h"
#include "arg2.h"



BOOL bOverWrite;
BOOL bCompress;


static void Error (PSZ psz1, PSZ psz2)
   {
   printf (psz1, psz2);
   exit (1);
   }



USHORT UnZotFile (FILE *fpIn, PSZ pszIn)
   {
   char   szDrive[_MAX_DRIVE], szDir[_MAX_DIR], 
          szOut[_MAX_FNAME+4], szExt[_MAX_EXT],
          szTmp[16];
   ULONG  ulTotalIn  = 0;
   ULONG  ulTotalOut = 0;
   FILE   *fpOut;
   USHORT i, uInSize, uOutSize;

   printf ("   ");
   _splitpath (pszIn, szDrive, szDir, szOut, szExt);

   fread (szTmp, 1, 4, fpIn);
   if (strncmp (szTmp, "ZOT\x1A", 4))
      {
      printf ("%s not a ZOT file\n", pszIn);
      return 0;
      }
   fread (szExt+1, 1, 3, fpIn);
   for (i=1; i<4; i++)
      szExt[i] -= !!szExt[i];
   strcat (szOut, szExt);

   if (!bOverWrite && !access (szOut, 0))
      {
      printf ("Warning: %s exists, UNZOT not performed\n", szOut);
      return 0;
      }
   if (!(fpOut = fopen (szOut, "wb")))
      {
      printf ("Warning: Unable to open %s, UNZOT not performed\n", szOut);
      return 0;
      }

   printf ("UNZOTTING %s...", szOut);

   while (!feof (fpIn))
      {
      fpEfp (fpOut, &uOutSize, fpIn, &uInSize);
      ulTotalIn  += uInSize;
      ulTotalOut += uOutSize;
      }
   printf ("\b\b\b   \n");
   fclose (fpOut);
   return 1;

   }




USHORT ZotFile (FILE *fpIn, PSZ pszIn)
   {
   char   szDrive[_MAX_DRIVE], szDir[_MAX_DIR], 
          szOut[_MAX_FNAME+4], szExt[_MAX_EXT];
   ULONG  ulTotalIn  = 0;
   ULONG  ulTotalOut = 0;
   FILE   *fpOut;
   USHORT i, uInSize, uOutSize;

   printf ("   ");
   szExt[0] = szExt[1] = szExt[2] = szExt[3] = '\0';
   _splitpath (pszIn, szDrive, szDir, szOut, szExt);
   for (i=1; i<4; i++)
      szExt[i] += !!szExt[i];

   strcat (szOut, ".ZOT");

   if (!bOverWrite && !access (szOut, 0))
      {
      printf ("Warning: %s exists, ZOT not performed\n", szOut);
      return 0;
      }
   if (!(fpOut = fopen (szOut, "wb")))
      {
      printf ("Warning: Unable to open %s, ZOT not performed\n", szOut);
      return 0;
      }

   printf ("ZOTTING %s...", szOut);
   fputs ("ZOT\x1A", fpOut);
   fwrite (szExt+1, 1, 3, fpOut);

   while (!feof (fpIn))
      {
      fpIfp (fpOut, &uOutSize, fpIn, 0, &uInSize);
      ulTotalIn  += uInSize;
      ulTotalOut += uOutSize;
      }
   printf ("\b\b\b   \n");
   fclose (fpOut);
   return 1;
   }


void Usage ()
   {
   printf ("Dingle [-z] [-u] [-o] [-h] fnames\n");
   exit (0);
   }


int main (int argc, char *argv[])
   {
   USHORT  i, uCount = 0;
   FILE    *fpIn;
   PSZ     psz;

   InitCompressModule (TRUE, 3, 1);
   BuildArgBlk ("^Z ^U ^H ^O ^M ?");

   if (FillArgBlk (argv)) Error (GetArgErr (), "");

   if (IsArg ("?") || IsArg ("H") || argc == 1) Usage ();

   bOverWrite = IsArg ("O");
   bCompress  = !IsArg ("U");

   for (i=0; psz = GetArg (NULL, i); i++)
      {
      if (!(fpIn = fopen (psz, "rb")))
         {
         printf ("Warning: Unable to open %s\n", psz);
         continue;
         }
      if (bCompress)
         uCount += ZotFile (fpIn, psz);
      else
         uCount += UnZotFile (fpIn, psz);
      fclose (fpIn);
      }
   if (i > 1)
      printf ("\n %d file%s processed.\n", uCount, (uCount > 1 ? "s" : ""));
   return 0;
   }


