/*
 * Arg2.c
 *
 * Craig Fitzgerald
 *
 * This file is used by the EBS module
 * 
 * This file provides an easy method to handle those nasty command
 * line parameters.
 * 
 */


#include <os2.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "arg2.h"

#define TIME __TIME__
#define DATE __DATE__

#define STRLEN    512
#define VALLEN    128

typedef PSZ  *PPSZ;

typedef struct instblk
   {
   USHORT   uIndex;
   PSZ      pszVal;
   } INST;
typedef INST *PINST;


typedef struct
   {
   PSZ      pszParam;
   BOOL     bMinimal;
   BOOL     bNoCase;
   CHAR     cValType;
   USHORT   uCount;
   PINST    pinst;
   } ARG;
typedef ARG  *PARG;



BOOL   bInit   = FALSE;
PARG   parg;
PINST  pinstFree;
USHORT uGlobalIndex = 0;

USHORT uArgErr = 0;
char   szArgErr[STRLEN];
char   szErrBuf[STRLEN];


char *Errors[] = {"No Error",                                     // 0
                  "Unknown Parameter, or, argument required for", // 1
                  "Parameter in bad form",                        // 2
                  "BuildArgBlk must be called first",             // 3
                  "EOL Encountered before Param name read",       // 4
                  "Value expected but not found",                 // 5
                  "Requested Parameter not defined",              // 6
                  "Ambiguous Parameter",                          // 7
                  "Bad Parameter Value Type Flag for",            // 8
                  "Requested Parameter not Entered",              // 9
                  "Index too large",                              // 10
                  NULL};                                  


USHORT GetStringVal (PPSZ ppsz, PSZ psz, BOOL bIgnoreSwitch);

/*******************************************************************/
/*******************************************************************/

USHORT SetErr (USHORT uErr, PSZ pszErr)
   {
   uArgErr = uErr;
   if (pszErr)
      strcpy (szArgErr, pszErr);
   else 
      szArgErr[0] = '\0';
   return uErr;
   }



USHORT IsArgErr (void)
   {
   return uArgErr;
   }



PSZ GetArgErr (void)
   {
   char  szTmp[STRLEN];
   PSZ   p;

   p = szArgErr;
   GetStringVal (&p, szTmp, TRUE);
   sprintf (szErrBuf, "%s : \"%s\"", Errors[uArgErr], szTmp);
   return szErrBuf;
   }



char Eat (PPSZ ppsz, PSZ pszEatList)
   {
   while (**ppsz && strchr (pszEatList, **ppsz))
      (*ppsz)++;
   return **ppsz;
   }



BOOL EatChar (PPSZ ppsz, CHAR c)
   {
   BOOL   bReturn;

   Eat (ppsz, " \t\n");
   if (bReturn = ((CHAR) **ppsz == (CHAR) c))
      (*ppsz)++;
   return bReturn;
   }



USHORT BuildArgInfo (PSZ pszArgStr)
   {
   USHORT i,uArgs = 0;
   CHAR   c, szParam[VALLEN];

   parg = (PARG) malloc (sizeof (ARG));
   parg->pszParam = NULL;

   while (1)
      {
      if (!Eat (&pszArgStr, " \t\n"))
         break;

      uArgs++;
      parg = (PARG) realloc (parg, sizeof (ARG) * (uArgs+1));
      parg[uArgs - 1].pinst = NULL;
      parg[uArgs - 1].uCount = 0;

      if (parg[uArgs-1].bMinimal = (*pszArgStr == '*'))
         pszArgStr++;

      if (parg[uArgs-1].bNoCase = (*pszArgStr == '^'))
         pszArgStr++;

      i = 0;
      if (!(szParam[i++] = *pszArgStr++))
         return SetErr (4, pszArgStr);

      while (*pszArgStr && !strchr ("@$=:?-%# \t\n", *pszArgStr))
         szParam[i++] = *pszArgStr++;
      szParam[i] = '\0';

      parg[uArgs-1].pszParam = strdup (szParam);

      c = parg[uArgs-1].cValType = (CHAR)(*pszArgStr ? *pszArgStr: ' ');

      if (strchr ("\n\t\r\b", c))
         parg[uArgs-1].cValType = ' ';

      if (!*pszArgStr++)
         break;
      }
   parg[uArgs].pszParam = NULL;
   }



void Flatten (PPSZ argv, PSZ pszCmdLine)
   {
   *pszCmdLine = '\0';

   for (argv++; *argv != NULL; argv++)
      strcat (strcat (pszCmdLine, *argv), " ");
   }



