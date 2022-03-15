/***************************************************************************
                          spielfeld.cpp  -  description
                             -------------------
    begin                : Sat Oct 2 1999
    copyright            : (C) 1999 by immi
    email                : cuyo@pcpool.mathematik.uni-freiburg.de

Modified 2001-2008,2010,2011 by the cuyo developers

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
#include <vector>

#include "sdltools.h"
#include "font.h"

#include "inkompatibel.h"
#include "nachbariterator.h"
#include "cuyo.h"
#include "spielfeld.h"
#include "knoten.h"
#include "aufnahme.h"
#include "sound.h"
#include "ui.h"

/* Wird nur included für die Konstanten taste_* */
#include "prefsdaten.h"


/***** Konstanten, die was einstellen *****/


/* Wie weit unten müssen Graue sein, damit ein neues Fall kommt? */
#define neues_fall_platz 5


/* Wenn grade neue Graue auf einen maximal hohen Turm gefallen sind,
   soll man auf jeden Fall noch ein bisschen Zeit haben, ihn wieder
   zu verkleinern. Deshalb tauchen Graue ein Stück weiter unten als
   der Hetzrand auf:
 */
/* Wo relativ zum Hetzrand... */
/* ... tauchen neue Steine auf? */
#define hetzrand_dy_auftauch 8
/* ... ist die maximal erlaubte Turmhöhe? */
#define hetzrand_dy_erlaubt 0

/* Wie oft wird versucht, die "+"-Blops am Spielanfang unverbunden
   zu machen? */
#define startlevel_rand_durchgaenge 10


/* Wie lange wird ein Message angezeigt? */
#define message_gesamtzeit 45
#define message_aufabdeckzeit 16
#define message_blink 4

/***** Rückgabe-Konstanten u. ä. *****/

	
/* Werte für mModus */

/* In diesem Modus *wird* ein Spielfeld angezeigt (wenn das ui uns malen
   laesst). Das passiert uebliecherweise am Ende, wenn der Benutzer noch
   schauen will, wie viele Punkte er hat. */
#define modus_keinspiel 0
/* In diesem Modus ist kein Spielfeld zu sehen. */
#define modus_lade 1

#define modus_testflopp 30  // nur ein Übergangsmodus: Es muss noch getestet
                            // werden, ob was platzt
#define modus_flopp 31
#define modus_rutschnach 40
#define modus_neue_graue 41 // Da kommen grad neue Graue vom Himmel. So lange
                            // das noch passiert, kann kein neues Fall kommen.
                            // Irgend wann sind die Grauen weit genug unten;
                            // Dann wird ein neues Fall losgeschickt und der
                            // Modus gewechselt. Und zwar nach rutschnach,
                            // falls noch Graue unterwegs sind und nach
                            // testflopp, falls sie schon gelandet sind (z. B.
                            // auf hohen Türmen.)
               
#define modus_fallplatz 90 // da war grad was am runterfallen, während das
                     // Spiel beendet wurde; da lassen wir es halt platzen

// das Spiel ist zu Ende (alle Schlussanimationen sind fertig abgelaufen,
// wir warten nur noch darauf, dass stopLevel() aufgerufen wird):
#define modus_warte_auf_stop 91

/* Modus-Diagramm:

         ,-- warte_auf_stop <----,
         |        ^              |
         |        |          fallplatz <-,
         |        |                      |
         |        +-----------------> testflopp --> flopp
         |        |                     |    ^        |
         |        |                     |    |        |
         |        |                     |    |        |
         v        |                     |    |        |
   keinspiel --> neue_graue <-----------´    |        |
                       |                     |        |
                       `-------------> rutschnach <---+



   - testflopp wird schon am Anfang von Spielschritt ausgewertet. (d. h. der
     Übergang von da woandershin dauert keinen Zeitschritt)
   - Nur in keinspiel ist kein Spielfeld sichtbar
	 - Mit Escape kann von jedem Modus zu keinspiel gewechselt werden
*/

				
/* Werte für mRueberReihenModus */
#define rrmodus_nix 0
#define rrmodus_gib_rueber 10
#define rrmodus_gib_runter 11
#define rrmodus_bekomm_hoch 20
#define rrmodus_bekomm_rueber 21




/* Rückgabewerte für rutschNach() */
#define rutschnach_nix 0
#define rutschnach_wenig 1 // Wenn nur im unteren Bereich noch was nachrutscht,
// so dass das neue Fall kommen kann
#define rutschnach_viel 3
// Brauche: rutschnach_viel | rutschnach_wenig = rutschnach_viel


/* Rückgabewerte von sigWillReihe bzw. gebReihe */
#define bewege_reihe_nein 0
/* "Moment, ich bin noch mit der vorigen Reihe beschäftigt. Vielleicht
    gleich." */
#define bewege_reihe_moment 1
#define bewege_reihe_ja 2




Spielfeld::Spielfeld(bool re)
  : mText(""), mDaten(re,this), mFall(this, re), mSemiglobal()
{
  mSemiglobal.setBesitzer(0, ort_absolut(absort_semiglobal, re));

  /* für das Reihenrüberschieben... */
  mRechterSpieler = re;

  mModus = modus_keinspiel;
}

Spielfeld::~Spielfeld(){
}







/* Hauptmalroutine: Malt alles, was sich geaendert hat, neu */
void Spielfeld::malUpdateAlles() {

  //CASSERT(mModus != modus_keinspiel);

  if (mUpdateAlles)
    Area::updateRect(0, 0, grx * gric, gry * gric);

  if (mModus == modus_lade) {

    if (mUpdateAlles) {
      Area::fillRect(0, 0, gric*grx, gric*gry, Color(150, 150, 150));
      malSchrift(true, mText, mTextX0, mTextX1);
    }
    
  } else if (Cuyo::getSpielPause()) {
    /* Spiel steht auf Pause */

    if (mUpdateAlles) {
      Area::fillRect(0, 0, gric*grx, gric*gry, Color(150, 150, 150));
      Bilddatei * pausebild = Cuyo::getPauseBild();
      CASSERT(pausebild);
      pausebild->malBild((gric*grx - pausebild->getBreite())/2,
		         (gric*gry - pausebild->getHoehe())/2);


      Font::gGame->drawText(ld->mLevelName, gric*grx / 2, 5 * gric, AlignCenter);

      /* Schrift drübermalen. In der Mitte, falls es kein Pausebild gibt. */
      malSchrift(!pausebild,FontStr(
		   _("Game paused\n\nSpace = Resume\nEsc = Abort Game\n\n")),
		 0,gric*grx);
    }

  } else {
    malUpdateSpielsituation();
  }
  
  mUpdateAlles = false;
}   // malUpdateAlles




