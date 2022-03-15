/***************************************************************************
                          variable.cpp  -  description
                             -------------------
    begin                : Thu Jul 20 2000
    copyright            : (C) 2000 by Immi
    email                : cuyo@pcpool.mathematik.uni-freiburg.de

Modified 2002,2005,2006,2008,2010,2011 by the cuyo developers

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "variable.h"
#include "blop.h"
#include "cuyointl.h"
#include "ort.h"


/** Erzeugt eine Müll-Variable. Wird verwendet, wenn es einen Fehler
    gab (üblicherweise Variable nicht definiert). Die Müll-Variable
    versucht, Folgefehler zu vermeiden, so dass wenigstens noch fertig
    geparst werden kann. */
Variable::Variable():
  mDeklaration(0)
{
}


Variable::Variable(//Str datna, int znr, 
                   VarDefinition * d,
                   Ort * ort):
  //mDateiName(datna), mZeilenNr(znr),
  mDeklaration(d), mOrt(ort) {
  
}



Variable::~Variable() {
  if (mOrt) delete mOrt;
}



Str Variable::toString() const {
  Str ret1 = getName();
  if (mOrt) ret1 = ret1 + mOrt->toString();
  return ret1;

/*  Str ret2;
  return _sprintf("%s (%s:%d)", ret.data(), mDateiName.data(), mZeilenNr);
*/
}




bool Variable::Ort_hier() const {
  if (mOrt)
    return mOrt->hier();
  else
    return true;
}


int Variable::getNummer() const {
  if (!mDeklaration)
    return 0;
  return mDeklaration->mNummer;
}


ort_absolut Variable::getOrt(ort_absolut vonhieraus, Blop & fuer_code) const {
  if (mOrt)
    return mOrt->berechne(vonhieraus, fuer_code);
  else
    return vonhieraus;
}


bool Variable::istKonstante() const {
  if (!mDeklaration) {
    /* Müllvariable sollten sich lieber für Variablen als für Konstanten
       halten. Das führt zu weniger Folgefehlern. */
    return false;
  }
  return mDeklaration->mArt == vd_konstante;
}


int Variable::getDefaultWert() const {
  if (!mDeklaration)
    return 0;
  return mDeklaration->mDefault;
}

Str Variable::getName() const {
  if (!mDeklaration)
    return _("<unknown>");
  return mDeklaration->mName;
}

