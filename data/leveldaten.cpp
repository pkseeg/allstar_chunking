/***************************************************************************
                          leveldaten.cpp  -  description
                             -------------------
    begin                : Fri Jul 21 2000
    copyright            : (C) 2000 by Immi
    email                : cuyo@pcpool.mathematik.uni-freiburg.de

Modified 2001-2006,2008-2011 by the cuyo developers
Modified 2012 by Bernhard R. Link
Maintenance modifications 2012 by the cuyo developers

Modified 2011 by Bernhard R. Link
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

#include "leveldaten.h"
#include "cuyointl.h"
#include "fehler.h"
#include "pfaditerator.h"
#include "datendatei.h"
#include "knoten.h"
#include "prefsdaten.h"
#include "global.h"
#include "font.h"
#include "aufnahme.h"

#include "blop.h"

/* Provisorischerweise werden die Punktefeld-Schriften direkt von
   hier initialisiert */
#include "punktefeld.h"


#define toptime_default 50


using namespace std;


/* Globale Variable mit den Level-Daten */

LevelDaten * ld;

/** */
LevelDaten::LevelDaten(const Version & version): mLevelCache(),
  mVersion(version),
  mLevelGeladen(false),
/* Wir basteln uns ein Array, dessen Indizierung nicht mit 0 beginnt...: */
  mSorten(mSortenIntern - blopart_min_sorte),
  /* Noch keine Sorten geladen */
  mAnzFarben(0)
/* Uninitialisierte Blops entstehen sehr frueh, und die haben auch schon
   einen Bildstapel. Und der greift auf mStapelHoehe() zu. */
 // mStapelHoehe(0)
{
  for (int teil=0; teil<ldteile_anzahl; teil++) {
    mLevelConf[teil] = new DatenDatei();
    mLCGeladen[teil] = false;
  }
  ld = this;
  
  /* Auch die negativen Sorten sind noch nicht geladen: */
  for (int i = blopart_min_sorte; i < 0; i++)
    mSorten[i] = NULL;

  /* So; jetzt haben die Variablen einen gueltigen Zustand,
     und wir koennen schon mal erste Dinge laden. */
  ladLevelSummary(true,version);
}


/** */
LevelDaten::~LevelDaten() {
  entladLevel();

  for (int teil=0; teil<ldteile_anzahl; teil++)
    delete mLevelConf[teil];
}