/** Malt die ganzen Blops, usw. */
void Spielfeld::malUpdateSpielsituation() {
  bool updateSchrift = false;

  int hintergrund_y = (ld->mMitHintergrundbildchen
		       ? (ld->mSpiegeln
			  ? 0
			  : gric*gry - ld->mHintergrundBild.getHoehe())
		       : 0);
  if (ld->mMitHintergrundbildchen)
    ld->mHintergrundBild.setAsBackground(hintergrund_y);

  /*** Ganzen Hintergrund am Stueck malen, falls alles neu zu malen ist ***/
  if (mUpdateAlles) {
    updateSchrift = true;
    Area::fillRect(0, 0, gric*grx, gric*gry, ld->mHintergrundFarbe);

    if (ld->mMitHintergrundbildchen) {
      ld->mHintergrundBild.malBild(0,hintergrund_y);
    }
  }


  /* Oberstes y, in dem was upzudaten ist. Wird verwendet um zu bestimmen,
     ob der Hetzrand neu zu malen ist */
  int hoechstes_y = gric*(gry + 2);
  

  /*** Blops malen (nur die, die noetig sind) ***/
  for (int x = 0; x < grx; x++)
    for (int y = 0; y < mDaten.getGrY(); y++)// Evtl. auch Rüberreihe
      if (mDaten.getFeld(x, y).takeUpdaten() || mUpdateBlop[x][y] ||
          mUpdateAlles) {
        updateSchrift = true;
        mUpdateBlop[x][y] = false;
        int xx, yy;
        getFeldKoord(x, y, xx, yy);
	if (y < hoechstes_y) hoechstes_y = y;
	
	/* Bereich muss auf Bildschirm upgedatet werden */
        if (!mUpdateAlles) {
          SDL_Rect r = SDLTools::rect(xx, yy, gric, gric);
          spiegelRect(r);
	  Area::updateRect(r);
	}
	
	/* Blop malen */
	if (yy > mHetzrandYPix - gric) { // nicht unter Hetzrand versteckt?
	  if (!mUpdateAlles) {
	    /* Nicht alles updaten? Dann den Hintergrund unter jedem Blop
	       einzeln malen */
            SDL_Rect r = SDLTools::rect(xx, yy, gric, gric);
            spiegelRect(r);
            Area::fillRect(r, ld->mHintergrundFarbe);
	    if (ld->mMitHintergrundbildchen) {
              r.y -= hintergrund_y;
	      ld->mHintergrundBild.malBildAusschnitt(xx, yy, r);
	    }
	  }
          mDaten.getFeld(x, y).malen(xx, yy);
	}  // if blop nicht unter Hetzrand versteckt
      }

  /*** Fallendes malen ***/
  if (mFall.existiert()) {
    mFall.malen();
    /* Im Moment wird nicht drauf geprüft, ob das Fallende die Schrift
       kaputt gemacht haben könnte => Schrift muss auf jeden Fall neu
       gemalt werden, falls der Fall existiert. */
    updateSchrift = true;
  }
  
  /*** Hetzrand malen ***/
  /* Der nichtbild-Hetzrand */
  if (hoechstes_y < mHetzrandYPix) {
    SDL_Rect r = SDLTools::rect(0, 0, gric*grx, mHetzrandYPix);
    spiegelRect(r);
    Area::fillRect(r, ld->hetzrandFarbe);
    updateSchrift = true;
  }
  /* Der Bild-Hetzrand */
  if (ld->mMitHetzbildchen &&
      hoechstes_y < mHetzrandYPix + ld->mHetzrandUeberlapp) {
    SDL_Rect r = SDLTools::rect(0, 0, gric*grx, mHetzrandYPix);
    spiegelRect(r);
    Area::fillRect(r, ld->hetzrandFarbe);
    ld->mHetzBild.malBild(0,
		      ld->mSpiegeln ?
		      gric*gry - mHetzrandYPix - ld->mHetzrandUeberlapp :
		      mHetzrandYPix + ld->mHetzrandUeberlapp -
                      ld->mHetzBild.getHoehe()
		      );
    updateSchrift = true;
  }
  
  if (updateSchrift)
    malSchrift(true, mText, mTextX0, mTextX1);
  
  mUpdateAlles = false;
} // malUpdateSpielsituation



/** Malt die Schrift auf den Bildschirm; genauer: auf p.
    In der richtigen Farbe. */
void Spielfeld::malSchrift(bool mitte /*= true*/,
                           const FontStr & text,
			   int x0, int x1) {

  /* Wenn eh kein Text da ist, dann lieber gar nix mit Schriften
     machen; wer weiß, ob das langsam ist. */
  if (text == FontStr(""))
    return;
  
  int x = gric*grx / 2;
  int y = mitte ? gric * gry / 2 : gric * (gry - 2);
  
  TextAlign align = mitte ? AlignCenter : AlignBottom;

  Font * font = Font::gGame; // mTextKlein ? Font::gGame : Font::gGame;

  Area::setClip(SDLTools::rect(x0,0,x1-x0,gric*gry));
  font->drawText(text, x, y, align);
  Area::noClip();
}






/** Wenn der Level gespiegelt ist, wird auch das Rechteck gespiegelt. */
void Spielfeld::spiegelRect(SDL_Rect & r) {
  if (ld->mSpiegeln) {
    r.y = gry * gric - r.y - r.h;
  }
}



/* Liefert einen Blop, der dem Zeichen c in Startdist enstpricht.
   Wenn mitZufall != 0 ist, wird da reingeschrieben, ob Zufall bei der
   Blopsortenwahl beteiligt ist. */
Blop startDistBlop(const Str & keystr, bool * mitZufall = 0) {

  int key = ld->liesDistKey(keystr);

  if (mitZufall) *mitZufall = false;

  int wv = -1;
  
  if (key == distkey_farbe) wv=wv_farbe;
  else if (key == distkey_grau) wv=wv_grau;
  else if (key == distkey_gras) wv=wv_gras;

  if (wv!=-1) {
    if (mitZufall) *mitZufall = true;
    if (ld->mVerteilungSumme[wv]==0)
      /* TRANSLATORS: "startdist" is a programming keyword and should not
	 be translated. */
      throw Fehler(_("\"%s\" used in startdist, although all %s are 0."),
		   keystr.data(), cVerteilungsNamen[wv]);
    int sorte = ld->zufallsSorte(wv);
    return Blop(sorte);

  } else if (key == distkey_leer) {
    return Blop(blopart_keins);

  } else {
    /* Jetzt suchen wir die Sorte, deren distkey am knappsten unter key ist. */
    int i = blopart_min_sorte;
    int bestkey = -1;
    int besti = 0; /* Just to avoid a warning below about besti being
		      uninitialized. */
    for (; i<ld->mAnzFarben; i++) {
      int distkey = ld->mSorten[i]->getDistKey();
      if (distkey!=distkey_undef && distkey<=key && bestkey<distkey) {
	bestkey = distkey;
	besti = i;
      }
    }
    if (bestkey==-1)
      /* TRANSLATORS: "distkey" is a programming keyword and should not
	 be translated. */
      throw Fehler(_("\"%s\" used as a distkey but no such distkey specified."),
		   keystr.data());
    return Blop(besti,key-bestkey);
    
  }
}


/* Liefert zurueck, zu wie vielen Nachbarblops sich Blop b
   an Position x,y verbinden wuerde.
   Wird von createStartDist() verwendet. */
