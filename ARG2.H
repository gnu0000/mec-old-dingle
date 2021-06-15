/*
 * ARG2.h
 *
 * 03/03/92
 * (c) 1992 Info Tech.
 * Craig Fitzgerald
 *
 * Command line argument parsing library
 *
 * -------------------------------------------------------------------------
 * | Setup Functions                                //                     |
 * | -------------------------                      //                     |
 * | USHORT BuildArgBlk (PSZ pszArgDef);            // init fn             |
 * | USHORT FillArgBlk (PSZ argv[]);                // for cmd line        |
 * | USHORT FillArgBlk2 (PSZ pszArgStr);            // for env strings etc.|
 * |                                                //                     |
 * | Query Functions                                //                     |
 * | -------------------------                      //                     |
 * | USHORT IsArg (PSZ pszArg);                     // # times entered     |
 * | PSZ    GetArg (PSZ pszArg, USHORT uIndex);     // getarg value        |
 * | USHORT GetArgIndex (PSZ pszArg, USHORT uIndex);// position in cmd line|
 * | USHORT EnumArg (pszArg, pszVal, USHORT i);     // get i-th argument   |
 * |                                                //                     |
 * | Error Functions                                //                     |
 * | -------------------------                      //                     |
 * | USHORT IsArgErr (void);                        // was there an error? |
 * | PSZ    GetArgErr (void);                       // get error string    |
 * | void   Dump(void);                             // for debug           |
 * -------------------------------------------------------------------------
 *
 *
 * ARG2 provides a means to easily access your command line 
 * arguments.  To use this module you need to perform 3 steps
 * (in order):
 *
 *  1> Create an Argument Block (using BuildArgBlk)
 *
 *  2> Fill the argument Block (FillArgBlk and/or FillArgBlk2)
 *
 *  3> Query Info about the parameters (IsArg, GetArg, GetArgIndex)
 *
 *  See individual Fn descriptions for there use.
 */




/* This function sets up what parameters are valid and how they
 * may be specified. This is done in an Argument Definition string
 * Its form is one or more of this: (separated by spaces)
 *
 *  *^PName@
 *  ||  |  |
 *  ||  |  | Prameter value specifier
 *  ||  |  | ------------------------
 *  ||  |    - (space) Parameter has no value
 *  ||  |  - - Parameter has no value and is 'combinable'
 *  ||  |  $ - Parameter value is preceeded by whitespace
 *  ||  |  @ - Parameter value is not preceeded by whitespace
 *  ||  |  = - Parameter value is preceeded by = sign
 *  ||  |  : - Parameter value is preceeded by : sign
 *  ||  |  % - Parameter value is type @ $ = or :, all ok
 *  ||  |  ? - Parameter may have a value (if val, type %)
 *  ||  |  # - like % but allow val to start with '-' or '/'
 *  ||  |
 *  ||  |
 *  ||  Parameter name to look for
 *  ||
 *  ||
 *  |Case Insensitivity Flag, Leave out to be Case Sensitive
 *  |
 *  |
 *  Minimul Matching Flag, leave out for exact match only
 *
 * 'combinable' means that multiple options may immediatly follow
 * one another. for example, having -a and -b specified as -ab.
 *
 * EXAMPLES:
 *
 *  BuildArgBlk ("^a- ^b- ^c- ^x- ^y- ^z-")
 *     several simple combinable arguments. They are all case Insensitive
 *     and have no values.  This would accept a command line like this:
 *        -aBc -x -Y /z /azy
 *
 *  BuildArgBlk ("*^XSize$ *^YSize$")
 *     these are minimul matching, case Insensitive, and have values.
 *     The following command line parameters would be legal:
 *        -XSize 100 -YSize 100
 *        -X 100 -Y 100 -xs 200
 *        -y 100 -Xs 100
 *
 *  BuildArgBlk ("*^XSize% *^YSize%")
 *     Like above but would also allow:
 *        -X100 -y=100 -xs = 125 -x:123 -x :456
 *
 *  a nonzero return signifies an error, see GetArgErr to see error string.
 */
extern USHORT BuildArgBlk (PSZ pszArgDef);



/*
 * This function you just call with your argv and it will add the 
 * command line parameters to the argument block.
 *
 *  a nonzero return signifies an error, see GetArgErr to see error string
 */
extern USHORT FillArgBlk (PSZ argv[]);



/*
 * This function is added so you may add additional parameters, say from
 * an enviroment variable or a file. Input is a string.
 *
 *  a nonzero return signifies an error, see GetArgErr to see error string
 */
extern USHORT FillArgBlk2 (PSZ pszArgStr);







/*
 * Use this function to see if an argument has been entered.
 * Use the full argument name as used in BuildArgBlk. (i.e. no
 * minimal match string name) but case may not be important.
 * If you use NULL for pszArg, this will return the number of
 * 'free' arguments.  'free' arguments are arguments that are not 
 * preceeded by a switched (- or /) character
 *
 * EXAMPLE:  i = IsArg("XSize");
 *
 * The return value is the number of times it has been specified.
 * a 0xFFFF return signifies an error.
 */
extern USHORT IsArg (PSZ pszArg);


/*
 * This will retrieve values from parameters that have values.
 * pszArg is the arg whose value you want, and uIndex is the
 * instance you want (0 based)
 * If you use NULL for pszArg, this will return a 'free' parameter
 * 
 *
 *  
 * EXAMPLE: your code has: BuildArgBlk ("*^XSize$ *^YSize$")
 *          cmd line is:   -Y 5 -X 0 -Y 6 -X 50 -X 150 -X 300 -X 440 jim joe
 *          then:          pszVal = GetArg ("XSize", 2);
 *               pszVal would be "150"
 *          and:           pszVal = GetArg (NULL, 1);
 *               pszVal would be "jim"
 *
 * a NULL return signifies an error, see GetArgErr to see error
 */
extern PSZ GetArg (PSZ pszArg, USHORT uIndex);


/*
 * all parameters encountered are numbered and stored sequentially
 * starting from 0. This value has nothing to do with its argc 
 * position (mainly to do with combinable params & values)
 * use this fn like the one above.
 *
 * EXAMPLE:  if setup is like above example, then
 *            uIndex = GetArg ("XSize", 2);
 *            uIndex == 4
 *
 * a 0xFFFF return signifies an error.
 */
extern USHORT GetArgIndex (PSZ pszArg, USHORT uIndex);



/* 
 * This will return the parameters in the order entered.
 * pass the uIndex of the parameter to receive (0 based)
 * pszArg will contain the argument (unless ptr is null)
 * pszVal will contain the argument value (unless ptr is null)
 * Free arguments will have no pszVal.
 *
 * a return of 0 means uIndex > count of parameters
 * a return of 1 means a regular parameter
 * a return of 2 means parameter is of type 'free'
 */
extern USHORT EnumArg (PSZ pszArg, PSZ pszVal, USHORT uIndex);



/*
 * returns current state
 * 0 means no error.
 */
extern USHORT IsArgErr (void);


/*
 * if in an error state, calling this will 
 * give a descriptive error string
 */
extern PSZ GetArgErr (void);


/*
 * for debug
 * this prints all info about all parameters
 */
extern void Dump(void);

