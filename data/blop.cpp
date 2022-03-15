/***************************************************************************
                          blop.cpp  -  description
                             -------------------
    begin                : Thu Jul 20 2000
    copyright            : (C) 2000 by Immi
    email                : cuyo@pcpool.mathematik.uni-freiburg.de

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
#include "blop.h"
#include "variable.h"
#include "code.h"
#include "knoten.h"
#include "cuyo.h" // Um Punkte senden zu können
#include "spielfeld.h"
#include "aufnahme.h"





/** Erzeugt einen uninitialisierten Blop (der noch nicht verwendet
    werden kann, aber dafür auch keine Fehler hervorruft, wenn irgend
    was anderes nicht initialisiert ist). Das Uninitialisiert sein
    eines Blops erkennt man daran, dass mDaten 0 ist. */
Blop::Blop(): mFreilaufend(true), mBesitzer(0),
  mOrt(absort_nirgends),
  mDaten(0), mDatenAlt(0),
  mZeitNummerDatenAlt(-1),
  mZeitNummerInitStapel(-1),
  /* Uninitialisierten Bildstapel erzeugen. Sonst wird versucht, auf
     noch uninitialisierte Felder von ld zuzugreifen. */
  mBild(false), mBildAlt(false), mWillInitEvent(false)
{
}



/** Constructor... s ist SortenNr. */
Blop::Blop(int s, int v /*=-1*/):
  mKettenGroesse(1),
  mFreilaufend(true), mBesitzer(0), mOrt(absort_nirgends),
  mDatenLaenge(ld->mLevelKnoten->getDatenLaenge()),
  mZeitNummerDatenAlt(-1),
  mZeitNummerInitStapel(-1),
  mWillInitEvent(false)
{
  mDaten = new int[mDatenLaenge];
  mDatenAlt = new int[mDatenLaenge];
  /* Zustandsdaten auf Default-Werte initialisieren */
  for (int i = 0; i < mDatenLaenge; i++)
    switch (ld->mSorten[s]->getDefaultArt(i)) {
      case da_nie:
      case da_keinblob:
	break;
      case da_init:
      case da_kind:
      case da_event:
	mDaten[i] = ld->mSorten[s]->getDefault(i);
        break;
      default:
        CASSERT(false);
    }

  mDaten[spezvar_kind] = s;
  mDaten[spezvar_version] =
    (v==-1 ? Aufnahme::rnd(ld->mSorten[s]->getVersions()) : v);
  
  /* In Zukunft soll das mal nicht mehr möglich sein:
  if (gGleichZeit) {
    print_to_stderr("Neuer Blop in Gleichzeit\n");
  }
  */
  
  setWillInitEvent(true);
}



Blop::Blop(const Blop & b): mFreilaufend(true), mBesitzer(0),
  mOrt(absort_nirgends),
  mDatenLaenge(ld->mLevelKnoten->getDatenLaenge()),
  mZeitNummerDatenAlt(-1),
  mZeitNummerInitStapel(-1),
  mWillInitEvent(false)
{
  CASSERT(b.mDaten);
  CASSERT(b.mDatenLaenge == ld->mLevelKnoten->getDatenLaenge());
  mDaten = new int[mDatenLaenge];
  mDatenAlt = new int[mDatenLaenge];
  kopiere(b);
}

Blop::~Blop() {
  setWillInitEvent(false);
  if (mDaten)
    delete[] mDaten;
  if (mDatenAlt)
    delete[] mDatenAlt;
}


/** Tut, was man erwartet. */
Blop & Blop::operator=(const Blop & b) {
  CASSERT(b.mDaten);
  CASSERT(b.mDatenLaenge == ld->mLevelKnoten->getDatenLaenge());
    
  if (!mDaten || mDatenLaenge != ld->mLevelKnoten->getDatenLaenge()) {
    /* Wir wechseln wohl grad in einen neuen Level. Zumindest
       ändert sich unsere Datenlänge. Oder wir waren noch überhaupt
       nicht initialisiert. */
    if (mDaten)
      delete[] mDaten;
    if (mDatenAlt)
      delete[] mDatenAlt;
    mDatenLaenge = ld->mLevelKnoten->getDatenLaenge();
    mDaten = new int[mDatenLaenge];
    mDatenAlt = new int[mDatenLaenge];
  }

  kopiere(b);
  return *this;
}