int Spielfeld::blopVerbindungen(const Blop & b, int x, int y) {

  int n = b.getVariable(spezvar_kind);
  int ret = 0;
  
  for (NachbarIterator ni(ld->mSorten[n], x, y); ni; ++ni)
    if (mDaten.koordOK(ni.mX, ni.mY)) {
      if (mDaten.getFeld(ni.mX, ni.mY).getVariable(spezvar_kind) == n)
        ret++;
    }
  return ret;
}



/* Startdist aus Leveldaten auslesen und ins Spielfeld schreiben */
void Spielfeld::createStartDist() {
  /* Etwas heuristisch dafür sorgen, dass keine gleichen Blops
     nebeneinander sind. Hier sind die Variablen dafür. */
  /* Welche Blops sollen noch nachbarentfremdet werden? */
  std::vector<int> entfremdenX, entfremdenY;
  std::vector<Str> entfremdenK;

  int len = ld->mDistKeyLen;  // nur etwas kürzer

  /* Zeilen durchgehen... */
  for (int i = 0; i < ld->mAnfangsZeilen->getLaenge(); i++) {
    int y = gry - ld->mAnfangsZeilen->getLaenge() + i;
    Str zeile = ld->mAnfangsZeilen->getDatum(i,type_WortDatum)->getWort();
    
    if (zeile.length() != grx*len && zeile.length() != 2*grx*len)
      /* TRANSLATORS: "distkey" are "startdist" are programming keywords
	 and should not be translated. */
      throw Fehler(_("All values for distkey have length %d, so lines in startdist must have length %d or %d"), len, grx*len, 2*grx*len);

    int verschiebung = ((zeile.length()>grx*len) && mRechterSpieler ? grx : 0);

    for (int x = 0; x < grx; x++) {
      Str key = zeile.mid((verschiebung+x)*len, len);
      bool mitZufall;
      mDaten.getFeld(x, y) = startDistBlop(key, &mitZufall);
      
      if (mitZufall) {
        /* Zufall an Blopsortenauswahl beteiligt? Dann auf wenig
	   Nachbarschaften optimieren */
	for (int i = 0; i < startlevel_rand_durchgaenge; i++) {
	  entfremdenX.push_back(x);
	  entfremdenY.push_back(y);
	  entfremdenK.push_back(key);
	}
      }
	
    }
  }
  
  /* Hier kommt das eigentliche Nachbarentfremden */
  while (!entfremdenX.empty()) {
    CASSERT(ld->mAnzFarben);
    /* Zufälliges Element aus dem pr-Array rausnehmen */
    int si = entfremdenX.size();
    int prnr = Aufnahme::rnd(si);
    int x = entfremdenX[prnr];
    int y = entfremdenY[prnr];
    Str key = entfremdenK[prnr];
    si--;
    entfremdenX[prnr] = entfremdenX[si];
    entfremdenY[prnr] = entfremdenY[si];
    entfremdenK[prnr] = entfremdenK[si];
    entfremdenX.resize(si);
    entfremdenY.resize(si);
    entfremdenK.resize(si);
    
    /* Ok, mal sehen, ob wir fuer den Blop bei x,y eine bessere
       Idee haben als das, was da bisher ist */
    Blop b_neu = startDistBlop(key);
    int verb_alt = blopVerbindungen(mDaten.getFeld(x, y), x, y);
    int verb_neu = blopVerbindungen(b_neu, x, y);
    
    /* Neuer Blop ist besser; also kommt der hin */
    if (verb_neu < verb_alt)
      mDaten.getFeld(x, y) = b_neu;
  }
}



/** Schaltet die Spielfeld-Anzeige aus, damit "Loading level"
    angezeigt werden kann... */
void Spielfeld::ladeLevelModus() {
  setUpdateAlles();
  mModus = modus_lade;
}




/** Danach muss noch einmal animiere() aufgerufen werden, damit alle
    Blops wissen, wie sie aussehen, und damit die Grafik gemalt wird. */
void Spielfeld::startLevel() {

  setUpdateAlles();

  /* Startmodus: Graue dürfen kommen. Da noch keine da sind, kommt ein neues
     Fall */
  mModus = modus_neue_graue;
  mKettenreaktion = false;
	
  mRueberReihenModus = rrmodus_nix;
  mWillHartnaeckigReihe = false;
  mHochVerschiebung = 0; // Bild unverschoben
  mGrauAnz = 0; // Anzahl der Grauen, die auf ihr Kommen warten

  mMessageText = "";
  mMessageZeit = 0;
  
  /* Während der Spielfeld-Erzeugung werden ein Haufen Blops erzeugt und
     wieder vernichtet. Die sollen nicht alle gleich ein Init-Event
     bekommen. */
  //Blop::setInitEventsAutomatisch(false);

  /* Spielfeld leeren. */
  mDaten.init();
		
  /* Kein Fallendes */
  mFall.initialisiere();
  mFall.zerstoere();

  mSemiglobal = Blop(blopart_semiglobal);

  /* Startdist auslesen und ins Spielfeld schreiben */
  createStartDist();
  
  /* Levelzeit... */
  mZeit = 0;
  mHetzrandYPix = 0;
}



/** Spiel abbrechen (sofort, ohne Animation; oder die
    Animation ist schon vorbei). */
void Spielfeld::stopLevel() {

  setUpdateAlles();

  mModus = modus_keinspiel;
}


/** Einmal pro Schritt aufrufen; kümmert sich ggf. um blinkendes
    Message */
void Spielfeld::blinkeMessage() {
  /* Ggf. Message-Text blinken lassen */
  if (mMessageZeit > 0) {
    mMessageZeit--;
    int x0,x1;
    if (message_gesamtzeit-mMessageZeit<message_aufabdeckzeit) {
      x0=0;
      x1=(message_gesamtzeit-mMessageZeit)*gric*grx/message_aufabdeckzeit;
    } else if (mMessageZeit<message_aufabdeckzeit) {
      x0=(message_aufabdeckzeit-mMessageZeit)*gric*grx/message_aufabdeckzeit;
      x1=gric*grx;
    } else {x0=0; x1=gric*grx;};
    setText(mMessageText,false,x0,x1);
  }
}


/** Sorgt dafür, dass bei Spielende bei geeigneter Gelegenheit auch in diesem
    Spielfeld das Spiel beendet wird */
void Spielfeld::testeSpielende() {
  if (mModus != modus_testflopp) return;

  if (!Cuyo::getSpielLaeuft()) {
    /* Evtl. Fall platzen lassen */
    if (mFall.existiert()) {
      mFall.lassPlatzen();
      mModus = modus_fallplatz;
    } else
      mModus = modus_warte_auf_stop;
  }
}


