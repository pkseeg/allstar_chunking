/***************************************************************************
                          datendatei.cpp  -  description
                             -------------------
    begin                : Sun Jul 1 2001
    copyright            : (C) 2001 by Immi
    email                : cuyo@karimmi.de

Modified 2002,2005,2006,2010,2011 by the cuyo developers

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

#include "datendatei.h"
#include "knoten.h"


/** In parser.yy definiert. */
void parse(const Str & name, DefKnoten * erg);


DatenDatei::DatenDatei():
  mDaten(new DefKnoten()) /* Leerer Knoten ohne Vater */
{
  /* Das Squirrel sitzt am Anfang ganz oben */
  initSquirrel();
}

DatenDatei::~DatenDatei() {
  delete mDaten;
}




/** Entfernt alles, was bisher geladen wurde. Aufrufen, wenn man alles
    neu laden möchte. */
void DatenDatei::leeren() {
  delete mDaten;
  mDaten = new DefKnoten();

  /* Squirrel neu setzen. (Sonst stimmt der Pointer nicht mehr.) */
  initSquirrel();
}



/** Lädt die angegebene Datei. (Kann mehrmals aufgerufen werden, um
    mehrere Dateien gleichzeitig zu laden.) */
void DatenDatei::laden(const Str & name) {
  parse(name, mDaten);
}






/***** Squirrel-Methoden *****/


/** Setzt das Squirrel an die Wurzel des Baums. */
void DatenDatei::initSquirrel() {
  mSquirrelPosString = "";
  mSquirrelKnoten = mDaten;
  mSquirrelCodeKnoten = mDaten;
}


/** Liefert true, wenn das Squirrel sich an einer Stelle des Baums
    befindet, die existiert. */
bool DatenDatei::existiertSquirrelKnoten() const {
  return mSquirrelKnoten;
}


/** Liefert die Position des Squirrels als String. */
Str DatenDatei::getSquirrelPosString() const {
  return mSquirrelPosString;
}


/** Das Eichhörnchen klettert weiter weg von der Wurzel. Wird von
    DatenDateiPush benutzt. */
void DatenDatei::kletterWeiter(const Str & na, const Version & version) {

  /* Neuen Abschnittnamen bauen */
  if (!mSquirrelPosString.isEmpty())
    mSquirrelPosString += '/';
  mSquirrelPosString += na;

  /* Knoten zum neuen Abschnitt suchen */
  mSquirrelKnoten = (DefKnoten *) getEintragKnoten(na, version, true,
						   type_DefKnoten);
  
  /* Wenn dieser Unterabschnitt existiert, dann auch die Codeen
     dort suchen. */
  if (mSquirrelKnoten)
    mSquirrelCodeKnoten = mSquirrelKnoten;
}



/** Liefert die Squirrel-Position zurück (und zwar
    mSquirrelCodeKnoten; siehe dort). */
DefKnoten * DatenDatei::getSquirrelPos() const {
  return mSquirrelCodeKnoten;
}



/***** Eintrag-Methoden *****/




/** Liefert den angegebenen Eintrag beim Squirrel.
    Prüft, ob der Typ der gewünschte ist.
    Liefert 0, wenn's den Eintrag nicht gibt, aber defaultVorhanden.
    Throwt bei sonstigem Fehler. */
Knoten * DatenDatei::getEintragKnoten(const Str & schluessel,
				      const Version & version,
				      bool defaultVorhanden, int typ) const {

  if (!mSquirrelKnoten) {
    if (defaultVorhanden)
      return 0;
    else throw Fehler(_("%s required but not defined"),schluessel.data());
  }
  
  Knoten * ret = mSquirrelKnoten->getKind(schluessel,version,
					  defaultVorhanden);

  if (ret)
    if (typ != type_egal && ret->type() != typ)
      throw Fehler(_("Wrong type on the righthand side of %s%s="),
                   schluessel.data(), version.toString().data());
    
  return ret;
}


	

/** Gibt's den Eintrag? */
bool DatenDatei::hatEintrag(const Str & schluessel) const {
  return mSquirrelKnoten->enthaelt(schluessel);
}

/** Liefert den Eintrag, wenn er existiert, sonst null, wenn
    defaultVorhanden, sonst wird gethrowt. */
const DatenKnoten * DatenDatei::getEintrag(const Str & schluessel,
					   const Version & version,
					   bool defaultVorhanden,
					   int typ
					   /*= type_EgalDatum*/) const {
  Knoten * e = getEintragKnoten(schluessel, version, defaultVorhanden,
				type_ListenKnoten);
  if (e)
    return ((ListenKnoten *) e)->getDatum(0)->assert_datatype(typ);
  else
    return 0;
}

/** Dito für Wörter. Default für def ist "". */
Str DatenDatei::getWortEintragOhneDefault(const Str & schluessel,
					       const Version & version) const {
  return getEintrag(schluessel,version,false,type_WortDatum)->getWort();
}

Str DatenDatei::getWortEintragMitDefault(const Str & schluessel,
					      const Version & version,
					      Str def /*= Str()*/) const {
  const DatenKnoten * e = getEintrag(schluessel,version,true,type_WortDatum);
  if (e)
    return e->getWort();
  else
    return def;
}

