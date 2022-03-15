/***************************************************************************
                        version.h  -  description
                             -------------------
    begin                : Sun Mar 26 2006
    copyright            : (C) 2006 by Mark Weyer
    email                : cuyo-devel@nongnu.org

Modified 2006,2008-2011 by the cuyo developers

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/



#include "fehler.h"
#include "version.h"
#include "cuyointl.h"


#define anz_packs 7
const char * packs_merkmale[anz_packs+1] = {
  "main", "all", "game", "extreme", "nofx", "weird", "contrib", 0};
const char * packs_namen[anz_packs] = {
  // TRANSLATORS: Name of a level track
  N_("Standard"),
  // TRANSLATORS: Name of a level track
  N_("All levels"),
  // TRANSLATORS: Name of a level track
  N_("Games"),
  // TRANSLATORS: Name of a level track
  N_("Extremes"),
  // TRANSLATORS: Name of a level track
  N_("No FX"),
  // TRANSLATORS: Name of a level track
  N_("Weird"),
  // TRANSLATORS: Name of a level track
  N_("Contributions")};
const char * packs_infos[anz_packs] = {
  N_("A selection of levels for the common taste"),
  N_("All official levels"),
  N_("Levels simulating board games or other computer games"),
  N_("Levels setting records such as `fastest dropping ceiling'"),
  N_("Plain levels without any disturbing tweaks to the rules"),
  N_("Levels interpreting the general theme of cuyo more broadly"),
  N_("Levels donated and maintained by the community")
};
Dimension Version::gLevelpack = Dimension();



#define anz_schwer 3
const char * schwer_merkmale[anz_schwer+1] = {"easy", "", "hard"};
const char * schwer_namen[anz_schwer+1] = {
  // TRANSLATORS: Name of a difficulty setting
  N_("Easy"),
  // TRANSLATORS: Name of a difficulty setting
  N_("Normal"),
  // TRANSLATORS: Name of a difficulty setting
  N_("Hard")};
const char * schwer_infos[anz_schwer+1] = {
  "",
  "",
  ""
};
Dimension Version::gSchwierig = Dimension();



/*
   ausschoepfend sollte man möglichst wenig verwenden,
   da später ständig eine vollständige Suche
   über den davon aufgespannten Raum stattfindet.

   ausschliessend ist davon nicht betroffen.
   Also im Zweifelsfall immer eine Default-Version erfinden.
*/

const char * sprachen[3] = {"de","fr",0};
const char * spielerzahlen[3] = {"1","2",0};

const char * const * ausschliessend[anzahl_dima+1] = {
  schwer_merkmale,
  sprachen,
  0
};

const char * const * ausschoepfend[anzahl_dim-anzahl_dima+1] = {
  spielerzahlen,
  packs_merkmale,
  0
};




using namespace std;


set<Str> vereinigung(const set<Str> & s1,const set<Str> & s2) {
  set<Str> v = s1;
  set<Str>::const_iterator i = s2.begin(),
                                e = s2.end();
  for (; i!=e; ++i)
    v.insert(*i);
  return v;
}

set<Str> durchschnitt(const set<Str> & s1,const set<Str> & s2) {
  set<Str> d = set<Str>();
  set<Str>::const_iterator i  = s1.begin(),
                                e1 = s1.end(),
                                e2 = s2.end();
  for (; i!=e1; ++i)
    if (s2.find(*i)!=e2)
      d.insert(*i);
  return d;
}

set<Str> differenz(const set<Str> & s1,const set<Str> & s2) {
  set<Str> d = set<Str>();
  set<Str>::const_iterator i  = s1.begin(),
                                e1 = s1.end(),
                                e2 = s2.end();
  for (; i!=e1; ++i)
    if (s2.find(*i)==e2)
      d.insert(*i);
  return d;
}