/** kopiert die Daten von b. Erwartet, dass die Datenlängen schon
    übereinstimmen. */
void Blop::kopiere(const Blop & b) {
  /* Daten kopieren */
  for (int i = 0; i < mDatenLaenge; i++)
    mDaten[i] = b.mDaten[i];
    
  mZeitNummerDatenAlt = b.mZeitNummerDatenAlt;
  if (gGleichZeit && mZeitNummerDatenAlt == gAktuelleZeitNummerDatenAlt) {
    /* mDatenAlt[] wird grad benutzt. Da müssen wir das wohl
       mitkopieren. */
    for (int i = 0; i < mDatenLaenge; i++)
      mDatenAlt[i] = b.mDatenAlt[i];
  }
  
  mZeitNummerInitStapel = b.mZeitNummerInitStapel;
  if (mZeitNummerInitStapel < gAktuelleZeitNummerInitStapel) {
    /* Bildstapel kopieren; aber nur, wenn er nicht sowieso
       frisch initialisiert werden muss. */
    mBild = b.mBild;
  }
  
  /* BildAlt sollte nach einem Kopieren nicht mehr benötigt werden; 	 
     also brauchen wir es nicht zu kopieren. Aber vielleicht ist 	 
     unser mBildAlt noch von einem vorigen Level, und die Stapelgröße 	 
     stimmt noch nicht. Deshalb neuen Bildstapel erzeugen. 	 
     Der neue Stapel hat übrigens mAnz = -1, so dass - falls 	 
     über neumalen entschieden wird - auf jeden Fall neu gemalt wird. */ 	   mBildAlt = BildStapel(); 	 


  mKettenGroesse = b.mKettenGroesse;

  setWillInitEvent(b.mWillInitEvent);
}



/** Setzt Besitzer und Besitzer-Informationen. Braucht nur am Anfang einmal
    aufgerufen zu werden. Muss auch für den Global-Blop aufgerufen werden;
    sonst darf kein Code ausgeführt werden. */
void Blop::setBesitzer(BlopBesitzer * bes /*= 0*/,
    ort_absolut ort /* = ort_absolut(absort_nirgends)*/) {
  mFreilaufend = false;
  mBesitzer = bes;
  mOrt = ort;
}




/** malt den Blop; xx und yy sind in Pixeln angegeben;
    der Hintergrund wird vorher gelöscht. */
void Blop::malen(int xx, int yy) const {
  /* Ggf. gelazyten initStapel()-Aufruf nachholen */
  braucheInitStapel();

  try { // Um die Fehlermeldung zu verbessern...

    CASSERT(mDaten);
    
    //if (mX == 0 && mY == 19)
    //  mBild.print();

    /* Der Bildstapel soll sich gefälligst selbst malen */
    mBild.malen(xx, yy);

  } catch (Fehler fe) {
    /* TRANSLATORS: This is to improve an error message.
       The first %s is a description of the place where the error occured.
       The second %s is the error message to be improved. */
    throw Fehler(_("%s, during drawing:\n%s"),
		 toString().data(),fe.getText().data());
  }

} // malen








/** liefert die Art zurück */
int Blop::getArt() const {
  CASSERT(mDaten);
  if (mDaten[spezvar_kind] >= 0)
    return blopart_farbe;
  else
    return mDaten[spezvar_kind];
}


/** liefert die Farbe zurück, aber nur, wenns wirklich ein farbiges
    Blop ist (sonst wird keine_farbe zurückgeliefert) */
int Blop::getFarbe() const {
  CASSERT(mDaten);
  if (mDaten[spezvar_kind] >= 0)
    return mDaten[spezvar_kind];
  else
    return keine_farbe;
}

/** liefert die Version zurück. */
int Blop::getVersion() const {
  CASSERT(mDaten);
  return mDaten[spezvar_version];
}

/** liefert true, wenn der Blop am platzen ist */
bool Blop::getAmPlatzen() const {
  CASSERT(mDaten);
  return mDaten[spezvar_am_platzen] != 0;
}




/** Führt die ganzen Animationen durch (Codeanimationen und platzen).
    Sollte innerhalb einer Gleichzeit aufgerufen werden. */