void LevelDaten::ladLevelSummary(bool aufJedenFall, const Version & version) {

  if (aufJedenFall || (version!=mVersion)) {
  
    /* Wenn noch ein Level geladen war, dann wird der jetzt ungueltig.
       Damit nicht jemand auf die Idee kommt, den nochmal zu verwenden,
       lieber entladen. */
    entladLevel();
    
  
    mVersion = version;

    mSpielerZahl = (mVersion.enthaelt("1") ? 1 : 2);

    /* Jetzt ist erst mal nix mehr geladen. Erst wenn ladLevelSummary()
       durchgelaufen ist, ist wieder was da. */
    mLCGeladen[ldteil_summary] = false;

    /* Abkuerzung */
    DatenDatei * conf = mLevelConf[ldteil_summary];

	/* Falls schon mal was geladen wurde: Erst mal alles
	   wieder rausschmeissen. */
	conf->leeren();

	/* Jetzt ist auch der richtige Zeitpunkt zum lazy loeschen. */
	mLevelConf[ldteil_level]->leeren();
	mLevelCache = set<Str>();

	/* Im weiteren Verlauf wird irgendwann mIntLevelNamen ausgefuellt.
	   Falls da vorher noch Muell drin war, loeschen wir das. */
	mIntLevelNamen.clear();

	/* Hier findet das parsen statt. */
	mSammleLevel = false;
	conf->laden("summary.ld");

	/* Hat der Benutzer noch eine eigene Leveldatei angegeben?
	   Dann wird die jetzt auch noch geladen. */
	if (gDateiUebergeben) {

	  /* Weil die Datei noch nicht durch genSummary.pl durch ist,
	     braucht sie vielleicht den Inhalt von globals.ld.
	     Also laden wir alles, was in globals= steht.
	     Im Unterschied zu unten diesmal aber nach ldteil_summary. */
	  ListenKnoten* global = conf ->
	    getListenEintrag("globals",mVersion,false);
	  if (global) {
	    int l = global->getLaenge();
	    mSammleLevel = false;
	    for (int i=0; i<l; i++)
	      conf->laden(global->getDatum(i,type_WortDatum)->getWort());
	  }

	  /* Und wir nehmen die bisher definierten Level aus dem Knoten-Baum
	     wieder raus, damit wir keine doppelt definierten Level bekommen,
	     wenn der Benutzer den Namen eines Levels uebergeben hat, der schon
	     von main.ld included wird. (loeschAlleLevel() laesst den Level
	     Namens "Title" drin...) */
	  conf->initSquirrel();
	  conf->getSquirrelPos()->loeschAlleLevel();

	  /* Gepfuscht: Wir wollen, dass die uebergebene Datei da gesucht
	     wird, wo sie liegt und nicht bei den normalen Leveln o.ae.
	     Wenn wir den default-Pfad loeschen, wird sie zumindest als
	     erstes da gesucht. */
	  PfadIterator::loescheDefault();

	  /* Hier findet schon wieder parsen statt. */
	  mSammleLevel = true;
	  conf->laden(vergissPfad(gLevelDatei));

          mAngeordnet = false;

	} else {
	  ListenKnoten * ignored = conf->getListenEintrag("ignorelevel",mVersion,true);
	  int il = (ignored == NULL)?0:ignored->getLaenge();

	  ListenKnoten * lena = conf->getListenEintrag("level",mVersion,false);
	  int l = lena->getLaenge();
	  mIntLevelNamen.resize(l);
	  int ti = 0;
	  for (int i = 0; i < l; i++) {
	    int j;
	    Str wort = lena->getDatum(i,type_WortDatum)->getWort();
	    for (j = 0; j < il ; j++) {
	      if (wort == ignored->getDatum(j,type_WortDatum)->getWort())
	        break;
	    }
	    if (j < il)
	      continue;
	    mIntLevelNamen[ti++]=wort;
	  }
	  mIntLevelNamen.resize(ti);

	  mAngeordnet = conf->getBoolEintragMitDefault("ordered", mVersion, true);

	}

        /* Jetzt noch alles das laden, was in globals= steht.
	   Das wird aber schon nach ldteil_level geladen. */
	{
  	  ListenKnoten* global = conf ->
	    getListenEintrag("globals",mVersion,false);
	  if (global) {
	    int l = global->getLaenge();
	    mSammleLevel = false;
	    for (int i=0; i<l; i++) {
	      Str datei = global->getDatum(i,type_WortDatum)->getWort();
	      mLevelConf[ldteil_level]->laden(datei);
	      mLevelCache.insert(datei);
	    }
	  }
	}

	mLCGeladen[ldteil_summary] = true;


    /* OK, Laden war erfolgreich. */
    mLCGeladen[ldteil_summary] = true;
  }
}

void LevelDaten::ladLevelConfig() {

    /* Jetzt ist erst mal nix mehr geladen. Erst wenn ladLevelConfig()
       durchgelaufen ist, ist wieder was da. */
    mLCGeladen[ldteil_level] = false;

	CASSERT(mLCGeladen[ldteil_summary]);
	{
	  DatenDateiPush ddp(*(mLevelConf[ldteil_summary]),
			     mIntLevelName, mVersion);
	  Str datei = (gDateiUebergeben
            ? mLevelConf[ldteil_summary]->
	        getWortEintragMitDefault("filename",mVersion,
					 vergissPfad(gLevelDatei))
	    : mLevelConf[ldteil_summary]->
                getWortEintragOhneDefault("filename",mVersion));
	  if (mLevelCache.find(datei)==mLevelCache.end()) {
  	      /* Sonst braeuchten wir gar nichts zu machen */
	    mSammleLevel = false;
	    mLevelConf[ldteil_level]->laden(datei);
	    mLevelCache.insert(datei);
	  }
	}
    /* OK, Laden war erfolgreich. */
    mLCGeladen[ldteil_level] = true;
}




/** Wird waehrend des Parsens (d. h. innerhalb von ladLevel*() von
    DefKnoten aufgerufen, wenn ein neuer Level gefunden wurde. Fuegt
    den Level in die Liste der Level ein. ladLevelSummary() kann sich
    danach immernoch entscheiden, ob es die Liste wieder loescht und
    durch die "level=..."-Liste ersetzt. */
