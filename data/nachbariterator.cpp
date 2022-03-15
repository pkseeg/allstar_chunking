/***************************************************************************
                          nachbariterator.cpp  -  description
                             -------------------
    begin                : Thu Jul 26 2001
    copyright            : (C) 2001 by Immi
    email                : cuyo@karimmi.de

Modified 2001,2002,2005,2006,2008,2010,2011 by the cuyo developers

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cuyointl.h"
#include "sorte.h"
#include "nachbariterator.h"
#include "fehler.h"

NachbarIterator::NachbarIterator(const Sorte * s, int x, int y):
    mNachbarschaft(s->getNachbarschaft()), mX0(x), mY0(y), mI(0), mEnde(false)
{
  setXY();
}
NachbarIterator::~NachbarIterator(){
}


/** Nächster Nachbar */
NachbarIterator & NachbarIterator::operator++() {
  if (!mEnde) {
    mI++;
    setXY();
  }
  return *this;
}

	
void NachbarIterator::setXY() {
  char bx, by, ri;
  switch (mNachbarschaft) {
  /* Die Nachbarschafts-Konstanten stehen in sorte.h */
  /* Die ri-Konstanten auch; siehe dort zu welcher Buchstabe
     was bedeutet. */
  case nachbarschaft_normal:
    bx = "1232"[mI];
    by = "2321"[mI];
    ri = "caCA"[mI];
    break;
  case nachbarschaft_schraeg:
    bx = "1133"[mI];
    by = "1313"[mI];
    ri = "dbBD"[mI];
    break;
  case nachbarschaft_6:
    bx =            "221133"[mI];
    by = ((mX0&1) ? "131212" : "132323")[mI];
    ri =            "AadbBD"[mI];
    break;
  case nachbarschaft_6_schraeg:
    bx =            "1133"[mI];
    by = ((mX0&1) ? "1212" : "2323")[mI];
    ri =            "dbBD"[mI];
    break;
  case nachbarschaft_springer:
    bx = "00134431"[mI];
    by = "13443100"[mI];
    ri = "hgfeHGFE"[mI];
    break;
  case nachbarschaft_dame:
    bx = "12321133"[mI];
    by = "23211313"[mI];
    ri = "caCAdbBD"[mI];
    break;
  case nachbarschaft_6_3d:
    bx = ((mX0&1) ? "04221" : "04223")[mI];
    by = ((mX0&1) ? "22132" : "22132")[mI];
    ri = ((mX0&1) ? "cCAaI" : "cCAai")[mI];
    break;
  case nachbarschaft_garnichts:
    bx = 0;
    by = 0;
    ri = 0;
    break;
  default:
    /* TRANSLATORS: "neighbours" is a programming keyword that should not
       be translated. */
    throw Fehler("%s",_("Non-existent neighbours value."));
  }
  if (!bx)
    mEnde = true;
  else {
    mX = mX0 + bx - '2';
    mY = mY0 + by - '2';
    ri -= 'A';
    mDir = mDirOpp = ri & 31;
    if (ri >= 32) mDir += 16;
    else mDirOpp += 16;
    mDir = 1 << mDir;
    mDirOpp = 1 << mDirOpp;
  }
}
