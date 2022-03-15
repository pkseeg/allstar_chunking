/***************************************************************************
                          knoten.cpp  -  description
                             -------------------
    begin                : Mit Jul 12 22:54:51 MEST 2000
    copyright            : (C) 2000 by Immi
    email                : cuyo@pcpool.mathematik.uni-freiburg.de

Modified 2002,2003,2005,2006,2008-2011 by the cuyo developers
Maintenance modifications 2012 by Bernhard R. Link
Maintenance modifications 2012 by the cuyo developers

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

#include "cuyointl.h"
#include "knoten.h"
#include "sorte.h"
#include "leveldaten.h"
#include "blop.h"
#include "global.h"
#include "layout.h"


/*************************************************************************/
/* Was von vornerein so im Namespace rumliegt */


/* xxx_anz wird in blop.h definiert. */

char spezvar_namen[spezvar_anz][22] = {
  "file", "pos", "kind", "version", "qu",
  "out1", "out2",
  "", // kind_beim_letzten_draw_aufruf; ist nicht für den User gedacht.
  "inhibit", "weight", "behaviour",
  "falling_speed", "falling_fast_speed",
  "" // am_platzen; kriegt der Cual-Code (im Moment) nur als Konstante
     // "exploding" zu sehen (0 = nicht am platzen; sonst 1 - 8)
};

int spezvar_default[spezvar_anz] = {
  0, 0, blopart_ausserhalb, 0, viertel_alle,
  spezvar_out_nichts, spezvar_out_nichts,
  blopart_ausserhalb,
  0, 1, 0,
  6, gric,
  0
};

int spezvar_defaultart[spezvar_anz] = {
  da_event, da_event, da_keinblob, da_keinblob, da_event,
  da_event, da_event,
  da_init,
  da_init, da_init, da_nie,
  da_init, da_init,
  da_init
};


/* Reihenfolge -1, -2, -3, ... */
char spezconst_namen[spezconst_anz][22] = {
  "turn", "connect", "falling", "size", "loc_x", "loc_y", "loc_p",
  "players", "falling_fast", "exploding", "loc_xx", "loc_yy", "basekind", "time"
};

int spezconst_default[spezconst_anz] = {
  0, 0, 0, 0, -1, -1, 0,
  0, 0, 0, -1, -1, blopart_ausserhalb, 0
};


#define const_anz (21+4+2*9+6+8)
//int const_anz = feste_konst_anz;
char const_namen[const_anz][27] = {
  /* Viertelstückchen */
  "Q_ALL",
  "Q_TL", "Q_TR", "Q_BL", "Q_BR",
  "Q_TL_TL", "Q_TR_TL", "Q_BL_TL", "Q_BR_TL",
  "Q_TL_TR", "Q_TR_TR", "Q_BL_TR", "Q_BR_TR",
  "Q_TL_BL", "Q_TR_BL", "Q_BL_BL", "Q_BR_BL",
  "Q_TL_BR", "Q_TR_BR", "Q_BL_BR", "Q_BR_BR",
  
  /* Sortennamen */
  "nothing", "outside", "global", "semiglobal",
  /* Buchstaben, die in den Strings in Nachbariterator verwendet werden. */
  /*   A        B         C        D         E          F          G          H          I */
  "DIR_U", "DIR_UR", "DIR_R", "DIR_DR", "DIR_UUL", "DIR_UUR", "DIR_RRU", "DIR_RRD", "DIR_F",
  "DIR_D", "DIR_DL", "DIR_L", "DIR_UL", "DIR_DDR", "DIR_DDL", "DIR_LLD", "DIR_LLU", "DIR_B",

  /* spezvar_verhalten */
  "explodes_on_size", "explodes_on_explosion", "explodes_on_chain_reaction",
  "calculate_size", "goalblob", "floats",

  /* Nachbarschaften */
  "neighbours_rect", "neighbours_diagonal", "neighbours_hex6",
  "neighbours_hex4", "neighbours_knight", "neighbours_eight", "neighbours_3D",
  "neighbours_none"
};