/** Liefert den Eintrag als Zahl. */
int DatenDatei::getZahlEintragOhneDefault(const Str & schluessel,
					  const Version & version) const {
  return getEintrag(schluessel,version,false,type_ZahlDatum)->getZahl(0);
}

int DatenDatei::getZahlEintragMitDefault(const Str & schluessel,
					 const Version & version,
					 int def /*= 0*/) const {
  const DatenKnoten * e = getEintrag(schluessel,version,true,type_ZahlDatum);
  if (e) {
    return e->getZahl(0);
  } else
    return def;
}

bool intZuBool(int i) {
  switch (i) {
  case 0: return false;
  case 1: return true;
  default: throw Fehler(_("0 or 1 expected, got %d."),i);
  }
}

bool DatenDatei::getBoolEintragOhneDefault(const Str & schluessel,
					   const Version & version) const {
  return intZuBool(getZahlEintragOhneDefault(schluessel,version));
}


bool DatenDatei::getBoolEintragMitDefault(const Str & schluessel,
					  const Version & version,
					  bool def) const {
  return intZuBool(getZahlEintragMitDefault(schluessel,version,(def ? 1 : 0)));
}


/** Liefert den Eintrag als Farbe. */
Color DatenDatei::getFarbEintragOhneDefault(const Str & schluessel,
					    const Version & version) const {
  ListenKnoten * e = getListenEintrag(schluessel,version,false);
  if (e->getLaenge() != 3)
    throw Fehler("%s",_("Color (r,g,b) expected"));
            
  return Color(e->getDatum(0)->assert_datatype(type_ZahlDatum)->getZahl(),
		e->getDatum(1)->assert_datatype(type_ZahlDatum)->getZahl(),
		e->getDatum(2)->assert_datatype(type_ZahlDatum)->getZahl());
}

Color DatenDatei::getFarbEintragMitDefault(const Str & schluessel,
				  const Version & version,
				  const Color & def /*= black*/) const {
  ListenKnoten * e = getListenEintrag(schluessel,version,true);

  if (e == 0) return def;
  
  if (e->getLaenge() != 3)
    throw Fehler("%s",_("Color (r,g,b) expected"));
            
  return Color(e->getDatum(0)->assert_datatype(type_ZahlDatum)->getZahl(),
		e->getDatum(1)->assert_datatype(type_ZahlDatum)->getZahl(),
		e->getDatum(2)->assert_datatype(type_ZahlDatum)->getZahl());
}

/** Liefert einen Eintrag als Knoten */
ListenKnoten * DatenDatei::getListenEintrag(const Str & schluessel,
					    const Version & version,
					    bool defaultVorhanden) const {
  Knoten * e = getEintragKnoten(schluessel, version, defaultVorhanden,
				type_ListenKnoten);
  if (e) {
    /* Früher freute sich der Aufrufer darüber, daß er wusste, daß die
       Liste nur Wörter enthält. Es könnten noch Probleme existieren */
    ListenKnoten * ret = (ListenKnoten*) e;
    for (int i = 0; i < ret->getLaenge(); i++)
      if (ret->getKind(i)->type() != type_DatenKnoten)
        throw Fehler("%s",_("List of atomic data expected"));
    return ret;
  } else
    return 0;
}



/** Sucht einen Code beim Squirrel oder näher an der Wurzel.
    Behält den Besitz am Code/an der VarDefinition.
    Throwt bei nicht-existenz. */
Code * DatenDatei::getCode(const Str & name,
			   const Version & version, bool defaultVorhanden) {

  /* Einen Codeabschnitt sollte es eigentlich immer geben - zumindest
     wenn die Datei auf ist. Aber die sollte immer auf sein. */
  CASSERT(mSquirrelCodeKnoten);

  return (Code *) mSquirrelCodeKnoten->getDefinition(namespace_prozedur,
						     name, version,
						     defaultVorhanden);
}

VarDefinition * DatenDatei::getVarDef(const Str & name,
				      const Version & version,
				      bool defaultVorhanden) {

  /* Einen Codeabschnitt sollte es eigentlich immer geben - zumindest
     wenn die Datei auf ist. Aber die sollte immer auf sein. */
  CASSERT(mSquirrelCodeKnoten);

  return (VarDefinition *) mSquirrelCodeKnoten->getDefinition(
      namespace_variable, name, version, defaultVorhanden);
}





/***************************************************************************/





DatenDateiPush::DatenDateiPush(DatenDatei & c,
			       const Str & name, const Version & version,
                               bool verlange /*= true*/): mConf(c) {
  mMerkName = mConf.mSquirrelPosString;
  mMerkKnoten = mConf.mSquirrelKnoten;
  mMerkCodeKnoten = mConf.mSquirrelCodeKnoten;
  mConf.kletterWeiter(name,version);
  if (verlange && !mConf.existiertSquirrelKnoten())
    throw Fehler(_("Section %s does not exist."),
                 mConf.getSquirrelPosString().data());
}

DatenDateiPush::~DatenDateiPush() {
  mConf.mSquirrelPosString = mMerkName;
  mConf.mSquirrelKnoten = mMerkKnoten;
  mConf.mSquirrelCodeKnoten = mMerkCodeKnoten;
}