void LevelDaten::levelGefunden(Str lna) {
  if (mSammleLevel)
    mIntLevelNamen.push_back(lna);
}



/** Laed ein paar Sorten. Wird mehrfach von ladLevel() aufgerufen. */
void LevelDaten::ladSorten(const Str & ldKeyWort, int blopart) {
  ListenKnoten * picsnamen;
  picsnamen = mLevelConf[ldteil_level]->
    getListenEintrag(ldKeyWort,mVersion,true);
  if (picsnamen) {
    int neueNamen = picsnamen->getLaenge();
    int neueFarben = picsnamen->getImpliziteLaenge();
    /** Die Nummern der Sorten wurden schon beim parsen in knoten.cpp
        festgelegt. Hier tun wir unser bestes, die selben Nummern zu
	bekommen. nr ist die, die zum naechsten logischen Listeneintrag
	gehoert. */
    int nr = mLevelConf[ldteil_level]->getSquirrelPos()->
      getSortenAnfang(ldKeyWort);
    if (nr+neueFarben > max_farben_zahl)
      /* TRANSLATORS: "pics" is a programming keyword and should not be
	 translated */
      throw Fehler(_sprintf(_("Number of pics exceeds limit %d"),
			    max_farben_zahl));

    for (int namen_nr = 0; namen_nr < neueNamen; namen_nr++) {
      mSorten[nr] = new Sorte(picsnamen->getKernDatum(namen_nr,type_WortDatum)
		                ->getWort(),
			      mVersion, blopart);
      nr++;
 
      for (int i = picsnamen->getVielfachheit(namen_nr)-1; i>0; i--) {
	mSorten[nr]=mSorten[nr-1];
	nr++;
      }
    }
  }
}



/* Gibt Speicher frei */
void LevelDaten::entladLevel() {
  mLevelGeladen = false;
  
  /* Wir pruefen vor dem Freigeben nicht, mLevelGeladen = true war:
     vielleicht war aufgrund eines Fehlers irgendwas in einem
     halbgeladenen Zustand; dann wuerden wir auch gerne allen
     Speicher freigeben, der grad belegt war */
  
  /* Positive Sorten loeschen */
  for (int bnr=mAnzFarben-1; bnr>0; bnr--)
    if (bnr == 0 || mSorten[bnr]!=mSorten[bnr-1]) {
      if (mSorten[bnr])  // Koennte NULL sein falls ladefehler
        delete mSorten[bnr];
    }
  mAnzFarben = 0;

  /* Die negativen Sorten loeschen */
  for (int bnr = blopart_min_sorte; bnr < 0; bnr++)
    if (mSorten[bnr]) {
      delete mSorten[bnr];
      mSorten[bnr] = NULL;
    }
}



