/***************************************************************************
                          spielfeld.cpp  -  description
                             -------------------
    begin                : Sat Oct 2 1999
    copyright            : (C) 1999 by immi
    email                : cuyo@pcpool.mathematik.uni-freiburg.de

Modified 2002,2006,2008,2010,2011 by the cuyo developers

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <ctime>
#include <cstdio>

#include "punktefeld.h"
#include "leveldaten.h"
#include "layout.h"

#define L_ziffern_rechts 2
#define L_ziffern_dx 18
#define L_ziffern_rechts_eng 4
#define L_ziffern_dx_eng 15


Bilddatei * Punktefeld::gZiffernBild[2];


unsigned int Punktefeld::gRandSeed = (time(0) % 8191) | 1;


Punktefeld::Punktefeld():
        mPunkte(-1), mAugenZu(-1), mUpdateNoetig(true) {
}



void Punktefeld::setPunkte(int p) {
  mPunkte = p;
  mUpdateNoetig = true;
}



void Punktefeld::zwinkerSchritt() {
  if (mAugenZu >= 0) mUpdateNoetig = true;
  
  /* Hat mir mal jemand einen Pseudozufalls-Algorithmus? */
  gRandSeed = (gRandSeed * 2369) % 8191;
  
  //fprintf(stderr, "%d\n", gRandSeed);
  if (gRandSeed < 300) {
    mAugenZu = gRandSeed / 60;
    mUpdateNoetig = true;
  } else
    mAugenZu = -1;
}






/* Malt, falls noetig, die Punkte neu */
void Punktefeld::updateGraphik(bool force/* = false*/) {

  if (!mUpdateNoetig && !force) return;
  mUpdateNoetig = false;

  Area::updateRect(0, 0, L_infos_breite, 32);
  Area::fillRect(0, 0, L_infos_breite, 32, ld->mHintergrundFarbe);

  bool eng = mPunkte > 99999;
  int x = L_infos_breite - (eng ? L_ziffern_rechts_eng : L_ziffern_rechts);
  if (mPunkte >= 0) {
    int pt = mPunkte;
    int n = 0;
    do {
      x -= eng ? L_ziffern_dx_eng : L_ziffern_dx;
      gZiffernBild[mAugenZu == n]->malBildchen(x, 4, pt % 10);
      n++;
      pt /= 10;
    } while (pt > 0);

  }
} 



void Punktefeld::init() {
  for (int i=0; i<2; i++)
    gZiffernBild[i] = new Bilddatei();

  gZiffernBild[0]->laden("pktZiffern.xpm");
  gZiffernBild[1]->laden("pktZiffern2.xpm");
  
  AutoColor::gGame.addUser(gZiffernBild[0]);
  AutoColor::gGame.addUser(gZiffernBild[1]);
}

void Punktefeld::destroy() {
  for (int i=0; i<2; i++)
    delete gZiffernBild[i];
}