/** Zusammenhangskomponenten bestimmen und ggf. Explosionen auslösen */
void Spielfeld::testeFlopp() {
  if (mModus != modus_testflopp) return;

  /* Hat jemand einen Platztest bestellt? (Allerdings mit dem Platzen noch
     warten, falls das Fall grade am zerfallen ist.)...
     Änderung: Es wird nicht mehr geschaut, ob jemand einen Platztest
     bestellt hat. Etwas ineffektiver. */
  if (!mFall.istEinzel()) {

    /* Platzt was? (calcFlopp sendet ggf. auch die Grauen an den Mitspieler) */
    if (calcFlopp()) {
      mModus = modus_flopp;
      Sound::playSample(sample_explodier,
			mRechterSpieler ? so_rsemi : so_lsemi);
    }
  }
}


/** Um Fall kümmern. (Aber nicht darum, neues Fall zu erzeugen). */
void Spielfeld::fallSchritt() {
  /* Um Fall kümmern. Macht auch ggf. notwendig gewordene
     Verwandlungen. */
  mFall.spielSchritt();
  /* Für den KIPlayer: Jetzt ist das Fall nicht mehr neu. */
  mFallIstNeu = false;
}


/** Ein Schritt vom Spiel.
    Animationen wird *nicht* gemacht. Dazu muss animiere() aufgerufen werden.
    (Weil alle Animationen innerhalb einer eigenen Gleichzeit
    stattfinden sollen.)
    spielSchritt() sollte innerhalb einer Gleichzeit aufgerufen
    werden für evtl. auftretende Events. */
void Spielfeld::spielSchritt() {

  /* Immernoch in Modus testflopp? Dann wollte wohl nix explodieren. Also
     wieder in einen gravitations-Modus wechseln. */
     
  if (mModus == modus_testflopp) {
    /* Keine Sofort-Explosion, also keine Kettenreaktion */
    mKettenreaktion = false;
			
    /* Wenn ein Fall existiert, dürfen grad keine Grauen nachkommen. Sonst
       schon. */
    if (mFall.existiert())
      mModus = modus_rutschnach;
    else
      mModus = modus_neue_graue;			
  }  // Ende von mModus == modus_testflopp

	
  switch (mModus) {
  					
  case modus_flopp: // etwas ist am explodieren
  	
    /* Einfach nur warten, bis die Blops fertig explodiert sind... */	 	
    if (!mDaten.getWasAmPlatzen()) {
      /* Ja, fertig explodiert. */

      /* Jetzt haben wir es uns verdient, vielleicht eine Reihe zu
         bekommen. */
      bekommVielleichtReihe();
    
      /* Wenn wir wieder in Modus testflopp kommen (und sofort eine
	 Explosion auftaucht), ist es eine Kettenreaktion. */
      mModus = modus_rutschnach;
      mKettenreaktion = true;
    }
    break;
  		
  case modus_rutschnach: {
    /* Steine runterrutschen lassen. */
    bool passiert_was = rutschNach(false) != rutschnach_nix;
  		
    /* Ist jetzt (in diesem Schritt) wirklich was passiert? */
    if (!passiert_was) {
      /* Nein. Das Nachrutschen ist also fertig. */
  			
      /* Jetzt: Nächste Explosion? */
      mModus = modus_testflopp;
    }		// if (es ist nix mehr nachgerutscht)
    break;
  }
  case modus_neue_graue: {
    /* Graue Steine runterrutschen lassen. */
    int rn = rutschNach(true);
  		
    /* Die Grauen sind weit genug unten, um ein neues Fall loszuschicken */
    if (rn != rutschnach_viel) {

      /* Aber nur, wenn das Spiel noch läuft... */
      if (Cuyo::getSpielLaeuft()) {
  		
	if (mFall.erzeug()) {
        
          /* Für den KIPlayer abpeichern, dass ein neues Fall kommt. */
          mFallIstNeu = true;
          
	  /* OK, neues Fall unterwegs. Sind überhaupt noch Graue
             unterwegs? Wenn ja, dann weiter im Gravitationsmodus;
             sonst auf Explosionen testen. */
	  if (rn == rutschnach_wenig)
	    mModus = modus_rutschnach;
	  else
	    mModus = modus_testflopp;
	} else {
	  /* Kein Platz mehr für neuen Stein => tot */
          Cuyo::spielerTot();
	  CASSERT(!Cuyo::getSpielLaeuft());
	  /* Allerdings können wenigstens die Grauen noch fertig runterfallen.
	     Also sind wir noch nicht bereit zum stoppen. */
	}
      } else {
	/* Spiel läuft eigentlich gar nicht mehr. Wenn die Grauen ganz auf dem
	   Boden angekommen sind, dann sind wir auch sterbebereit */
	if (rn == rutschnach_nix)
	  mModus = modus_warte_auf_stop;
      }
    }
    
    break;
  }
  	  			  	
  case modus_fallplatz: // Steine, die am runterfallen waren, zerplatzen 
    // in der Luft, weil das Spiel zu Ende ist
  	
    /* fertig explodiert? */
    if (!mFall.getAmPlatzen()) {
      /* Wir sind jetzt mit der Animation fertig und warten auf spielStop() */
      mFall.zerstoere();
      mModus = modus_warte_auf_stop;
    }
  		
    break;
  		
  case modus_warte_auf_stop:
    /* Wir warten nur darauf, dass wir ein stopLevel() bekommen */
    break;
  default:
    CASSERT(false);
  } // switch mModus


  /* ggf. Spiel für dieses Spielfeld beenden */
  testeSpielende();

}   // spielSchritt()






/** Führt alle Animationen durch.
    Sollte innerhalb einer Gleichzeit aufgerufen werden. */
void Spielfeld::animiere() {

  //CASSERT(gGleichZeit);
  /* Die ganzen festliegenden Blops animieren */
  mDaten.animiere();
  /* Fallendes animieren */
  mFall.animiere();
  mSemiglobal.animiere();
}




/** Liefert die Koordinaten eines Felds in Pixeln zurück (ungespiegelt) */
void Spielfeld::getFeldKoord(int x, int y, int & xx, int & yy) const {
  xx = x * gric;
  yy = y * gric - mHochVerschiebung - (ld->mSechseck && (x & 1)) * gric / 2;
}




/** Führt eine der nachfolgenden Tasten-Routinen aus.
    (t = taste_*). */
void Spielfeld::taste(int t) {
  switch (t) {
    case taste_links: tasteLinks(); break;
    case taste_rechts: tasteRechts(); break;
    case taste_dreh: tasteDreh(); break;
    case taste_fall: tasteFall(); break;    
    default: CASSERT(false);
  }
}


/** Bewegt das Fall eins nach links */
void Spielfeld::tasteLinks() {
  Blop::beginGleichzeitig();
  mFall.tasteLinks();
  mSemiglobal.execEvent(event_keyleft);
  Blop::endGleichzeitig();
  mFall.playSample(sample_links);
}

/** Bewegt das Fall eins nach rechts */
void Spielfeld::tasteRechts() {
  Blop::beginGleichzeitig();
  mFall.tasteRechts();
  mSemiglobal.execEvent(event_keyright);
  Blop::endGleichzeitig();
  mFall.playSample(sample_rechts);
}

