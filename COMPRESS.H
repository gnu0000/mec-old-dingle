/*
 * Compress.h
 * Craig Fitzgerald
 *
 * Compression module
 *
 * szIsz (pszImploded, uCBuffSize, puSize, pszExploded, uDataSize)
 * szEsz (pszExploded, uBuffSize, puSize, pszImploded)
 * szIfp (*fp, puSize, pszBuff, uDataSize)
 * fpEsz (pszBuff, uBuffSize, puSize, *fp)
 * fpIfp (*fpOut, puOut, *fpIn, uDataSize, puIn)
 * fpEfp (*fpOut, puOut, *fpIn, puIn);
 * InitCompressModule (bMalloc, uWinSize, uMode)
 *
 * PSZ    pszWorkBuff;
 * PSZ    pszERROR;
 *
 *
 * In this module, the following nomenclature/notation is used:
 *  Imploded data is data that has been compressed 
 *  Exploded data is data that has been uncompressed, hence the original data
 *  The functions in this module implode and explode data to and from
 *  memory buffers and files.  Functions are named as follows:
 *
 *  (Source)(operation)(Destination)
 *
 *  where Source and Destination either sz or fp (for buffer or file)
 *  and operation is either I (for Implode) or E (for Explode)
 *
 *  so that the function szIfp takes the data from a buffer, compresses it
 *  and writes the compressed data to a file handle.
 *
 * The parameters are ordered so that the destination parameters always
 * preceed the source parameters. Note that When the source data is 
 * compressed data, there is no source data size parameter, as the size
 * of a compressed stream is stored in that stream.
 *
 * see individual function header declarations below for more info
 * about the function.
 */


/*
 * Compression work area
 * This is malloced from either the InitCompressModule fn
 * or from the user.
 */
extern PSZ    pszWorkBuff;


/*
 * Error String valid after call to a compression fn.
 * non zero return from a fn signifies an error.
 */
extern PSZ    pszERROR;




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
USHORT InitCompressModule (BOOL bMalloc, USHORT uWinSize, USHORT uMode);



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
              USHORT  uDataSize);  // Src Exploded data size


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
              PSZ     pszImploded);// Src  Imploded data buffer



/*
 * Buffer ---> IMPLODE ---> File
 *
 * Implodes data from a buffer to a file
 * uSize is the size of the buffer to write.
 * if uSize=0, the buffer is considered to be null terminated
 * the buffer must still however be smaller than 64k
 * puSize may be null.
 */
USHORT szIfp  (FILE    *fp,       // Dest file for Imploded data
               PUSHORT puSize,    // Dest Imploded data size
               PSZ     pszBuff,   // Src data buffer
               USHORT  uDataSize);// Src data size


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
              FILE    *fp);      // Src file with Imploded data


/*
 * File ---> IMPLODE ---> File
 *
 * Implodes data from a file to a file
 * puIn  is the size of the data read in  (may be null)
 * puOut is the size of the data written  (may be null)
 * uDataSize is the size of the data to read in, 0 means read entire file.
 * if uDataSize is 0 and the file is >64k, only the first 64k is read,
 *  and you will need to call this in a look until feof(fpIn)
 *  (and you will then need to read the file in a loop too)
 */
USHORT fpIfp (FILE    *fpOut,    // Dest file for imploded data
              PUSHORT puOut,     // Dest size of imploded data
              FILE    *fpIn,     // Src file of data to implode
              USHORT  uDataSize, // Src size of data to implode
              PUSHORT puIn);     // Src size of data imploded



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
              PUSHORT puIn);  // Src size of data expanded

