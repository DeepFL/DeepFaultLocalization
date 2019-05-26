
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <stdarg.h>

#define ml_max_xio_buf 4100
int fn_log_f( char * fi_fmt, ...)
{
  char fl_msg[ml_max_xio_buf];
  int numChars;
  va_list fl_ap;

  if (!fi_fmt)
    {
      return 10;
    }

  fl_msg[0] = '\0';

  va_start(
           fl_ap,
           fi_fmt);

#ifdef WIN32
  numChars = _vsnprintf(
#else
  numChars =  vsnprintf(
#endif
                        fl_msg,
                        (size_t)ml_max_xio_buf,
                        fi_fmt,
                        fl_ap);
  fl_msg[ml_max_xio_buf - 1] = '\0';
  va_end(fl_ap);
  fwrite(fl_msg,(size_t) 1, strlen(fl_msg), stdout);
  return 0;
}

/* Wraps call to fopen and checks the errno global variable for error codes. */

FILE* fn_f_open(const char * filename, const char * mode)
{
    FILE * fptr = fopen(filename, mode);
    int err = errno;
    if ( (fptr == NULL) && (err != 0) )
      {
        fn_log_f("- I_ERR_FOP_X; error; fopen of %s with mode %s returned errno %d\n",
                 filename, mode, err);
        perror("- I_ERR_FOP_X; error; ");
      }
    return fptr;
}
