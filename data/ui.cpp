/***************************************************************************
                          ui.cpp  -  description
                             -------------------
    begin                : Mit Jul 12 22:54:51 MEST 2000
    copyright            : (C) 2006 by Immi
    email                : cuyo@pcpool.mathematik.uni-freiburg.de

Modified 2006,2008,2010,2011 by the cuyo developers

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/* Set to 1 to artificially make graphics slow. This is to test the
   routine which skips images */
#define TEST_SLOW_GRAPHICS 0


#if TEST_SLOW_GRAPHICS
#include <unistd.h>
#endif

#include <vector>
#include <stdio.h>


#include <SDL.h>
#include "sdltools.h"

#include "font.h"

#include "cuyointl.h"
#include "ui.h"
#include "ui2cuyo.h"
#include "punktefeld.h"
#include "fehler.h"
#include "global.h"
#include "sound.h"

#include "layout.h"
#include "prefsdaten.h"

#include "blatt.h"

#include "bilddatei.h"


namespace UI {
/*************************** Private Variablen **************************/
/* (stehen nicht in der .h-Datei) */


int mRealWindowWidth = -1;
int mRealWindowHeight = -1;



/** Wird auf true gesetzt, wenn beendet werden soll */
bool mQuit;

/* Wird in einem Event (oder Zeitschritt) auf true gesetzt, wenn
   die Graphik neu gemalt werden muss. Wird auf false gesetzt, wenn
   neu gemalt wird. Das neu-malen-und-auf-false-setzen passiert evtl.
   nicht sofort, wenn wir in Zeitnot sind. */
bool mGraphikUpdateNoetig;

BlattSpiel * mBlattSpiel;
BlattHauptmenu * mBlattMenu;


/** Welches "Blatt" wird grad angezeigt */
Blatt * mBlatt;




/*************************** Private Methoden **************************/
/* (stehen nicht in der .h-Datei) */


/** Führt alle Menü-Befehle aus */
void menuAufruf(int id);

void vielleichtAllesAnzeigen();


/***************************************************************************/



void init() {

  Bilddatei::init();
  PrefsDaten::init();
  SDLTools::initSDL(mRealWindowWidth, mRealWindowHeight);
  Font::init();
  Sound::init();
  Blatt::initBlaetter();
  
  Sound::setMusic("cuyo.it");
  
  Punktefeld::init();
  
  mBlattSpiel = new BlattSpiel();
  mBlattMenu = new BlattHauptmenu();

  /* Hier wird cuyo initialisiert */
  Cuyo::init();

  mBlattMenu->oeffnen(false);
  
  sofortAllesAnzeigen();
}


void destroy() {
  Cuyo::destroy();
  
  delete mBlattMenu;
  delete mBlattSpiel;

  Punktefeld::destroy();
  Blatt::destroyBlaetter();
  Sound::destroy();
  Font::destroy();
  Area::destroy();

  Bilddatei::destroy();
}



void doEvent(SDL_Event & evt) {
  /* GGf. debug-Tasten behandeln */
  if (evt.type == SDL_KEYDOWN && (evt.key.keysym.mod & (KMOD_ALT | KMOD_META)))
    if (Cuyo::debugKeyEvent(evt.key.keysym))
      return;
      
  /* Quit-Event */
  if (evt.type == SDL_QUIT) {
    quit();
    return;
  }
    
  /* Restliche Events werden an das Blatt weitergeleitet */
  mBlatt->doEvent(evt);
}



/* Ist vielleicht zur Zeit etwas unnötig, eine eigene ui-Zeitschritt-Fkt
   zu haben. Vielleicht kommt aber ja mal wieder was dazu, was das ui
   tun will */
void zeitSchritt() {
  mBlatt->zeitSchritt();
}



void run() {
  mQuit = false;
  int zeit = SDL_GetTicks() + 80;
  /* Wie oft hintereinander ist die Graphik grad ausgefallen? */
  int graphikAusgefallen = 0;

  while (!mQuit) {
  
  
    /* Alle wartenden Events aus der Queue holen. Wenn eins dieser Events
       ein Graphik-Update fordert, dann wird das sofort ausgeführt, auch
       wenn wir in Zeitnot sind. Deshalb von früher rumliegende
       Graphikupdate-Bestellungen kurz wegspeichern */
    bool gun_merk = mGraphikUpdateNoetig;
    mGraphikUpdateNoetig = false;
    SDL_Event evt;
    while (SDLTools::pollEvent(evt))
      doEvent(evt);
      
    if (mGraphikUpdateNoetig) {
      sofortAllesAnzeigen();
      graphikAusgefallen = 0;
    } else {
      mGraphikUpdateNoetig = gun_merk;
    }
    
    /* Ein bisschen Zeit für's Betriebsystem */
    /* (Geht das nicht besser?) */
    SDL_Delay(10);
    
    int akt_zeit = SDL_GetTicks();
    if (akt_zeit > zeit) {
      zeitSchritt();

      /* Wenn wir schon einen ganzen Zeitschritt verspaetung haben
         (und die Graphik nicht grad eben schon zu oft ausgefallen ist),
         dann Graphik ausfallen lassen */
      bool ausfallen = akt_zeit - zeit > 80 && graphikAusgefallen < 3;
      if (ausfallen) graphikAusgefallen++; else graphikAusgefallen = 0;
      
      if (!ausfallen) {
        /* Graphik (falls nötig) updaten. Möglicherweise handelt es sich
           um einen Update, der gar nicht von diesem zeitSchritt()-Aufruf
           gefordert wurde, sondern von einem älteren */
        vielleichtAllesAnzeigen();
      }

      zeit += 80;
      /* Nicht zu viel Verspaetung ansammeln lassen: Wenn wir schon 5
         Zeitschritte verspaetung haben, ist wohl nix zu machen; dann
	 laeuft das Spiel halt langsamer */
      if (zeit < akt_zeit - 300)
        zeit = akt_zeit - 300;
    }
  }
}


// 
// 
// void malText(const Str & text) {
// 
//   /* Wenn man das alles besser macht, sollte man hier vielleicht nur dann
//      neu malen, wenn sich auch was geaendert hat */
// 
//   Area::fillRect(0, 0, L_fenster_breite, L_fenster_hoehe, Color(30, 30, 70));
//   Font::gNormal->drawText(text, L_fenster_breite / 2, L_fenster_hoehe / 2);
//   Area::updateAll();
// }


void quit() {
  mQuit = true;
}



void sofortAllesAnzeigen() {
#if TEST_SLOW_GRAPHICS
  usleep((int) (1000 * 1000 * 0.3));
#endif

  /*
  für xtrace:
   XInternAtom(qt_xdisplay(),"zeitschritt B",1);
  */

  /* Hier finden die ganzen Area::xxx Malbefehle statt. Die Malroutinen
     sind selbst dafür verantwortlich, nur das neu zu malen, was sich
     geändert hat. Außerdem müssen sie Area::updateRect() aufrufen für
     die Bereiche, wo sie was neu gemalt haben */
  mBlatt->anzeigen();
  
  /* Der Bereich, der mit Area::updateRect() markiert wurde, wird an den
     X-Server geschickt. */
  Area::doUpdate();
  
  /*
  für xtrace:
  XInternAtom(qt_xdisplay(),"zeitschritt C",1);
  */

  mGraphikUpdateNoetig = false;
}


void vielleichtAllesAnzeigen() {
  if (mGraphikUpdateNoetig)
    sofortAllesAnzeigen();
}


/* Das aufrufen reicht nicht, damit alles neu gemalt wird:
   Irgendwer muss Area::updateRect() aufrufen, damit das
   neue Bild auch an den X-Server geschickt wird.
   */
void nachEventAllesAnzeigen() {
  mGraphikUpdateNoetig = true;
}






void setPunkte(int sp, int pt) {
  //mPunktefeld[sp]->setPunkte(pt);
  mBlattSpiel->setPunkte(sp, pt);
}

void randNeuMalen() {
  mBlattSpiel->randNeuMalen();
}



void startSpiel(int lnr) {
  mBlattSpiel->oeffnen(lnr);
}


/** Cuyo teilt dem ui mit, dass das Spiel zu Ende ist */
void stopSpiel() {
  mBlattMenu->oeffnen(false);
  Sound::setMusic("cuyo.it");
}



/* Ein Blatt sollte selbst in seiner oeffnen()-Methode setBlatt(this)
   aufrufen. Kann man aber auch woanders aufrufen, wenn das entsprechende
   Blatt grad keinen oeffnen()-Aufruf braucht. */
void setBlatt(Blatt * b) {
  mBlatt = b;
  /* Der gesamte Bildschirm hat sich geändert, nicht nur das, was das
     (neu) aktive Blatt vielleicht denkt: */
  Area::updateAll();
  nachEventAllesAnzeigen();
}


void setGeometry(int width, int height) {
  mRealWindowWidth = width;
  mRealWindowHeight = height;
}


}
