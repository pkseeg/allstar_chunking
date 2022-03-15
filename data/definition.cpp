/***************************************************************************
                          definition.cpp  -  description
                             -------------------
    begin                : Mit Jul 12 22:54:51 MEST 2000
    copyright            : (C) 2000 by Immi
    email                : cuyo@pcpool.mathematik.uni-freiburg.de

Modified 2002,2006,2010,2011 by the cuyo developers

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <cstdlib>
#include <cstdio>

#include "definition.h"




/* Zur Klasse Definition gibt's nix... */



/*************************************************************************/
/* VarDefinition */

VarDefinition::VarDefinition(Str na, int d, int a, int da, int n):
  mName(na), mDefault(d), mArt(a), mDefaultArt(da), mNummer(n)
{
  //print_to_stderr(_sprintf("vc %s (nr. %d) = %d\n", na.data(), n, d));
}


/*
Str VarDefinition::toString() const {
  return _sprintf("var (=%d)", mDefault);
}
*/