/** fuellt alle Daten in diesem Objekt fuer Level nr aus; throwt bei Fehler */
void LevelDaten::ladLevel(int nr) {

  if (!mLCGeladen[ldteil_summary])
    throw Fehler("%s",_("Sorry, no working level description file available."));

  /* Ggf. Speicher von altem Level freigeben */
  entladLevel();

  /* In den obersten Abschnitt der level descr springen. Wir koennten uns
     woanders befinden, wenn es irgend wann mal einen Fehler gegeben hatte. */
  mLevelConf[ldteil_summary]->initSquirrel();

  /* Nur fuer den Fall eines fruehen throws... */
  mLevelName = "";
  
  /** Fuer bessere Fehlerausgaben. */
  Str fehlerpos = "";
  
  mIntLevelName = getIntLevelName(nr);

  ladLevelConfig();

  if (!mLCGeladen[ldteil_level])
    throw Fehler("%s",_("Sorry, no working level description file available."));

  try {

    /* In den Abschnitt dieses Levels springen. (Springt automatisch
       bei } wieder raus. */
    DatenDateiPush ddp(*(mLevelConf[ldteil_level]), mIntLevelName, mVersion);

    DatenDatei * daten = mLevelConf[ldteil_level];

    // TRANSLATORS: This prepends an error message with position information
    fehlerpos = _sprintf(_("At position %s (or somewhere below): "),
			 daten->getSquirrelPos()->getDefString().data());

    /* DefKnoten des Levels abspeichern. */
    mLevelKnoten = daten->getSquirrelPos();

    /* Level-Name */
    mLevelName = _(daten->getWortEintragOhneDefault("name",mVersion).data());

    /* Level-Autor */
    mLevelAutor = _(daten->
      getWortEintragOhneDefault("author",mVersion).data());

    /* Beschreibungstext (optional) */
    mBeschreibung = _(daten->
      getWortEintragMitDefault("description", mVersion, "").data());

    /* Wie viele Steine muessen zusammen, damit sie platzen?
       (optional, da je Sorte definierbar (muss man dann aber auch tun)) */
    mPlatzAnzahlDefault = daten->getZahlEintragMitDefault
      ("numexplode", mVersion, PlatzAnzahl_undefiniert);
    mPlatzAnzahlMin = PlatzAnzahl_undefiniert;
    mPlatzAnzahlMax = PlatzAnzahl_undefiniert;
    mPlatzAnzahlAndere = false;

    /* Hintergrundfarbe... (optional; Default: weiss)
       Achtung: Die Hintergrundfarbe muss gesetzt werden, _bevor_
       Bildchen geladen werden, da es als Bonus-Farbe im XPM
       "Background" gibt... (im Moment nur fuer Explosion sinnvoll) */
    mHintergrundFarbe = daten->
      getFarbEintragMitDefault("bgcolor", mVersion, Color(255, 255, 255));

    /* Hintergrundbilchen (optional) */
    mMitHintergrundbildchen = daten->hatEintrag("bgpic");
    if (mMitHintergrundbildchen)
      mHintergrundBild.laden(daten->
			     getWortEintragOhneDefault("bgpic", mVersion));

    /* Schriftfarbe... (optional; Default: dunkelgrau) */
    mSchriftFarbe = daten->
      getFarbEintragMitDefault("textcolor",mVersion,Color(60, 60, 60));
    setSchriftFarbe(mSchriftFarbe);

    /* Hetzrandfarbe... (optional; Default: hellgrau) */
    hetzrandFarbe = daten->
      getFarbEintragMitDefault("topcolor", mVersion, Color(200, 200, 200));

    /* Hetzrandgeschwindigkeit (optional) */
    hetzrandZeit = daten->
      getZahlEintragMitDefault("toptime", mVersion, toptime_default);
    if (hetzrandZeit < 1)
      /* TRANSLATORS: "toptime" is a programming keyword which should not
	 be translated. */
      throw Fehler("%s",_("toptime must be positive"));

    /* Hetzrandbildchen (optional) */
    mMitHetzbildchen = daten->hatEintrag("toppic");
    if (mMitHetzbildchen) {
      mHetzBild.laden(daten->getWortEintragOhneDefault("toppic",mVersion));
  	
      /* Hetzrandueberlapp (optional) */
      mHetzrandUeberlapp = daten->
	getZahlEintragMitDefault("topoverlap", mVersion, mHetzBild.getHoehe());
    } else
      mHetzrandUeberlapp = 0;

    mHetzrandStop = daten->getZahlEintragMitDefault("topstop", mVersion, 0);

    /* Gras nur bei Kettenreaktion? (optional) */
    mGrasBeiKettenreaktion = daten->
      getBoolEintragMitDefault("chaingrass", mVersion, false);

    /* Senkrecht spiegeln? (optional) */
    mSpiegeln = daten->getBoolEintragMitDefault("mirror",mVersion,false);

    /* Neuer fall wird (horizontal) zufaellig plaziert? (optional) */
    mFallPosZufaellig = daten->
      getBoolEintragMitDefault("randomfallpos", mVersion, false);

    /* Andere Nachbarschaft? (optional) */
    mNachbarschaft = daten->
      getZahlEintragMitDefault("neighbours", mVersion, nachbarschaft_normal);
    if (mNachbarschaft < 0 || mNachbarschaft > nachbarschaft_letzte)
      /* TRANSLATORS: "neighbours" is a programming keyword which
	 should not be translated. */
      throw Fehler("%s",_("neighbours out of range"));
    /* Sechseck-Raster? */
    mSechseck =
      mNachbarschaft == nachbarschaft_6 ||
      mNachbarschaft == nachbarschaft_6_schraeg ||
      mNachbarschaft == nachbarschaft_6_3d;


    /* Zufaellige Graue? (optional) */
    mZufallsGraue = daten->
      getZahlEintragMitDefault("randomgreys", mVersion, zufallsgraue_keine);
  	  	
    /* Wo sind welche Grasbildchen am Anfang? */
    mAnfangsZeilen = daten->getListenEintrag("startdist",mVersion,false);


    /***** Noch ein paar einzelne Bilder laden *****/
    
    /* Explosion laden. Das darf erst nach dem Laden der Hintergrundfarbe
       passieren. */
    mExplosionBild.laden(daten->
			 getWortEintragMitDefault("explosionpic",mVersion,
						  "explosion.xpm"));

    /* Schriftfarbe der Punkte neu setzen. (Provisorisch) */
    Punktefeld::init();

    mDistKeyLen = 0;
    

    /***** Blops laden *****/

    /* Falls waehrend des Ladevorgangs ein Fehler auftritt, muessen
       alle noch ungeladenen Sorten NULL sein.
       Eigentlich waere es ja am einfachsten, wenn mAnzFarben immer
       die bisherige Anzahl der geladenen Sorten angibt. Das geht
       aber nicht, da ladSorten() die Sorten evtl. nicht der Reihe
       nach laedt. Das liegt daran, dass die Sortennummern nicht
       in der Reihenfolge vergeben werden, wie hier ladSorten auf-
       gerufen wird, sondern in der Reihenfolge, wie's in der
       ld-Datei steht. */
    mAnzFarben = daten->getSquirrelPos()->getSortenAnzahl();
    for (int i = 0; i < mAnzFarben; i++)
      mSorten[i] = NULL;

    /* Wie viele Bilder malt ein Blop in einem Schritt hoechstens?
       Erst mal keine. Die Sorten erhoehen diese Variablen selbst, wenn
       man sie laedt. */
    mStapelHoehe = 0;
    mNachbarStapelHoehe = 0;

    ladSorten("pics",blopart_farbe);
    ladSorten("startpic",blopart_gras);
    ladSorten("greypic",blopart_grau);

    /* Leer-Bildchen (optional) */
    mMitLeerBildchen = daten->hatEintrag("emptypic");
    /* Auch, wenn es kein Leer-Bildchen gibt, soll es geladen werden;
       dann liefert getEintrag() "" zurueck, und Sorte::laden() weiss,
       dass es nur alle Werte auf Defaults setzen soll. */
    mSorten[blopart_keins] = new Sorte(daten->
        getWortEintragMitDefault("emptypic", mVersion, ""),
      mVersion, blopart_keins);

    /* Globaler Code (optional) */
    mSorten[blopart_global] = new Sorte("global", mVersion, blopart_global);
    mSorten[blopart_semiglobal] = new Sorte("semiglobal", mVersion,
					    blopart_semiglobal);
    
    
    /* Ok, alle Sorten geladen. Wenn wir noch mNachbarStapelhoehe zu
       mStapelHoehe addieren, stimmt diese Variable.
       Ab jetzt duerfen also Blops erzeugt werden
       (wenn man moechte). */
    //print_to_stderr(_sprintf("mStapelHoehe = %d, nsh = %d\n", mStapelHoehe, mNachbarStapelHoehe));    
    mStapelHoehe += mNachbarStapelHoehe;

    /* Nachbearbeitungen */
    if (mDistKeyLen==0)
      mDistKeyLen=1;

    /* Entstehungswahrscheinlichkeiten */
    mKeineGrauenW = daten->getZahlEintragMitDefault("nogreyprob", mVersion, 0);
    if (mKeineGrauenW<0)
      /* TRANSLATORS: "nogreyprob" is a programming keyword which
	 should not be translated. */
      throw Fehler("%s",_("nogreyprob must not be negative"));
    for (int i=0; i<anzahl_wv; i++) {
      mVerteilungSumme[i]=0;
      for (int j=blopart_min_sorte; j<mAnzFarben; j++)
	mVerteilungSumme[i] += mSorten[j]->getVerteilung(i);
    }
    if (mVerteilungSumme[wv_farbe]==0)
      /* TRANSLATORS: Don't worry about the %s, it will be a programming
	 keyword. */
      throw Fehler(_("At least one %s must be positive."),
		   cVerteilungsNamen[wv_farbe]);
    if (mVerteilungSumme[wv_grau]+mKeineGrauenW==0)
      /* TRANSLATORS: "nogreyprob" is a programming keyword which
	 should not be translated.
         Also, don't worry about the %s, it will be a programming keyword. */
      throw Fehler(_("nogreyprob or at least one %s must be positive."),
		   cVerteilungsNamen[wv_grau]);


   /* Musik (optional) */
   mMusik = daten->getWortEintragMitDefault("music", mVersion, "");

  /***** KI-Player-Bewertungen *****/
    mKINHoehe = daten->getZahlEintragMitDefault("aiu_height", mVersion, 10);
    mKINAnFarbe = daten->
      getZahlEintragMitDefault("aiu_color", mVersion, 10 * mAnzFarben);
    mKINAnGras = daten->getZahlEintragMitDefault("aiu_grass", mVersion, 20);
    mKINAnGrau = daten->getZahlEintragMitDefault("aiu_grey", mVersion, 10);
    mKINZweiUeber = daten->
      getZahlEintragMitDefault("aiu_two_above", mVersion, mKINAnFarbe / 2);
    mKINEinfarbigSenkrecht = daten->
      getZahlEintragMitDefault("aiu_monochromic_vertical", mVersion,
			       mKINAnFarbe);

  } catch (Fehler f) {
    Str fs = fehlerpos;
    /*if (!mLevelConf[ldteil_level]->getSquirrelPosString().isEmpty())
      fs += ", Section " + mLevelConf[ldteil_level]->getSquirrelPosString();
    fs += ":\n" + f.getText() + "\n";*/
    fs += f.getText() + "\n";
    
    if (!mLevelName.isEmpty())
      // TRANSLATORS: This postpends an error message with some information
      fs += _sprintf(_(" (Level \"%s\")\n"), mLevelName.data());
    throw Fehler(fs);
  }
  
  mLevelGeladen = true;
  mLevelNummer = nr;
} // ladLevel


