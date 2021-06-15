#include <os2.h>
#include <implode.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RPROC unsigned far pascal
#define WPROC void far pascal


typedef struct
   {
   PSZ    pszBuff;
   FILE   *fp;
   USHORT uIndex;
   USHORT uSize;
   } FLOW;
typedef FLOW *PFLOW;


FLOW SRC;
FLOW DEST;

PSZ    pszWorkBuff;
BOOL   bNULLTERM;
USHORT uERROR;
PSZ    pszERROR;

USHORT uWINDOWSIZE = 4096;
USHORT uMODE       = CMP_BINARY;


char *pszErrors[] = { /*0*/  "No Error",
                      /*1*/  "Null Buffer Pointer",
                      /*2*/  "Buffer Too Small",
                      /*3*/  "Invalid Window Size",
                      /*4*/  "Invalid Mode",
                      /*5*/  "Corrupt data",
                      /*6*/  "Truncated Data",
                      /*7*/  "",
                      /*8*/  ""
                    };


/*------ Buffer to zipper ------*/
RPROC ReadBuff (char far *pszBuff, PUSHORT puSize)
   {
   USHORT i;

   if (!bNULLTERM)
      i = min (*puSize, SRC.uSize - SRC.uIndex);
   else
      for (i=0; i < *puSize && SRC.pszBuff[i + SRC.uIndex]; i++);
   memcpy (pszBuff, SRC.pszBuff + SRC.uIndex, i);
   SRC.uIndex += i;
   return i;
   }


/*------ zipper to Buffer ------*/
WPROC WriteBuff (char far *pszBuff, PUSHORT puSize)
   {
   USHORT i;

   i = min (*puSize, DEST.uSize - DEST.uIndex);
   if (*puSize > DEST.uSize - DEST.uIndex)
      uERROR = 2;
   memcpy (DEST.pszBuff + DEST.uIndex, pszBuff, i);
   DEST.uIndex += i;
   }


/*------ file to zipper ------*/
RPROC ReadFile (char far *pszBuff, PUSHORT puSize)
   {
   USHORT i;
   size_t uRead;

   i = min (*puSize, SRC.uSize - SRC.uIndex);
   uRead = fread(pszBuff, 1, i, SRC.fp);
   SRC.uIndex += uRead;
   return (USHORT) uRead;
   }


/*------ zipper to file ------*/
WPROC WriteFile (char far *pszBuff, PUSHORT puSize)
   {
   fwrite(pszBuff, 1, *puSize, DEST.fp);
   DEST.uIndex += *puSize;
   }


USHORT Error (USHORT uErrorVal)
   {
   pszERROR = pszErrors [uErrorVal];
   return uErrorVal;
   }



void InitFlow (PFLOW pfl, PSZ pszBuff, FILE *fp, USHORT uIndex, USHORT uSize)
   {
   pfl->pszBuff= pszBuff;
   pfl->fp     = fp;
   pfl->uIndex = uIndex;
   pfl->uSize  = uSize;
   }




/*
 * Buffer ---> IMPLODE ---> Buffer
 *
 * Implodes data from a buffer to a buffer
 * if uDataSize=0, source buffer is assumed to be null term.
 * if uCBuffSize=0, no bounds checking is performed for pszImploded
 * on return pUSize contains the size of the Imploded data
 * puSize may be null.
 */
USHORT szIsz (PSZ     pszImploded, // Dest Imploded data buff
              USHORT  uCBuffSize,  // Dest buff Size
              PUSHORT puSize,      // Dest Imploded data size
              PSZ     pszExploded, // Src Exploded data buff
              USHORT  uDataSize)   // Src Exploded data size
   {
   InitFlow (&SRC,  pszExploded, NULL,   0, uDataSize);
   InitFlow (&DEST, pszImploded, NULL,   2, (uCBuffSize ? uCBuffSize : 0xFFFF));

   bNULLTERM = !uDataSize;
   uERROR    = 0;

   if (puSize)   *puSize = 0;
   if (!pszExploded || !pszImploded) return Error (1);

   switch (implode(ReadBuff, WriteBuff, pszWorkBuff, &uMODE, &uWINDOWSIZE))
      {
      case CMP_INVALID_DICTSIZE: return Error (3);
      case CMP_INVALID_MODE:     return Error (4);
      }
   (USHORT)(DEST.pszBuff) = DEST.uIndex;
   *puSize = DEST.uIndex;
   return Error (uERROR);
   }



/*
 * Buffer ---> EXPLODE ---> Buffer
 *
 * Explodes data from a buffer to a buffer
 * uBuffSize is the size of the Exploded buffer (0=no check)
 * on return puSize contains the size of the Exploded data
 * puSize may be null.
 */