bool legal(const set<Str> & s) {
  /* Vorbedingung: s enthält "" nicht. */

  set<set<Str> >::const_iterator i = Version::gAusschliessend.begin(),
                                      e = Version::gAusschliessend.end();
  for (; i!=e; ++i)
    if (durchschnitt(s,*i).size()>1)
      return false;

  i = Version::gAusschoepfend.begin();
  e = Version::gAusschoepfend.end();
  for (; i!=e; ++i)
    if (durchschnitt(s,*i).size()>1)
      return false;

  return true;
}

Str setToString(const set<Str> & s) {
  Str ret = "";

  if (s.size()>0) {
    ret += "[";
    set<Str>::const_iterator i = s.begin(),
                                  e = s.end();
    CASSERT(i!=e);
    ret += *i;
    ++i;
    for (; i!=e; ++i)
      ret += "," + *i;
    ret += "]";
  }

  return ret;
}

set<Str> stringsetAusCharstern2(const char * const * const daten) {
  set<Str> ret = set<Str>();
  for (const char * const * i = daten; *i; i++)
    if (**i)   // leeren String rauswerfen
      ret.insert(Str(*i));
  return ret;
}

set<set<Str> > stringset2AusCharstern3
    (const char * const * const * const daten) {
  set<set<Str> > ret = set<set<Str> >();
  for (const char * const * const * i = daten; *i; i++)
    ret.insert(stringsetAusCharstern2(*i));
  return ret;
}


/* Testet, ob version, erweitert um alle Werte für alle Dimensionen,
   die ab (einschließlich) iausschoepfend (was ein Iterator in
   gAusschoepfend ist) kommen, unterstützt wird.
   schluessel ist der Name der versionierten Definition,
   für die Fehlermeldung, die ausgegeben wird, wenn eine Erweiterung fehlt. */
void testeAusschoepfend(const Str & schluessel,
			const set<set<Str> > & versionen,
			set<Str> & version,
			set<set<Str> >::const_iterator iausschoepfend) {
  if (iausschoepfend==Version::gAusschoepfend.end()) {
    set<set<Str> >::const_iterator i = versionen.begin(),
                                        e = versionen.end();
    bool gefunden = false;
    for (; !gefunden && i!=e; ++i)
      gefunden = differenz(*i,version).size()==0;    /* Teilmengentest */
    if (!gefunden)
      /* TRANSLATORS: The composition "%s%s" is programming specific and
	 should not be translated. */
      throw Fehler(_("%s lacks version %s%s"), schluessel.data(),
		   schluessel.data(), setToString(version).data());
  }
  else {
    set<Str>::const_iterator i = iausschoepfend->begin(),
                                  e = iausschoepfend->end();
    ++iausschoepfend;
    for (; i!=e; ++i) {
      version.insert(*i);
      testeAusschoepfend(schluessel,versionen,version,iausschoepfend);
      version.erase(*i);
    }
  }
}


/* Wie testeAusschoepfend(), aber statt Fehlermeldung wird false zurückgegeben,
   wenn eine Version fehlt. Außerdem werden nur Erweiterungen betrachtet,
   die mit version verträglich sind, und die echte Erweiterungen von
   basisversion sind.
   Anwendung: eine Version ist redundant, wenn sie ohnehin nie genommen
   würde, weil jede echte Erweiterung direkt unterstützt wird. */
bool testeAusschoepfendRedundant(const set<Str> & basisversion,
				 const set<set<Str> > & versionen,
				 set<Str> & version,
				 set<set<Str> >::const_iterator
  				   iausschoepfend) {
  bool ret;
  if (iausschoepfend==Version::gAusschoepfend.end()) {
    ret = false;
    set<set<Str> >::const_iterator i = versionen.begin(),
                                        e = versionen.end();
    for (; !ret && i!=e; ++i)
      ret = differenz(*i,version).size()==0
	&& differenz(*i,basisversion).size()!=0;
  }
  else {
    if (durchschnitt(*iausschoepfend,version).size()==0) {
        /* So ein Merkmal ist noch nicht drin*/

      ret = true;
      set<Str>::const_iterator i = iausschoepfend->begin(),
                                    e = iausschoepfend->end();
      ++iausschoepfend;
      for (; ret && i!=e; ++i) {
        version.insert(*i);
        ret = testeAusschoepfendRedundant(basisversion,versionen,
					  version,iausschoepfend);
        version.erase(*i);
      }
    }
    else {
      ++iausschoepfend;
      ret = testeAusschoepfendRedundant(basisversion,versionen,
					version,iausschoepfend);
    }
  }
  return ret;
}



