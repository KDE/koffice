/* config.h.  Generated by cmake from config.h.cmake */

/* Define to 1 if you have the <crt_externs.h> header file. */
#cmakedefine HAVE_CRT_EXTERNS_H 1

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
   */
#cmakedefine HAVE_DIRENT_H 1

/* Define to 1 if you have the `fabsl' function. */
/* #undef HAVE_FABSL */

/* Define to 1 if you have the `fseek64' function. */
/* #undef HAVE_FSEEK64 */

/* Define to 1 if fseeko (and presumably ftello) exists and is declared. */
#define HAVE_FSEEKO 1

/* Define to 1 if you have the `ftell64' function. */
/* #undef HAVE_FTELL64 */

/* Define to 1 if you have the <ieeefp.h> header file. */
#cmakedefine HAVE_IEEEFP_H 1

/* Define to 1 if you have the <inttypes.h> header file. */
#cmakedefine HAVE_INTTYPES_H 1


/* Define to 1 if you have the <memory.h> header file. */
#cmakedefine HAVE_MEMORY_H 1

/* Define to 1 if you have the `mkstemp' function. */
#cmakedefine HAVE_MKSTEMP 1

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
#cmakedefine HAVE_NDIR_H 1

/* Define if your system needs _NSGetEnviron to set up the environment */
#cmakedefine HAVE_NSGETENVIRON 1

/* GetMagickInfoList has different number of arguments with versions >= 6.1.3
   */
/* #undef HAVE_OLD_GETMAGICKINFOLIST */

/* Defines if your system has the OpenEXR library */
#cmakedefine HAVE_OPENEXR 1

/* Define to 1 if you have the <paper.h> header file. */
/* #undef HAVE_PAPER_H */

/* Define to 1 if you have the <paths.h> header file. */
#cmakedefine HAVE_PATHS_H 1

/* Define to 1 if you have the `popen' function. */
#define HAVE_POPEN 1

/* define if you have libreadline available */
#define HAVE_READLINE 1

/* Define if you have res_init */
#cmakedefine HAVE_RES_INIT 1

/* Define if you have the res_init prototype */
#cmakedefine HAVE_RES_INIT_PROTO 1

/* Define to 1 if you have the `rewinddir' function. */
#define HAVE_REWINDDIR 1

/* Define if you have a STL implementation by SGI */
#define HAVE_SGI_STL 1

/* Define to 1 if you have the `snprintf' function. */
#cmakedefine HAVE_SNPRINTF 1

/* Define to 1 if you have the <stdint.h> header file. */
#cmakedefine HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#cmakedefine HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#cmakedefine HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#cmakedefine HAVE_STRING_H 1

/* Define if you have strlcat */
#cmakedefine HAVE_STRLCAT 1

/* Define if you have the strlcat prototype */
#cmakedefine HAVE_STRLCAT_PROTO 1

/* Define if you have strlcpy */
#cmakedefine HAVE_STRLCPY 1

/* Define if you have the strlcpy prototype */
#cmakedefine HAVE_STRLCPY_PROTO 1

/* Define to 1 if you have the <sys/bitypes.h> header file. */
#cmakedefine HAVE_SYS_BITYPES_H 1

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
#cmakedefine HAVE_SYS_DIR_H 1

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
#cmakedefine HAVE_SYS_NDIR_H 1

/* Define to 1 if you have the <sys/param.h> header file. */
#cmakedefine HAVE_SYS_PARAM_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#cmakedefine HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#cmakedefine HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine HAVE_UNISTD_H 1

/* Define to 1 if you have the `vsnprintf' function. */
#cmakedefine HAVE_VSNPRINTF 1



/* Define the PREFIX where to install this package */
#define PREFIX "${CMAKE_INSTALL_PREFIX}"

/* The size of a `char *', as computed by sizeof. */
#define SIZEOF_CHAR_P ${SIZEOF_CHAR_P}

/* The size of a `int', as computed by sizeof. */
#define SIZEOF_INT ${SIZEOF_INT}

/* The size of a `long', as computed by sizeof. */
#define SIZEOF_LONG ${SIZEOF_LONG}

/* The size of a `short', as computed by sizeof. */
#define SIZEOF_SHORT ${SIZEOF_SHORT}


/* Version number of package */
#define VERSION "1.4.88"

/* Defined if compiling without arts */
/* #undef WITHOUT_ARTS */

/* Define to 1 if your processor stores words with the most significant byte
   first (like Motorola and SPARC, unlike Intel and VAX). */
/* #undef WORDS_BIGENDIAN */

/* Defines the executable of xmllint */
#define XMLLINT "/usr/bin/xmllint"




/*
 * AIX defines FD_SET in terms of bzero, but fails to include <strings.h>
 * that defines bzero.
 */

#if defined(_AIX)
#include <strings.h>
#endif



#if defined(HAVE_NSGETENVIRON) && defined(HAVE_CRT_EXTERNS_H)
# include <sys/time.h>
# include <crt_externs.h>
# define environ (*_NSGetEnviron())
#endif


/* Number of bits in a file offset, on hosts where this is settable. */
#define _FILE_OFFSET_BITS 64


#if !defined(HAVE_RES_INIT_PROTO)
#ifdef __cplusplus
extern "C" {
#endif
int res_init(void);
#ifdef __cplusplus
}
#endif
#endif



#if !defined(HAVE_STRLCAT_PROTO)
#ifdef __cplusplus
extern "C" {
#endif
unsigned long strlcat(char*, const char*, unsigned long);
#ifdef __cplusplus
}
#endif
#endif



#if !defined(HAVE_STRLCPY_PROTO)
#ifdef __cplusplus
extern "C" {
#endif
unsigned long strlcpy(char*, const char*, unsigned long);
#ifdef __cplusplus
}
#endif
#endif


/* Define to 1 to make fseeko visible on some hosts (e.g. glibc 2.2). */
/* #undef _LARGEFILE_SOURCE */

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */


/*
 * On HP-UX, the declaration of vsnprintf() is needed every time !
 */

#if !defined(HAVE_VSNPRINTF) || defined(hpux)
#if __STDC__
#include <stdarg.h>
#include <stdlib.h>
#else
#include <varargs.h>
#endif
#ifdef __cplusplus
extern "C"
#endif
int vsnprintf(char *str, size_t n, char const *fmt, va_list ap);
#ifdef __cplusplus
extern "C"
#endif
int snprintf(char *str, size_t n, char const *fmt, ...);
#endif


#if defined(__SVR4) && !defined(__svr4__)
#define __svr4__ 1
#endif
