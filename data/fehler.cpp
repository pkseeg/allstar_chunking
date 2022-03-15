/***************************************************************************
                          fehler.cpp  -  description
                             -------------------
    begin                : Fri Apr 20 2001
    copyright            : (C) 2001 by Immi
    email                : cuyo@karimmi.de

Modified 2002,2003,2006,2010,2011 by the cuyo developers

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


/* Auf 1 setzen, wenn man mit gdb arbeitet und "backtrace" machen können
   möchte, wenn ein Fehler gethrowt wird. */
#define BEI_FEHLER_NACH_GDB 0


#include <cstdlib>
#include <cstdio>
#if BEI_FEHLER_NACH_GDB
#include <signal.h>
#endif

#include "fehler.h"
#include "blop.h"
#include "global.h"



/** Erzeugt einen Fehler, bei dem der Text noch nicht gesetzt ist.
    Ist nur für iFehler() gedacht. */
Fehler::Fehler(): mIntern(false), mMitZeile(false) {
  /* Wenn der Fehler während einer Gleichzeit passiert, soll letztere
     abgebrochen werden. */
  Blop::abbruchGleichzeitig();
#if BEI_FEHLER_NACH_GDB
  raise(SIGTRAP);
#endif
}


Fehler::Fehler(Str text) : mIntern(false), mText(text), mMitZeile(false)
  {}


Fehler::Fehler(const char * fmt, ...):
  mIntern(false), mMitZeile(false)
{
  /* Wenn der Fehler während einer Gleichzeit passiert, soll letztere
     abgebrochen werden. */
  Blop::abbruchGleichzeitig();
  
  va_list va;
  va_start(va, fmt);
  setText(_vsprintf(fmt, va));
  va_end(va);
#if BEI_FEHLER_NACH_GDB
  print_to_stderr(mText+"\n");
  raise(SIGTRAP);
#endif
}



Fehler iFehler(const char * fmt, ...) {
  
#if BEI_FEHLER_NACH_GDB
  {
    va_list va;
    va_start(va, fmt);
    print_to_stderr(_vsprintf(fmt, va));
    va_end(va);
  }
  raise(SIGTRAP);
#endif

  Fehler ret;
  
  /* Interner Fehler. */
  ret.mIntern = true; 
  
  /* Fehlertext setzen */
  va_list va;
  va_start(va, fmt);
  ret.setText(_vsprintf(fmt, va));
  va_end(va);
  
  return ret;
}


/** Liefert true, wenn dieser Fehler gerne eine Log-Datei schicken würde,
    d. h. wenn es ein interner Fehler ist oder - bei nicht internen
    Fehlern - wenn _keine_ ld-Datei an Cuyo übergeben wurde. */
bool Fehler::getSendLog() const {
  return mIntern || !gDateiUebergeben;
}



/** True, wenn ggf. "please send log" angehängt werden soll. GGf heißt:
    Wenn getSendLog() true liefert. */
Str Fehler::getText(bool mitLog /*= false*/) const {
  if (getSendLog() && mitLog)
    return mText + "\n" + send_log_string;
  else
    return mText;
}




void Fehler::setText(const Str & t) {
  mText = t;
}




