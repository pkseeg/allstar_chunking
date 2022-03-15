/***************************************************************************
                          pfaditerator.cpp  -  description
                             -------------------
    begin                : Thu Jul 26 2001
    copyright            : (C) 2001 by Immi
    email                : cuyo@karimmi.de

Modified 2001,2002,2004,2006-2008,2010,2011 by the cuyo developers

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

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include "fehler.h"
#include "pfaditerator.h"
#include "cuyointl.h"
#include "global.h"

enum {
  pfad_default,
  pfad_uebergeben,
  pfad_installation,
  anz_pfade
};


int PfadIterator::gDefaultPfad = -1;


PfadIterator::PfadIterator(Str dat, bool auch_gz /*= false*/, 
    bool setzDefault /*= false*/):
  mPos(-1), mDatei(dat), mSetzDefault(setzDefault), mAuch_gz(auch_gz),
  mIstAbsolut(dat[0] == '/')
{
  /* Damit wir hier nicht auch nochmal die ganzen Überspringungen testen
     müssen, die in operator++() stehen, setzen wir mPos als erstes auf
     -1 und führen dann gleich ein ++ aus. */
  ++(*this);
}

PfadIterator::~PfadIterator() {
  /** Soll der default-Pfad gesetzt werden? */
  if (mSetzDefault) {

    if (mIstAbsolut) {
      /* Gepfuscht: Absolute Pfade können nicht als default-Pfad gesetzt
         werden. Einzige Ausnahme: Es handelt sich um den übergebenen Pfad. */
      if (nimmPfad(gLevelDatei) == nimmPfad(mDatei)) {
	/* Uff, Glück gehabt. */
        gDefaultPfad = pfad_uebergeben;
      } else
        throw Fehler("%s",_("Internal error: got confused with all those paths I have to deal with."));
    } else {
      /* Alles normal. Kein absoluter Pfad. */
      if (mPos != pfad_default && mPos < anz_pfade)
	gDefaultPfad = mPos;
    }
  }
}


/** Nächster Pfad */
PfadIterator & PfadIterator::operator++() {
  plusplusIntern();

  if (mPos >= anz_pfade) {

    #if HAVE_LIBZ
      if (mAuch_gz)
        mDatei = mDatei + "[.gz]";
    #endif

    Str versuche = "";
    mPos = -1; plusplusIntern();
    do {
      versuche = versuche + "\n  " + pfad();
      plusplusIntern();
    } while (mPos<anz_pfade);

    throw Fehler(_("Could not find \"%s\".\nTried the following places:%s"),
		 mDatei.data(), versuche.data());
  }

  return *this;
}

	
/** Aktueller Pfad */
Str PfadIterator::pfad() const {

  /* Wenn absoluter Pfad angegeben, dann den auch zurückliefern. */
  if (mIstAbsolut)
    return mDatei;

  /* Als erstes im default-Pfad nachschauen */
  int n = mPos;
  if (n == pfad_default) n = gDefaultPfad;

  Str r;
  switch (n) {
    case pfad_uebergeben:
      CASSERT(gDateiUebergeben);
      /* gLevelDatei enthält Pfad und Name einer ld-Datei. Wir nehmen
	 uns davon den Pfad: */
      r = _sprintf("%s%s", nimmPfad(gLevelDatei).data(), mDatei.data());
      break;

    case pfad_installation:
      /* PKGDATADIR wird mit -D übergeben. (Siehe src/Makefile.am)
	 Unter WIN32 wird PKGDATADIR als "cuyo-x.xx\\data" definiert,
	 falls es nicht übergeben wurde.
	 Unter MacOsX soll ein Verzeichnis relativ zum Programmpfad
	 verwendet werden... */
#ifdef MACOSX
      r = _sprintf("%s../Resources/%s", gCuyoPfad.data(), mDatei.data());
#else
      r = _sprintf("%s/%s", PKGDATADIR, mDatei.data());
#endif
      break;

    default:
      CASSERT(false);
  }
  // Debug-Ausgabe:
  // print_to_stderr(_sprintf("%s\n", r.data());fflush(stdout));
  return r;
}


/** Der wesentliche Teil von ++(), aber ohne Fehlertest */
void PfadIterator::plusplusIntern() {
  mPos++;
  
  if (mIstAbsolut) {
    /* Bei absolutem Pfad ist nicht viel zu tun; ++ sollte nur
       einmal vom Constructor aufgerufen werden; wenn es nochmal
       aufgerufen wird, bedeutet das, dass die Datei nicht existiert.
       Gut, dass mPos die Aufrufe zählt... */

    if (mPos > 0) {
      /* Dann wird bald die Fehlermeldung ausgegeben. */
      mPos = anz_pfade;
    }
  
  } else {

    /* Wenn es noch keinen Default-Pfad gibt, dann überspringen */
    if (mPos == pfad_default && gDefaultPfad == -1)
      mPos++;

    /* Wenn keine ld-Datei an cuyo übergeben wurde, dann haben wir
       da auch keinen Pfad, in dem wir suchen könen. Also in dem
       Fall überspringen. */
    if (mPos == pfad_uebergeben && !gDateiUebergeben)
      mPos++;

    /* Nicht noch ein zweites Mal im Default-Pfad suchen */
    if (mPos == gDefaultPfad) mPos++;
  } // Ende von: kein absoluter Pfad
}


/** Vergisst, welcher Pfad als default gesetzt wurde */
void PfadIterator::loescheDefault() {
  gDefaultPfad = -1;
}



