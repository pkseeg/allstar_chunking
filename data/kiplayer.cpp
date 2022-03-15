/***************************************************************************
                          kiplayer.cpp  -  description
                             -------------------
    begin                : Wed Jul 25 2001
    copyright            : (C) 2001 by Immi
    email                : cuyo@karimmi.de

Modified 2001-2003,2005,2006,2011 by the cuyo developers

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <cstdio>

#include "nachbariterator.h"
#include "spielfeld.h"
#include "kiplayer.h"
#include "aufnahme.h"
#include "fehler.h"
#include "prefsdaten.h"

KIPlayer::KIPlayer(Spielfeld * sp): mSp(sp) {
  mDaten = mSp->getDatenPtr();
  mFall = mSp->getFall();
}
KIPlayer::~KIPlayer(){
}

/** Teilt der KI mit, dass ein neuer Level anfängt. (Initialisiert
    alles.) */
void KIPlayer::startLevel() {
  /* Tja, also. So richtig viel zum Initialisieren gibt's nich. */
}


/** Einmal pro Spielschritt aufrufen, wenn der Computer
auch spielen soll */
void KIPlayer::spielSchritt(){
  int fall = mSp->getFallModus();

  if (fall == fallmodus_neu) {
    /* Ein neuer Stein kommt. Wo wollen wir ihn denn hin haben? */
		
    /* Die Menge der Züge mit bester Bewertung raussuchen */
    int bestx[grx * 4], bestr[grx * 4];
    int bestanz = 0;
    int bestbew = -0x7fff;
    for (int x = 0; x < grx; x++) {
      for (int r = 0; r < 4; r++) {
	int bew = bewerteZweiBlops(x, r);
	if (bew > bestbew) {
	  bestbew = bew;
	  bestanz = 0;
	}
	if (bew == bestbew) {
	  bestx[bestanz] = x;
	  bestr[bestanz] = r;
	  bestanz++;
	}
      }
    }
		
    int r = Aufnahme::rnd(bestanz);
    mNochDx = bestx[r] - grx / 2 + 1;
    mNochDr = bestr[r];
    mZuTun = true;
    mNochWart = PrefsDaten::getKIGeschwLin();
  }
	
  if (mZuTun && fall != fallmodus_keins) {
    mNochWart -= 1;
    if (mNochWart <= 0) {
      mNochWart += PrefsDaten::getKIGeschwLin();
      if (mNochDr != 0) {
	mSp->tasteDreh();
	mNochDr--;
      } else if (mNochDx > 0) {
	mSp->tasteRechts();
	mNochDx--;
      } else if (mNochDx < 0) {
	mSp->tasteLinks();
	mNochDx++;
      } else {
	mSp->tasteFall();
	mZuTun = false;
      }
    }
  }
	
}




/** Liefert zurück, wie gut ein Blop der Farbe f in Spalte y wäre,
    um dy nach oben verschoben. */
int KIPlayer::bewerteBlop(int x, int dy, int f) {

  if (x < 0 || x >= grx)
    return -0x6666;

  /* Richtige y-Koordinate suchen. (getFeldArt liefert auch oberhalb
     des Spielfelds blopart_keins zurück.) */
  int y = gry - 1;
  while (mDaten->getFeldArt(x, y) != blopart_keins)
    y--;
	
  y -= dy;

  /* Wie steht's mit dem Abstand nach oben? */
  int yr = y - mSp->getHetzrandYAuftauch();
  if (yr <= 0)
    return -0x6666;
  int ret = -ld->mKINHoehe * gry / yr;
	
  /* Womit verbinden wir? */
  for (NachbarIterator i(ld->mSorten[f], x, y); i; ++i)
    if (mDaten->koordOK(i.mX, i.mY)) {
      if (mDaten->getFeld(i.mX, i.mY).getFarbe() == f)
	ret += ld->mKINAnFarbe;
      if (mDaten->getFeldVerhalten(i.mX, i.mY, verhindert_gewinnen))
	ret += ld->mKINAnGras;
      if (mDaten->getFeldVerhalten(i.mX, i.mY, platzt_bei_platzen))
	ret += ld->mKINAnGrau;
    }
	
  /* Zwei über was von der gleichen Farbe? */
  if (mDaten->koordOK(x, y + 2))
    if (mDaten->getFeld(x, y + 2).getFarbe() == f)
      ret += ld->mKINZweiUeber;
  return ret;
}

/** Liefert zurück, wie gut das Fallende bei x in Richtung r
    wäre. (r = Anzahl der Dreh-Tastendrücke) */
int KIPlayer::bewerteZweiBlops(int x, int r) {
  int f0 = mFall[0].getFarbe();
  int f1 = mFall[1].getFarbe();

  /* Für Level, bei denen die Farbe sich beim Drehen ändert...: */
  //for (int i = 0; i < r; i++) {
  //  f0 = ld->mDrehWechsel[f0];
  //  f1 = ld->mDrehWechsel[f1];
  //}
	
  /* Wenn gespiegelt, dann die beiden senkrecht-Richtungen vertauschen */
  if (ld->mSpiegeln && (r & 1)) r ^= 2;
	
  int senkbon = (f0 == f1) * ld->mKINEinfarbigSenkrecht;
  switch (r) {
  case 0:
    return bewerteBlop(x, 0, f0) + bewerteBlop(x + 1, 0, f1);
  case 1:
    return bewerteBlop(x, 1, f0) + bewerteBlop(x, 0, f1) + senkbon;
  case 2:
    return bewerteBlop(x + 1, 0, f0) + bewerteBlop(x, 0, f1);
  case 3:
    return bewerteBlop(x, 0, f0) + bewerteBlop(x, 1, f1) + senkbon;
  }
  CASSERT(false);
  return 0; /* Gegen Warnungen... */
}