/* Eigentlich sollten hier die Konstanten aus bilddatei.h verwendet werden;
   das wäre aber so viel Tipparbeit... */
int const_werte[const_anz] = {
  /* Viertelstückchen */
  viertel_alle,
  0, 5, 10, 15,
  0, 1, 2, 3,
  4, 5, 6, 7,
  8, 9, 10, 11,
  12, 13, 14, 15,
  
  /* Sortennamen */
  blopart_keins, blopart_ausserhalb, blopart_global, blopart_semiglobal,
  
  /* Richtungskonstanten für inhibit */
  0x00000001, 0x00000002, 0x00000004, 0x00000008, 0x00000010, 0x00000020, 0x00000040, 0x00000080, 0x00000100,
  0x00010000, 0x00020000, 0x00040000, 0x00080000, 0x00100000, 0x00200000, 0x00400000, 0x00800000, 0x01000000,

  /* Bits fürs spezvar_verhalten */
  platzt_bei_gewicht, platzt_bei_platzen, platzt_bei_kettenreaktion,
  berechne_kettengroesse, verhindert_gewinnen, schwebt,

  /* Nachbarschaften */
  nachbarschaft_normal, nachbarschaft_schraeg, nachbarschaft_6,
  nachbarschaft_6_schraeg, nachbarschaft_springer, nachbarschaft_dame,
  nachbarschaft_6_3d, nachbarschaft_garnichts
};



/* Für den internen Gebrauch in Schleifen: Die pics-artigen keys. */
const char * gPicsetc[] = {
  "pics",
  "greypic",
  "startpic",
  0
};




/*************************************************************************/
/* Knoten */


/** Liefert einen String zurück, der angibt, wo dieser Code
    definiert wurde (für Fehlermeldungen) */
Str Knoten::getDefString() const {
  return _sprintf("%s:%d", mDateiName.data(), mZeilenNr);
}




/*************************************************************************/
/* DefKnoten */


/** Erzeugt den Top-Knoten. */
DefKnoten::DefKnoten():
  Knoten("?", 0),
  mVater(0), mTiefe(tiefe_global),
  mErstLevelDannCual(0),
  mVarNrBei(0),
  mBoolNrBei(-1),
  mDefaultWerte(std::vector<int>()),
  mDefaultArten(std::vector<int>()),
  mSortenAnfaenge()
{
  /* Die vordefinierten Namespace-Dinge einfügen. */
  speicherGlobaleVordefinierte();
}


/** Erzeugt einen Unter-Knoten. */
DefKnoten::DefKnoten(Str datna, int znr, DefKnoten * vater):
  Knoten(datna, znr),
  mVater(vater), mTiefe(vater->mTiefe + 1),
  mErstLevelDannCual(0),
  /* Das ganze Variablen-Zeug vom Vater übernehmen...: */
  mVarNrBei(vater->mVarNrBei),
  mBoolNrBei(vater->mBoolNrBei),
  mDefaultWerte(vater->mDefaultWerte),
  mDefaultArten(vater->mDefaultArten),
  mSortenAnfaenge()
{
}

DefKnoten::~DefKnoten() {
  tKnotenMap::Index it(mKinder);
  for(; !it.ende(); ++it)
    delete it.datum();
}




Str DefKnoten::toString() const {
  Str ret = "{\n";
  tKnotenMap::constIndex it(mKinder);
  for(; !it.ende(); ++it) {
    ret += it.schluessel() + it.version().toString()
        + "=" + it.datum()->toString() + "\n";
  }
  ret += "}\n";
  return ret;
}