Dimension::Dimension() : mInitialized(false) {
}

void Dimension::init(int g,
		     const char * const * const m,
		     const char * const * const n,
		     const char * const * const i) {
  mGroesse=g;
  for (int j=0; j<mGroesse; j++) {
    mMerkmale.push_back(m[j]);
    mProsaNamen.push_back(_(n[j]));
    mErklaerungen.push_back(_(i[j]));
  }
  mInitialized=true;
}

int Dimension::suchMerkmal(const Str & merkmal) {
  if (!mInitialized)
    throw(iFehler("%s",_("Uninitialized dimension")));
  for (int i=0; i<mGroesse; i++)
    if (mMerkmale[i]==merkmal)
      return i;
  return -1;
}



Version::Version() : mMerkmale() {}

void Version::nochEinMerkmal(const Str & merkmal) {
  CASSERT(merkmal!="");
  mMerkmale.insert(merkmal);
}

bool Version::enthaelt(const Str & merkmal) const {
  return mMerkmale.find(merkmal)!=mMerkmale.end();
}

bool Version::speziellerAls(const Version & andere) const {
  set<Str>::const_iterator i=andere.mMerkmale.begin(),
                                e=andere.mMerkmale.end(),
                                ende=mMerkmale.end();
  for (; i!=e; ++i)
    if (mMerkmale.find(*i)==ende)
      return false;
  return true;
}

bool Version::operator < (const Version & v2) const {
  return mMerkmale < v2.mMerkmale;
}

bool Version::operator == (const Version & v2) const {
  return mMerkmale == v2.mMerkmale;
}

bool Version::operator != (const Version & v2) const {
  return !(mMerkmale == v2.mMerkmale);
}

Str Version::toString() const {
  return setToString(mMerkmale);
}

Str Version::extractMerkmal(int dim, const Str & def) {
  Str ret;
  /* Weil mMerkmale sowieso "" nicht enthält,
     braucht es uns auch nicht zu kümmern,
     wenn es in auspraegungen vorkommt. */
  set<Str> merkmale = durchschnitt(mMerkmale,auspraegungen(dim));
  switch (merkmale.size()) {
  case 0:
    ret = def;
    break;
  case 1:
    {
      ret = *(merkmale.begin());
      mMerkmale.erase(mMerkmale.find(ret));
    }
    break;
  default:
    throw setToString(merkmale);
    break;
  }
  return ret;
}

int Version::extractMerkmal(const Dimension & dim, int def) {
  bool gefunden=false;
  int i=0;
  for (; !gefunden && i<dim.mGroesse; i++)
    gefunden = mMerkmale.find(dim.mMerkmale[i]) != mMerkmale.end();
  int ret = i-1;
  for (; i<dim.mGroesse; i++)
    if (mMerkmale.find(dim.mMerkmale[i]) != mMerkmale.end())
      throw (dim.mMerkmale[ret] + dim.mMerkmale[i]);

  if (gefunden)
    return ret;
  else
    return def;
}


void Version::init() {
  gLevelpack.init(anz_packs,packs_merkmale,packs_namen,packs_infos);
  gSchwierig.init(anz_schwer,schwer_merkmale,schwer_namen,schwer_infos);
}

const set<Str> Version::auspraegungen(int dim) {
  set<Str> ret = set<Str>();
  CASSERT((dim>=0) && (dim<anzahl_dim));
  if (dim<anzahl_dima) {
    ret = stringsetAusCharstern2(ausschliessend[dim]);
    ret.insert("");
  }
  else
    ret = stringsetAusCharstern2(ausschoepfend[dim-anzahl_dima]);
  return ret;
}

