/***************************************************************************
                          cuyointl.cpp  -  description
                             -------------------
    begin                : Sun Nov 9 2008
    copyright            : (C) 2008 by Mark Weyer
    email                : cuyo-devel@nongnu.org

Modified 2009-2011 by the cuyo developers

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <config.h>
#include <cstdlib>
#include <cstring>
#include "cuyointl.h"


char * copy(const char * s) {
  size_t l = strlen(s)+1;
  char * ret = (char*) malloc(l*sizeof(char));
  memcpy((void*) ret, (void*) s, l*sizeof(char));
  return ret;
}

#ifdef ENABLE_NLS

  #include <locale.h>
  #include <langinfo.h>
  #include <string.h>
  #include <iconv.h>
  #include <stdio.h>
  #include <errno.h>

  const char * font_charset = "ISO-8859-1";
  const char * stdout_charset;

  const char * our_gettext(const char * to_translate) {
    if (*to_translate)
      return gettext(to_translate);
    else return "";
  }

  void init_NLS() {

    /* PACKAGE comes from config.h.
       LOCALEDIR is given on the command line, but also originates from
       the configure run. */

    setlocale (LC_ALL, "");
    stdout_charset = nl_langinfo(CODESET);
    bindtextdomain (PACKAGE, LOCALEDIR);
    textdomain (PACKAGE);
    bind_textdomain_codeset (PACKAGE, font_charset);
  }

  char * convert_error(char * ret, char * s, const char * message) {
    fputs(message,stderr);
    free(ret);
    return s;
  }

  char * convert_from_internal(const char * to_charset,
			       const char * source,
			       size_t & outlen) {
      /* In this function, we assume that the iconv implementation is sane
	 enough not to behave worse for unconvertable but correctly encoded
	 characters than for illegal encodings. */

    /* We make a copy of the source string in order to discard the const. */
    char * s = copy(source);

    size_t inlen = strlen(s);
    char * inpos=s;
    size_t inleft=inlen;

    size_t outsize=inlen;
    char * ret = (char*) malloc((outsize+1)*sizeof(char));
      /* The +1 is always to have room for the final null character. */
    char * outpos=ret;
    size_t outleft=outsize;

    iconv_t handle = iconv_open(to_charset, font_charset);
    if (handle == (iconv_t) -1)
      return convert_error(ret,s,"Character conversion not possible.");

    while (*inpos) {
      size_t res = iconv(handle,&inpos,&inleft,&outpos,&outleft);
      if (res == (size_t) -1)
	/* An error occured */
	switch (errno) {
  	  case EILSEQ:
	    /* An invalid character. We just skip it. */
	    inpos++;
	    break;
	  case E2BIG: {
	    /* The output buffer was too small.
	       We enlarge it and retry (in the next iteration of the loop). */
	    size_t outused = outpos-ret;
	    outsize *= 2;
	    ret = (char*) realloc((void*) ret, (outsize+1)*sizeof(char));
              /* The +1 is always to have room for the final null character. */
	    outpos = ret+outused;
	    outleft = outsize-outused;
	    break;
	  }
	  default:
	    return convert_error(ret,s,"Internal error during conversion.");
	}
    }

    /* The input string is finished, but the conversion may still have to
       put some stuff in the output string. */
    bool finished = false;
    while (!finished) {
      size_t res = iconv(handle,NULL,&inleft,&outpos,&outleft);
      if (res == (size_t) -1)
	/* An error occured */
	switch (errno) {
	  case E2BIG: {
	    /* The output buffer was too small.
	       We enlarge it and retry (in the next iteration of the loop). */
	    size_t outused = outpos-ret;
	    outsize *= 2;
	    ret = (char*) realloc((void*) ret, (outsize+1)*sizeof(char));
              /* The +1 is always to have room for the final null character. */
	    outpos = ret+outused;
	    outleft = outsize-outused;
	    break;
	  }
	  default:
	    return convert_error(ret,s,"Internal error during conversion.");
	}
      else finished=true;
    }

    if (iconv_close(handle) == -1)
      return convert_error(ret,s,"Internal error during conversion.");
    *outpos = 0;

    free(s);

    outlen = outpos-ret;
    return ret;
  }

  char * convert_for_font(const char * s) {return copy(s);}

  char * convert_for_stdout(const char * s, size_t & len) {
    return convert_from_internal(stdout_charset, s, len);
  }

  char * convert_for_window_title(const char * s) {
    size_t len;
    return convert_from_internal("UTF-8", s, len);
  }

#else

  void init_NLS() {}
  char * convert_for_font(const char * s) {return copy(s);}
  char * convert_for_stdout(const char * s, size_t & len) {
    len = strlen(s);
    return copy(s);
  }
  char * convert_for_window_title(const char * s) {return copy(s);}

#endif