void DefKnoten::fuegeEin(const Str & na, const Version & version,
			 Knoten * wert) {
  if (mKinder.enthaelt(na,version))
    throw Fehler(_("\"%s\" already defined."), na.data());
  mKinder.neuerEintrag(na,version,wert);

  /* Wenn ein neuer Level geparst wird, will das ld evtl. wissen.
     (Falls der Benutzer neu gemachte Level ausprobieren will,
     die noch nicht in "level=..." stehen.) */
  if (mTiefe == tiefe_global && wert->type() == type_DefKnoten) {
    ld->levelGefunden(na);
    
    /* Und bei der Gelegenheit merken wir uns gleich noch, dass schon
       ein Level in diesem Knoten ist. Wenn jetzt noch Cual-Code kommt,
       ist das gefährlich, und es soll eine Warnung ausgegeben werden. */
    mErstLevelDannCual |= 1;
  }
  
  /* Auf Level-Ebene evtl. noch ein paar Konstanten einfügen... */
  if (mTiefe == tiefe_level) {
    /* pics-Eintrag? Dann können ja jetzt die pics-Konstanten erzeugt
       werden. */

    /* Achtung, verwirrend: wert ist nicht der Wert der Konstante,
       sondern das, was rechts vom "=" in der ld-Datei steht. */
    for (const char ** i=gPicsetc; *i; i++)
      if (na==*i)
	speicherPicsConst(version, wert, *i);
    /* Wenn es den Namen schon gab, sind wir ganz still, denn das ist OK. */
    try {
      if (na == "emptypic")
	speicherKnotenConst(version, wert, blopart_keins);
    } catch (Fehler f) {}
  }

  /* Wenn es eine Konstante ist, wird auch noch eine Variable draus gemacht. */
  const DatenKnoten * wert_;
  switch (wert->type()) {
    case type_DatenKnoten:
      wert_ = (const DatenKnoten *) wert;
      break;
    case type_ListenKnoten:
      if (((ListenKnoten*) wert)->getImpliziteLaenge() == 1)
        wert_ = ((ListenKnoten*) wert)->getDatum(0);
      else
        wert_ = 0;
      break;
    default:
      wert_ = 0;
  }
  if (wert_)
    if (wert_->datatype() == type_ZahlDatum)
      speicherDefinition(namespace_variable, na, version,
        new VarDefinition(na, wert_->getZahl(), vd_konstante, da_keinblob, 0));
}


/** Löscht alle Kinder raus, die DefKnoten sind und nicht
    "Title" heißen.
    Wird von LevelDaten::ladLevelSummary() gebraucht. */
void DefKnoten::loeschAlleLevel() {
  tKnotenMap::Index it(mKinder);
  while (!it.ende()) {
    Knoten * k = it.datum();
    tKnotenMap::Index nae=it;  ++nae;
    if (k->type() == type_DefKnoten && it.schluessel() != "Title") {
      delete k;
      mKinder.loescheEintrag(it);
    }
    it = nae;
  }
}


/***** Methoden für den Codespeicher *****/



/** Speichert alle vordefinierten Variablen in den
    Namespace, außer die pics-Konstanten. Wird vom Constructor
    des WurzelKnotens aufgerufen. */
void DefKnoten::speicherGlobaleVordefinierte() {

  Version vallg = Version();

  /* Spezial-Variablen (z. B. file); Nummern ab 0 */
  /* Das sollten die ersten Variablen sein, die erzeugt werden;
     sonst stimmen nachher die Nummern nicht. */
  CASSERT(mVarNrBei == 0);
  for (int i = 0; i < spezvar_anz; i++) {
    /* Nicht direkt mit speicherDefinition() erzeugen; sonst
       werden die Default-Werte nicht nochmal in mDefaultWerte
       gespeichert. */
    neueVarDefinition(spezvar_namen[i], vallg,
		      spezvar_default[i], spezvar_defaultart[i]);
  }
  
  /* Spezial-Konstanten (d. h. Variablen, die nur gelesen werden
     werden können), z. B. verbindung; Nummern < 0 */
  for (int i = 0; i < spezconst_anz; i++)
    speicherDefinition(namespace_variable, spezconst_namen[i], vallg,
        new VarDefinition(spezconst_namen[i], spezconst_default[i],
                          vd_spezconst, da_nie, -i-1)
      );

  /* Ganz normale feste Konstanten (z. B. nochange) werden so abgespeichert,
     daß auch außerhalb von cual darauf zugegriffen werden kann. */
  for (int i = 0; i < const_anz; i++)
    fuegeEin(const_namen[i], vallg, new ZahlKnoten("",0,const_werte[i]));

}