void LevelDaten::erneuerLevel(int nr) {
  CASSERT(mLevelGeladen);
  CASSERT(nr == mLevelNummer);
  setSchriftFarbe(mSchriftFarbe);
}





/** Sollte am Anfang des Levels aufgerufen werden; kuemmert sich
    um den Global-Blop */
void LevelDaten::startLevel() const {
  Blop::gGlobalBlop = Blop(blopart_global);
  // Damit Code ausgefuehrt werden darf:
  Blop::gGlobalBlop.setBesitzer(0,ort_absolut(absort_global)); 
}


/** Sollte einmal pro Spielschritt aufgerufen werden (bevor
    Spielfeld::spielSchritt() aufgerufen wird). Kuemmert sich 
    um den Global-Blop */
void LevelDaten::spielSchritt() const {
  Blop::gGlobalBlop.animiere();
}


/** Hilfsfunktion fuer getLevelAnz und getIntLevelName. Sucht nach dem
    ersten "." in na. */
int getPunktPos(Str na) {
  for (int i = 0; i < (int) na.length(); i++)
    if (na[i] == '.') return i;
  return -1;
}


/** Liefert zurueck, wie viele Level es gibt. */
int LevelDaten::getLevelAnz() const {
  if (!mLevelConf[ldteil_summary])
    throw Fehler("%s",_("Sorry, no working level description file available."));
  
  return mIntLevelNamen.size();
}

