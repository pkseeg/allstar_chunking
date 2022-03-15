/***************************************************************************
                          sorte.cpp  -  description
                             -------------------
    begin                : Fri Apr 20 2001
    copyright            : (C) 2001 by Immi
    email                : cuyo@karimmi.de

Modified 2001-2003,2005,2006,2008,2010,2011 by the cuyo developers

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

#include "cuyointl.h"
#include "leveldaten.h"
#include "datendatei.h"

#include "bilddatei.h"

#include "fehler.h"
#include "sorte.h"
#include "code.h"
#include "knoten.h"
#include "global.h"

#include "blop.h"




const char cEventNamen[event_anz][33] = {
  "", /* Der Mal-Code heißt einfach "wuff" und nicht "wuff.draw" oder so. */
  "init", "turn", "land", "changeside", "connect",
  "row_up", "row_down",
  "keyleft", "keyright", "keyturn", "keyfall"
};

const char* cVerteilungsNamen[anzahl_wv] = {
  "colourprob","greyprob","goalprob"
};




void Sorte::setzeDefaults(DefKnoten * quelle) {
  int DatenLaenge = ld->mLevelKnoten->getDatenLaenge();
  mDatenDefault = new int[DatenLaenge];
  mDatenDefaultArt = new int[DatenLaenge];
  for (int i=0; i<DatenLaenge; i++) {
    mDatenDefault[i] = quelle->getDefaultWert(i);
    mDatenDefaultArt[i] = quelle->getDefaultArt(i);
  }

  /* Wenn behaviour nicht gesetzt (das heißt überschrieben) wurde,
     tragen wir jetzt den Default nach. */
  if (mDatenDefaultArt[spezvar_verhalten]==da_nie) {
    mDatenDefaultArt[spezvar_verhalten] = da_kind;
    int verhalten = 0;
    switch (mBlopart) {
      case blopart_gras: verhalten =
          platzt_bei_kettenreaktion
          + (ld->mGrasBeiKettenreaktion ? 0 : platzt_bei_platzen)
          + verhindert_gewinnen;
        break;
      case blopart_grau: verhalten =
          platzt_bei_kettenreaktion
          + platzt_bei_platzen;
        break;
      case blopart_farbe: verhalten =
          platzt_bei_gewicht
          + berechne_kettengroesse;
        break;
      case blopart_keins: verhalten =
          schwebt;
        break;
    }
    mDatenDefault[spezvar_verhalten] = verhalten;
  }
}



/** Wird von Sorte() aufgerufen; ausgelagert, weil Sorte() langsam
    lang und unübersichtlich wird */
void Sorte::ladeCualEvents(const Version & version) {
  /***** Mal-cual-Code laden. (Sonderbehandlung; nicht so wie die
         anderen Events.) *****/

  mEventCode[event_draw] = ld->mLevelConf[ldteil_level]->
    getCode(mName, version, true);

    /* Wenn's den Code nicht gibt, dann default-Code verwenden; aber
       nur, wenn es sich nicht um den Global-Code handelt. Und wenn's
       keine Bildchen gibt, wird per default auch nicht gemalt. */
  if (!mEventCode[event_draw] && mBilddateien.size() > 0
      && mBlopart != blopart_global && mBlopart != blopart_semiglobal) {

      /* Welchen Default nehmen wir? */
      Str dname;
      if (mBilddateien.size() > 1)
        dname = "default3";
      else if (mBilddateien[0]->anzBildchen() > 1) {
        /* Farbblops und nix-Blops verbinden sich per default;
	   andere nicht. */
        if (mBlopart == blopart_gras)
          dname = "default2g";
        else
          dname = "default2";
      } else
        dname = "default1";

      if (gDebug)
        print_to_stderr(_sprintf(_("Using %s for %s\n"), dname.data(), 
				 mName.data()));
      mEventCode[event_draw] =
	   ld->mLevelConf[ldteil_level]->getCode(dname,version, false);
  }

  /* Den Leveldaten mitteilen, wie viele Bildchen unsere Blops
     höchstens gleichzeitig malen. */
  if (mEventCode[event_draw]) {
    int sh = mEventCode[event_draw]->getStapelHoehe(ld->mNachbarStapelHoehe);
    if (sh > ld->mStapelHoehe)
      ld->mStapelHoehe = sh;
  }


  /***** Cual-Code für andere Events laden (optional) *****/

  /* i erst bei 1 los; 0 ist event_draw, der eine Sonderbehandlug ist. */
  for (int i = 1; i < event_anz; i++) {
    mEventCode[i] = ld->mLevelConf[ldteil_level]->
      getCode(mName + "." + cEventNamen[i], version, true);

    /* Wenn im Event-Code Mal-Befehle vorkommen, geben wir
       im Debug-Modus eine Warnung aus. */
    if (mEventCode[i] && gDebug) {
      int nsh = 0;
      if (mEventCode[i]->getStapelHoehe(nsh) > 0 || nsh > 0)
	/* TRANSLATORS: "%s.%s" is a programming construction and should
	   not be translated */
	print_to_stderr(_sprintf(_("Warning: %s.%s contains drawing commands.\n"
				   "Drawing is not allowed during events.\n"),
				 mName.data(), cEventNamen[i]));
    }
  }
}