/** Speichert die Pics-Konstanten. (picsliste sollte der pics-Knoten sein.)
    Wird von fuegeEin(...) aufgerufen, wenn es die pics bekommt.
    Alternativ auch bei greypic und startpic.
    Welches davon steht in schluessel */
void DefKnoten::speicherPicsConst(const Version& version, Knoten * picsliste,
				  const char* schluessel) {
  /* Eigentlich könnten wir gleich eine Fehlermeldung
     ausspucken, wenn der Typ von picsliste falsch ist, aber die anderen
     entsprechenden Typ-Fehlermeldungen kommen auch erst beim Starten des
     Levels. Also hier erst mal ruhig sein. */
  if (picsliste->type() == type_ListenKnoten) {

    /* Erstmal den Anfang ermitteln (wenn wir das noch nicht getan haben).
       Das ist die Summe der impliziten Listenlängen zu den anderen
       Schlüsseln, die schon vorkamen.
       Und zwar in der korrekten, das heißt globalen Version.
       Daß wir die schon jetzt kennen, stellt VersionMap automatisch sicher,
       indem ein Aufruf von Bestapproximierende die VersionMap für spätere
       Einträge sperrt. */
    int anfang;
    std::map<Str,int>::const_iterator i = mSortenAnfaenge.find(schluessel),
                                      e = mSortenAnfaenge.end();
    if (i==e) {
      anfang = getSortenAnzahl();
      mSortenAnfaenge[schluessel] = anfang;
    } else
      anfang = i->second;

    ListenKnoten * lki = (ListenKnoten*) picsliste;
    int nummer=anfang;
    for (int i = 0; i < lki->getLaenge(); i++) {
      /* picsEndungWeg() steht in global.* */
      Str varna = picsEndungWeg(lki->getKernDatum(i,type_WortDatum)
				     ->getWort());
      /* Wenn es den Namen schon gab, sind wir ganz still, denn das ist OK. */
      if (!mCodeSpeicher[namespace_variable].enthaelt(varna,version))
        speicherDefinition(namespace_variable, varna, version,
            new VarDefinition(varna, nummer, vd_konstante, da_nie, 0));
      nummer += lki->getVielfachheit(i);
    }
  }
}


/** Speichert eine Konstante mit dem Namen, der in nameKnoten steht und
    dem angegebenen Wert. nameKnoten ist hoffentlich ein ListenKnoten
    mit genau einem Eintrag. Wird von fuegeEin() aufgerufen, um die
    Gras-, die Grau- und die nix-Konstante abzuspeichern, wenn es die
    bekommt. */
void DefKnoten::speicherKnotenConst(const Version & version,
				    Knoten * nameKnoten, int wert) {
  /* Keine Fehlermeldung bei falschem Typ; siehe speicherPicsConst() */
  if (nameKnoten->type() == type_ListenKnoten) {
    ListenKnoten * lk = (ListenKnoten*) nameKnoten;
    if (lk->getLaenge() == 1) {
      Str varna = picsEndungWeg(lk->getDatum(0,type_WortDatum)
				     ->getWort());
      speicherDefinition(namespace_variable, varna, version,
          new VarDefinition(varna, wert, vd_konstante, da_nie, 0)
        );
    }
  }
}



/* Erzeugt eine neue Var-Definition und speichert sie ab. Dabei
   bekommt sie auch gleich eine Nummer. (Aufzurufen, wenn eine
   VarDefinition geparst wurde.) def ist der Default-Wert. */
void DefKnoten::neueVarDefinition(const Str & na, const Version & version,
				  int def, int defart) {
  //print_to_stderr(_sprintf("Define %s\n", na.data()));
  if (na == "") {
    /* Wenn kein Name angegeben wurde, handelt es sich wohl um eine
       Spez-Var, die für den User unsichtbar sein soll. Dann nur
       die Variable erzeugen, aber keine Definition abspeichern. */
    neueVariable(def,defart);
  } else {
    speicherDefinition(namespace_variable, na, version,
      new VarDefinition(na, def, vd_variable, defart,
			neueVariable(def,defart)));
  }
}