/** Lifert zurück, ob die Level der Reihe nach gespielt werden müssen */
int LevelDaten::getAngeordnet() const {
  if (!mLevelConf[ldteil_summary])
    throw Fehler("%s",_("Sorry, no working level description file available."));

  return mAngeordnet;
}


/** Liefert den internen Namen von Level nr zurueck. */
Str LevelDaten::getIntLevelName(int nr) const {
  if (nr == level_titel)
    return Str("Title");

  int pos = nr - 1;
  Str s = mIntLevelNamen[pos];
  int pp = getPunktPos(s);
  if (pp == -1)
    return s;
  else
    return s.left(pp);
  
}

/** Liefert den Namen von Level nr zurueck. Liefert "???" bei Fehler. */
Str LevelDaten::getLevelName(int nr) const {
  try {
    DatenDateiPush ddp(*(mLevelConf[ldteil_summary]),
		       getIntLevelName(nr), mVersion);

    return _(mLevelConf[ldteil_summary]->
      getWortEintragOhneDefault("name", mVersion).data());

  } catch (Fehler f) {
    // TRANSLATORS: This indicates an unknown value
    return _("???");
  }
}


/** Liefert die Nummer des Levels mit dem angegebenen Namen zurueck,
    oder 0, wenn der Level nicht existiert. */