/** Lädt die Sorte mit dem angegebenen Namen. Schaut auch in der
    entsprechenden Gruppe von mLevelConf[ldteil_level] nach, setzt die Gruppe
    aber danach zurück. blopart muss nur angegeben werden, damit ein
    paar Art-abhängige Defaults richtig gesetzt werden können.
    Throwt Fehler, wenn erfolglos. */
Sorte::Sorte(const Str & name, const Version & version, int blopart) {

  mBlopart = blopart;

  /* Wenn der Name direkt eine Bild-Datei ist, wird bei mName
     noch die Endung weggeschnitten. picsEndungWeg() steht
     in global.* */
  mName = picsEndungWeg(name);

  /* Für die restlichen Variablen erst mal ein paar Defaults setzen */
  /* Rand-Verbindung nur bei nix-Blop. */
  for (int i = 0; i < 4; i++)
    mVerbindetMitRand[i] = mBlopart == blopart_keins;
  /* Default Nachbarschaft = levelweite Nachbarschaftseinstellung */
  mNachbarschaft = ld->mNachbarschaft;
  /* dito PlatzAnzahl */
  mPlatzAnzahl = ld->mPlatzAnzahlDefault;
  /* Keine Bildchen */
  loeschBilder(); // Alte Bilder löschen
  /* Keine Event-Programme */
  for (int i = 0; i < event_anz; i++)
    mEventCode[i] = 0;
  /* Wahrscheinlichkeiten */
  mVerteilung[wv_farbe] = (mBlopart==blopart_farbe ? 1 : 0);
  mVerteilung[wv_grau] = (mBlopart==blopart_grau ? 1 : 0);
  mVerteilung[wv_gras] = (mBlopart==blopart_gras ? 1 : 0);
  CASSERT(anzahl_wv==3);
  mDistKey = (mBlopart==blopart_gras ? 10 : distkey_undef);
  mVersions = (mBlopart==blopart_gras ? 52 : 1);

  /* Wenn name leer ist, dann gibt's nix zu laden. Das sollte eigentlich
     nur beim Leerbildchen passieren, falls das in der ld-Datei nicht
     angegeben ist. (Wenn der Global-Blop nicht angegeben ist, hat
     es trotzdem den Namen "global"; das einzige, was dann passiert,
     ist, dass weiter unten der Cual-Code nicht geladen wird.) */
  if (name.isEmpty()) {
    /* Für bessere Debug-Ausgaben setzen wir uns aber trotzdem noch
       einen schönen Namen. */
    switch (mBlopart) {
      case blopart_keins: mName = "nothing"; break;
      //case blopart_farbe: mName = "color"; break;
      //case blopart_gras: mName = "grass"; break;
      //case blopart_grau: mName = "grey"; break;
      //case blopart_global: mName = "global"; break;
      /* TRANSLATORS: "Sorte" (both) and "mBlopart" are programming identifiers
	 and should not be translated */
      default: throw iFehler(_("Internal Error in Sorte::Sorte(): mBlopart = %d"), mBlopart);
    }
    setzeDefaults(ld->mLevelKnoten);
    mBasekind = mBlopart;
    return;
  }


  /***** Sortenabschnitt laden. *****/
  
  {
  
    /* In den Unterabschnitt zu diesem Bildchen gehen. false heißt: Wenn's
       den Abschnitt nicht gibt, nicht gleich throwen */
    DatenDateiPush ddp(*ld->mLevelConf[ldteil_level], name, version, false);

    DatenDatei * dd = ld->mLevelConf[ldteil_level];

    setzeDefaults(dd->getSquirrelPos());

    VarDefinition * basekinddef = (VarDefinition*) dd->getSquirrelPos()->
      getDefinition(namespace_variable,picsEndungWeg(name),version,false);
    CASSERT(basekinddef->mArt==vd_konstante);
    mBasekind = basekinddef->mDefault;

    /* Beim global-Blop sollte es Grafik-Zeug nicht geben. Wir
       versuchen einfach gar nicht erst, es zu laden. Insbesondere
       wollen wir auch kein Default-Bild laden. */
    if (mBlopart != blopart_global) {

      /* Gibt es zu diesem Bildchen überhaupt einen Abschnitt in
	 der Config-Datei? */
      if (dd->existiertSquirrelKnoten()) {
	/* OK, es gibt in der Gruppe mit diesem Namen einen pics-Eintrag;
           also gibt's insbesondere diese Gruppe... */

	/* Andere Nachbarschaft als level-weite globale? (optional) */
	mNachbarschaft = dd->
	  getZahlEintragMitDefault("neighbours",version, mNachbarschaft);
	if (mNachbarschaft < 0 || mNachbarschaft > nachbarschaft_letzte)
          throw Fehler("%s",_("neighbours out of range"));

	/* dito PlatzAnzahl, aber irgendwo muß es definiert werden
           (im Gegensatz zu mNachbarschaft gibt es keinen Default) */
	mPlatzAnzahl = dd->
	  getZahlEintragMitDefault("numexplode", version, mPlatzAnzahl);

	/* Bilder laden (optional) */
	int anz_bi;
	ListenKnoten* namen = dd->getListenEintrag("pics",version,true);
	if (namen)
          anz_bi = namen->getLaenge();
	else
          anz_bi = 0;
	mBilddateien.resize(anz_bi);

	/* Falls ein Throw während des Ladens der Bilder kommt, ist das Array
           erst halb gefüllt. Damit es später richtig gelöscht werden kann,
           sollten nicht existente Bilder 0-Pointer sein... */
	for (int i = 0; i < anz_bi; i++)
          mBilddateien[i] = 0;

	for (int bnr = 0; bnr < anz_bi; bnr++) {
          Str bild_name = namen->getDatum(bnr,type_WortDatum)->getWort();

          //Bilddatei * neu_b = new Bilddatei();
          //neu_b->laden(bild_name);
          //mBilddateien[bnr] = neu_b;
          mBilddateien[bnr] = new Bilddatei();
          mBilddateien[bnr]->laden(bild_name);
	}

	for (int i=0; i<anzahl_wv; i++)
	  mVerteilung[i] = dd->
	    getZahlEintragMitDefault(cVerteilungsNamen[i], version,
				     mVerteilung[i]);

	/* Ende: In Config existiert Abschnitt für dieses Bildchen */
      } else {
	/* Kein Abschnitt in der Config für dieses Icon. Also direkt den Namen
           als Bilddateinamen verwenden. (Und zwar den Namen _mit_ Endung.) */
	mBilddateien.resize(1);
	mBilddateien[0] = new Bilddatei();
	mBilddateien[0]->laden(name);
      }

    } // Ende von: kein Global-Blop

    /* Was man für startdist so braucht. */
    Str distkey = dd->getWortEintragMitDefault("distkey", version, "");
    if (distkey!="") {
      mDistKey = ld->liesDistKey(distkey);
      if (mDistKey<0)
	/* TRANSLATORS: "distkey" is a programming keyword and should not be
	   translated */
        throw Fehler(_("illegal distkey %s for %s"),
		     distkey.data(), mName.data());
    }
    mVersions = dd->getZahlEintragMitDefault("versions", version, mVersions);
    if (mVersions<1)
      /* TRANSLATORS: "versions" is a programming keyword and should not be
	 translated */
      throw Fehler(_("illegal versions=%d for %s"), mVersions, mName.data());

    /* Und jetzt nochmal PlatzAnzahl verwalten
       - aber nur für per Default gewichtsplatzende */
    if ((mDatenDefault[spezvar_verhalten] & platzt_bei_gewicht)!=0) {
      /* Prüfen, ob PlatzAnzahl inzwischen einen gültigen Wert hat */
      if (mPlatzAnzahl==PlatzAnzahl_undefiniert)
	/* TRANSLATORS: "numexplode" is a programming keyword and should not be
	   translated */
        throw Fehler(_("numexplode undefined for %s"),mName.data());

      ld->neue_PlatzAnzahl(mPlatzAnzahl);
    }

    /* Wahrscheinlichkeiten überprüfen */
    for (int i=0; i<anzahl_wv; i++) {
      if (mVerteilung[i]<0)
	throw Fehler(_("%s must not be negative"),cVerteilungsNamen[i]);
      if (mVerteilung[i]>0) {
	if (i!=wv_farbe) {
	  switch (mBlopart) {
	  case blopart_gras:
	  case blopart_grau:
	  case blopart_keins:
	  case blopart_farbe:
	    break;
	  case blopart_global:
	  case blopart_semiglobal:
	    /* TRANSLATORS: "global" and "semiglobal" are programming keywords
	       and should not be translated */
	    throw Fehler(_("%s>0 not allowed for global or semiglobal"),
			 cVerteilungsNamen[i]);
	    break;
	  default:
	    CASSERT(0);
	    break;
	  }
	} else {
	  switch (mBlopart) {
	  case blopart_gras:
	  case blopart_grau:
	  case blopart_farbe:
	    break;
	  case blopart_keins:
	  case blopart_global:
	  case blopart_semiglobal:
	    /* TRANSLATORS: "nothing", "global", and "semiglobal" are
	       programming keywords and should not be translated */
	    throw Fehler(_("%s>0 not allowed for nothing, global or semiglobal"),
			 cVerteilungsNamen[i]);
	    break;
	  default:
	    CASSERT(0);
	    break;
	  }
	}
      }
    }

    ladeCualEvents(version);

  } // Ende: DatenDatei-Push
}