/* Speichert eine neue Definition - Code oder Variable. Noch unschön:
   Sollte von außen nur für Code aufgerufen werden. Bei Variablen immer
   neueVarDefinition verwenden! */
void DefKnoten::speicherDefinition(int ns, const Str & na,
				   const Version & version, Definition * f) {
/*  print_to_stderr(_sprintf("Speichere %s = %s\n", na.data(),
         f->toString().data()));*/
  if (mCodeSpeicher[ns].enthaelt(na,version))
    throw Fehler(_("\"%s\" already defined."), na.data());

  /* (Vielleicht kennt Papi den Code schon, aber das kann man
     überschreiben.) */
     
  mCodeSpeicher[ns].neuerEintrag(na,version,f);
  
  /* Wenn wir der globale Knoten sind und schon eine Level-Definition kam,
     dann Warnung ausgeben. */
  if (gDebug && mErstLevelDannCual == 1) {
    /* TRANSLATORS: "Cual" is a programming language's name */
    print_to_stderr(_("Warning: There's global Cual code _after_ some level definitions. Be sure\n"
              "not to use that Cual code in the levels before it. (Due to a bug, this will\n"
	      "sometimes not result in an error message but in strange behaviour.)\n"));
    mErstLevelDannCual |= 2;
  }
}



/** Liefert eine Code-Definition aus diesem Speicher oder von
    weiter oben. Throwt bei Nichtexistenz.
    Achtung: Behält den Besitz an der Defintion. */
Definition * DefKnoten::getDefinition(int ns, const Str & na,
				      const Version & version,
				      bool defaultVorhanden) {

  Definition * ret;

  if (!mCodeSpeicher[ns].enthaelt(na)) {
    /* Wir kennen den Code nicht. Also Papi fragen. */
    if (mVater)
      return mVater->getDefinition(ns, na, version, defaultVorhanden);
      
    /* Ups, wir haben ja gar keinen Papi. */
    if (defaultVorhanden)
      return 0;
    else
      /* TRANSLATORS: "<<" and ">>" are programming keywords
	 and should not be translated. */
      throw Fehler(_("\"%s\" not defined inside << >>."), na.data());
  }
  
  ret = mCodeSpeicher[ns].Bestapproximierende(na,version,defaultVorhanden);
  
  return ret;
}



/** Liefert ein Kind von hier oder von weiter oben. */
Knoten * DefKnoten::getVerwandten(const Str & na, const Version & version,
				  bool defaultVorhanden) {

  Knoten * ret;

  if (!mKinder.enthaelt(na)) {
    /* Das ist nicht mein Kind. Also Papi fragen. */
    if (mVater)
      return mVater->getVerwandten(na,version,defaultVorhanden);
      
    /* Ups, wir haben ja gar keinen Papi. */
    if (defaultVorhanden)
      return 0;
    else
      throw Fehler(_("\"%s\" not defined."), na.data());
  }
  
  ret = mKinder.Bestapproximierende(na,version,defaultVorhanden);
  
  return ret;
}




/***** Variablen-Nummern-Verwaltung *****/


/** Erzeugt eine unbenannte Variable
    und liefert die Nummer zurück. */
int DefKnoten::neueVariable(int def, int defart) {
  /* Sorten-Variablen gibt's nicht (wirklich). Die kommen in den
     Level-Knoten */
  if (mTiefe == tiefe_sorte)
    return mVater->neueVariable(def,defart);

  CASSERT((int) mDefaultWerte.size() == mVarNrBei);
  CASSERT((int) mDefaultArten.size() == mVarNrBei);
  //print_to_stderr(_sprintf("t=%d  %d %d\n", mTiefe, mDefaultWerte.size(), mVarNrBei));
  /* Das kann man auch schneller machen... (immer mehr auf einmal
     reservieren) falls das QT nicht eh tut */
  mDefaultWerte.resize(mVarNrBei + 1);
  mDefaultArten.resize(mVarNrBei + 1);
  mDefaultWerte[mVarNrBei] = def;
  mDefaultArten[mVarNrBei] = defart;
  
  return mVarNrBei++;
}



