/***************************************************************************
                          cuyo.cpp  -  description
                             -------------------
    begin                : Mit Jul 12 22:54:51 MEST 2000
    copyright            : (C) 2000 by Immi
    email                : cuyo@pcpool.mathematik.uni-freiburg.de

Modified 2001-2008,2010,2011 by the cuyo developers
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

/* 1 bedeutet, dass manche Signale abgefangen werden, um die Log-Datei
   abzuspeichern. */
#define signale_fangen 0



#include <cstdlib>
#include <cstdio>

#if signale_fangen
#include <csignal>
#endif

#include <SDL.h>

#include "cuyointl.h"
#include "sound.h"
#include "aufnahme.h"
#include "prefsdaten.h"
#include "kiplayer.h"
#include "spielfeld.h"
#include "fehler.h"
#include "global.h"
#include "sdltools.h"

#include "ui.h"

#include "ui2cuyo.h"
#include "cuyo.h"

/* # Zeitschritte, die nach der Zeitbonus-Animation noch gewartet wird */
#define nachbonus_wartezeit 50




namespace Cuyo {

/*************************** Private Variablen **************************/
/* (stehen nicht in der .h-Datei) */

/***** Debug-Variablen *****/
/* Es gibt außerdem noch die globale Variable gDebug; */
bool mEinzelschritt;
bool mZeitlupe;
int mZaehlerZeitlupe;
bool mRueberReihenTest;
bool mAbspielen;
bool mSchnellvorlauf;



/** Das Bild, das angezeigt wird, während das Spiel auf Pause steht. */
Bilddatei * mPauseBild;


/** Die beiden Spielfelder... */
Spielfeld* mSpielfeld[max_spielerzahl];

/** Computer-Spieler. */
KIPlayer * mKI;



/***** Zustandsvariablen *****/

/** global-Modus (Spielerübergreifend). */
enum global_modus {
  gmodus_kein_spiel,
  gmodus_spiel_start, // wie gmodus_spiel, aber Text muss noch gelöscht werden
  gmodus_spiel,
  /* die folgenden beiden Modi bedeuten beide, dass das 
     Spiel (der Level) zu Ende geht, aber dass wir noch warten, bis
     die letzten Animationen fertig abgelaufen sind */
  gmodus_warte_verloren,   // ... weil ein Spieler tot ist
  gmodus_warte_gewonnen, // ... weil der Level fertig ist
  gmodus_bonus_animation, // Zeit-Bonus bekommen...
  gmodus_bonus_warte, 	  // Nach dem Zeit-Bonus noch ein bisschen warten
  gmodus_ende_warte  // Kurz vor gmodus_kein_spiel: Benutzer darf noch
              // seine Punkte bewundern. Wenn er eine Taste drueckt, geht's
	      // Zurueck ins Menue.
} mGModus;


/** True, wenn das Spiel auf Pause steht */
bool mPause;


/** true, wenn das Spiel grad nicht weitergehen soll, sondern auf einen
    Tastendruck gewartet wird. */
bool mWarteAufTaste;
/* Wenn > 0, wird höchstens so lange auf Taste gewartet */
int mWarteTimeout;

/** Hier wird reingespeichert,
    welche Version (zuletzt) auf der Kommandozeile stand.
    Achtung! Das passiert schon vor Cuyo::init(), also vor der
    Lebenszeit von Cuyo, wenn Cuyo eine hätte. */
Version mKommandoZeilenVersion;

/** Die Versionen, die über die Kommandozeile übergeben wurden,
    und die wir nicht anders verwalten. */
Version mZusatzVersionen;
Str mSprache;
int mSchwierig;
int mLevelpack;
/** Ein oder zwei Spieler? (Hat auch beim Spielen gegen die KI den Wert 2,
    und *nicht* spz_ki) */
int mSpielerZahl;
/** Falls zwei Spieler: Gegen Computer? */
bool mGegenKI;

/** Aktuelle Level-Nummer; enthält, wenn grade kein Spiel läuft, die Nummer
    vom Level zum weiterspielen. Ist (wenn kein Spiel läuft) 0, wenn es
    keine vorige Level-Nummer gibt */
int mLevelNr;
/** (Interner) Name des aktuellen Levels. Enthält, wenn grade kein Spiel
    läuft, den Namen vom Level zum weiterspielen. Ist (wenn kein Spiel
    läuft) "", wenn es keine vorige Level-Nummer gibt*/
//Str mIntLevelName;
int mPunkte[max_spielerzahl];
/** Wird nur während der Zeitbonus-Animation gebraucht... */
int mZeitBonus;




/*************************** Private Methoden **************************/
/* (stehen nicht in der .h-Datei) */


/** tut alles, was beim Starten eines Levels
    getan werden muss; liefert false, wenn es
    den Level gar nicht mehr gibt. Throwt bei Fehler. */
bool startLevel();
/** tut alles, was beim Stoppen eines Levels
    getan werden muss (ohne Animation, d. h. entweder
    ist die Animation schon vorbei oder es gibt halt keine). */
void stopLevel();
/** Setzt die Punktzahl für Spieler sp */
void setPunkte(int sp, int pu);
/** Gibt die Fehlermeldung bestehend aus t und fe aus:
    Sowohl als Text im Cuyo-Fenster
    (bei anz_sp vielen Spielern) als auch auf stderr. 
    mitLog wird an fe.getText() weitergegeben. (D. h.: soll
    ggf. die Send-Log-Meldung ausgegeben werden?) */
void printFehler(int anz_sp, Str t, const Fehler & fe,
                       bool mitLog = false);


/** Macht einen Schritt der Hetzrand-kommt-am-Ende-runter-Animation. */
void bonusAnimationSchritt();
/** Unterfunktion von zeitSchritt();
    Hier passiert die eigentliche Arbeit. */
void zeitSchrittIntern();

/** stoppt das Spiel sofort (egal, ob grad ein Level läuft oder nicht) */
void stopSpiel(bool noch_anzeigen = false);
/** Die Haupt-Spielschritt-Routine, während das Spiel läuft.
    Ruft alle anderen spielschritt()-Routinen auf. */
void spielSchritt();
/** Wird von startLevel() und von spielSchritt() aufgerufen. Lässt
    sämtliche Blops animieren. */
void animiere();

Version berechneVersion();

/* Übernimmt die Dinge, die durch version spezifiziert werden.
   Alles andere bleibt beim alten,
   bis auf daß der alte Wert von mZusatzVersionen verlorengeht.  */
void setzeVersion(const Version & version);



/** Liefert den Namen und Pfad der Prefs-Datei zurück
    ($HOME/.cuyo) */
Str getPrefsName();
/** Liefert den Namen und Pfad der Log-Datei zurück
    ($HOME/cuyo.log) */
Str getLogName();


void signaleAn();
void signaleAus();



/** Startet das Spiel für die eingestellte Spielerzahl und mit dem
    angegebenen Level */
void startSpiel(int level) {
  CASSERT(mGModus == gmodus_kein_spiel);

  ld->ladLevelSummary(false,berechneVersion());

  mLevelNr = level;

  /* Damit der Level nicht schon beim Start angehalten ist oder schnell
     läuft */
  mEinzelschritt = false;
  mSchnellvorlauf = false;

  for (int i = 0; i < max_spielerzahl; i++)
    setPunkte(i, 0);

  try {
    if (!startLevel())
      throw Fehler("%s",_("There are no (more?) (working?) levels."));

  } catch (Fehler f) {
    print_to_stderr(f.getText()+"\n");
    for (int i = 0; i < mSpielerZahl; i++)
      mSpielfeld[i]->setText(f.getText(), true);

    /* Sicherheitshalber...: */
    stopSpiel();

    return;
  }
}




/** tut alles, was beim Starten eines Levels
    getan werden muss; liefert false, wenn es
    den Level gar nicht mehr gibt. Throwt bei Fehler. */
bool startLevel() {

  /* So viele Level gibt's doch gar nicht */
  if (mLevelNr > ld->getLevelAnz())
    return false;

  /* Der Levelname sollte schon bekannt sein. Wir geben ihn gleich mal im
     Fenstertitel aus. */
  SDLTools::setLevelTitle(ld->getLevelName(mLevelNr));

  /* Muss der Level neu geladen werden, oder ist er noch von vorher
     im Speicher? */
  if (!ld->mLevelGeladen || ld->mLevelNummer != mLevelNr) {
  
    /* Ok, neu laden. Erst mal alten Level entladen, damit während
       der "loading level"-Anzeige nicht noch falsche Informationen
       über den alten Level angezeigt werden. */
    ld->entladLevel();

    /* Für "Loading level" */
    ld->setSchriftFarbe(Color(80, 80, 80));

    /* Level-Daten laden */
    for (int i = 0; i < mSpielerZahl; i++) {
      mSpielfeld[i]->ladeLevelModus();
      mSpielfeld[i]->setText(_sprintf(_("Score: %d\n\nLoading Level %d...\n\n"),
	        mPunkte[i], mLevelNr));
    }
    /* "Loading Level" gleich anzeigen. Außerdem den Rand neu malen, da
       wir die Schriftfarbe geändert haben und das Numexplode vom alten
       level ungültig ist und so. */
    UI::randNeuMalen();
    UI::sofortAllesAnzeigen();

    /* So, und jetzt den neuen Level laden. (Braucht etwas Zeit) */
    ld->ladLevel(mLevelNr);

  }
  else ld->erneuerLevel(mLevelNr);

  //for (int i = 0; i < max_spielerzahl; i++) {
  //  /* Punkte-Anzeige muss geupdatet werden: Zu Level passende Farbe.
  //     (Etwas gepfuscht) */
  //  setPunkte(i, mPunkte[i]);
  //}
  /* Einige Farben haben sich durch das level-laden geändert. Also
     nach dem Level alles neu malen. Und da die mal-Routine nicht auf
     die Idee kommt, dass die Ränder um den Level neu an den X-Server
     geschickt werden müssen, selbst Area::updateAll() aufrufen. */
  UI::randNeuMalen();
	
  /* Aufnehmen / Abspielen starten */
  Aufnahme::init(mAbspielen, getSpielerModus());

  /***** Allen Leuten erzählen, dass jetzt ein Level anfängt. *****/
  
  /* Für den Global-Blop... */
  ld->startLevel();

  /* Hier ist die Stelle, wo wir darauf gucken, wie viele Spieler mitspielen;
     nur für so viele Spieler wird das Spiel wirklich gestartet */
  for (int i = 0; i < mSpielerZahl; i++) {

    mSpielfeld[i]->startLevel();
    Str PlatzAnzahlFormat;
    if (ld->mPlatzAnzahlMin == ld->mPlatzAnzahlMax)
      PlatzAnzahlFormat=_sprintf(ngettext("1 blop explodes",
					  "%d blops explode",
					  ld->mPlatzAnzahlMin),
				 ld->mPlatzAnzahlMin);
    else
      if (ld->mPlatzAnzahlAndere)
	PlatzAnzahlFormat=_sprintf(_("between %d and %d blops explode"),
				  ld->mPlatzAnzahlMin,ld->mPlatzAnzahlMax);
      else
	PlatzAnzahlFormat=_sprintf(_("%d or %d blops explode"),
				  ld->mPlatzAnzahlMin,ld->mPlatzAnzahlMax);
    // TRANSLATORS: The seond %s (after "by ") is the level's author's name
    mSpielfeld[i]->setText(_sprintf(_("Score: %d\n\n"
                "Level %d\n%s\nby %s\n\n"
                "%s\n%s\n"
		"%s\n\n"
		"Space = Start"),
	    mPunkte[i],
	    mLevelNr, ld->mLevelName.data(), ld->mLevelAutor.data(),
	    PlatzAnzahlFormat.data(),
	    ld->mGrasBeiKettenreaktion ? _("Chain reaction necessary\n") : "",
	    ld->mBeschreibung.data()));
  }
  
  /* Alle Blops einmal animieren, damit sie wissen, wie sie aussehen.
     Dabei werden auch die ganzen Init-Events verschickt. */
  animiere();

  /* Auch die KI möchte wissen, wenn ein neuer Level anfängt. */
  if (mGegenKI)
    mKI->startLevel();


  Sound::setMusic(ld->mMusik);
	
  mGModus = gmodus_spiel_start;
  mPause = false;
  mWarteAufTaste = true;
  mWarteTimeout = 0;
  return true;
}



/** tut alles, was beim Stoppen eines Levels
    getan werden muss (ohne Animation, d. h. entweder
    ist die Animation schon vorbei oder es gibt halt keine). */
void stopLevel() {
  SDLTools::setMainTitle();
			
  for (int i = 0; i < mSpielerZahl; i++)
    mSpielfeld[i]->stopLevel();
}




/** Setzt die Punktzahl für Spieler sp */
void setPunkte(int sp, int pu){
  mPunkte[sp] = pu;
  UI::setPunkte(sp, pu);
}












/** stoppt das Spiel sofort (egal, ob grad ein Level läuft oder nicht) */
void stopSpiel(bool noch_anzeigen/* = false*/) {
  /* Evtl. wird stopSpiel() aufgerufen, wenn das Spiel noch gar nicht
     richtig fertig gestartet wurde; dann wurde mGModus vielleicht noch
     nicht gesetzt, aber es soll trotzdem was gestoppt werden... */
  /*if (mGModus == gmodus_kein_spiel)
    return;*/

  stopLevel();

  if (noch_anzeigen) {
    mGModus = gmodus_ende_warte;
    mWarteAufTaste = true;
  } else {
    mGModus = gmodus_kein_spiel;
    UI::stopSpiel();
  }
}










/** Die Haupt-Spielschritt-Routine, während das Spiel läuft.
    Ruft alle anderen spielschritt()-Routinen auf. */
void spielSchritt() {

  /* Den Schritt aufnehmen bzw. abspielen. Beim Abspielen werden
     hier auch ggf. Tastendrücke abgespielt. */
  Aufnahme::recSchritt(mSpielfeld);

  /*** Die einzelnen Teile eines Spielschritts ausführen: ***/

  /* Who will be the one killing me for this #define? */
  /* Answer: The first one to try
       for (int i=1; i<=10; i++)
         ALLE_SPIELER->do_stuff(i);
  */
#define ALLE_SPIELER for (int i = 0; i < mSpielerZahl; i++) mSpielfeld[i]

  /* ggf. Message blinken lassen */
  ALLE_SPIELER->blinkeMessage();

  /* Hetzrand runterbewegen; evtl. sterben */
  ALLE_SPIELER->bewegeHetzrand();

  /* Evtl. zufällige Graue an die Spieler senden. (Blops werden nicht
     erzeugt; es wird nur gespeichert, dass das noch Graue auf das
     ankommen warten. */
  ALLE_SPIELER->zufallsGraue();

  Blop::beginGleichzeitig();
  ALLE_SPIELER->fallSchritt();
  Blop::endGleichzeitig();

  /* Reihen hin/her. (In einer Gleichzeit, für evtl. auftretende events.) */
  Blop::beginGleichzeitig();
  ALLE_SPIELER->rueberReihenSchritt();
  Blop::endGleichzeitig();

  /* Neue explosionen testen. Da inzwischen vielleicht neue Blops
     entstanden sind und alle einen event bekommen, erst mal fehlende
     init-Events senden. Sollte in Zukunft automatisch am Anfang
     jeder Gleichzeit passieren. */
  Blop::sendeFehlendeInitEvents();
  Blop::beginGleichzeitig();
  ALLE_SPIELER->testeFlopp();
  Blop::endGleichzeitig();

  /* Rest vom normalen Spielschritt */
  Blop::beginGleichzeitig();
  ALLE_SPIELER->spielSchritt();
  Blop::endGleichzeitig();

#undef ALLE_SPIELER

  /* Animationen und Grafik-Ausgabe */
  animiere();



  /* Ggf. auch einen Spielschritt für die KI */
  if (mGegenKI)
    mKI->spielSchritt();



  if (mGModus == gmodus_spiel) {
    /* Nur wenn das Spiel wirklich noch läuft (und nicht nur darauf gewartet
       wird, das wir beenden können: Testen, ob wir gewonnen haben */

    /* Wurde grade das restliche Gras vernichtet? */
    for (int i = 0; i < mSpielerZahl; i++)
      if (mSpielfeld[i]->istGrasDa())
        goto noch_gras_da;
      
    /* Dann warten wir jetzt nur noch auf einen guten Moment zum beenden. */
    mGModus = gmodus_warte_gewonnen;
      
noch_gras_da:;
  } else {
    /* Nur wenn das Spiel kurz davor ist, beendet zu werden:
       Checken, obs jetzt wirklich beendet werden kann. */
    
    CASSERT(mGModus == gmodus_warte_gewonnen ||
            mGModus == gmodus_warte_verloren);
    /* Spiel (Level) soll beendet werden; aber sind auch beide
       Spieler bereit? */
			
    bool bereit = true;
    for (int i = 0; i < mSpielerZahl; i++)
      if (!mSpielfeld[i]->bereitZumStoppen())
	bereit = false;

    if (bereit) {
      /* OK, alle bereit. */
			
      /* Gewonnen oder verloren? */
      if (mGModus == gmodus_warte_gewonnen) {

        Sound::playSample(sample_levelwin,so_global);
				
	/* Hier darf noch nicht stopLevel() aufgerufen werden, weil sonst
	   das Spielfeld nicht mehr angezeigt wird */
	
	/* Gewonnen. Level als gewonnen abspeichern. Aber nicht im
	   Debug-Modus. */
	if (!gDebug)
	  PrefsDaten::schreibGewonnenenLevel(mSpielerZahl > 1, mLevelNr);
				
	/* Jetzt kommt die Zeitbonus-Animation */
	mGModus = gmodus_bonus_animation;
				
	/* Noch keine Bonus-Punkte */
	mZeitBonus = 0;
      } else {
        Sound::playSample(sample_levelloose, so_global);

	/* Spiel zu Ende weil verloren;
           true = Spieler darf sich noch seine Punkte anschauen */
	stopSpiel(true);
				
	for (int i = 0; i < mSpielerZahl; i++)
	  mSpielfeld[i]->setText(_sprintf(
            _("Game over\n\nScore: %d\n\n"), mPunkte[i]));
      }
					
    }	// Ende: if bereit
  } // Ende: if warten, bis alle für's Spielende bereit sind


}  // spielSchritt()




/** Macht einen Schritt der Hetzrand-kommt-am-Ende-runter-Animation. */
void bonusAnimationSchritt() {
  /* Hetzrand runterrutschen lassen */
  bool ba_fertig = true; // "= true" nur um eine Warnung zu ersparen

  mZeitBonus += punkte_fuer_zeitbonus;

#define bonusMessage _("Level %s complete!\n\nTime Bonus: %d\nScore: %d\n\n%s")

  for (int i = 0; i < mSpielerZahl; i++) {

    /* Neuen Text (mit neuer Punkt-Zahl) schreiben */
    mSpielfeld[i]->setText(_sprintf(bonusMessage,
	      ld->mLevelName.data(), mZeitBonus, mPunkte[i], " "));

    /* Rand runterrutschen lassen */		
    ba_fertig = mSpielfeld[i]->bonusSchritt();

    /* Punkte bekommen */
    setPunkte(i, mPunkte[i] + punkte_fuer_zeitbonus);
  }

  /* Unten angekommen? (Sollte bei beiden Spielern gleichzeitig passieren) */
  if (ba_fertig) {
    mGModus = gmodus_bonus_warte;

    for (int i = 0; i < mSpielerZahl; i++) {
      mSpielfeld[i]->setText(_sprintf(bonusMessage,
	      ld->mLevelName.data(), mZeitBonus, mPunkte[i],
	      _("Space = Continue")));
    }

    mWarteAufTaste = true;
    /* Kein Warte-Timeout mehr. Einfach Leertaste drücken */
    //mWarteTimeout = nachbonus_wartezeit;
  } // Ende: if Bonus-Animation fertig

#undef bonusMessage

}


/** Unterfunktion von zeitSchritt();
    Hier passiert die eigentliche Arbeit. */
void zeitSchrittIntern() {

  /* Während Pause läuft das Spiel nicht weiter */
  if (mPause)
    return;
  
  /* Warten wir grad drauf, dass der Benutzer eine Taste drückt? */
  if (mWarteAufTaste) {

    if (mWarteTimeout) {
      /* Wir warten nicht beliebig lang */
      mWarteTimeout--;
      if (mWarteTimeout)
	return;
			
      /* Zu lange gewartet. Jetzt reicht's! */
      mWarteAufTaste = false;
    } else
      return;
  }
  
  /* Einzelschritt-Modus. Nach diesem Schritt gleich wieder auf
     Tastendruck warten. */
  if (mEinzelschritt) {
    mWarteAufTaste = true;
  }
  
  
  if (mGModus == gmodus_ende_warte) {
    /* Spieler hat sich seine Punkte (oder was auch immer sont)
       fertig angeschaut. Jetzt koennen wir zurueck ins Menue */
    mGModus = gmodus_kein_spiel;
    UI::stopSpiel();
    return;
  }

  if (mGModus == gmodus_bonus_warte) {
    /* Wir haben grade ein bisschen gewartet, damit der Benutzer seine
       Level-Endpunkte bewundern kann. Jetzt geht's weiter mit dem nächsten
       Level... */
			
    /* Alten Level stoppen (ist vermutlich nicht wirklich nötig) */
    stopLevel();
 		
    /* neuer Level */
    
    mLevelNr++;
    try {
      if (!startLevel()) {
        /* Es gibt gar keine Level mehr; also Spiel beenden */
        stopSpiel(true);
        for (int i = 0; i < mSpielerZahl; i++) {
          mSpielfeld[i]->setText(
            _sprintf(mSpielerZahl == 2 && mPunkte[i] > mPunkte[1-i] ? 
                _("***\nYou won even a bit more!!!\n\nScore: %d\n***\n\n") :
                _("***\nYou won!!!\n\nScore: %d\n***\n\n")  ,
	      mPunkte[i]));
        }
        mLevelNr = 0; // Jetzt kann man nicht mehr Restart last level
      }
    } catch (Fehler fe) {
      printFehler(mSpielerZahl, _("Could not start level:\n"), fe);
      stopSpiel();
    }
    return;
  } 	
	
  /* Spiel grade erst gestartet? Dann Texte löschen */
  if (mGModus == gmodus_spiel_start) {
    for (int i = 0; i < mSpielerZahl; i++)
      mSpielfeld[i]->setText("");
    mGModus = gmodus_spiel;
  }

  CASSERT(mGModus == gmodus_bonus_animation || mGModus == gmodus_spiel ||
          mGModus == gmodus_warte_gewonnen || mGModus == gmodus_warte_verloren);


  /* Ok, hier kommt der eigentliche Spielschritt... */
  if (mGModus == gmodus_bonus_animation) {
    /* ... Spielschritt Bonusanimation */
    bonusAnimationSchritt() ;

  } else {
    /* ... normaler Spielschritt */
    

    /* Evtl. Signale abfangen, um die Logdatei abspeichern zu können. */
    signaleAn();
    
    try {
      /* Hier findet das eigentliches Spiel statt */
      spielSchritt();
      
    } catch (Fehler fe) {
      signaleAus();
      bool log_gespeichert = false;

      try {
        Aufnahme::speichern(getLogName());
	log_gespeichert = true;
      } catch (Fehler) {}

      /* Im Debug-Modus soll das Spielfeld weiter angezeigt werden,
         wenn ein Fehler während des Spiels passiert. */
      stopSpiel(gDebug);
      
      /* Wenn das Speichern der log-Datei geklappt hat, soll ggf.
         die "send log to..."-Meldung ausgegeben werden. */
      printFehler(mSpielerZahl, _("Error during the game:\n"), fe,
                  log_gespeichert);
      return;
    }
    	      
    signaleAus();
  }



}


/** Wird von startLevel() und von zeitSchritt() aufgerufen. Lässt
    sämtliche Blops animieren. */
void animiere() {

  /* Vielleicht hätten gerne einige Blops noch ein Init-Event... */
  Blop::sendeFehlendeInitEvents();
  
  Blop::beginGleichzeitig();
  
  /* Alle Grafiken löschen */
  Blop::lazyInitStapel();

  /* Erst mal das globale Blop ausführen. */
  ld->spielSchritt();
  
  /* Die eigentliche Animation */
  for (int i = 0; i < mSpielerZahl; i++)
    mSpielfeld[i]->animiere();
    
  Blop::endGleichzeitig();
}



/**  */
void neuePunkte(bool reSp, int pt){
  int sp = (reSp ? 1 : 0);
  int punkte = mPunkte[sp] + pt;
  if (punkte<0) punkte=0;
  setPunkte(sp,punkte);
}



/** wird aufgerufen, wenn ein Spieler tot ist */
void spielerTot() {
  mGModus = gmodus_warte_verloren;
}





/* reSp sendet g Graue an den anderen Spieler */
void sendeGraue(bool reSp, int g) {
  mSpielfeld[!reSp]->empfangeGraue(g);
}


/** reSp bittet den anderen Spieler um eine Reihe. Er selbst
    hat Höhe h. Antwort ist eine der Konstanten bewege_reihe_xxx */
int bitteUmReihe(bool reSp, int h) {
  return mSpielfeld[!reSp]->bitteUmReihe(h);
}


/** reSp will einen Stein vom anderen Spieler (rüberreihe) */
void willStein(bool reSp, Blop & s) {
  mSpielfeld[!reSp]->gebStein(s);
}





/** Liefert den Namen und Pfad der Log-Datei zurück
    ($HOME/cuyo.log) */
Str getLogName() {
  char * ho = getenv("HOME");
  if (!ho) {
    /* Unter Windows zum Beispiel... */
    print_to_stderr(_("Warning: Env-Variable $HOME not found. Using the current directory for cuyo.log"));
    return "cuyo.log";
  }
  if (ho[strlen(ho) - 1] == '/')
    return Str(ho) + "cuyo.log";
  else
    return Str(ho) + "/cuyo.log";
}



/** Gibt die Fehlermeldung bestehend aus t und fe aus:
    Sowohl als Text im Cuyo-Fenster
    (bei anz_sp vielen Spielern) als auch auf stderr. 
    mitLog wird an fe.getText() weitergegeben. (D. h.: soll
    ggf. die Send-Log-Meldung ausgegeben werden?) */
void printFehler(int anz_sp, Str t, const Fehler & fe,
                       bool mitLog /*= false*/) {
  t += fe.getText(mitLog);
  print_to_stderr(t+"\n");
  for (int i = 0; i < anz_sp; i++) {
    /* zweites true = Kleine Schrift... */
    mSpielfeld[i]->setText(t, true);
  }
}






#if signale_fangen

void signalHandler(int s) {
  try {
    Aufnahme::speichern(getLogName());
    /* send_log_string ist in fehler.h definiert. */
    print_to_stderr(send_log_string+"\n");
  } catch (Fehler) {}
  
  /* Braucht man das? Geht das? */
  raise(s);
}


void signaleAn() {
  /*{
    struct sigaction act; 
    act.sa_handler = speicherFehler;
    act.sa_mask = 0;
    act.sa_flags = SA_ONESHOT;
    sigaction(SIGSEGV, &act, 0);
  }*/
  signal(SIGILL, signalHandler);  // illegal instruction
  signal(SIGSEGV, signalHandler); // segmentation fault
  signal(SIGFPE, signalHandler);  // floating point exception
}

void signaleAus() {
  signal(SIGILL, SIG_DFL);  // illegal instruction
  signal(SIGSEGV, SIG_DFL); // segmentation fault
  signal(SIGFPE, SIG_DFL);  // floating point exception
}

#else
void signaleAn() {
}

void signaleAus() {
}
#endif



/********************** public Methoden (für das Spiel) **********************/
/* (stehen in cuyo.h) */









/** liefert true, wenn das Spiel normal läuft, false
		wenn das Spiel am zuende gehen ist */
bool getSpielLaeuft() {
  CASSERT(mGModus != gmodus_kein_spiel);
  return mGModus == gmodus_spiel;
}


/** liefert true, wenn das Spiel gepaust ist. */
bool getSpielPause() {
  return mPause;
}


int getLevelNr() {
  return mLevelNr;
}

/** Liefert die Anzahl der Mitspieler zurück. */
int getSpielerZahl() {
  return mSpielerZahl;
}


/** Liefert das Pause-Bildchen zurück */
Bilddatei * getPauseBild() {
  return mPauseBild;
}


/** Liefert true, wenn debug-Rüberreihen-Test aktiv ist */
bool getRueberReihenTest() {
  return mRueberReihenTest;
}


/** Liefert ein Spielfeld zurück. */
Spielfeld * getSpielfeld(bool reSp) {
  return mSpielfeld[reSp];
}




/*********************** public Methoden fürs ui **************************/
/* (stehen in ui2cuyo.h) */


void init() {
  mEinzelschritt = false;
  mZeitlupe = false;
  mZaehlerZeitlupe = 0;
  mRueberReihenTest = 0;
  mAbspielen = 0;
  mSchnellvorlauf = 0;
  mPauseBild = 0;
  mZusatzVersionen = Version();
  mSprache = "";
  mSchwierig = Version::gSchwierig.suchMerkmal("");
  mSpielerZahl = 1;
  mLevelpack = Version::gLevelpack.suchMerkmal("main");
  CASSERT(mLevelpack>=0);

  /* Jetzt gehen auf der Kommandozeile spezifizierte Versionen ein. */
  setzeVersion(mKommandoZeilenVersion);
    
  /* Hier findet das parsen statt. (Fehler werden im try-catch-Block
     der main()-Routine ausgegeben.) Das LevelDaten-Objekt weist
     sich selber der globalen Variable ld zu. */
  new LevelDaten(berechneVersion());
  

  for (int i = 0; i < max_spielerzahl; i++) {
    /* Spielfeld erzeugen */
    mSpielfeld[i] = new Spielfeld(i > 0); // (i > 0) = rechter Spieler
  }

  /* Pause-Bild laden */
  mPauseBild = new Bilddatei();
  mPauseBild->laden("pause.xpm");

  /* KI-Spieler erzeugen */
  mKI = new KIPlayer(mSpielfeld[1]);

  setSpielerModus(1); // Sollte erst nach Erzeugen der Menüs aufgerufen werden...

  /* Damit Weiterspielen grad nicht geht. */
  mLevelNr = 0;
		
  mGModus = gmodus_kein_spiel;
	
  mWarteAufTaste = false;
}

void destroy(){
  delete ld;
  delete mKI;
}



void setPause(bool pause) {
  mPause = pause;
  for (int i = 0; i < mSpielerZahl; i++)
    mSpielfeld[i]->setUpdateAlles();
  UI::nachEventAllesAnzeigen();
}


/** Ein key-Event halt... (Kümmert sich um alle Tasten,
    die während des Spiels so gedrückt werden...). */
void keyEvent(const SDL_keysym & taste) {

  if (mPause) {
    switch (taste.sym) {
      case SDLK_ESCAPE:
        stopSpiel();
        UI::nachEventAllesAnzeigen();
	break;
      case ' ':
      case SDLK_RETURN:
      case SDLK_KP_ENTER:
        setPause(false);
	break;
      default:
        break;
    }
    return;
  }


  if (taste.sym == SDLK_ESCAPE) {
    /* Auf Pause schalten */
    setPause(true);
    return;
  }

  if (mWarteAufTaste) {
    if (taste.sym == ' ' || taste.sym == SDLK_RETURN ||
        taste.sym == SDLK_KP_ENTER) {
      mWarteAufTaste = false;
      mWarteTimeout = 0;
      return;
    }
    
  } else if (mGModus == gmodus_spiel) {
    int sp, t;
    if (PrefsDaten::getTaste(taste.sym, sp, t)) {
      /* Im 1-Spieler-Modus und im KI-Modus alle Tastendrücke an
	 Spieler 1 senden: */
      if (mSpielerZahl == 1 || mGegenKI) sp = 0;

      Aufnahme::recTaste(sp, t);
      mSpielfeld[sp]->taste(t);
      return;
    }
  } // if spiel läuft
  
  
  /* Event noch nicht verarbeitet. Dann vielleicht debug? */
}

  
/** Eine Taste wurde gedrueckt, von der das ui befunden hat, dass
    es sich um eine debug-Taste handeln koennte.
    Liefert false zurueck, wenn der Debug-Modus gar nicht aktiv ist. */
bool debugKeyEvent(const SDL_keysym & taste) {
  if (!gDebug)
    return false;

  char buch = taste.sym;
  if ((taste.mod & KMOD_SHIFT) && buch >= 'a' && buch <= 'z')
    buch = buch - 'a' + 'A';

  if (buch == 'b') {
    for (int i = 0; i < mSpielerZahl; i++)
      mSpielfeld[i]->empfangeGraue(1);
    print_to_stderr(_("Debug: Receive greys\n"));
    
  } else if (buch == 'e') {
    mEinzelschritt = !mEinzelschritt;
    print_to_stderr(_sprintf(_("Debug: Single step mode = %d\n"),
			     mEinzelschritt));
    if (!mEinzelschritt) {
      /* Nicht noch ein letztes Mal auf Taste warten... */
      mWarteAufTaste = false;
    }
    
  } else if (buch == 'f') {
    if (mGModus==gmodus_spiel) {
      for (int i=0; i<mSpielerZahl; i++)
        mSpielfeld[i]->resetFall();
      print_to_stderr(_("Debug: Replace fall\n"));
    }

  } else if (buch == 'L') {
    if (mGModus == gmodus_kein_spiel) {
      print_to_stderr(_("Debug: Load log file\n"));
      Aufnahme::laden(getLogName());
      setSpielerModus(Aufnahme::getSpielerModus());
      Str lna = Aufnahme::getLevelName();
      mLevelNr = ld->getLevelNr(lna);

      // TRANSLATORS: The %d is the level number
      print_to_stderr(_sprintf(_("Level %s (%d)\nFor replay start level.\n"),
			       lna.data(), mLevelNr));
      mAbspielen = true;
    } else
      print_to_stderr(_("Debug: Do *not* load log file during the game.\n"));
    
  } else if (buch == 'l') {
    mAbspielen = !mAbspielen;
    print_to_stderr(_sprintf(_("Debug: Replay log file = %d\n"), mAbspielen));
    
  } else if (buch == 'g') {
    /* Hoffentlich darf man gModus einfach so ändern... */

    if (mGModus == gmodus_spiel) {
      mGModus = gmodus_warte_gewonnen;
      print_to_stderr(_("Debug: Win instantly\n"));
    }
    
  } else if (buch == 'h') {
    print_to_stderr(_("Debug: Help (all keys to be combined with Alt)\n"
		      "  b: Receive greys\n"
		      "  e: Single step mode on/off\n"
                      "  f: Replace fall\n"
		      "  g: Win instantly\n"
		      "  h: Help (this one)\n"
		      "  L: Load log file\n"
		      "  l: Replay log file on/off\n"
		      "  r: Reload level\n"
		      "  S: Save log file\n"
		      "  t: Test of row-exchange on/off\n"
		      "  v: Fast forward on/off\n"
		      "  z: Slow motion on/off\n"));
    
  } else if (buch == 'r') {
    if (mGModus == gmodus_kein_spiel) {
      print_to_stderr(_("Debug: Reload levelconf\n"));
      try {
   	ld->ladLevelSummary(true,berechneVersion());
      } catch (Fehler fe) {
	printFehler(2, _("Could not reload the level description file because of the following error:\n"),
	            fe);
      }
    } else
      print_to_stderr(_("Debug: Do *not* load log file during the game.\n"));

  } else if (buch == 'S') {
    print_to_stderr(_("Debug: Save log file\n"));
    Aufnahme::speichern(getLogName());
    
  } else if (buch == 't') {
    mRueberReihenTest = !mRueberReihenTest;
    print_to_stderr(_sprintf(_("Debug: Test of row-exchange = %d\n"),
			     mRueberReihenTest));
    
  } else if (buch == 'v') {
    mSchnellvorlauf = !mSchnellvorlauf;
    print_to_stderr(_sprintf(_("Debug: Fast forward = %d\n"), mSchnellvorlauf));
    
  } else if (buch == 'z') {
    mZeitlupe = !mZeitlupe;
    print_to_stderr(_sprintf(_("Debug: Slow motion = %d\n"), mZeitlupe));
  }
  
  return true;
}



/*
für xtrace:
extern "C" {
int XInternAtom(Display *display, char *atom_name, int only_if_exists);
}
*/



/** Die Haupt-Zeitschritt-Routine. Wird direkt
    vom ui aufgerufen. Ruft alle spielschritt()-Routinen u.ä. auf. */
void zeitSchritt() {
  CASSERT(mGModus != gmodus_kein_spiel);

/*
für xtrace:
 XInternAtom(qt_xdisplay(),"zeitschritt A",1);
*/

  /* Zeitlupen-Debug-Modus? */
  if (mZeitlupe) {
    mZaehlerZeitlupe++;
    if (mZaehlerZeitlupe == 5)
      mZaehlerZeitlupe = 0;
    else
      return;
  }

  if (mSchnellvorlauf) {
    for (int i = 0; i < 3; i++)
      if (mGModus != gmodus_kein_spiel) zeitSchrittIntern();
  } else {
    zeitSchrittIntern();
  }

  /* Brauche kein UI::allesAnzeigen(); darum kuemmert sich das spiel-Blatt selbst */
}  // zeitSchritt()


/** Markiert alle Graphik auf upzudaten; danach muss noch malSpielfeld()
    aufgerufen werden, um das Update wirklich zu machen */
void setUpdateAlles() {
  for (int i = 0; i < mSpielerZahl; i++)
    mSpielfeld[i]->setUpdateAlles();
}


/** Spielfeld neu malen. Wird vom ui aufgerufen. */
void malSpielfeld(int sp) {
  mSpielfeld[sp]->malUpdateAlles();
}



int getSpielerModus() {
  if (mGegenKI)
    return spielermodus_computer;
  else
    return mSpielerZahl;
}

/** Setzt #Spieler, KI-Modus; gemerkte Level-Nummer wird auf 0
    zurueckgesetzt. Vorbedingung: Es läuft grad kein Spiel. */
void setSpielerModus(int spm) {
  CASSERT(mGModus==gmodus_kein_spiel);
  if (spm == spielermodus_computer) {
    mGegenKI = true;
    mSpielerZahl = 2;
  } else {
    mGegenKI = false;
    mSpielerZahl = spm;
  }
  
  mLevelNr = 0;
}


/** Der int ist der Index in Version::mLevelpack bzw Version::mSchwierig */
void setLevelpack(int i) {
  mLevelpack = i;
  
  /* Levelpack geändert => aktuelle Level-Nummer wird ungültig. */
  mLevelNr = 0;
}

int getLevelpack() {return mLevelpack;}

void setSchwierig(int i) {
  mSchwierig = i;

  /* Schwierigkeit geändert => aktuelle Level-Nummer wird ungültig. */
  mLevelNr = 0;
}

int getSchwierig() {return mSchwierig;}


/* Liefert Nr. des zuletzt gespielten Levels (oder 0) */
int getLetzterLevel() {
  return mLevelNr;
}



Version berechneVersion() {
  Version ret = mZusatzVersionen;
  ret.nochEinMerkmal(mSpielerZahl==1 ? "1" : "2");
  if (mSprache != "")
    ret.nochEinMerkmal(mSprache);
  if (Version::gSchwierig.mMerkmale[mSchwierig] != "")
    ret.nochEinMerkmal(Version::gSchwierig.mMerkmale[mSchwierig]);
  ret.nochEinMerkmal(Version::gLevelpack.mMerkmale[mLevelpack]);
  return ret;
}


/* Übernimmt die Dinge, die durch version spezifiziert werden.
   Alles andere bleibt beim alten,
   bis auf daß der alte Wert von mZusatzVersionen verlorengeht.  */
void setzeVersion(const Version & version) {
  mZusatzVersionen = version;

  /* Achtung! Jetzt enthält mZusatzVersionen erstmal zu viel.
     Aber der Überschuß wird gleich rausgelöscht. */

  try {
    mSpielerZahl = (
      mZusatzVersionen.extractMerkmal(dimaa_numspieler,
				      (mSpielerZahl==1 ? "1" : "2"))
        == "1"
      ? 1 : 2);
    mSprache = mZusatzVersionen.extractMerkmal(dima_sprache,mSprache);
    mSchwierig = mZusatzVersionen.extractMerkmal(Version::gSchwierig,
						 mSchwierig);
    mLevelpack = mZusatzVersionen.extractMerkmal(Version::gLevelpack,
						 mLevelpack);
  }
  catch(Str konflikt) {throw Fehler(_("Conflicting versions: %s"),
					 konflikt.data());}
}


}

