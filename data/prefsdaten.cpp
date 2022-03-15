/***************************************************************************
                          prefsdaten.cpp  -  description
                             -------------------
    begin                : Fri Jul 21 2000
    copyright            : (C) 2000 by Immi
    email                : cuyo@pcpool.mathematik.uni-freiburg.de

Modified 2003,2006,2008,2010,2011 by the cuyo developers

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
#include <cmath>

#include "stringzeug.h"
#include "prefsdaten.h"
#include "cuyointl.h"
#include "fehler.h"
#include "leveldaten.h"
#include "configdatei.h"
#include "global.h"

#include "sdltools.h"


namespace PrefsDaten {




/** Die Steuer-Tasten... */
SDLKey mTasten[2][4];
/** Welche Level wurden schon gewonnen: im 1-Spieler-Modus und im
    Mehr-Spieler-Modus */
std::set<Str> mGewonneneLevel[2];

/** Geschwindigkeit des KI-Players, so wie man's einstellen möchte */
int mKIGeschwLog;
/** Die Zahl, an der der KI-Player interessiert ist. */
double mKIGeschwLin;

bool mSound;




/** Lädt die Preferences aus wo-auch-immer-sie-abgespeichert-werden
in die Variablen. */
void liesPreferences();



void calcKILangsamLin() {
  /* 9 -> 1
     6 -> 2
     3 -> 4
     0 -> 8 */
  mKIGeschwLin = exp((9 - mKIGeschwLog) * log(2.0) / 3);
}






enum PrefsVersion {
  PV_first = 0,
  PV_SDLKeys = 1,
  PV_current = PV_SDLKeys
};




void init() {
  try {
    liesPreferences();
  } catch (Fehler f) {
    print_to_stderr(_("Error reading preferences file '.cuyo':\n"));
    print_to_stderr(f.getText()+"\n");
    exit(1);
  }
}


/** Returns true if level lnr has already been won. */
bool getLevelGewonnen(bool sp2, int lnr) {
  return mGewonneneLevel[sp2].find(ld->getIntLevelName(lnr))!=mGewonneneLevel[sp2].end();
}


/** sp2: true bei zweispielermodus */
void schreibGewonnenenLevel(bool sp2, int lnr) {
  Str intlena = ld->getIntLevelName(lnr);
  if (mGewonneneLevel[sp2].find(intlena)==mGewonneneLevel[sp2].end()) {
    /* Level war bisher noch nie gewonnen; also in Liste einfügen */
    mGewonneneLevel[sp2].insert(intlena);
    schreibPreferences();
  }
}



/** Liefert true, wenn die Taste k belegt ist, und speichert dann
    in sp und t ab, was die Taste tut. */
bool getTaste(SDLKey k, int & sp, int & t) {
  for (sp = 0; sp < 2; sp++)
    for (t = 0; t < taste_anz; t++)
      if (k == mTasten[sp][t])
        return true;
  return false;
}





/** Liefert den Namen und Pfad der Prefs-Datei zurück
    ($HOME/.cuyo) */
Str getPrefsName() {
  char * ho = getenv("HOME");
  if (!ho) {
    /* Unter Windows zum Beispiel... */
    // TRANSLATORS: ".cuyo" is a file name and should not be translated
    print_to_stderr(_("Warning: Env-Variable $HOME not found. Using the current directory for .cuyo"));
    return ".cuyo";
  }
  if (ho[strlen(ho) - 1] == '/')
    return Str(ho) + ".cuyo";
  else
    return Str(ho) + "/.cuyo";
}





