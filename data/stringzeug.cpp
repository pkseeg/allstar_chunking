/***************************************************************************
                        stringzeug.h  -  description
                             -------------------
    begin                : Mon Mar 20 2006
    copyright            : (C) 2006 by Mark Weyer
    email                : cuyo-devel@nongnu.org

Modified 2006,2008,2010,2011 by the cuyo developers

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "stringzeug.h"
#include "fehler.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>



Str::Str(const std::string & s) : inhalt(s) {}
Str::Str() : inhalt("") {}
Str::Str(const char * inhalt_) : inhalt(inhalt_) {CASSERT(inhalt_);}
Str::Str(const Str & s) : inhalt(s.inhalt) {}

bool Str::isEmpty() const {return inhalt.empty();}
int Str::length() const {return inhalt.length();}
char Str::operator [] (int i) const {
  CASSERT(i>=0);
  CASSERT((unsigned int) i<inhalt.length());
  return inhalt[i];
}
char & Str::operator [] (int i) {
  CASSERT(i>=0);
  CASSERT((unsigned int) i<inhalt.length());
  return inhalt[i];
}

const char * Str::data() const {return inhalt.c_str();}

Str Str::left(int l) const {return inhalt.substr(0,l);}
Str Str::mid(int s, int l) const {return inhalt.substr(s,l);}
Str Str::right(int s) const {return inhalt.substr(s);}

void Str::operator += (char c) {inhalt+=c;}
void Str::operator += (const Str & s) {inhalt += s.inhalt;}

bool Str::operator == (const Str & s) const {
  return inhalt == s.inhalt;
}

bool Str::operator < (const Str & s) const {
  return inhalt < s.inhalt;
}

bool Str::operator != (const Str & s) const {
  return inhalt != s.inhalt;
}

bool Str::operator != (const char * s) const {
  CASSERT(s);
  return inhalt != s;
}




Str operator + (const Str & s1, const Str & s2) {
  return s1.inhalt + s2.inhalt;
}

Str operator + (const Str & s1, const char * s2) {
  CASSERT(s2);
  return s1.inhalt + s2;
}

Str operator + (const Str & s, char c) {
  return s.inhalt + c;
}

Str operator + (char c, const Str & s) {
  return c + s.inhalt;
}


Str _sprintf(const char * format, ...) {
  va_list ap;
  va_start(ap, format);
  Str ret = _vsprintf(format, ap);
  va_end(ap);
  return ret;
}

Str _vsprintf(const char * format, va_list ap) {
  char * s;
  if (vasprintf(&s, format, ap) < 0)
    /* If this failed, we probably cannot even output an error message,
       so we do not bother to try. */
    exit(1);
  Str ret = s;
  free(s);
  return ret;
}

void print_to_stderr(const Str & str) {
  size_t len;
  char * s = convert_for_stdout(str.data(),len);
  fwrite(s, sizeof(char), len, stderr);
  free(s);
}

