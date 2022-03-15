/***************************************************************************
                          bilddatei.cpp  -  description
                             -------------------
    begin                : Fri Apr 20 2001
    copyright            : (C) 2001 by Immi
    email                : cuyo@karimmi.de

Modified 2001,2002,2006,2008,2010,2011 by the cuyo developers

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
#include <vector>

#include "sdltools.h"

#include "cuyointl.h"
#include "bilddatei.h"
#include "fehler.h"
#include "pfaditerator.h"
#include "stringzeug.h"
#include "xpmladen.h"
#include "inkompatibel.h"
#include "global.h"
#include "layout.h"


/**********************************************************************/


AutoColor AutoColor::gGame;


void AutoColor::setColor(const Color & c) {
  mColor = c;
  for (int i = 0; i < (int) mUser.size(); i++)
    mUser[i]->setFaerbung(mColor);
}


void AutoColor::addUser(Bilddatei * b) {
  mUser.push_back(b);
  b->setFaerbung(mColor);
}
  
void AutoColor::operator=(const Color & c) {
  setColor(c);
}



/**********************************************************************/

class BildOriginal {
 public:
  SDL_Surface * mBild;
 private:
  int mAnzUser;
  
 public:
  BildOriginal(SDL_Surface * b): mBild(b), mAnzUser(1) {}
  ~BildOriginal() { SDL_FreeSurface(mBild); }
  
  void anmelden() {
    mAnzUser++;
  }
  
  void abmelden() {
    mAnzUser--;
    if (mAnzUser == 0)
      delete this;
  }
  
};  // BildOriginal



/**********************************************************************/


Bilddatei::Bilddatei(): mBildOriginal(0), mBild(0), mNativBild(NULL) {
}
Bilddatei::~Bilddatei() {
  datenLoeschen();
}

Bilddatei::Bilddatei(Bilddatei * quelle, const Color & faerbung) :
  mBildOriginal(quelle->mBildOriginal), mBild(0), mNativBild(NULL), mMaskeOriginal(quelle->mMaskeOriginal), mMaske(quelle->mMaske), mBreite(quelle->mBreite), mHoehe(quelle->mHoehe),
    mName(quelle->mName), mGefaerbt(true), mFaerbung(faerbung), mScale(quelle->mScale) {
  CASSERT(mBildOriginal); // Kein Bild klonen, das selbst grad nicht geladen ist
  mBildOriginal->anmelden();
  gAlleBilddateien->push_back(this);
  bildNachbearbeiten();
}



void Bilddatei::datenLoeschen() {
  loescheAusZentralregister();
  if (mBild) {
    SDL_FreeSurface(mBild);
    mBild = 0;
  }
  if (mNativBild) {
    SDL_FreeSurface(mNativBild);
    mNativBild = NULL;
  }
  if (mBildOriginal) {
    mBildOriginal->abmelden();
    mBildOriginal = 0;
  }
}


/** Lädt das Bild mit dem angegebenen Namen. Sucht in verschiedenen
    Pfaden danach.Throwt ggf. */
void Bilddatei::laden(Str name) {

  datenLoeschen();
  
  mName = name;
  mGefaerbt = false;
  
  Str s = _sprintf("pics/%s", name.data());

  /* Bild in verschiedenen Pfaden suchen... (Der Pfaditerator throwt ggf.)
     ladXPM kann auch throwen: Wenn eine .xpm.gz-Datei existiert, die
     von meiner Routine nicht geladen werden kann. */
  SDL_Surface * bild;
  for (PfadIterator pi(s,true);
       !(bild = ladXPM(pi.pfad(),mMaskeOriginal)); ++pi) {}
  
  /* Ich will mit 32-Bit arbeiten */
  SDLTools::convertSurface32(bild);
  mBreite = bild->w;
  mHoehe = bild->h;
  
  mBildOriginal = new BildOriginal(bild);

  gAlleBilddateien->push_back(this);
  bildNachbearbeiten();
}


