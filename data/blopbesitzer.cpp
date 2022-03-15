/***************************************************************************
                          blopbesitzer.cpp  -  description
                             -------------------
    begin                : Sat Jul 14 2001
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
#include "blopbesitzer.h"
#include "fehler.h"
#include "spielfeld.h"

BlopBesitzer::BlopBesitzer(Spielfeld * spf) : mSpf(spf) {}

/** For Blopgitters, this returns the Blop at the specified coordinates.
    May not be called otherwise, or with incorrect coordinates.
    This is used for accessing variables at foreign coordinates. */
const Blop & BlopBesitzer::getFeld(int /*x*/, int /*y*/) const {
  throw iFehler("%s",_("Internal error in const BlopBesitzer::getFeld()"));
}

/** Dito */
Blop & BlopBesitzer::getFeld(int /*x*/, int /*y*/) {
  throw iFehler("%s",_("Internal error in BlopBesitzer::getFeld()"));
}

const Blop * BlopBesitzer::getFall(int a) const {
  return mSpf->getFall()+a;
}

Blop * BlopBesitzer::getFall(int a) {
  return mSpf->getFall()+a;
}

int BlopBesitzer::getSpezConst(int /*vnr*/, const Blop *) const {
  return spezconst_defaultwert;
}