void Blop::animiere() {
  CASSERT(gGleichZeit);
  CASSERT(!mFreilaufend);
  /* Wenn ein Blop auf ein Init-Event wartet, darf noch nix anderes
     aufgerufen werden. */
  CASSERT(!mWillInitEvent);

  /* Ggf. gelazyten initStapel()-Aufruf nachholen. Man könnte genau
     untersuchen, wann während animiere() dieser Aufruf benötigt wird,
     aber irgend wann wird er sowieso in jedem Spielschritt benötigt,
     also macht es keinen Sinn, sich Mühe zu geben. */
  braucheInitStapel();
  
  try { // Um ggf. die Fehlermeldung zu verbessern...

    CASSERT(mDaten);

    /* Erst mal ein paar Initialisierungen. Muss auch gemacht werden,
       wenn der Blop nix von animieren weiß, weil der Änderungswunsch
       initialisiert werden soll. Und vielleicht wollen ja Nachbarn
       was mit diesem Blop tun. */
    initSchritt();

    
    /* Normale (sorteneigene) Animation */

    /* Wenn die Sorte gewechselt hat, müssen wir evtl. noch ein busy-Reset
       verschicken. */
    if (mDaten[spezvar_kind] != mDaten[spezvar_kind_beim_letzten_draw_aufruf]
        && mDaten[spezvar_kind_beim_letzten_draw_aufruf] != -1) {
      Code * alt_co =
        (mDaten[spezvar_kind_beim_letzten_draw_aufruf] == blopart_ausserhalb
          ? 0
          : ld->mSorten[mDaten[spezvar_kind_beim_letzten_draw_aufruf]]
	      ->getEventCode(event_draw));
       if (alt_co)
         alt_co->busyReset(*this);
    }
    mDaten[spezvar_kind_beim_letzten_draw_aufruf] = mDaten[spezvar_kind];

    
    /* Prüfen, ob überhaupt Mal-Code existiert. Das muss bei der
       nix-Sorte nicht sein. */
    Code * mc = getSorte()->getEventCode(event_draw);
    if (mc) {    
      /* Bei diesem eval werden die Bildchen neu in mBild eingefügt...
         Das ist beim Global-Blop aber nicht erlaubt. */
      mMalenErlaubt = true;
      mc->eval(*this);
    }


    /* Debug-Ausgaben schreiben... */
    mBild.setDebugOut(mDaten[spezvar_out1], mDaten[spezvar_out2]);



    /* Animation des Platzens */
    
    /* Zunächst: Leere Blops platzen nicht. (Wenn ein Blop während eines
       Cual-Programms mal ganz kurz das leere Blop ist, dann normalerweise
       so kurz, dass die nachfolgenden Zeilen es nicht merken. */
    if (mDaten[spezvar_kind] == blopart_keins)
      mDaten[spezvar_am_platzen] = 0;
    
    if (mDaten[spezvar_am_platzen] > 0) {
      mDaten[spezvar_am_platzen]++;
      if (mDaten[spezvar_am_platzen] > ld->mExplosionBild.anzBildchen()) {
        /* Letztes Platz-Bildchen. Im nächsten Schritt in
           nichts verwandeln. */
        setKindIntern(blopart_keins);
	/* Wenn der Blop sich als Gras verhält, dann soll er das nicht
	   mehr tun, nachdem er geplatzt ist. */
	mDaten[spezvar_am_platzen] = 0;
      }
    } // if Blop ist am platzen
    
  } catch (Fehler fe) {
    /* TRANSLATORS: This is to improve an error message.
       The first %s is a description of the place where the error occured.
       The second %s is the error message to be improved. */
    throw Fehler(_("%s, during animation:\n%s"),
                 toString().data(), fe.getText().data());
  }
}



/** Führt den Code des angegebenen Events aus (falls Code zu diesem
    Event existiert. Sollte innerhalb einer Gleichzeit aufgerufen
    werden.
    Die Event-Nummern sind in sorte.h definiert. */