void Bilddatei::klonen(Bilddatei & quelle) {
  datenLoeschen();

  mBildOriginal = quelle.mBildOriginal;
  CASSERT(mBildOriginal); // Kein Bild klonen, das selbst grad nicht geladen ist
  mBildOriginal->anmelden();
  mMaskeOriginal = quelle.mMaskeOriginal;
  mMaske = quelle.mMaske;

  mBreite = quelle.mBreite;
  mHoehe = quelle.mHoehe;
  mName = quelle.mName;
}



void Bilddatei::setFaerbung(const Color & faerbung) {
  mGefaerbt = true;
  mFaerbung = faerbung;
  bildNachbearbeiten();
}


/** malt das k-te Viertel vom n-te Bildchen an xx,yy. Oder evtl. das
    ganze Bildchen */
void Bilddatei::malBildchen(int xx, int yy,
			    int n, int k /*= viertel_alle*/) const {

  /* Richtigen Bildausschnitt suchen */
  if (n >= anzBildchen())
    throw Fehler(_("Image '%s' too small for Icon %d"), mName.data(), n);
  int bpr = mBreite / gric; /* Bildchen pro Reihe... */
  SDL_Rect srcr = SDLTools::rect(gric * (n % bpr), gric * (n / bpr), gric, gric);
  
  if (k != viertel_alle) {
    /* Richtiges Viertel in Datei wählen */
    if (k & viertel_qr) srcr.x += gric/2;
    if (k & viertel_qu) srcr.y += gric/2;
    /* Richtiges Ziel-Viertel wählen */
    if (k & viertel_zr) xx += gric/2;
    if (k & viertel_zu) yy += gric/2;
    srcr.w = srcr.h = gric/2;
  }

  Area::blitSurface(mBild, srcr, xx, yy);
  Area::maskBackground(&mMaske, srcr, xx, yy);
}

/** liefert zurück, wie viele Bildchen in dieser Datei sind. */
int Bilddatei::anzBildchen() const{
  return (mBreite / gric) * (mHoehe / gric);
}

/** liefert die Gesamthoehe in Pixeln zurück */
int Bilddatei::getBreite() const {
  return mBreite;
}

/** liefert die Gesamthoehe in Pixeln zurück */
int Bilddatei::getHoehe() const {
  return mHoehe;
}


/** Nur zum anschauen, nicht zum veraendern! Liefert das Bild in unskaliert
    und 32 Bit. */
SDL_Surface * Bilddatei::getSurface() const {
  return mBildOriginal->mBild;
}

void Bilddatei::setAsBackground(int y) {
  sorgeFuerNativBild();
  Area::setBackground(SDLTools::rect(0,y,mBreite,mHoehe),mNativBild);
}




/** malt das gesamte Bild */
void Bilddatei::malBild(int xx, int yy) const {
  Area::blitSurface(mBild, xx, yy);
  Area::maskBackground(&mMaske,SDLTools::rect(0,0,mBreite,mHoehe),xx,yy);
}

/** malt einen beliebigen Bildausschnitt */
void Bilddatei::malBildAusschnitt(int xx, int yy, const SDL_Rect & src) const {
  if (gDebug)
    if (src.x % 4 != 0 || src.y % 4 != 0 || src.w % 4 != 0 || src.h % 4 != 0) {
      print_to_stderr(_sprintf(_("\
* Warning: Some part of '%s' is drawn with coordinates which are\n\
* not divisible by 4. This may cause graphic glitches in downscaled mode.\n"),
			       mName.data()));
    }
  Area::blitSurface(mBild, src, xx, yy);
}


/* Malt das angegebene Rechteck (bzw. Teile davon) so oft, dass
   ein horizontaler Streifen der Länge l entsteht. Geht davon aus,
   das in dem Bildchen das src-Rechteck horizontal einheitlich ist.
   Je größer src, desto schneller geht das malen. */