void AddNewParam (USHORT i, PSZ pszVal)
   {
   parg[i].uCount++;
   parg[i].pinst = (PINST) realloc (parg[i].pinst, 
                                     sizeof (INST) * parg[i].uCount);
   parg[i].pinst[parg[i].uCount-1].uIndex = uGlobalIndex - 1;
   parg[i].pinst[parg[i].uCount-1].pszVal = (pszVal ? strdup (pszVal) : NULL);
   }



USHORT GetStringVal (PPSZ ppsz, PSZ psz, BOOL bIgnoreSwitch)
   {
   PSZ p = psz;

   *psz = '\0';
   Eat (ppsz, " \t\n");

   if (!bIgnoreSwitch && (**ppsz == '/' || **ppsz == '-'))
      return 5;

   while (**ppsz && !strchr (" \t\n", **ppsz))
      *psz++ = *(*ppsz)++;
   *psz = '\0';

   return !!(*p == '\0') * 2;
   }



/*
 * This fn expects the index of the matching arg blk
 *                 the the cmd line
 *                 the ptr into the cmd line just after the matched param 
 *
 * returns 0 if it added ok
 *         2 if bad form.
 *         5 if string val is bad or non existant
 */
USHORT AddMatchedParam (USHORT uIndex, PPSZ ppszCL, PSZ pszValPtr)
   {
   CHAR   szTmp[STRLEN];
   USHORT uError;

   switch (parg[uIndex].cValType)
      {
      case ' ':
         if (*pszValPtr != ' ')
            return 2;
         AddNewParam (uIndex, NULL);
         *ppszCL = pszValPtr;
         return 0;

      case '$':
         if (*pszValPtr != ' ')
            return 2;
         if (uError = GetStringVal (&pszValPtr, szTmp, 0))
            return uError;
         AddNewParam (uIndex, szTmp);
         *ppszCL = pszValPtr;
         return 0;

      case '=':
         if (!EatChar (&pszValPtr, '='))
            return 2;
         if (uError = GetStringVal (&pszValPtr, szTmp, 0))
            return uError;
         AddNewParam (uIndex, szTmp);
         *ppszCL = pszValPtr;
         return 0;

      case ':':
         if (!EatChar (&pszValPtr, ':'))
            return 2;
         if (uError = GetStringVal (&pszValPtr, szTmp, 0))
            return uError;
         AddNewParam (uIndex, szTmp);
         *ppszCL = pszValPtr;
         return 0;

      case '%':
         Eat (&pszValPtr, " \t\n:=");
         if (uError = GetStringVal (&pszValPtr, szTmp, 0))
            return uError;
         AddNewParam (uIndex, szTmp);
         *ppszCL = pszValPtr;
         return 0;

      case '#':
         Eat (&pszValPtr, " \t\n:=");
         if (uError = GetStringVal (&pszValPtr, szTmp, 1))
            return uError;
         AddNewParam (uIndex, szTmp);
         *ppszCL = pszValPtr;
         return 0;

      case '?':
         Eat (&pszValPtr, " \t\n:=");
         uError = GetStringVal (&pszValPtr, szTmp, 0);
         AddNewParam (uIndex, (uError ? NULL : szTmp));
         *ppszCL = pszValPtr;
         if (uError && (*(pszValPtr -1) == ' ' || *(pszValPtr -1) == '\t'))
            (*ppszCL)--;
         return 0;

      case '@':
         if (*pszValPtr == ' ')
            return 2;
         if (uError = GetStringVal (&pszValPtr, szTmp, 1))
            return uError;
         AddNewParam (uIndex, szTmp);
         *ppszCL = pszValPtr;
         return 0;

      case '-':
         AddNewParam (uIndex, NULL);
         *ppszCL = pszValPtr;
         return 0;
      }
   return 8;
   }



USHORT FindMinParam (PPSZ ppszCL)
   {
   USHORT i, uLen, uMatch, uCount = 0;
   SHORT  iComp;
   PSZ    p1, p2;

   uLen = strlen (*ppszCL);
   if (p1 = strchr (*ppszCL, '='))
      uLen = p1 - *ppszCL;

   if (p2 = strchr (*ppszCL, ' '))
      uLen = p2 - *ppszCL;

   if (p1 && p2 && p1 < p2)
      uLen = p1 - *ppszCL;

   for (i=0; parg && parg[i].pszParam; i++)
      {
      if (parg[i].bNoCase)
         iComp = strnicmp (parg[i].pszParam, *ppszCL, uLen);
      else
         iComp = strncmp (parg[i].pszParam, *ppszCL, uLen);

      uCount += !iComp;
      if (!iComp) uMatch = i;
      }

   if (!uCount)
      return SetErr (1, *ppszCL);             // no match

   if (uCount > 1)
      return SetErr (7, *ppszCL);             // >1 match

   if (!parg[uMatch].bMinimal)
      return SetErr (1, *ppszCL);             // match not bMin

   if (!parg[uMatch].cValType == '@' ||
      !parg[uMatch].cValType == '-' )
      return SetErr (1, *ppszCL);             // cannot match @ or - types

   return AddMatchedParam (uMatch, ppszCL, *ppszCL + uLen);
   }