/** Lädt die Preferences aus wo-auch-immer-sie-abgespeichert-werden
in die Variablen. */
void liesPreferences() {
	
  int i;
  ConfigDatei prd(getPrefsName());
  
  /* Welche Version hat die Prefs-Datei? */
  PrefsVersion version = (PrefsVersion)
                 prd.getZahlEintrag("prefsVersion", PV_first);


  /* Tastenbelegung */
	
  /* Default-Tasten */
  //int dt[2][4] = {{Qt::Key_A, Qt::Key_D, Qt::Key_W, Qt::Key_S},
  //  {Qt::Key_Left, Qt::Key_Right, Qt::Key_Up, Qt::Key_Down}};
  SDLKey dt[2][4] = {{SDLK_a, SDLK_d, SDLK_w, SDLK_s},
                     {SDLK_LEFT, SDLK_RIGHT, SDLK_UP, SDLK_DOWN}};
  char tn[4][9] = {"left", "right", "turn", "down"};
																	
  for (i = 0; i < 2; i++) {
    prd.setAbschnitt(_sprintf("keys %d", i + 1));
    for (int j = 0; j < 4; j++) {
      if (version >= PV_SDLKeys) {
        mTasten[i][j] = (SDLKey) prd.getZahlEintrag(tn[j], dt[i][j]);
	
      } else {
        /* Alte Version: Tasten stehen noch im Qt-Format in der Datei */
        int t = prd.getZahlEintrag(tn[j], -1111);
	if (t == -1111)
	  mTasten[i][j] = dt[i][j];
	else
          mTasten[i][j] = SDLTools::qtKey2sdlKey(t);
      }
    }
  }

  /* Gewonnene Level */
  prd.setAbschnitt();
  for (i = 0; i < 2; i++) {
    prd.getMengenEintrag(i ? "wonMultiPlayer" : "wonOnePlayer",
			 mGewonneneLevel[i]);
    /* Debug-Ausgabe: */
    /*Str t = prd->readEntry(i ? "wonMultiPlayer" : "wonOnePlayer");
      print_to_stderr(_sprintf("blub = %s\n", t.data()));*/
  }

  /* KI-Player-Geschwindigkeit */
  mKIGeschwLog = prd.getZahlEintrag("AISpeed", 6);
  calcKILangsamLin();

  /* Sound */
  mSound = prd.getZahlEintrag("Sound", 1);
}



/** Schreibt die Preferences nach wo-auch-immer-sie-abgespeichert-werden. */
void schreibPreferences() {  

  Str na = getPrefsName();

  FILE * f = fopen(na.data(), "w");
  if (!f) {
    print_to_stderr(_sprintf(_("Warning: Could not write preferences file \"%s\"."),
			     na.data()));
    return;
  }
	    
  fprintf(f, "# cuyo Preferences File\n\n");
  fprintf(f, "prefsVersion=%d\n\n", PV_current);
  for (int i = 0; i < 2; i++) {
    fprintf(f, i ? "wonMultiPlayer=" : "wonOnePlayer=");
    Str lena;
    std::set<Str>::const_iterator e = mGewonneneLevel[i].end();
    for (std::set<Str>::const_iterator j = mGewonneneLevel[i].begin();
	 j!=e; ++j)
      fprintf(f, "%s,", j->data());
    fprintf(f, "\n");
  }
  fprintf(f, "\n");
  fprintf(f, "AISpeed=%d\n\n", mKIGeschwLog);
  
  fprintf(f, "\n");
  fprintf(f, "Sound=%d\n\n", mSound);

  char tn[4][9] = {"left", "right", "turn", "down"};
  for (int i = 0; i < 2; i++) {
    fprintf(f, "[keys %d]\n", i + 1);
    for (int j = 0; j < 4; j++)
      fprintf(f, "%s=%d\n", tn[j], mTasten[i][j]);
    fprintf(f, "\n");
  }

  fclose(f);	
}




SDLKey getTaste(int sp, int t) { return mTasten[sp][t]; }
double getKIGeschwLin() { return mKIGeschwLin; }
int getKIGeschwLog() { return mKIGeschwLog; }

void setTaste(int sp, int t, SDLKey code) { mTasten[sp][t] = code; }
void setKIGeschwLog(int kigl) { mKIGeschwLog = kigl; calcKILangsamLin(); }

bool getSound() { return mSound; }
void setSound(bool s) { mSound = s; }


}

