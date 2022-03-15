/***************************************************************************
                          bildstapel.cpp  -  description
                             -------------------
    begin                : Thu Jul 20 2000
    copyright            : (C) 2000 by Immi
    email                : cuyo@pcpool.mathematik.uni-freiburg.de

Modified 2002,2003,2005,2006,2008,2010,2011 by the cuyo developers

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
#include <cstdlib>

#include "stringzeug.h"
#include "cuyointl.h"
#include "font.h"

#include "bildstapel.h"
#include "leveldaten.h"
#include "layout.h"
#include "fehler.h"



/** Erzeugt einen uninitialisierten Bildstapel */
BildStapel::BildStapel(bool): mMaxAnz(-1),
   mStapel(0) {
}


BildStapel::BildStapel():
  mMaxAnz(ld->mStapelHoehe), mAnz(-1), 
  mDebugOut1(spezvar_out_nichts), mDebugOut2(spezvar_out_nichts),
  mAmPlatzen(0)
{
  deepCreate();
}


BildStapel::~BildStapel() {
  deepLoesch();
}


BildStapel & BildStapel::operator=(const BildStapel & b) {
  CASSERT(b.mMaxAnz == ld->mStapelHoehe);
  
  /* Stapelhöhe ändert sich? Dann Speicher neu alloziieren. */
  if (b.mMaxAnz != mMaxAnz) {
    deepLoesch();
    mMaxAnz = b.mMaxAnz;
    deepCreate();
  }
  kopiere(b);
  return *this;
}


void BildStapel::kopiere(const BildStapel & b) {
  //Das wird eh schon oben getan: mMaxAnz = b.mMaxAnz;
  mAnz = b.mAnz;


  for (int i = 0; i < mAnz; i++)
    mStapel[i] = b.mStapel[i];

  mAmPlatzen = b.mAmPlatzen;
  mDebugOut1 = b.mDebugOut1;
  mDebugOut2 = b.mDebugOut2;
}



void BildStapel::deepCreate() {
  mStapel = new BildEbene[mMaxAnz]; CASSERT(mStapel);
}



void BildStapel::deepLoesch() {
  if (mStapel)
    delete[] mStapel;
  mStapel = 0;
}



bool BildStapel::operator==(const BildStapel & b) const {
  CASSERT(mStapel);
  if (mAnz != b.mAnz)
    return false;
  for (int i = 0; i < mAnz; i++)
    if (!(mStapel[i] == b.mStapel[i])) return false;
  if (mAmPlatzen != b.mAmPlatzen) return false;
  if (mDebugOut1 != b.mDebugOut1) return false;
  if (mDebugOut2 != b.mDebugOut2) return false;
  return true;
}


/** Entfernt alle Bilder. Aufrufen, bevor
    speichereBild() für jedes Bild aufgerufen wird. */
void BildStapel::initStapel(int platz) {
  CASSERT(mStapel);
  mAmPlatzen = platz;
  mAnz = 0;
}


void BildStapel::speichereBild(Sorte * so, int dat, int pos, int viertel,
                 int ebene /*= 0*/) {
  CASSERT(mStapel);
  
  /* Sollte nicht passieren, da mMaxAnz beim Level-Start eigentlich
     richtig ausgerechnet wird */
  if (mAnz >= mMaxAnz)
    throw iFehler("%s",_("Internal error: Too many pictures drawn for one single blob"));

  /* Gleich mal testen, ob die Werte für dat, pos und viertel ok sind. */
  /* Throwt, wenn dat out of range ist. */
  int maxpos = so->getBilddatei(dat)->anzBildchen();
  if (pos < 0 || pos >= maxpos)
    throw Fehler(_("Position pos=%d out of range (allowed for file=%d: 0 - %d)"),
                 pos, dat, maxpos - 1);

  if (viertel < viertel_min || viertel > viertel_max)
    throw Fehler(_("Quarter qu=%d out of range (allowed: %d - %d)"),
                 viertel, viertel_min, viertel_max);

  
  mStapel[mAnz].mSorte = so;
  mStapel[mAnz].mDat = dat;
  mStapel[mAnz].mPos = pos;
  mStapel[mAnz].mViertel = viertel;
  mStapel[mAnz].ebene = ebene;
  mAnz++;
}


void BildStapel::setDebugOut(int d1, int d2) {
  CASSERT(mStapel);
  mDebugOut1 = d1;
  mDebugOut2 = d2;
}




/** malt den Bildstapel. xx und yy sind in Pixeln angegeben;
    Stimmt die folgende Behauptung??
    der Hintergrund wird vorher gelöscht. */
void BildStapel::malen(int xx, int yy) const {
  CASSERT(mStapel);
  /* ggf. Position senkrecht spiegeln */
  if (ld->mSpiegeln) yy = gry * gric - yy - gric;


  /* Unter gewissen Umständen (wenn nicht gemalt werden darf) wird mAnz
     auf was negatives gesetzt. Nur um diesen Sicherheitscheck machen
     zu können. */
  CASSERT(mAnz >= 0);

  /* Bild-Stapel malen. */

  /* Alles in der Reihenfolge der Ebenen */
  for (int ebene = bildstapel_min_ebene; ebene <= bildstapel_max_ebene;
      ebene++) {

    for (int i = 0; i < mAnz; i++) {
      const BildEbene & e = mStapel[i];
      
      if (e.ebene == ebene) {

	try { // Um die Fehlermeldung zu verbessern...
	  e.mSorte->getBilddatei(e.mDat)->
             malBildchen(xx, yy, e.mPos, e.mViertel);
	} catch (Fehler fe) {
	  // TRANSLATORS: This is to add further information to error messages
	  throw Fehler(_("In kind %s, file %d:\n%s"),
                      e.mSorte->getName().data(), e.mDat, fe.getText().data());
	}
      
      }
    }
  }

  /* Evtl. Explosion malen */
  if (mAmPlatzen) {
    ld->mExplosionBild.malBildchen(xx, yy, mAmPlatzen - 1);
  }

  /* Debug-Output vom Animations-Code? */
  if (mDebugOut1 != spezvar_out_nichts)
    malDebug(xx, yy, mDebugOut1);
  if (mDebugOut2 != spezvar_out_nichts)
    malDebug(xx, yy + gric / 2, mDebugOut2);
} // malen




/** Liefert true, wenn der Stapel leer ist. Wird benutzt, um
    eine Fehlermeldung auszuspucken, wenn während eines Events
     gemalt wird. */
bool BildStapel::istLeer() const {
  CASSERT(mStapel);
  return mAnz == 0;
}


void BildStapel::print() const {
  print_to_stderr(_sprintf("d1=%d,d2=%d,p=%d",
			   mDebugOut1, mDebugOut2, mAmPlatzen));
  for (int i = 0; i < mAnz; i++)
    print_to_stderr(_sprintf(" %s:%d:%d:%d:%d",
			     mStapel[i].mSorte->getName().data(),
			     mStapel[i].mDat, mStapel[i].mPos,
			     mStapel[i].mViertel,
			     mStapel[i].ebene));
  print_to_stderr("\n");
}




/** Gibt die Zahl n aus. Wird für Debug-Output von malen()
    benutzt. */
void BildStapel::malDebug(int xx, int yy, int n) const {
  CASSERT(mStapel);
  Str str = _sprintf("%d", n);
  Font::gDbg->drawText(str, xx, yy, (TextAlign) (AlignTop | AlignLeft));
}