void Bilddatei::malStreifenH(int xx, int yy, int l, const SDL_Rect & src) const {
  int xx2 = xx + l;
  while (xx < xx2) {
    int xxneu = xx + src.w;
    if (xxneu > xx2) xxneu = xx2;
    Area::blitSurface(mBild, SDLTools::rect(src.x, src.y, xxneu - xx, src.h), xx, yy);
    xx = xxneu;
  }
}
/* Das selbe in vertikal */
void Bilddatei::malStreifenV(int xx, int yy, int l, const SDL_Rect & src) const {
  int yy2 = yy + l;
  while (yy < yy2) {
    int yyneu = yy + src.h;
    if (yyneu > yy2) yyneu = yy2;
    Area::blitSurface(mBild, SDLTools::rect(src.x, src.y, src.w, yyneu - yy), xx, yy);
    yy = yyneu;
  }
}




void Bilddatei::bildNachbearbeiten() {
  SDL_Surface* src = SDLTools::scaleSurface(mBildOriginal->mBild);
  mScale = SDLTools::getScale();
  mMaske.scale(mMaskeOriginal,mScale);
  
  if (mGefaerbt) {
    SDL_LockSurface(src);

    bool gleich = (src!=mBildOriginal->mBild);
      /* Wenn src *nicht* das Original ist, dann
	 kann direkt src gefärbt werden, d. h.
	 dst der Färbung = src der Färbung. */

    SDL_Surface* dst;
    if (gleich)
      dst=src;
    else {
      dst = SDLTools::createSurface32(src->w,src->h);
      SDL_LockSurface(dst);
    }

    for (int x = 0; x < src->w; x++)
      for (int y = 0; y < src->h; y++) {
	Uint32 & srcpix = SDLTools::getPixel32(src, x, y);
	Uint32 & dstpix = (gleich
          ? srcpix
          : SDLTools::getPixel32(dst, x, y));
	Uint8 rgba[4], r, g, b;
	SDL_GetRGBA(srcpix, src->format, &r, &g, &b, &rgba[3]);

	for (int i = 0; i < 3; i++)
          rgba[i] = (mFaerbung[i] * r + (255 - mFaerbung[i]) * g + 127) / 255;
	dstpix = SDL_MapRGBA(dst->format, rgba[0], rgba[1], rgba[2], rgba[3]);

      }

    SDL_UnlockSurface(src);
    if (!gleich) SDL_UnlockSurface(dst);

    src = dst;
  }



  if (mBild)
    SDL_FreeSurface(mBild);
  mBild = SDLTools::maskedDisplayFormat(src);

  if (mNativBild)
    SDL_FreeSurface(mNativBild);
  mNativBild=NULL;

  if (src!=mBildOriginal->mBild)
    SDL_FreeSurface(src);
}


void Bilddatei::sorgeFuerNativBild() {
  if (!mNativBild) {
    CASSERT(mBild);
    mNativBild=SDL_ConvertSurface(mBild,SDL_GetVideoSurface()->format,0);
    SDLASSERT(mNativBild);
  }
}





/**********************************************************************/



/** Ggf. alle existierenden Bildchen reskalieren. */
void Bilddatei::resizeEvent() {
  for (int i = 0; i < (int) gAlleBilddateien->size(); i++) {
    //print_to_stderr(_sprintf("%s: %d\n", gAlleBilddateien[i]->mName.data(), gAlleBilddateien[i]->mScale));
    if ((*gAlleBilddateien)[i]->mScale != SDLTools::getScale())
      (*gAlleBilddateien)[i]->bildNachbearbeiten();
  }
}


/* Liste aller geladenen Bilddateien */
std::vector<Bilddatei *> * Bilddatei::gAlleBilddateien;

void Bilddatei::loescheAusZentralregister() {
  for (int i = 0; i < (int) gAlleBilddateien->size(); i++)
    if ((*gAlleBilddateien)[i] == this) {
      (*gAlleBilddateien)[i] = (*gAlleBilddateien)[gAlleBilddateien->size() - 1];
      gAlleBilddateien->resize(gAlleBilddateien->size() - 1);
      return;
    }
  CASSERT("Internal error in Bilddatei registration");
}


void Bilddatei::init() {
  gAlleBilddateien = new std::vector<Bilddatei *>();
}

void Bilddatei::destroy() {
  delete gAlleBilddateien;
}