void Blop::execEvent(int evtnr) {

  try { // Um die Fehlermeldung zu verbessern...

    CASSERT(gGleichZeit);

    CASSERT(!mFreilaufend);
    /* Wenn ein Blop auf ein Init-Event wartet, darf noch nix anderes
       aufgerufen werden. (mWillInitEvent wird _vor_ der Event-Versendung
       auf false gesetzt.) */
    CASSERT(!mWillInitEvent);

    CASSERT(mDaten);

    Code * ec = getSorte()->getEventCode(evtnr);
    if (ec) {
      /* Während Events sollen Mal-Versuche zu throw führen. */
      mMalenErlaubt = false;
      ec->eval(*this);
    }

  } catch (Fehler fe) {
    /* TRANSLATORS: This is to improve an error message.
       The first %s is a description of the place where the error occured.
       The second %s is the name of an event.
       The third %s is the error message to be improved. */
    throw Fehler(_("%s, during %s event:\n%s"),
              toString().data(), cEventNamen[evtnr], fe.getText().data());
  }
}



/** Startet den Platzvorgang. Sollte nicht für leere Blops aufgerufen
    werden. */
void Blop::lassPlatzen() {
  CASSERT(mDaten);
  if (mDaten[spezvar_kind] == blopart_keins)
    throw Fehler("%s",_("Empty Blops are not allowed to explode."));
  if (mDaten[spezvar_am_platzen] == 0)
    mDaten[spezvar_am_platzen] = 1;
}


/** Teilt einem Farbblop die (neue) Größe seiner Kette mit. */
void Blop::setKettenGroesse(int anz) {
  CASSERT(mDaten);

  /* Kettengröße abspeichern, für den Fall, dass sie als spezconst
     abgefragt wird */
  mKettenGroesse = anz;
}



/** Liefert true, wenn der Blop sich seit dem letzten Aufruf von 	 
    takeUpdaten() verändert hat und deshalb neu gezeichnet werden muss. 	     Liefert auf jeden Fall true, wenn der Blop zwischendrin kopiert wurde 	     (mit = oder copy-Constructor). */ 	 
bool Blop::takeUpdaten() { 	 
  CASSERT(mDaten); 	 
  	 
  /* Ggf. gelazyten initStapel()-Aufruf nachholen */ 	 
  braucheInitStapel(); 	 
  	 
  if (mBild == mBildAlt) 	 
    return false; 	 
  mBildAlt = mBild; 	 
  return true; 	 
}



/***** Funktionen, die nix verändern *****/




/** Liefert true, wenn wir uns mit b verbinden wollen. Auch hier dürfen
    wir allerdings nur die alten Var-Werte von b lesen. Vermutlich liefert
    es das beste Ergebnis, wenn wir von uns auch den alten Wert nehmen. */
bool Blop::verbindetMit(const Blop & b) const {
  CASSERT(mDaten);
  return getVariableVergangenheit(spezvar_kind) ==
       b.getVariableVergangenheit(spezvar_kind);
}


/** Liest die aktuelle Sorte dieses Blops aus den Leveldaten aus. */
Sorte * Blop::getSorte(bool vergangenheit /*= false*/) const {
  return ld->mSorten[vergangenheit
		     ? getVariableVergangenheit(spezvar_kind)
		     : getVariable(spezvar_kind)];
}

/** Fragt beim Besitzer an, in welche Richtungen dieser Blop
    verbunden werden kann und liefert das zurück. */
int Blop::getVerbindungen() const {
  CASSERT(mDaten);
  if (mBesitzer) {
    return mBesitzer->getBesitzVerbindungen(mOrt.x, mOrt.y);
  } else
    return verbindung_solo;
}

/** liefert true, wenn sich der Blop auch mit dem angegebenen
    Rand verbindet */
bool Blop::verbindetMitRand(int seite) const {
  CASSERT(mDaten);
  return getSorte()->getVerbindetMitRand(seite);
}

/** Liefert zurück, wie viele Punkte dieser Stein zur Kettengröße
    beiträgt (normalerweise 1). */
int Blop::getKettenBeitrag() const {
  CASSERT(mDaten);
  return mDaten[spezvar_gewicht];
}

/** Liefert zurück, ob ein bestimmtes Platzverhalten vorliegt */
bool Blop::getVerhalten(int verhalten) const {
  return (mDaten[spezvar_verhalten] & verhalten)==verhalten;
}


/** Liefert zurück, welchem Spieler (0 oder 1) der Blop gehoert oder
    2 bei globalblop */
int Blop::getSpieler() const {
  switch (mOrt.art) {
    case absort_feld:
    case absort_fall:
    case absort_semiglobal:
      return mOrt.rechts;
    case absort_global:
    case absort_nirgends:
      return 2;
  }
  CASSERT(0);
  return 0;
}