/** Dreht das Fall */
void Spielfeld::tasteDreh() {
  Blop::beginGleichzeitig();
  mFall.tasteDreh1();
  mSemiglobal.execEvent(event_keyturn);
  Blop::endGleichzeitig();
  mFall.tasteDreh2();
  mFall.playSample(sample_dreh);
}

/** Ändert die Fallgeschwindigkeit vom Fall */
void Spielfeld::tasteFall() {
  Blop::beginGleichzeitig();
  mFall.tasteFall();
  mSemiglobal.execEvent(event_keyfall);
  Blop::endGleichzeitig();
  mFall.playSample(sample_fall);
}




/** berechnet, ob und welche Blops platzen müssen. Außerdem
    werden den Blops die Kettengrößen mitgeteilt, und Graue
    und Punkte verteilt */
bool Spielfeld::calcFlopp() {
  int x, y;
  bool wasPassiert = false;
  /* Die Punkte werden nachher nur gesendet, wenn wirklich was
     explodiert ist. */
  int punkte = mKettenreaktion ? punkte_fuer_kettenreaktion : 0;
  
  /* Event an Blops senden, damit sie ihre Verbindungsspecials
     ausführen können */
  mDaten.sendeConnectEvent();
	
  /* Anzahl der Grauen für den anderen Spieler, wenn etwas platzt:
     Mindestens 1. Die minimal mögliche Blop-Zahl, die platzt,
     wird später abgezogen. Und zwar von denen die größte.
     Ggf. wird was für Kettenreaktionen addiert */
  int grz = 1;
  if (mKettenreaktion) grz += graue_bei_kettenreaktion;
  /* Der größte PlatzAnzahl-Wert unter den Sorten, die jetzt platzen */
  int maxPlatzAnzahl = 0;

  /* Array für Zusammenhangskomponentensuche */
  int flopp[grx][gry];
	
  /* Erst mal muss nix platzen */
  for (x = 0; x < grx; x++)
    for (y = 0; y < gry; y++)
      flopp[x][y] = 0;
			
  /* Spielfeld absuchen */
  for (x = 0; x < grx; x++)
    for (y = 0; y < gry; y++)
      /* Blop, der noch nicht am platzen ist
         und seine Kettengröße wissen will? */
      if (mDaten.getFeld(x, y).getVerhalten(berechne_kettengroesse) &&
	  !mDaten.getFeld(x, y).getAmPlatzen()) {
        int kind = mDaten.getFeld(x,y).getVariable(spezvar_kind);
				/* Dann Kettengröße berechnen,... */
	int anz = calcFloppRec(flopp, x, y, kind, 1);

        /* ... maxPlatzAnzahl für grz aktualisieren ... */
	int PlatzAnzahl = ld
          -> mSorten[mDaten.getFeld(x,y).getVariable(spezvar_kind)]
	  -> getPlatzAnzahl();
        bool platzen = mDaten.getFeld(x,y).getVerhalten(platzt_bei_gewicht)
          && (anz>=PlatzAnzahl);
	if (platzen && (PlatzAnzahl > maxPlatzAnzahl))
	  maxPlatzAnzahl = PlatzAnzahl;

	/* ... und allen Blops in der Kette mitteilen. */
	int neue_pt = calcFloppRec(flopp, x, y, kind,
				   2, /* Größe mitteilen... */
				   platzen, mKettenreaktion,
				   anz);
	CASSERT(platzen  ||  neue_pt == 0);
				
	/* Wenn's Punkte gab, ist wohl auch was geplatzt */
	if (neue_pt > 0) {
	  punkte += neue_pt;
	  wasPassiert = true;
	  grz += anz;
	}	
      }

  /* Jetzt kennen wir maxPlatzAnzahl endgültig, jetzt lassens wir's wirken */
  grz -= maxPlatzAnzahl;

  /* Ist noch was explodiert? */
  if (wasPassiert) {
    /* Dann sende graue */
    Cuyo::sendeGraue(mRechterSpieler, grz);
    /* und bekomme Punkte */		
    Cuyo::neuePunkte(mRechterSpieler, punkte);
  }
	
  return wasPassiert;
} // calcFlopp()


int Spielfeld::calcFloppRec(int flopp[grx][gry], int x, int y,
			    int n, int w, bool platzen /*= false*/,
			    bool ist_kettenreaktion /*= false*/,
                            int anz /*= 0*/) {
  CASSERT(x >= 0 && x < grx && y >= 0 && y < gry);
			    
		
  Blop & b = mDaten.getFeld(x, y);

  if (b.getVariable(spezvar_kind)==n) {
    if (flopp[x][y] & w) // Hatten wir dieses Feld schon?
      return 0;
    int ret = 0;
    flopp[x][y] |= w;
    if (w == 2) {
      b.setKettenGroesse(anz);
      if (platzen) {
	b.lassPlatzen();
	if (b.getVerhalten(verhindert_gewinnen))
	  ret = punkte_fuer_gras;
	else
  	  ret = punkte_fuer_normales;
      }
    } else {
      ret = b.getKettenBeitrag();
    }
    
    /* Rekursiv weiteraufrufen: */
    for (NachbarIterator i(ld->mSorten[n], x, y); i; ++i)
      if (i.mX >= 0 && i.mX < grx && i.mY >= 0 && i.mY < gry) {
        /* Nicht koordOk() verwenden, weil das die Rüberreihe erlaubt, wenn
	   sie existiert (bei y == gry) */
	   
	/* Noch checken, ob einer der Blops ein "inhibit" für diese Verbindung
	   gesetzt hat. */
	if ((b.getVariable(spezvar_inhibit) & i.mDir) == 0 &&
  	    (mDaten.getFeld(i.mX, i.mY).getVariable(spezvar_inhibit) & i.mDirOpp) == 0)
          ret += calcFloppRec(flopp, i.mX, i.mY, n, w, platzen,
			      ist_kettenreaktion, anz);
      }
      
    return ret;
  } else if ((w==2) && platzen && (b.getVerhalten(platzt_bei_platzen)
       || (ist_kettenreaktion && b.getVerhalten(platzt_bei_kettenreaktion)))) {
    if (flopp[x][y] & 4) // Hatten wir dieses Feld schon?
      return 0;
    flopp[x][y] |= 4;
    b.lassPlatzen();
    return (b.getVerhalten(verhindert_gewinnen)
      ? punkte_fuer_gras : punkte_fuer_graues);
  } else return 0;

  CASSERT(0);
  return 0;  // Um keine Warnung zu bekommen
}







/** Graue von anderem Spieler bekommen; wird ignoriert, falls dieser
    Spieler grad nicht spielt */
void Spielfeld::empfangeGraue(int g){
  if (mModus == modus_keinspiel)
    return;
  for (int i=0; i<g; i++)
    if (Aufnahme::rnd(ld->mVerteilungSumme[wv_grau] + ld->mKeineGrauenW)
	 < ld->mVerteilungSumme[wv_grau])
      mGrauAnz++;
}




/** liefert die Höhe vom höchsten Türmchen... unter der Annahme, dass
		er schon komplett zusammengesackt ist. */