set<set<Str> > Version::gAusschliessend
  = stringset2AusCharstern3(ausschliessend);

set<set<Str> > Version::gAusschoepfend
  = stringset2AusCharstern3(ausschoepfend);




VersionMapIntern::IndexIntern::IndexIntern(VersionMapIntern & versionen) :
    eigner(&versionen.mVerzeichnis), intern1(versionen.mVerzeichnis.begin()) {
  if (!ende()) {
    intern2 = intern1->second.begin();
    CASSERT(intern2 != intern1->second.end());
  }
}

void VersionMapIntern::IndexIntern::operator++() {
  if (!ende()) {
    ++intern2;
    if (intern2 == intern1->second.end()) {
      ++intern1;
      if (!ende()) {
        intern2 = intern1->second.begin();
        CASSERT(intern2 != intern1->second.end());
      }
    }
  }
}

const Str & VersionMapIntern::IndexIntern::schluessel() const {
  CASSERT(!ende());
  return intern1->first;
}

void* VersionMapIntern::IndexIntern::datumIntern() const {
  CASSERT(!ende());
  return intern2->second;
}



VersionMapIntern::constIndexIntern::constIndexIntern
      (const VersionMapIntern & versionen) :
    eigner(&versionen.mVerzeichnis), intern1(versionen.mVerzeichnis.begin()) {
  if (!ende()) {
    intern2 = intern1->second.begin();
    CASSERT(intern2 != intern1->second.end());
  }
}

void VersionMapIntern::constIndexIntern::operator++() {
  if (!ende()) {
    ++intern2;
    if (intern2 == intern1->second.end()) {
      ++intern1;
      if (!ende()) {
        intern2 = intern1->second.begin();
        CASSERT(intern2 != intern1->second.end());
      }
    }
  }
}

const Str & VersionMapIntern::constIndexIntern::schluessel() const {
  CASSERT(!ende());
  return intern1->first;
}

const Version & VersionMapIntern::constIndexIntern::version() const {
  CASSERT(!ende());
  return intern2->first;
}

const void* VersionMapIntern::constIndexIntern::datumIntern() const {
  CASSERT(!ende());
  return intern2->second;
}



VersionMapIntern::VersionMapIntern() : mGeprueft(), mGut(), mVerzeichnis() {}

void VersionMapIntern::neuerEintragIntern(const Str & schluessel,
					  const Version & version,
					  void* inhalt) {
  if (geprueft(schluessel))
    /* TRANSLATORS: The composition "%s%s" is programming specific and
       should not be translated. */
    throw Fehler(_("%s was already accessed, new version %s%s is not allowed"),
		 schluessel.data(),
		 schluessel.data(), version.toString().data());

  map<Str,map<Version,void*> >::iterator i
    = mVerzeichnis.find(schluessel);
  if (i==mVerzeichnis.end())
    i = mVerzeichnis.insert(
        pair<Str,map<Version,void*> >(
	  schluessel, map<Version,void*>())).first;
  i->second.insert(pair<Version,void*>(version,inhalt));
}

void VersionMapIntern::loescheEintrag(IndexIntern & i) {
  CASSERT(!geprueft(i.intern1->first));
  i.intern1->second.erase(i.intern2);
  if (i.intern1->second.size()==0)
    mVerzeichnis.erase(i.intern1);
}

bool VersionMapIntern::enthaelt(const Str & schluessel) const {
  return mVerzeichnis.find(schluessel) != mVerzeichnis.end();
}

bool VersionMapIntern::enthaelt(const Str & schluessel,
                                const Version & version) const {
  map<Str,map<Version,void*> >::const_iterator i
    = mVerzeichnis.find(schluessel);
  if (i==mVerzeichnis.end())
    return false;
  else
    return i->second.find(version) != i->second.end();
}