int LevelDaten::getLevelNr(Str na) const {
  /** Alles noch seeehr ineffektiv... */
  int anz = getLevelAnz();
  for (int i = 1; i <= anz; i++)
    if (na == getIntLevelName(i))
      return i;
  return 0;
}


/** Wenn eine Sorte ihre PlatzAnzahl rausgefunden hat, teilt sie uns das mit */
void LevelDaten::neue_PlatzAnzahl(int PlatzAnzahl) {
  if (mPlatzAnzahlMin==PlatzAnzahl_undefiniert) {
    CASSERT(mPlatzAnzahlMax==PlatzAnzahl_undefiniert);
    CASSERT(!mPlatzAnzahlAndere);
    mPlatzAnzahlMin = PlatzAnzahl;
    mPlatzAnzahlMax = PlatzAnzahl;
  }
  else {
    CASSERT(mPlatzAnzahlMax!=PlatzAnzahl_undefiniert);
    mPlatzAnzahlAndere = mPlatzAnzahlAndere ||
      ((PlatzAnzahl!=mPlatzAnzahlMin) && (PlatzAnzahl!=mPlatzAnzahlMax) &&
      (mPlatzAnzahlMin!=mPlatzAnzahlMax));
      /* Warum hat C eigentlich kein "||=" ? */
    if (mPlatzAnzahlMin > PlatzAnzahl)
      mPlatzAnzahlMin = PlatzAnzahl;
    if (mPlatzAnzahlMax < PlatzAnzahl)
      mPlatzAnzahlMax = PlatzAnzahl;
  }
}


int LevelDaten::zufallsSorte(int wv) {
  CASSERT(mVerteilungSumme[wv]);
  int nummer = Aufnahme::rnd(mVerteilungSumme[wv]);
  int i=blopart_min_sorte;
  for (; nummer>=0; i++)
    nummer-=mSorten[i]->getVerteilung(wv);
  /* Jetzt ist i die erste Sorte, die zu weit ist. */
  return i-1;
}



int LevelDaten::liesDistKey(const Str & key) {
  if (key=="")
    return distkey_undef;

  if (mDistKeyLen==0)
    mDistKeyLen=key.length();
  else
    if (mDistKeyLen!=key.length())
      /* TRANSLATORS: "distkey" is a programming keyword which should not
	 be translated */
      throw Fehler(_("distkey \"%s\" does not have length %d as others do."),
		   key.data(), mDistKeyLen);

  switch (key[0]) {
  case '-': return distkey_grau;
  case '+': return distkey_farbe;
  case '*': return distkey_gras;
  case '.': return distkey_leer;
  default:
    bool anfang = true;
    int n=0;
    for (int i=0; i<key.length(); i++) {
      bool immernochanfang = false;
      n*=62;
      char c=key[i];
      if (c>='0' && c<='9')
	n+=c-'0';
      else
	if (c>='A' && c<='Z')
	  n+=c-'A'+10;
	else
	  if (c>='a' && c<='z')
	    n+=c-'a'+36;
	  else
	    if (anfang && c==' ')
	      immernochanfang = true;
	    else
	      /* TRANSLATORS: "startdist" and "distkey" are programming keyword
		 which should not be translated */
	      throw Fehler(_("Illegal character \"%c\" in startdist or distkey %s"),
			   c,key.data());
      anfang = immernochanfang;
    }
    if (anfang)
      /* TRANSLATORS: "startdist" and "distkey" are programming keyword
	 which should not be translated */
      throw Fehler("%s",_("All-spaces startdist entry or distkey is not allowed."));
    return n;
  }
}



const Version & LevelDaten::getVersion() const {
  return mVersion;
}



/** Setzt AutoColor::gGame */
void LevelDaten::setSchriftFarbe(Color f) {
  /* Changing AutoColor::gGame automatically recolors the game font
     and some other pictures */
  AutoColor::gGame = f;
}