int Spielfeld::getHoehe(){
  int h = 0;
  for (int x = 0; x < grx; x++) {
    int nh = 0;
    for (int y = 0; y < gry; y++)
      nh += mDaten.getFeldArt(x, y) != blopart_keins;
    if (nh > h) h = nh;
  }
	
  return h;		
}










/** sollte nur aufgerufen werden, wenn Cuyo::getSpielLaeuft()
    false liefert; liefert true, wenn alle Spiel-Stop-
    Animationen fertig sind; liefert übrigens auch true,
    wenn dieser Spieler gar nicht mitspielt */
bool Spielfeld::bereitZumStoppen(){
  if (mModus == modus_keinspiel || mModus == modus_warte_auf_stop)
    return true;
  else
    return false;
}



/** Zeigt t groß an. (Oder weniger groß.) */
void Spielfeld::setText(const Str & t_, bool kleine_schrift /*= false*/,
			int x0 /* =0 */, int x1 /* =gric*grx */) {
  FontStr t = t_;
  if ((! (mText == t)) || mTextKlein != kleine_schrift
      || x0!=mTextX0 || x1!=mTextX1) {
    mText = t;
    mTextKlein = kleine_schrift;
    mTextX0=x0;
    mTextX1=x1;

    Font * font = Font::gGame; //mTextKlein ? Font::gGame : Font::gGame;
    font->wordBreak(mText, gric*grx);
    
    setUpdateAlles();
  }
}


/** Für während des Spiels: Setzt einen Text, der ein paar mal
    aufblinkt. */
void Spielfeld::setMessage(Str mess) {
  mMessageText = mess;
  mMessageZeit = message_gesamtzeit;
}



/** Setzt das Rechteck x, y, w, h auf upzudaten. */
void Spielfeld::setUpdateRect(int x, int y, int w, int h) {
  for (int blopx = 0; blopx < grx; blopx++)
    for (int blopy = 0; blopy <= gry; blopy++) {
      int xx, yy;
      getFeldKoord(blopx, blopy, xx, yy);
      if (xx > x - gric && yy > y - gric &&
          xx < x + w && yy < y + h)
	mUpdateBlop[blopx][blopy] = true;
    }
}


/** Setzt alles auf upzudaten. */
void Spielfeld::setUpdateAlles() {
  mUpdateAlles = true;
}



/** Lässt den Hetzrand schnell runterkommen (für die
		Zeitbonus-Animation). Liefert true, wenn fertig. */
bool Spielfeld::bonusSchritt() {

  CASSERT(mModus != modus_keinspiel);

  /* Bis wohin soll der Hetzrand runtergehen? */
  int unten = gric * gry - ld->mHetzrandStop;

  /* Hetzrand runterkommen lassen */
  int ny = mHetzrandYPix + bonus_geschwindigkeit;
  if (ny > unten) ny = unten;

  setHetzrandYPix(ny);
  
  /* Hetzrand unten angekommen? */
  return ny == unten;
}


/** Liefert true, wenn grade ein Fallendes unterwegs ist.
    Wird vom KIPlayer benötigt */
int Spielfeld::getFallModus() const {
  //return mModus == modus_fall;
  if (mFall.existiert())
    if (mFallIstNeu)
      return fallmodus_neu;
    else
      return fallmodus_unterwegs;
  else
    return fallmodus_keins;
}

/** Liefert true, wenn noch Gras da ist (d. h. wenn nicht gewonnen.) */
bool Spielfeld::istGrasDa() const {
  /* Jedes Mal alles durchsuchen: ineffektiv. Aber wenigstens von unten
     nach oben, weil das Gras ja eher unten liegt... */
  for (int y = mDaten.getGrY() - 1; y >= 0; y--)
    for (int x = 0; x < grx; x++)
      if (mDaten.getFeld(x, y).getVerhalten(verhindert_gewinnen))
        return true;
  return false;
}


/** Liefert einen Pointer auf das Blopgitter zurück. */
/*const*/ BlopGitter * Spielfeld::getDatenPtr() /*const*/ {
  return &mDaten;
}

/** Liefert einen Pointer auf die fallenden Blops zurück.
    Wird von KIPlayer einmal am Anfang aufgerufen
    und gelegentlich bei @0 und  @1. */
const Blop * Spielfeld::getFall() const {
  return mFall.getBlop();
}

Blop * Spielfeld::getFall() {
  return mFall.getBlop();
}

int Spielfeld::getFallAnz() const {
  return mFall.getAnz();
}

void Spielfeld::resetFall() {
  if (mFall.existiert(1)) mFall.erzeug();
}

Blop & Spielfeld::getSemiglobal() {
  return mSemiglobal;
}


/** Liefert die Pos. zurück, an der neue Dinge oben
    auftauchen. */
int Spielfeld::getHetzrandYAuftauch() const{
  /* Siehe def. der Konstante... */
  return (mHetzrandYPix + hetzrand_dy_auftauch) / gric;
}

/** Liefert die Pos. zurück, bis wohin noch Dinge liegen
    dürfen, ohne dass man tot ist. */
int Spielfeld::getHetzrandYErlaubt() const{
  /* Siehe def. der Konstante... */
  return (mHetzrandYPix + hetzrand_dy_erlaubt) / gric;
}


 /** Ändert die Höhe vom Hetzrand auf y (in Pixeln). */
void Spielfeld::setHetzrandYPix(int y) { 	 
  if (y == mHetzrandYPix) 	 
    return; 	 

  int vy = mHetzrandYPix; 	 
  mHetzrandYPix = y; 	 

  /* War in einer alten Version wichtig:
     setUpdateRect darf erst aufgerufen werden, 	 
     _nachdem_ mHetzrandYPix  geändert wurde (weil sonst 	 
     der Hetzrand evtl. noch gar nicht in dem 	 
     übergebenen Rechteck drin liegt und deshalb kein 	 
     frisch-malen-Flag kriegt. */ 	 
  if (ld->mMitHetzbildchen) 	 
    setUpdateRect(0, vy - ld->mHetzBild.getHoehe() + ld->mHetzrandUeberlapp,
                  gric * grx, ld->mHetzBild.getHoehe() + y - vy); 	 
  else 	 
    setUpdateRect(0, vy, gric * grx, y - vy); 	 
} 	 
 

/** Bewegt den Hetzrand eins nach unten. Testet auch, ob dabei
    was überdeckt wird. */