Sorte::~Sorte(){
  delete[] mDatenDefault;
  delete[] mDatenDefaultArt;
  loeschBilder();
}







/** löscht die ganzen Bilder aus dem mBilddateien-Array */
void Sorte::loeschBilder() {
  for (int i = 0; i < (int) mBilddateien.size(); i++)
    if (mBilddateien[i])
      delete mBilddateien[i];
		
  /* Damit keine Pointer in der Luft hängen...: */
  mBilddateien.clear();
}



/***** Getter-Methoden, mit denen man auf die ganzen Informationen
       zugreifen kann. *****/


Str Sorte::getName() const {
  return mName;
}

int Sorte::getBasekind() const {return mBasekind;}

bool Sorte::getVerbindetMitRand(int r) const {
  return mVerbindetMitRand[r];
}

int Sorte::getNachbarschaft() const {
  return mNachbarschaft;
}

int Sorte::getPlatzAnzahl() const {
  return mPlatzAnzahl;
}

int Sorte::getVerteilung(int wv) const {return mVerteilung[wv];}
int Sorte::getDefault(int var) const {return mDatenDefault[var];}
int Sorte::getDefaultArt(int var) const {return mDatenDefaultArt[var];}


Code * Sorte::getEventCode(int evt) const {
  return mEventCode[evt];
}

int Sorte::getDistKey() const {return mDistKey;}
int Sorte::getVersions() const {return mVersions;}
  
Bilddatei * Sorte::getBilddatei(int nr) const {
  if (nr < 0 || nr >= (int) mBilddateien.size())
    /* TRANSLATORS: "file" in "file=%d" is a programming keyword and should
       not be translated */
    throw Fehler(_("File number file=%d out of range (allowed: 0 - %ld)"), nr,
               (long)(mBilddateien.size() - 1));
  return mBilddateien[nr];
}