/** Liefert einen String der Art "Blop Drachen at x,y".
    Für Fehlermeldungen. */
Str Blop::toString() const {
  return _sprintf(_("Blop %s at %s"), getSorte()->getName().data(),
    mOrt.toString().data());
}


/** Wird vom Code aufgerufen, wenn es Punkte geben soll. */
void Blop::bekommPunkte(int pt) const {
  if (getArt() == blopart_global)
    /* TRANSLATORS: "bonus" is a programming keyword that should not be
       translated. */
    throw Fehler("%s",_("bonus() does not work in the global blob."));
  Cuyo::neuePunkte(mOrt.rechts, pt);
}

/** Wird vom Code aufgerufen, wenn ein Message angezeigt werden soll. */
void Blop::zeigMessage(Str mess) const {
  if (getArt() == blopart_global)
    /* TRANSLATORS: "message" is a programming keyword that should not be
       translated. */
    throw Fehler("%s",_("message() does not work in the global blob."));
  Cuyo::getSpielfeld(mOrt.rechts)->setMessage(mess);
}


const ort_absolut & Blop::getOrt() const {
  return mOrt;
}



/*************************************************************/


/* Macht alle Initialisierungen, die vor jedem Schritt gemacht
   werden. */
void Blop::initSchritt() {
  /* Erst mal die alten Variablen-Werte merken...
     (Eigentlich wäre es ja sauberer, mDaten nicht direkt zu ändern,
     sondern setVariable() zu verwenden. Das würde dann selbst
     merkeAlteVarWerte() aufrufen. Aber egal.) */
  merkeAlteVarWerte();
  
  mDaten[spezvar_file] = 0;
  mDaten[spezvar_pos] = 0;
  mDaten[spezvar_quarter] = viertel_alle;
  mDaten[spezvar_out1] = spezvar_out_nichts;
  mDaten[spezvar_out2] = spezvar_out_nichts;
}



/* Default-Werte werden hier benötigt... */
extern int spezconst_default[spezconst_anz];


int Blop::getSpezConst(int vnr, bool vergangenheit /*= false*/) const {
  CASSERT(mDaten);
  switch (vnr) {
    case spezconst_connect:
      return getVerbindungen();
    case spezconst_size:
      return mKettenGroesse;
    case spezconst_loc_x:
      if (mOrt.art != absort_feld) break;
      return ld->mSpiegeln ? grx - 1 - mOrt.x : mOrt.x;
    case spezconst_loc_y:
      if (mOrt.art != absort_feld) break;
      return ld->mSpiegeln ? gry - 1 - mOrt.y : mOrt.y;
    case spezconst_loc_p:
      if ((mOrt.art == absort_global) || (mOrt.art == absort_nirgends))
        throw iFehler(
        /* TRANSLATORS: "Blop::getSpezConst" and "AbsOrtArt" are
	   programming keywords that should not be translated. */
        _("Internal error in Blop::getSpezConst(): Wrong AbsOrtArt %d"),
        mOrt.art);
       /* Hier sollte noch eine Fehlermeldung ausgegeben werden, wenn man
          das vom Global-Blop aus abfragen will. */
      return mOrt.rechts ? 2 : 1;
    case spezconst_players:
      return ld->mSpielerZahl;
    case spezconst_exploding:
      return getVariableVergangenheit(spezvar_am_platzen);
    case spezconst_basekind:
      return getSorte(vergangenheit)->getBasekind();
    case spezconst_time:
      return Cuyo::getSpielfeld(false)-> getZeit();
  }

  /* Nicht selbst bearbeitet? Dann nach oben weitergeben. */
  if (mBesitzer) {
    int r = mBesitzer->getSpezConst(vnr,this);
    if (r != spezconst_defaultwert)
      return r;
  }

  /* Wir haben keinen Besitzer oder der Besitzer weigert sich, uns 
     was mitzuteilen? Dann Default-Wert. */
  return spezconst_default[-vnr-1];
}


/** High-Level: Wird benutzt, wenn eine Variable im cual-Programm steht.
    Kümmert sich auch um all das @()-Zeug und die Zeitverzögerung. */