/* 
 *
 */
USHORT FindParam (PSZ *ppszCL, CHAR cType)
   {
   USHORT i, uLen, uRet;
   SHORT  iComp;

   for (i=0; parg && parg[i].pszParam; i++)
      {
      if (cType != parg[i].cValType)
         continue;
      if ((uLen = strlen (parg[i].pszParam)) > strlen (*ppszCL))
         continue;
      if (parg[i].bNoCase)
         iComp = strnicmp (parg[i].pszParam, *ppszCL, uLen);
      else
         iComp = strncmp (parg[i].pszParam, *ppszCL, uLen);

      if (iComp) continue;

      uRet = AddMatchedParam (i, ppszCL, *ppszCL + uLen);
      if (!uRet || uRet > 2)
         return SetErr (uRet, *ppszCL);
      }
   return SetErr (1, *ppszCL);
   }




void AddFreeParam (PPSZ ppszCL)
   {
   char     szStr[STRLEN];
   USHORT   i;

   GetStringVal (ppszCL, szStr, 1);
   for (i=0; pinstFree && pinstFree[i].pszVal; i++)
      ;
   pinstFree = (PINST) realloc (pinstFree, sizeof (INST) * (i+2));
   pinstFree[i].pszVal   = strdup (szStr);
   pinstFree[i].uIndex   = uGlobalIndex - 1;
   pinstFree[i+1].pszVal = NULL;
   }



USHORT Digest (PSZ pszCL)
   {
   USHORT   i, uRet;
   PSZ      pszLook;

   while (1)
      {
      if (!Eat (&pszCL, " \t\n"))
         break;

      if (*pszCL != '-' && *pszCL != '/')
         {
         uGlobalIndex++;
         AddFreeParam (&pszCL);
         continue;
         }
      pszCL++;

      while (1)
         {
         if (!*pszCL || *pszCL == ' ')
            break;

         uGlobalIndex++;

         pszLook = " $%#?=:@-";

         for (i=0; *pszLook != '\0'; pszLook++)
            {
            uRet = FindParam (&pszCL, *pszLook);
            if (!uRet || !uRet > 2)
               break;
            }

         if (uRet == 1)
            uRet = FindMinParam (&pszCL);

         if (!uRet) continue;

         return SetErr (uRet, pszCL);
         }
      }
   return SetErr (0, NULL);
   }




USHORT BuildArgBlk (PSZ pszArgDef)
   {
   char szCpy [STRLEN];
   PSZ  pszCpy = szCpy;


   if (!pszArgDef)
      return SetErr (3, NULL);

   strcpy (szCpy, pszArgDef);
   if (*pszCpy == '"')
      {
      pszCpy++;
      szCpy[strlen (szCpy) -1] = '\0';
      }

   bInit    = TRUE;
   parg     = NULL;
   pinstFree = NULL;
   return BuildArgInfo (pszCpy);
   }


 
USHORT FillArgBlk (PSZ argv[])
   {
   CHAR  szCmdLine [STRLEN];

   if (!bInit)
      return SetErr (3, NULL);
   Flatten (argv, szCmdLine);
   return Digest (szCmdLine);
   }



USHORT FillArgBlk2 (PSZ pszArgStr)
   {
   char szCpy [STRLEN];
   PSZ  pszCpy = szCpy;

   if (!bInit)
      return SetErr (3, NULL);
   if (!pszArgStr)
      return 0;

   strcpy (szCpy, pszArgStr);
   if (*pszCpy == '"')
      {
      pszCpy++;
      szCpy[strlen (szCpy) -1] = '\0';
      }
   strcat (pszCpy, " ");
   return Digest (pszCpy);
   }



/*******************************************************************/



USHORT IsArg (PSZ pszArg)
   {
   USHORT i;
   SHORT  iComp;

   if (!bInit)
      {
      SetErr (3, NULL);
      return 0xFFFF;
      }

   if (!pszArg) /*--- free Args ---*/
      {
      for (i=0; pinstFree && pinstFree[i].pszVal; i++)
         ;
      return i;
      }
   for (i=0; parg && parg[i].pszParam; i++)
      {
      if (parg[i].bNoCase)
         iComp = stricmp (parg[i].pszParam, pszArg);
      else
         iComp = strcmp (parg[i].pszParam, pszArg);

      if (!iComp)
         return parg[i].uCount;
      }
   SetErr (6, pszArg);
   return 0xFFFF;
   }