void DefKnoten::neuerDefault(int var, int def, int defart) {
  CASSERT(mVarNrBei>var);
  mDefaultWerte[var]=def;
  mDefaultArten[var]=defart;
}



/** Erzeugt eine unbenannte Bool-Variable und liefert
    die Nummer zurück. */
int DefKnoten::neueBoolVariable() {
  /* Sorten-Variablen gibt's nicht (wirklich). Die kommen in den
     Level-Knoten */
  if (mTiefe == tiefe_sorte)
    return mVater->neueBoolVariable();

  if (mBoolNrBei == -1) {
    /* Grad kein Platz mehr für Bools. Also neuen
       Platz erzeugen. */
    mBoolNrBei = bits_pro_int * neueVariable(0,da_init);
  }
  
  int ret = mBoolNrBei;
  mBoolNrBei++;
  
  /* Ist das aktuelle int voll mit Bools? */
  if (mBoolNrBei % bits_pro_int == 0) {
    mBoolNrBei = -1;
  }
  
  return ret;
}



int DefKnoten::getDatenLaenge() const {
  CASSERT(mTiefe == tiefe_level);
  return mVarNrBei;
}


  
/** Liefert den Default-Wert der Variable mit Nummer nr. Es
    muss aber eine richtige Variable sein, die echten Blop-
    Speicherplatz verbraucht. (Sonst soll man sich den Default-
    Wert aus der VarDefinition holen. Das hier ist nur für
    Variablen-Anfangs-Initialisierung.) */
int DefKnoten::getDefaultWert(int nr) const {
  if (mVarNrBei<=nr) {
    CASSERT(mVater);
    return mVater->getDefaultWert(nr);
  }
  else
    return mDefaultWerte[nr];
}

int DefKnoten::getDefaultArt(int nr) const {
  if (mVarNrBei<=nr) {
    CASSERT(mVater);
    return mVater->getDefaultArt(nr);
  }
  else
    return mDefaultArten[nr];
}


/** Liest mSortenAnfaenge aus. Throwt bei Nichtexistenz. */
int DefKnoten::getSortenAnfang(const Str & schluessel) const {
  std::map<Str,int>::const_iterator i = mSortenAnfaenge.find(schluessel);
  CASSERT(i!=mSortenAnfaenge.end());
  return i->second;
}


/** Liefert zurueck, wie viele Sortennummern insgesamt schon
    von mSortenAnfaenge belegt sind. */
int DefKnoten::getSortenAnzahl() const {
  std::map<Str,int>::const_iterator i;
    /* Anzahl der Sorten ermitteln
       Das ist die Summe der impliziten Listenlängen zu den anderen
       Schlüsseln, die schon vorkamen.
       Und zwar in der korrekten, das heißt globalen Version.
       Daß wir die schon jetzt kennen, stellt VersionMap automatisch sicher,
       indem ein Aufruf von Bestapproximierende die VersionMap für spätere
       Einträge sperrt. */

  /* Zu refaktorisieren:
     Eigentlich sollte man den Wert irgendwo zwischenspeichern statt
     ihn jedes mal neu auszurechnen... falls das moeglich ist, in
     Anbetracht der Versionierung. Damit kenne ich mich nicht aus. */

  int anzahl = 0;
  for (i=mSortenAnfaenge.begin(); i!=mSortenAnfaenge.end(); i++) {
    CASSERT(mKinder.enthaelt(i->first));
    Knoten* eintrag = mKinder.Bestapproximierende(i->first,
						  ld->getVersion(),
						  false);
    CASSERT(eintrag->type()==type_ListenKnoten);
    anzahl += ((ListenKnoten*) eintrag)->getImpliziteLaenge();
  }
  return anzahl;
}



/*************************************************************************/
/* ListenKnoten */


ListenKnoten::ListenKnoten(Str datna, int znr): Knoten(datna, znr) {}


ListenKnoten::~ListenKnoten() {
  for (int i = 0; i < (int) mKinder.size(); i++)
    delete(mKinder[i]);
}