int Blop::getVariable(const Variable & v) {
  if (v.Ort_hier())
    return getVariable(v.getNummer());
  else {
    ort_absolut ziel = v.getOrt(mOrt, *this);
    if (ziel.korrekt())
      return ziel.finde().getVariableVergangenheit(v.getNummer());
    else  /* Es ist ein outside-Blob,
             also den Default unabhängig von der Default-Art nehmen. */
      return v.getDefaultWert();
  }
}




/** High-Level: Wird benutzt, wenn eine Variable im cual-Programm steht.
    Kümmert sich auch um all das @()-Zeug und die Zeitverzögerung.
    Und um irgend welche Range-Checks u. ä. */
void Blop::setVariable(const Variable & v, int wert, int op) {
  /* Da will doch nicht jemand etwa eine Spezconst ändern?! */
  if (v.getNummer() < 0)
    throw Fehler(_("The variable \"%s\" is read-only."),
                 v.toString().data());

  if (v.Ort_hier())
    setVariable(v.getNummer(), wert, op);
  else {
    ort_absolut ziel = v.getOrt(mOrt, *this);
    if (ziel.korrekt())
      ziel.finde().setVariableZukunft(v.getNummer(), wert, op);
  }
}



/** Low-Level; wird von den High-Level-Funktionen aufgerufen und vom
    Cual-Programm bei internen Variablen.
    Achtung: Fremdblops sollten *immer* die zeitverschobenen Versionen
    benutzen. */
int Blop::getVariable(int vnr) const {
  CASSERT(mDatenLaenge == ld->mLevelKnoten->getDatenLaenge());
  
  if (vnr < 0)
    return getSpezConst(vnr);
  
  CASSERT(vnr < ld->mLevelKnoten->getDatenLaenge());
  return mDaten[vnr];
}




void Blop::setVariable(int vnr, int wert, int op) {
  merkeAlteVarWerte();
  setVariableIntern(vnr, wert, op);
}


/** Noch low-Levler: Speichert alte Werte nicht ab, wie sich das
    innerhalb einer Gleichzeit gehören würde. */
void Blop::setVariableIntern(int vnr, int wert, int op) {
  CASSERT(mDatenLaenge == ld->mLevelKnoten->getDatenLaenge());
  CASSERT(vnr >= 0 && vnr < ld->mLevelKnoten->getDatenLaenge());

  int oldValue = mDaten[vnr];

  switch (op) {
    case set_code: mDaten[vnr] = wert; break;
    case add_code: mDaten[vnr] += wert; break;
    case sub_code: mDaten[vnr] -= wert; break;
    case mul_code: mDaten[vnr] *= wert; break;
    case div_code: mDaten[vnr] = divv(mDaten[vnr],wert); break;
    case mod_code: mDaten[vnr] = modd(mDaten[vnr],wert); break;
    case bitset_code: mDaten [vnr] |= wert; break;
    case bitunset_code: mDaten [vnr] &= (-1-wert); break;
    default:
      /* TRANSLATORS: "Blop" and "setVariable" are
         programming keywords that should not be translated. */
      throw iFehler(_("Internal error in Blop::setVariable() (op = %d)"),
                   op);
  }

  /* Range-Check bei Spez-Vars */
  /* file, pos und qu werden erst beim Malen gecheckt. version braucht
     gar nicht gecheckt zu werden. out1 und out2 auch nicht. */
  switch (vnr) {
    case spezvar_kind:
      if (mDaten[vnr] < blopart_min_cual || mDaten[vnr] >= ld->mAnzFarben)
        /* TRANSLATORS: "kind" is a programming keyword that should not be
           translated. */
        throw Fehler(_("Value %d for kind out of range (allowed: %d - %d)"),
	             mDaten[vnr], blopart_min_cual, ld->mAnzFarben - 1);
      break;
  }

  if ((vnr==spezvar_kind) && (mDaten[vnr]!=oldValue))
    setKindIntern(mDaten[vnr]);
}


/** spezvar_kind ist eine ganz spezielle spezvar: Ganz lowlevel passiert
    hier noch mehr. */
void Blop::setKindIntern(int wert) {
  mDaten[spezvar_kind] = wert;
  for (int i = 0; i < mDatenLaenge; i++)
    switch (ld->mSorten[wert]->getDefaultArt(i)) {
      case da_nie:
      case da_keinblob:
      case da_init:
      case da_event:
	break;
      case da_kind:
        mDaten[i] = ld->mSorten[wert]->getDefault(i);
	break;
      default:
	CASSERT(false);
    }
}