PSZ GetArg (PSZ pszArg, USHORT uIndex)
   {
   USHORT i, j;
   SHORT  iComp;

   if (!bInit)
      {
      SetErr (3, NULL);
      return NULL;
      }

   if (!pszArg) /*--- free Args ---*/
      {
      for (i=0; pinstFree && pinstFree[i].pszVal; i++)
         if (i == uIndex)
            return pinstFree[i].pszVal;
      SetErr (6, pszArg);
      return NULL;
      }

   for (i=0; parg && parg[i].pszParam; i++)
      {
      if (parg[i].bNoCase)
         iComp = stricmp (parg[i].pszParam, pszArg);
      else
         iComp = strcmp (parg[i].pszParam, pszArg);
      if (iComp)
         continue;

      for (j=0; j< parg[i].uCount; j++)
         if (j == uIndex)
            return parg[i].pinst[j].pszVal;
      SetErr (9, pszArg);
      return NULL;
      }
   SetErr (6, pszArg);
   return NULL;
   }



USHORT GetArgIndex (PSZ pszArg, USHORT uIndex)
   {
   USHORT i, j;
   SHORT  iComp;

   if (!bInit)
      {
      SetErr (3, NULL);
      return 0xFFFF;
      }

   if (!pszArg) /*--- free Args ---*/
      {
      for (i=0; pinstFree && pinstFree[i].pszVal; i++)
         if (i == uIndex)
            return pinstFree[i].uIndex;
      SetErr (6, pszArg);
      return 0xFFFF;
      }

   for (i=0; parg && parg[i].pszParam; i++)
      {
      if (parg[i].bNoCase)
         iComp = stricmp (parg[i].pszParam, pszArg);
      else
         iComp = strcmp (parg[i].pszParam, pszArg);
      if (iComp)
         continue;

      for (j=0; j< parg[i].uCount; j++)
         if (j == uIndex)
            return parg[i].pinst[j].uIndex;
      SetErr (9, pszArg);
      return 0xFFFF;
      }
   SetErr (6, pszArg);
   return 0xFFFF;
   }



USHORT EnumArg (PSZ pszArg, PSZ pszVal, USHORT uIndex)
   {
   USHORT i, j;

   if (pszArg) pszArg[0] = '\0';
   if (pszVal) pszVal[0] = '\0';
   if (!bInit)
      return !SetErr (3, NULL);
   if (uIndex >= uGlobalIndex)
      return !SetErr (10, NULL);


   for (i=0; pinstFree && pinstFree[i].pszVal; i++)
      if (pinstFree[i].uIndex == uIndex)
         {
         if (pszArg) strcpy (pszArg, pinstFree[i].pszVal);
         return 1;
         }

   for (i=0; parg && parg[i].pszParam; i++)
      for (j=0; j< parg[i].uCount; j++)
         if (parg[i].pinst[j].uIndex == uIndex)
            {
            if (pszArg) strcpy (pszArg, parg[i].pszParam);
            if (parg[i].pinst[j].pszVal && pszVal)
               strcpy (pszVal, parg[i].pinst[j].pszVal);
            return 2;
            }

   SetErr (9, NULL);
   return FALSE;
   }



/*******************************************************************/



void Dump (void)
   {
   USHORT i,j ;

   printf ("ARG Cmd Line Parse Utility v1.0     Craig Fitzgerald      %s  %s\n\n", TIME, DATE);

   for (i=0; parg[i].pszParam; i++)
      {
      printf ("bMin: %d   bNoCase: %d   cType: %c   uCount: %d   Param: %s\n", 
              parg[i].bMinimal, 
              parg[i].bNoCase, 
              parg[i].cValType, 
              parg[i].uCount, 
              parg[i].pszParam);

      for (j=0; j <parg[i].uCount && parg[i].pinst; j++)
         printf ("   Index: %d   Val : %s\n", 
                 parg[i].pinst[j].uIndex,  
                 parg[i].pinst[j].pszVal);
      printf ("\n");
      }
   for (i=0; pinstFree && pinstFree[i].pszVal; i++)
      {
      printf ("Free: Index: %d    Val: %s\n", 
              pinstFree[i].uIndex, 
              pinstFree[i].pszVal); 
      }

   }



//cdecl main (int argc, char *argv[])
//   {
//   USHORT uRet; 
//   PSZ    psz, pArgStr;
//
//
//   pArgStr = getenv ("PARG");
//
//   printf ("%s\n\n", pArgStr);
//
//   if (uRet = BuildArgBlk (pArgStr))
//      {
//      psz = GetArgErr ();
//      printf ("Build Error : %s\n\n", psz);
//      }
//
//   if (uRet = FillArgBlk (argv))
//      {
//      psz = GetArgErr ();
//      printf ("Fill Arg Error : %s\n\n", psz);
//      }
//
//   Dump ();
//   return 0;
//   }