Str ListenKnoten::toString() const {
  Str ret;
  bool mitte = false;
  for (int i = 0; i < (int) mKinder.size(); i++) {
    if (mitte) ret += ", ";
    mitte = true;
   
    ret += mKinder[i]->toString();
  }
  return ret;
}




int ListenKnoten::getVielfachheit(int nr) const {
  DatenKnoten * k = (DatenKnoten *) (mKinder[nr]);
  CASSERT(k->type() == type_DatenKnoten);
  if (k->datatype()==type_VielfachheitDatum)
    return k->getZahl();
  else
    return 1;
}



int ListenKnoten::getImpliziteLaenge() const {
  return getLaengeBis(getLaenge());
}



int ListenKnoten::getLaengeBis(int nr) const {
  int ret=0;
  for (int i=0; i<nr; i++)
    ret += getVielfachheit(i);
  return ret;
}


const DatenKnoten * ListenKnoten::getDatum(int nr,
					   int solltyp /*= type_EgalDatum*/) {
  Knoten * k = mKinder[nr];
  CASSERT(k->type() == type_DatenKnoten);
  return ((DatenKnoten *) k)->assert_datatype(solltyp);
}



const DatenKnoten * ListenKnoten::getKernDatum(int nr,
					       int solltyp
					       /*= type_EgalDatum*/) {
  const DatenKnoten * d = getDatum(nr);
  if (d->datatype() == type_VielfachheitDatum)
    d = ((VielfachheitKnoten *) d)->getNurDasWort();
  return d->assert_datatype(solltyp);
}



const DatenKnoten * ListenKnoten::getImplizitesDatum(int nr,
						     int solltyp
						     /*= type_EgalDatum*/) {
  int nr_ = 0;
  int vielfachheit = getVielfachheit(0);
  while (nr>=vielfachheit) {
    nr -= vielfachheit;
    nr_++;
    vielfachheit = getVielfachheit(nr_);
  }
  return getKernDatum(nr_,solltyp);
}


/** Setzt voraus, daß es nur einen Eintrag gibt. Gibt diesen Eintrag. */
const DatenKnoten * ListenKnoten::getEinzigesDatum(int solltyp
                                             /*= type_EgalDatum*/) {
  if (getImpliziteLaenge()!=1)
    throw Fehler("%s",_("Value is a list and should not."));
  return getDatum(solltyp);
}


/*************************************************************************/
/* DatenKnoten */

const DatenKnoten * DatenKnoten::assert_datatype(int solltyp) const {
  if ((solltyp != datatype()) && (solltyp!=type_EgalDatum)) {
    Str solltyp_Darstellung = _("<unknown type, please report>");
    if (solltyp==type_WortDatum)
      solltyp_Darstellung = "string";
    if (solltyp==type_ZahlDatum)
      solltyp_Darstellung = "int";
    if (solltyp==type_VielfachheitDatum)
      solltyp_Darstellung = "string * int";
    throw Fehler(_("%s is of wrong type. %s expected"),
		 toString().data(),
		 solltyp_Darstellung.data());
  }
  return this;
}




/*************************************************************************/
/* WortKnoten */

Str WortKnoten::getWort(int rolle /*= wortrolle_einziges*/) const {
  CASSERT(rolle==wortrolle_einziges);
  return mWort;
}

Str WortKnoten::toString() const {
  return mWort;
}




/*************************************************************************/
/* ZahlKnoten */

int ZahlKnoten::getZahl(int rolle /*= zahlrolle_einziges*/) const {
  CASSERT(rolle==zahlrolle_einzige);
  return mZahl;
}

Str ZahlKnoten::toString() const {
  return _sprintf("%d", mZahl);
}




/*************************************************************************/
/* VielfachheitKnoten */

int VielfachheitKnoten::getZahl(int rolle /*= zahlrolle_einzige*/) const {
  CASSERT(rolle==zahlrolle_einzige);
  return mZahl;
}

Str VielfachheitKnoten::getWort(int rolle
				     /*= wortrolle_einziges*/) const {
  CASSERT(rolle==wortrolle_einziges);
  return mWort;
}

Str VielfachheitKnoten::toString() const {
  return _sprintf("%s * %d",mWort.data(),mZahl);
}