USHORT szEsz (PSZ     pszExploded, // Dest Exploded data buff
              USHORT  uBuffSize,   // Dest buff size
              PUSHORT puSize,      // Dest Exploded data size
              PSZ     pszImploded) // Src  Imploded data buffer
   {
   InitFlow (&SRC,  pszImploded, NULL,   2, *((PUSHORT)pszImploded));
   InitFlow (&DEST, pszExploded, NULL,   0, (uBuffSize ? uBuffSize : 0xFFFF));

   bNULLTERM = 0;
   uERROR    = 0;

   if (puSize)   *puSize = 0;
   if (!pszExploded || !pszImploded) return Error (1);

   switch (explode(ReadBuff, WriteBuff, pszWorkBuff))
      {
      case CMP_INVALID_DICTSIZE: return Error (3);
      case CMP_INVALID_MODE:     return Error (4);
      case CMP_BAD_DATA:         return Error (5);
      case CMP_ABORT:            return Error (6);
      }

   if (DEST.uIndex < DEST.uSize)
      pszExploded [DEST.uIndex] = '\0';

   if (puSize)   *puSize = DEST.uIndex;
   return Error (uERROR);
   }



/*
 * Buffer ---> IMPLODE ---> File
 *
 * Implodes data from a buffer to a file
 * uDataSize is the size of the buffer to write.
 * if uDataSize=0, the buffer is considered to be null terminated
 * the buffer must still however be smaller than 64k
 * puSize may be null.
 */
USHORT szIfp  (FILE    *fp,       // Dest file for Imploded data
               PUSHORT puSize,    // Dest Imploded data size
               PSZ     pszBuff,   // Src data buffer
               USHORT  uDataSize) // Src data size
   {
   fpos_t fPos, fPos2;

   InitFlow (&SRC,  pszBuff, NULL, 0, uDataSize);
   InitFlow (&DEST, NULL,    fp,   2, 0);

   bNULLTERM = !uDataSize;
   uERROR    = 0;

   if (puSize)   *puSize = 0;
   if (!pszBuff) return Error (1);

   fgetpos (fp, &fPos);
   fwrite  (&DEST.uIndex, 2, 1, fp);  /*--- save position to write datasize ---*/
   switch (implode (ReadBuff, WriteFile, pszWorkBuff, &uMODE, &uWINDOWSIZE))
      {
      case CMP_INVALID_DICTSIZE: return Error (3);
      case CMP_INVALID_MODE:     return Error (4);
      }
   fgetpos (fp, &fPos2);
   fsetpos (fp, &fPos);
   fwrite  (&DEST.uIndex, 2, 1, fp);  /*--- save Imploded datasize ---*/
   fsetpos (fp, &fPos2);

   if (puSize)  *puSize = DEST.uIndex;
   return Error (uERROR);
   }



/*
 * File ---> EXPLODE ---> Buffer
 *
 * Explodes data from file to buffer.
 * if uBuffSize is 0, buffer overflow is not checked.
 * on return puSize contains the size of the data returned.
 * puSize may be null.
 */
USHORT fpEsz (PSZ     pszBuff,   // Dest buffer
              USHORT  uBuffSize, // Dest buffer size
              PUSHORT puSize,    // Dest Exploded data size
              FILE    *fp)       // Src file with Imploded data
   {
   USHORT uSize;

   fread(&uSize, 2, 1, fp);
   InitFlow (&SRC,  NULL,    fp,   2, uSize);
   InitFlow (&DEST, pszBuff, NULL, 0, (uBuffSize ? uBuffSize : 0xFFFF));
   if (puSize)   *puSize = 0;

   bNULLTERM = FALSE;
   uERROR    = 0;

   if (puSize)   *puSize = 0;
   if (!pszBuff) return Error (1);

   switch (explode(ReadFile, WriteBuff, pszWorkBuff))
      {
      case CMP_INVALID_DICTSIZE: return Error (3);
      case CMP_INVALID_MODE:     return Error (4);
      case CMP_BAD_DATA:         return Error (5);
      case CMP_ABORT:            return Error (6);
      }
   if (DEST.uIndex < DEST.uSize)
      pszBuff [DEST.uIndex] = '\0';

   if (puSize)   *puSize = DEST.uIndex;
   return Error (uERROR);
   }


/*
 * File ---> IMPLODE ---> File
 *
 * Implodes data from a file to a file
 * puIn  is the size of the data read in  (may be null)
 * puOut is the size of the data written  (may be null)
 * uDataSize is the size of the data to read in, 0 means read entire file.
 */
USHORT fpIfp (FILE    *fpOut,    // Dest file for imploded data
              PUSHORT puOut,     // Dest size of imploded data
              FILE    *fpIn,     // Src file of data to implode
              USHORT  uDataSize, // Src size of data to implode
              PUSHORT puIn)      // Src size of data imploded
   {
   fpos_t fPos, fPos2;

   InitFlow (&SRC,  NULL, fpIn,  0, (uDataSize ? uDataSize : 0xFFFF));
   InitFlow (&DEST, NULL, fpOut, 2, 0);
   if (puOut)   *puOut = 0;
   if (puIn)    *puIn  = 0;

   bNULLTERM = FALSE;
   uERROR    = 0;

   fgetpos (fpOut, &fPos);
   fwrite  (&DEST.uIndex, 2, 1, fpOut);  /*--- save position to write datasize ---*/
   switch (implode (ReadFile, WriteFile, pszWorkBuff, &uMODE, &uWINDOWSIZE))
      {
      case CMP_INVALID_DICTSIZE: return Error (3);
      case CMP_INVALID_MODE:     return Error (4);
      }
   fgetpos (fpOut, &fPos2);
   fsetpos (fpOut, &fPos);
   fwrite  (&DEST.uIndex, 2, 1, fpOut);  /*--- save Imploded datasize ---*/
   fsetpos (fpOut, &fPos2);

   if (puOut)  *puOut = DEST.uIndex;
   if (puIn)   *puIn  = SRC.uIndex;
   return Error (uERROR);
   }