void Spielfeld::bewegeHetzrand(){
  /* Hat der Hetzrand Steine überdeckt? (Bräuchte eigentlich
     nur getestet werden, wenn sich
     getHetzrandYErlaubt() seit dem letzten Check erhöht hat...) */
  int hye = getHetzrandYErlaubt();
  if (hye > 0) {
    for (int x = 0; x < grx; x++)
      if (mDaten.getFeldArt(x, hye - 1) != blopart_keins) {
	/* Hetzrand überdeckt Stein => tot */
	Cuyo::spielerTot();
	CASSERT(!Cuyo::getSpielLaeuft());
	/* Evtl. muss auch bei uns noch eine Animation fertig
	   ablaufen; deshalb noch nicht gleich den mModus auf
	   modus_warte_auf_stop setzen, sondern
	   einfach die Dinge laufen lassen */
	break;
      }
  }
  /* Levelzeit hochzählen, Hetzrand kommen lassen...; das darf
     erst geschehen,  nachdem getestet wurde, ob was überdeckt
     wurde, weil vielleicht vorher grade
     was in der Höhe aufgetaucht ist, wo es jetzt überdeckt würde */
  mZeit++;
  setHetzrandYPix(mZeit / ld->hetzrandZeit);
}
/** Sendet sich selbst (ggf.) zufällige Graue */
void Spielfeld::zufallsGraue(){
  /* Ggf. sich selbst Graue schicken */
  if (ld->mZufallsGraue != zufallsgraue_keine)
    if (Aufnahme::rnd(ld->mZufallsGraue) == 0)
      empfangeGraue(1);
}

/** Lässt in der Luft hängende Blops ein Stück runterfallen.
    Liefert zurück, ob sich nichts bewegt hat, nur unten oder auch oben.
    Bei auchGraue = true, kommen auch Graue, die ganz über
    dem Spielfeld hängen. */
int Spielfeld::rutschNach(bool auchGraue){
  int ret = rutschnach_nix;
		
  /* Anzahl der Spalten, in denen Platz für ein neues Graues ist */
  int grauplatz = 0;

  /* Zeile, in der neue Graue auftauchen */
  int hya = getHetzrandYAuftauch();
			
  /* Steine nachrutschen lassen */
  for (int x = 0; x < grx; x++) {

    /* Zunächst mal die Steine, die schon im Spielfeld sind, runterrutschen
       lassen; die können übrigens auch noch leicht über dem Hetzrand sein
       (also sicherheitshalber bis ganz oben gehen) */
    bool unten_platz = false;
    for (int y = gry-1; y>=0; y--) {
      if (unten_platz && !mDaten.getFeldVerhalten(x,y,schwebt)) {
        mDaten.getFeld(x, y + 1) = mDaten.getFeld(x, y);
        mDaten.getFeld(x, y) = Blop(blopart_keins);
        if (y >= hya + neues_fall_platz)
          ret |= rutschnach_wenig;
        else
          ret |= rutschnach_viel;
      }
      unten_platz=mDaten.getFeldArt(x,y)==blopart_keins;
    }

    /* Ist in dieser Spalte Platz für ein Graues? */
    if (mDaten.getFeld(x, hya).getArt() == blopart_keins)
      grauplatz++;
  } // for x

  /* Dürfen neue Graue kommen? */
  if (auchGraue) {
    for (int x = 0; x < grx; x++) {
      /* Ist in Grauauftauchhöhe der Spalte Platz? (Achtung:
	 Spalte voll ist nicht der einzige Grund für keinen
	 Grauplatz. Der Hetzrand könnte auch
	 grade ein Feld runtergekommen sein.) */
      if (mDaten.getFeldArt(x, hya) == blopart_keins) {
	CASSERT(grauplatz > 0);
	if (Aufnahme::rnd(grauplatz) < mGrauAnz) {
	  mDaten.getFeld(x, hya) = Blop(ld->zufallsSorte(wv_grau));
	  ret |= rutschnach_viel;
	  mGrauAnz--;
	}
	grauplatz--;
      }
    }
  }
	
  return ret;
}
/** Liefert die Pos. vom Hetzrand in Pixeln zurück. Wird
    vom Fall gebraucht. */
int Spielfeld::getHetzrandYPix() const {
  return mHetzrandYPix;
}




/** Kümmert sich um hin- und hergeben von Reihen. */
void Spielfeld::rueberReihenSchritt() {
  switch (mRueberReihenModus) {
  case rrmodus_nix:
    break;
			
  case rrmodus_gib_rueber:
    /* Nichts tun. Nur warten, bis uns der andere die Reihe ganz
       weggenommen hat. */
    if (mRestRueberReihe == 0)
      mRueberReihenModus = rrmodus_gib_runter;

    /*
    if (mRechterSpieler) {
      if (mDaten.getFeldArt(0, gry) == blopart_keins)
	mRueberReihenModus = rrmodus_gib_runter;
    } else {
      if (mDaten.getFeldArt(grx - 1, gry) == blopart_keins)
	mRueberReihenModus = rrmodus_gib_runter;
	}*/
  		
    break;
  case rrmodus_gib_runter:
    /* alles zum updaten markieren */
    setUpdateAlles();
  		
    /* Bild verschieben */
    mHochVerschiebung -= reihe_rueber_senkrecht_pixel;
  	
    /* Weit genug verschoben? */
    if (mHochVerschiebung <= 0) {
      mHochVerschiebung = 0;
  	  	
      /* Rueberreihe vernichten */
      mDaten.setRueberReihe(false);
  	  	
      /* und fertig */
      mRueberReihenModus = rrmodus_nix;
    }
    break;
  case rrmodus_bekomm_hoch:
    /* alles zum updaten markieren */
    setUpdateAlles();
	
    /* Bild verschieben */
    mHochVerschiebung += reihe_rueber_senkrecht_pixel;
  	
    /* Weit genug verschoben? */
    if (mHochVerschiebung >= gric) {
      mHochVerschiebung = gric;
  	  	  	  	
      mRueberReihenModus = rrmodus_bekomm_rueber;
      mRestRueberReihe = grx;
    }
    break;
			
  case rrmodus_bekomm_rueber:
    /* Neuen Stein vom anderen Spieler bestellen */
    Blop neuer;
    Cuyo::willStein(mRechterSpieler, neuer);
    
    /* Von welcher Seite kommt die Reihe? */
    if (mRechterSpieler) {
      /* von links: Reihe nach rechts schieben */
      for (int x = grx - 1; x > 0; x--)
        mDaten.getFeld(x, gry) = mDaten.getFeld(x - 1, gry);
      mDaten.getFeld(0, gry) = neuer;

      /* Seitenwechsel-Event */
      //Blop::beginGleichzeitig();
      mDaten.getFeld(0, gry).execEvent(event_changeside);
      //Blop::endGleichzeitig();
      		
    } else {
      /* von rechts: Reihe nach links schieben */
      for (int x = 0; x < grx - 1; x++)
        mDaten.getFeld(x, gry) = mDaten.getFeld(x + 1, gry);
      mDaten.getFeld(grx - 1, gry) = neuer;

      /* Seitenwechsel-Event */
      //Blop::beginGleichzeitig();
      mDaten.getFeld(grx - 1, gry).execEvent(event_changeside);
      //Blop::endGleichzeitig();
    }

    mRestRueberReihe--;

    /* Fertig? */
    if (mRestRueberReihe == 0) {
      mRueberReihenModus = rrmodus_nix;

      mSemiglobal.execEvent(event_row_up);

      /* Gleich sagen alle Blops einzeln, dass sie geupdatet werden muessen,
	 aber alles auf einmal updaten ist effizienter */
      setUpdateAlles();

      /* Im Datenarray alles nach oben schieben. */
      for (int x = 0; x < grx; x++)
	for (int y = 0; y < gry; y++)
	  mDaten.getFeld(x, y) = mDaten.getFeld(x, y + 1);
      /* In der Rüberreihe sind noch alte Restblops; aber die Rüberreihe
         wird jetzt eh deaktiviert */
      mDaten.setRueberReihe(false);
      /* Optisch soll sich nix ändern; jetzt ist das Spielfeld wieder in
	 Normalposition */
      mHochVerschiebung = 0;

      /* Vielleicht sollten wir gleich noch eine Reihe bekommen?
         Mal nachschauen. Allerdings ist der andere Spieler noch mit
	 der letzten Reihe beschäftigt. Deshalb warten wir hartnäckig
	 so lange, bis der sich richtig entschieden hat, ob er uns eine
	 Reihe gibt oder nicht. */
      mWillHartnaeckigReihe = true;
    }
    break;
  }    // switch (mRueberReihenModus)
  
  /* Wir haben irgend wann beschlossen, so lange beim anderen Spieler
     nach einer Reihe zu fragen, bis der weiß, ob er uns eine gibt: */
  if (mWillHartnaeckigReihe) {
    /* bekommVielleichtReihe() liefert true, wenn es sich lohnt, im
       nächsten Schritt nochmal nachzufragen. */
    if (!bekommVielleichtReihe())
      mWillHartnaeckigReihe = false;
  }
}