/** Zeitverschobener Variablenzugriff: Fremdblops sollten immer diese
    Routinen verwenden. */
/** Liefert den Wert der Variable zum Anfang der Gleichzeit zurück. */
int Blop::getVariableVergangenheit(int vnr) const {
  CASSERT(mDatenLaenge == ld->mLevelKnoten->getDatenLaenge());
  
  if (vnr < 0)
    return getSpezConst(vnr,true);
  
  CASSERT(vnr < ld->mLevelKnoten->getDatenLaenge());
  
  /* Wenn unsere Variablen in dieser Gleichzeit noch nicht geändert wurden,
     wurden sie auch noch nicht nach mDatenAlt kopiert. Dann den Wert aus
     mDaten[] holen. */
  if (mZeitNummerDatenAlt == gAktuelleZeitNummerDatenAlt)
    return mDatenAlt[vnr];
  else
    return mDaten[vnr];
}

/** Setzt die Variable am Ende der Gleichzeit. */
void Blop::setVariableZukunft(int vnr, int wert, int op) {
  CASSERT(gGleichZeit);

  if (gZZAnz >= (int) gZZ.size()) {
    /* Nicht genug Speicher im ZZ-Array? Dann vergrößern. */
    gZZ.resize(gZZ.size() * 2 + 16);
  }
  
  gZZ[gZZAnz++] = tZZ(this, vnr, wert, op);
}







/** Speichert, falls nötig, die Variablenwerte in mDatenAlt, für
    zeitverzögerten Variablenzugriff. Falls nötig bedeutet: Falls
    sie in dieser Gleichzeit noch nicht gespeichert wurden.
    Wird von set[Bool]Variable(vnr) aufgerufen, bevor eine Variable
    geändert wird. */
void Blop::merkeAlteVarWerte() {
  CASSERT(gGleichZeit);
  if (mZeitNummerDatenAlt < gAktuelleZeitNummerDatenAlt) {
    for (int i = 0; i < mDatenLaenge; i++)
      mDatenAlt[i] = mDaten[i];
    mZeitNummerDatenAlt = gAktuelleZeitNummerDatenAlt;
  }
}



/** Schaut, ob noch ein Lazy-Evaluation-initStapel()-Aufruf aussteht
    und führt ihn ggf. aus. (Siehe lazyInitStapel().)
    Ist zwar eigentlich nicht wirklich const, wird aber in Situationen
    aufgerufen, wo man sich eigentlich const fühlen möchte. */
void Blop::braucheInitStapel() const {
  if (mZeitNummerInitStapel < gAktuelleZeitNummerInitStapel) {
    ((BildStapel &) mBild).initStapel(mDaten[spezvar_am_platzen]);
    (long &) mZeitNummerInitStapel = gAktuelleZeitNummerInitStapel;
  }
}



/** Speichert das aktuelle Bild (d. h. aus den spezvar file und pos)
    in die Mal-Liste */
void Blop::speichereBild() {
  CASSERT(mDaten);

  /* Ggf. gelazyten initStapel()-Aufruf nachholen. Eigentlich unnötig,
     da das schon am Anfang von animiere() passiert sein sollte. Ich
     fühl' mich aber wohler. */
  braucheInitStapel();

  if ((!mMalenErlaubt) || (!mOrt.bemalbar()))
    throw Fehler("%s",_("Drawing is not allowed at the moment."));
  mBild.speichereBild(getSorte(), mDaten[spezvar_file], mDaten[spezvar_pos],
     mDaten[spezvar_quarter]);
}


/** Speichert das aktuelle Bild (d. h. aus den spezvar file und pos)
    in die Mal-Liste von einem anderen Blop, und zwar so, dass es
    in Ebene ebene gemalt wird. */