/*
 * File ---> EXPLODE ---> File
 *
 * Explodes data from a file to a file
 * puIn  is the size of the data read in  (may be null)
 * puOut is the size of the data written  (may be null)
 */
USHORT fpEfp (FILE    *fpOut, // Dest file for exploded data
              PUSHORT puOut,  // Dest size of exploded data
              FILE    *fpIn,  // Src file with imploded data
              PUSHORT puIn)   // Src size of data expanded
   {
   USHORT uSize;

   fread(&uSize, 2, 1, fpIn);
   InitFlow (&SRC,  NULL, fpIn,  2, uSize);
   InitFlow (&DEST, NULL, fpOut, 0, 0);
   if (puOut)   *puOut = 0;
   if (puIn)    *puIn  = 0;

   bNULLTERM = FALSE;
   uERROR    = 0;

   switch (explode(ReadFile, WriteFile, pszWorkBuff))
      {
      case CMP_INVALID_DICTSIZE: return Error (3);
      case CMP_INVALID_MODE:     return Error (4);
      case CMP_BAD_DATA:         return Error (5);
      case CMP_ABORT:            return Error (6);
      }
   if (puOut)   *puOut = DEST.uIndex;
   if (puIn)    *puIn  = uSize;
   return Error (uERROR);
   }


/*
 * Initialization routine for compression module
 * 
 * bMalloc
 *   if true, pszWorkBuff is allocated 35256 bytes with malloc.
 *   if false it is assumed that you will alloc at least
 *   35256 bytes to use the implode functions or 12574 for explode fns
 * 
 * uWinSize
 *   the sliding window size for implode. the choices are 1,2, or 3.
 *   with 1 being the fastest with least compression. default is 3
 *
 * uMode
 *   the modes are 1 and 2 with 1 being the best for binary data and
 *   2 being the best for ascii data. the default is 1.
 *
 * This init fn need not be called at all if you are happy with the 
 * defaults and you malloc pwzWorkBuff yourself
 *
 * This fn may be called more than once to change the mode and buff size
 * as long as the bMalloc is set TRUE at most once.
 */
USHORT InitCompressModule (BOOL bMalloc, USHORT uWinSize, USHORT uMode)
   {
   switch (uWinSize)
      {
      case 1: uWINDOWSIZE = 1024;
      case 2: uWINDOWSIZE = 2048;
      case 3: uWINDOWSIZE = 4096;
      }
   switch (uMode)
      {
      case 1: uMODE = CMP_BINARY;
      case 2: uMODE = CMP_ASCII;
      }

   if (bMalloc)
      {
      if (!(pszWorkBuff = malloc(35256U)))
         return 1;
      }
   return 0;
   }




/****************************************************************/
/*********************  Module Test  ****************************/
/****************************************************************/
//
//#define SSIZE 1024
//
//int main (int argc, char *argv[])
//   {
//   PSZ    psz;
//   USHORT uSize;
//   FILE   *fp;
//
//   InitCompressModule (TRUE, 3, 1);
//
//   /*-- test szIfp and fpEsz --*/
//   fp  = fopen ("test.bin", "wb");
//   psz = "This is a test to see if this is a test";
//   szIfp (fp, &uSize, psz, 0);
//   printf ("%s\nstring, %d bytes, wrote in %d bytes\n", psz, strlen (psz), uSize);
//
//   psz = "This is a string with mane eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee's";
//   szIfp (fp, &uSize, psz, 0);
//   printf ("%s\nstring, %d bytes, wrote in %d bytes\n", psz, strlen (psz), uSize);
//
//   psz = "HHHHHHHHEEEEEEEEELLLLLLLLLLLLLLLLOOOOOOOOOO";
//   szIfp (fp, &uSize, psz, 0);
//   printf ("%s\nstring, %d bytes, wrote in %d bytes\n", psz, strlen (psz), uSize);
//   fclose (fp);
//
//   psz = malloc (1024);
//
//   fp  = fopen ("test.bin", "rb");
//   fpEsz (psz, 0, &uSize, fp);
//   printf ("%s\nstring, %d bytes, read in %d bytes\n", psz, strlen (psz), uSize);
//   fpEsz (psz, 0, &uSize, fp);
//   printf ("%s\nstring, %d bytes, read in %d bytes\n", psz, strlen (psz), uSize);
//   fpEsz (psz, 0, &uSize, fp);
//   printf ("%s\nstring, %d bytes, read in %d bytes\n", psz, strlen (psz), uSize);
//   fclose (fp);
//
//   return 0;
//   }
//