/** liefert zurück, ob wir dem anderen
    Spieler eine Reihe geben (er hat Höhe h); Antwort ist
    eine der Konstanten bewege_reihe_xxx */
int Spielfeld::bitteUmReihe(int h) {

  /* Wenn wir nicht mitspielen, geben wir natürlich auch keine Reihe her */	
  if (mModus == modus_keinspiel)
    return bewege_reihe_nein;
	
  /* Wenn das Spiel zu Ende gehen soll, keine Reihe mehr rübergeben */
  if (!Cuyo::getSpielLaeuft())
    return bewege_reihe_nein;
		
  /* Wenn wir schon irgendwie mit einer Reihe beschäftigt sind, soll der
     andere sich erst noch ein bisschen gedulden. */
  if (mRueberReihenModus != rrmodus_nix)
    return bewege_reihe_moment;
		
  /* Sind wir hinreichend höher als der andere? Oder testen wir im
     Debug-Modus das Reihen hin und her schieben? */
  if (getHoehe() < h + 2 && !Cuyo::getRueberReihenTest())
    return bewege_reihe_nein;
		
  /* Ist unsere unterste Reihe (komplett) verfügbar? */
  for (int x = 0; x < grx; x++)
    if (mDaten.getFeldArt(x, gry - 1) == blopart_keins ||
	mDaten.getFeld(x, gry - 1).getAmPlatzen())
      return bewege_reihe_nein;
	
  /* Wenn wir dem anderen Spieler eine Reihe geben wollen, dann
     machen wir uns auch mal dafür bereit */

  mRueberReihenModus = rrmodus_gib_rueber;
  mRestRueberReihe = grx;

  mSemiglobal.execEvent(event_row_down);

  /* Gleich sagen alle Blops einzeln, dass sie geupdatet werden muessen,
     aber alles auf einmal updaten ist effizienter */
  setUpdateAlles();

  /* Im Datenarray alles nach unten schieben. */
  mDaten.setRueberReihe(true);
  for (int x = 0; x < grx; x++) {
    for (int y = gry; y > 0; y--)
      mDaten.getFeld(x, y) = mDaten.getFeld(x, y - 1);
    /* Oben eine Reihe leer einfügen. */
    mDaten.getFeld(x, 0) = Blop(blopart_keins);
  }
  /* Optisch soll sich noch nix ändern... */
  mHochVerschiebung = gric;

  /* Hier fehlt noch ein Signal an die Steine der neuen untersten Reihe,
     um ihnen mitzuteilen, dass ihre Zshgskomponente kleiner geworden ist. */

  /* Übrigens: Das Reihe-Rübergeben fängt (für uns)
     noch nicht gleich an; erst, wenn der andere
     sein Zeug hochgeschoben hat. */

  return bewege_reihe_ja;
}


/** gibt einen Stein an den anderen Spieler
    rüber; Blop wird in s zurückgeliefert */
void Spielfeld::gebStein(Blop & s){
  CASSERT(mRueberReihenModus == rrmodus_gib_rueber);
  CASSERT(mRestRueberReihe > 0);
	
  if (mRechterSpieler) {
    /* nach links... */
    s = mDaten.getFeld(0, gry);
    for (int x = 0; x < grx - 1; x++)
      mDaten.getFeld(x, gry) = mDaten.getFeld(x + 1, gry);
    mDaten.getFeld(grx - 1, gry) = Blop(blopart_keins);
  } else {
    /* nach rechts... */
    s = mDaten.getFeld(grx - 1, gry);
    for (int x = grx - 1; x > 0; x--)
      mDaten.getFeld(x, gry) = mDaten.getFeld(x - 1, gry);
    mDaten.getFeld(0, gry) = Blop(blopart_keins);
  }

  mRestRueberReihe--;
}


/** Prüft, ob ein Reihenbekommen sinnvoll wäre und initiiert es ggf.
    (Unterhält sich auch mit dem anderen Spieler). Liefert true,
    wenn es jetzt grad nicht möglich war, eine Reihe zu bekommen,
    aber nur weil der andere Spieler noch damit beschäftigt war,
    von einer vorigen Reihe sein Spielfeld runterzuschieben. Unter
    manchen Umständen wird dann später nochmal probiert, eine
    Reihe zu bekommen. */
bool Spielfeld::bekommVielleichtReihe() {
  /* Können wir eine Reihe vertragen? */
  if (mRueberReihenModus != rrmodus_nix)
    return false;
    
  /* Dann mal beim anderen Spieler anfragen, ob wir eine bekommen */
  int erg = Cuyo::bitteUmReihe(mRechterSpieler, getHoehe());
  if (erg == bewege_reihe_ja) {
    mDaten.setRueberReihe(true);
    /* Die Rüberreihe löschen. (Da kann noch Müll von vorher drin sein. */
    for (int x = 0; x < grx; x++)
      mDaten.getFeld(x, gry) = Blop(blopart_keins);
    mRueberReihenModus = rrmodus_bekomm_hoch;
  }
  /* Wenn der andere Spieler grad noch mit einer alten Reihe beschäftigt
     ist, liefern wir true zurück, um es vielleicht etwas später nochmal
     zu probieren. */
  return erg == bewege_reihe_moment;
}



/** Liefert die (rüberreihenbedingte) Hochverschiebung
    des gesamten Spielfelds. Wird vom Fall benötigt (um
    seine Koordinaten in Feldern zu berechnen). */
int Spielfeld::getHochVerschiebung(){
  return mHochVerschiebung;
}


int Spielfeld::getZeit() const {return mZeit;}