void Blop::speichereBildFremd(Ort & ort, int ebene) {
  CASSERT(mDaten);

  ort_absolut ziel = ort.berechne(mOrt,*this);
  if (ziel.korrekt()) {
    Blop & b = ziel.finde();

    if ((!mMalenErlaubt) || (!b.mOrt.bemalbar()))
      throw Fehler("%s",_("Drawing is not allowed at the moment."));

    /* Ggf. gelazyten initStapel()-Aufruf nachholen. An dieser Stelle
       ist das wichtig, weil der bemalte Blop evtl. noch nicht animiert()
       wurde. */
    b.braucheInitStapel();

    b.mBild.speichereBild(
          getSorte(), mDaten[spezvar_file], mDaten[spezvar_pos],
          mDaten[spezvar_quarter], ebene);
  }
}



/********** Init-Event-Verwaltung **********/

/** Setzt, ob dieser Blop ein Init-Event will. */
void Blop::setWillInitEvent(bool wie) {
  if (mWillInitEvent == wie)
    return;
  mWillInitEvent = wie;
  
  if (mWillInitEvent) {
    mWIEPos = gWIEListe.size();
    gWIEListe.push_back(this);
    
  } else {
    gWIEListe[mWIEPos] = 0;
  }
}


std::vector<Blop *> Blop::gWIEListe;

/** Ruft alle noch fehlenden Init-Events aus. */
void Blop::sendeFehlendeInitEvents() {
  beginGleichzeitig();
  for (int i = 0; i < (int) gWIEListe.size(); i++)
    if (gWIEListe[i]) {
      gWIEListe[i]->mWillInitEvent = false;
      gWIEListe[i]->execEvent(event_init);
    }
  gWIEListe.clear();
  endGleichzeitig();
}


void Blop::playSample(int nr) const {mOrt.playSample(nr);}



/********** Statisches Zeug **********/

bool Blop::gGleichZeit = false;
long Blop::gAktuelleZeitNummerDatenAlt = 0;
long Blop::gAktuelleZeitNummerInitStapel = 0;
/** Liste Zukunfts-Zuweisungen, die sich während einer Gleichzeit
    ansammeln. */
std::vector<Blop::tZZ> Blop::gZZ;
int Blop::gZZAnz;
//bool Blop::gInitEventsAutomatisch = true;



/** Wenn es für den Cual-Programmierer so aussehen soll, als würden Dinge
    gleichzeitig passieren (d. h. @ greift zeitverzögert zu), dann sollte
    man erst beginGleichzeitig() aufrufen, dann die ganzen Blop-Programm-
    aufrufe, und dann endGleichzeitig().
    */
void Blop::beginGleichzeitig() {
  CASSERT(!gGleichZeit);
  gGleichZeit = true;
  
  gAktuelleZeitNummerDatenAlt++;
  
  /* Bis jetzt gibt's noch keine Zukunftszuweisungen: */
  gZZAnz = 0;
}


/** Siehe beginGleichzeitig(). */
void Blop::endGleichzeitig() {
  CASSERT(gGleichZeit);
  gGleichZeit = false;
  
  /* Zukunftszuweisungen durchführen */
  for (int i = 0; i < gZZAnz; i++) {
    const tZZ & zz = gZZ[i];
    zz.mBlop->setVariableIntern(zz.mVNr, zz.mWert, zz.mOperation);
  }
}



/** Bricht eine Gleichzeit einfach ab. Wird beim Auftreten von Fehlern
    aufgerufen, und zwar vom Constructor von Fehler(). */
void Blop::abbruchGleichzeitig() {
  gGleichZeit = false;
}


/** Tut so, als würde es initStapel() für alle Blops aufrufen (d. h. die
    Grafiken löschen. In Wirklichkeit passiert das mit Lazy-Evaluation,
    d. h. erst dann, wenn's wirklich gebraucht wird.
    Gebraucht wird's natürlich, wenn ein Blop animiert wird. Aber auch,
    wenn ein Nachbarblop etwas auf diesen Blop malt. */
void Blop::lazyInitStapel() {
  /* Wenn ein Blop sieht, dass seine mZeitNummerInitStapel-Variable
     kleiner ist als gAktuelleZeitNummerInitStapel, ruft er initStapel()
     auf. */
  gAktuelleZeitNummerInitStapel++;
}


/** Stellt ein, ob Blops */
// void Blop::setInitEventsAutomatisch(bool iea) {
//   gInitEventsAutomatisch = iea;
// }



/***** Für globales Animationszeug *****/


/** Der Blop, der ld->mGlobalCode ausführt. Wird am Ende von
    LevelDaten::ladLevel() initialisiert. */
Blop Blop::gGlobalBlop;