void* VersionMapIntern::BestapproximierendeIntern
     (const Str & schluessel, const Version & version,
      bool defaultVorhanden) const {

  assertWohlgeformt(schluessel, defaultVorhanden);

  map<Str,map<Version,void*> >::const_iterator versionen
    = mVerzeichnis.find(schluessel);
  if (versionen == mVerzeichnis.end()) {
    CASSERT(defaultVorhanden);
    return 0;
  }

  /* Erstmal einen suchen, der überhaupt passt. */
  map<Version,void*>::const_iterator i = versionen->second.begin(),
                                     e = versionen->second.end();
  while (i==e ? false : !version.speziellerAls(i->first))
    ++i;
  if (i==e) {
    CASSERT(defaultVorhanden);
    return 0;
  }

  /* Und dann verbessern. */
  map<Version,void*>::const_iterator best = i;
  for (++i; i!=e; ++i)
    if (version.speziellerAls(i->first) &&
        i->first.speziellerAls(best->first))
      best = i;
  return best->second;
}

bool VersionMapIntern::geprueft(const Str & schluessel) const {
  return (mGeprueft.find(schluessel)!=mGeprueft.end());
}

void VersionMapIntern::assertWohlgeformt(const Str & schluessel,
				   bool defaultVorhanden) const {
  if (geprueft(schluessel)) {
    if (mGut.find(schluessel)==mGut.end())
      throw Fehler(_("Previous problem with %s still persists"),
		   schluessel.data());
    return;
  }

  /* Schonmal den Prüfstempel verteilen, vielleicht vergessen wir's sonst.
     Insbesondere, wenn die Prüfung nicht bestanden wird (Der Stempel sagt
     nicht "bestanden", sondern wirklich nur "geprüft".). */
  mGeprueft.insert(schluessel);


  /* Jetzt Ballast abwerfen, damit es übersichtlicher wird.
     Dabei testen wir gleich auf Legalität. */
  set<set<Str> > versionen = set<set<Str> >();
  if (enthaelt(schluessel)) {
    map<Str,map<Version,void*> >::const_iterator i0
      = mVerzeichnis.find(schluessel);
    map<Version,void*>::const_iterator i = i0->second.begin(),
                                       e = i0->second.end();
    for (; i!=e; ++i) {
      if (!legal(i->first.mMerkmale))
	/* TRANSLATORS: The composition "%s%s" is programming specific and
	   should not be translated. */
	throw Fehler(_("Illegal version %s%s"),
		     schluessel.data(), i->first.toString().data());
      versionen.insert(i->first.mMerkmale);
    }
  }
  else if (!defaultVorhanden)
    throw Fehler(_("%s required but not defined"),schluessel.data());

  /* Jetzt kommt der Test, ob Versionen immer eindeutig sind. */

  set<set<Str> >::const_iterator i1 = versionen.begin(),
                                      i2,
                                      e  = versionen.end();
  for (; i1!=e; ++i1)
    for (i2 = i1, ++i2; i2!=e; ++i2) {
      set<Str> v = vereinigung(*i1,*i2);
      if (legal(v))
	if (versionen.find(v)==e)
	  /* TRANSLATORS: The composition "%s%s" is programming specific and
	     should not be translated. */
	  throw Fehler(_("%s%s not uniquely defined"),
		       schluessel.data(), setToString(v).data());
    }

  /* Und jetzt der Redundanzcheck. */
  set<Str> leer = set<Str>();
  for (i1 = versionen.begin(); i1!=e; ++i1) {
    set<Str> version = *i1;
    if (testeAusschoepfendRedundant(*i1,versionen,leer,
				    Version::gAusschoepfend.begin()))
      /* TRANSLATORS: The composition "%s%s" is programming specific and
	 should not be translated. */
      throw Fehler(_("%s%s eclipsed by more specialized versions"),
		   schluessel.data(), setToString(version).data());
  }

  /* Und schließlich der Test, ob auch alle vorhanden sind. */
  if (defaultVorhanden)
    versionen.insert(set<Str>());
  testeAusschoepfend(schluessel, versionen, leer,
		     Version::gAusschoepfend.begin());

  mGut.insert(schluessel);
}

