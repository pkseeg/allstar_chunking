/***************************************************************************
                          xpmladen.cpp  -  description
                             -------------------
    begin                : Fri Apr 20 2001
    copyright            : (C) 2001 by Immi
    email                : cuyo@karimmi.de

Modified 2002,2003,2006,2008-2011 by the cuyo developers

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

#include "sdltools.h"

#include "cuyointl.h"
#include "fehler.h"
#include "xpmladen.h"
#include "leveldaten.h"
#include "global.h"
#include "inkompatibel.h"

#if HAVE_LIBZ
#include <zlib.h>
#endif


/* Die nachfolgenden Variablen existieren nur innerhalb von xpmladen.cpp */
static char * gDatAnfang;
static char * gDatBei;
static char * gDatEnde;

/* True, wenn eine .xpm.gz-Datei gefunden wurde. Diese Var. ist nur
   nötig, damit wenn eine gezippte Datei gefunden wurde, die aber nicht
   von meinem Algorithmus lesbar ist, nicht ein "Datei nicht gefunden"
   ausgegeben wird. */
static bool gDateiGezippt;


/* Damit in ladeDateiLow nicht ein Haufen #ifdefs stehen müssen, hier
   ein paar Dummy-Definitionen für wenn's die zlib nicht gibt. */
   
   
#if !HAVE_LIBZ
typedef int gzFile;
int gzopen(char *, char *) {return 0;}
bool gzeof(int) {return 0;}
int gzread(int, char *, int) {return 0;}
void gzclose(int) {}
#endif



/** Wird von ladeDatei benutzt, um entweder eine normale oder eine
    gz-Datei zu laden... */
bool ladeDateiLow(Str na, bool gz) {
  FILE * f = 0; // Das "= 0" ist nur um keine Warnungen zu bekommen
  gzFile gzf = 0;

  /* Datei öffnen */
  if (gz) {
    /* Das "b" bedeutet binary-Datei. */
    gzf = gzopen(na.data(), "rb");
    if (!gzf) return false;
  } else {
    f = fopen(na.data(), "r");
    if (!f) return false;
  }
  
  int bei = 0;
  
  while ( gz  ?  !gzeof(gzf)  :  !feof(f) ) {
    /* So viel wollen wir diesmal auf einmal lesen: So viel wie wir
       schon haben, plus 4096 */
    int neu = bei + 4096;

    /* Speicher für neu zu ladendes alloziieren. (Wenn noch nix
       geladen war, ist gDatAnfang = 0 und realloc äquivalent zu malloc) */
    gDatAnfang = (char *) realloc(gDatAnfang, bei + neu);
  
    if (gz) {
      bei += neu = gzread(gzf, gDatAnfang + bei, neu);

      if (neu < 0) {
        gzclose(gzf);
        /* gzerror() could improve the error message... */
        throw Fehler("%s",_("Read error"));
      }
    } else {
      bei += fread(gDatAnfang + bei, 1, neu, f);
      if (ferror(f)) {
        fclose(f);
        throw Fehler("%s",_("Read error"));
      }
    }
  }
  
  /* OK, jetzt ist wahrscheinlich zu viel Speicher alloziiert.
     Korrigieren wir das mal noch ein bisschen. Das "+1" ist, weil
     wir am Ende noch eine "0" anhängen wollen. */
  gDatAnfang = (char *) realloc(gDatAnfang, bei + 1);
  
  gDatEnde = gDatAnfang + bei;
  *gDatEnde = 0;

  if (gz)
    gzclose(gzf);
  else  
    fclose(f);
  
  return true;  
}



/** Läd die angegebene Datei komplett. Danach zeigt gDatAnfang auf den
    Anfang und gDatEnde auf das Ende. Fügt außerdem noch eine 0 ans Ende
    an.
    Liefert false, wenn die Datei nicht gefunden wird.
    Sucht auch nach der Datei na.gz. */
bool ladeDatei(Str na) {

  /* Erst mal versuchen, eine ungezippte Datei zu laden. */
  if (ladeDateiLow(na, false)) return true;
  
  /* Und dann eine gezippte. Aber nur mit zlib */
  #if HAVE_LIBZ
  if (ladeDateiLow(na + ".gz", true)) {
    gDateiGezippt = true;
    return true;
  }
  #endif
  
  return false;
}



void leerWeg() {
  while (*gDatBei == ' ' || *gDatBei == '\t' || *gDatBei == '\n' || *gDatBei == '\r')
    gDatBei++;
}

void leerUndKommentarWeg() {
  while (1) {
    leerWeg();
    /* Kein Kommentar-Anfang? Dann fertig */
    if (gDatBei[0] != '/' || gDatBei[1] != '*')
      return;
    /* Kommentar weglesen */
    gDatBei += 2;
    while (gDatBei[0] != '*' || gDatBei[1] != '/') {
      if (!gDatBei[0])
        throw Fehler("%s",_("Endless comment."));
      gDatBei++;
    }
    gDatBei += 2;
  }
}

/** Erwartet, dass ein s kommt (davor ist whitespace erlaubt) */
void erwarte(const char * s) {
  const char *t = s;
  while (*s) {
    if (*gDatBei != *s)
      throw Fehler(_("\"%s\" expected"), t);
    gDatBei++;
    s++;
  }
}


/** Das gleiche mit char */
void erwarte(char c) {
  if (*gDatBei != c)
    throw Fehler(_("'%c' expected; found: '%c' (filepos: %d)"), c, *gDatBei,
              (int) (gDatBei - gDatAnfang));
  gDatBei++;
}



/** Liest so lange, bis ein c auftaucht. */
void liesBis(char c) {
  while (*gDatBei != c) {
    if (*gDatBei == 0)
      throw Fehler(_("'%c' expected"), c);
    gDatBei++;
  }
  gDatBei++;
}


/** Prüft, ob der String s jetzt kommt. Wenn ja, wird
    er weggelesen */
bool kommtString(const char * s) {
  char * merk = gDatBei;
  while (*s) {
    if (*gDatBei != *s) {
      gDatBei = merk;
      return false;
    }
    gDatBei++;
    s++;
  }
  return true;
}



int getInt() {
  int ret = 0;
  bool geht = false;
  leerWeg();
  while (*gDatBei >= '0' && *gDatBei <= '9') {
    ret = ret * 10 + *gDatBei - '0';
    gDatBei++;
    geht = true;
  }
  if (!geht)
    throw Fehler("%s",_("Number expected"));
  return ret;
}



int decodeHex1(char a) {
  if (a >= '0' && a <= '9')
    return a - '0';
  if (a >= 'A' && a <= 'F')
    return a - 'A' + 10;
  if (a >= 'a' && a <= 'f')
    return a - 'a' + 10;
  throw Fehler("%s",_("Hex number expected"));
}

int getHex() {
  int ret = decodeHex1(*gDatBei++) * 16;
  return ret + decodeHex1(*gDatBei++);
}




struct info {
  Uint32 farbcode;
  bool hintergrund;
};

union suchbaum {
  info einziges;
  info blatt[256];
  suchbaum * weiter[256];

  suchbaum(int tiefe) {
    if (tiefe>1)
      for (int i=0; i<256; i++) weiter[i]=NULL;
  }

  inline info* rein(int tiefe) {
    if (tiefe==0)
      return &einziges;
    suchbaum* such = this;
    for (;tiefe-->1;) {
      if (!such->weiter[(unsigned int)*gDatBei])
        such->weiter[(unsigned int)*gDatBei] = new suchbaum(tiefe);
      such = such->weiter[(unsigned int)*gDatBei++];
    }
    return (such->blatt) + (*gDatBei++);
  }

  inline info raus(int tiefe) {
    if (tiefe==0)
      return einziges;
    suchbaum* such = this;
    for (;tiefe-->1;) {
      if (!such->weiter[(unsigned int)*gDatBei])
        throw Fehler("%s",_("Undefined pixel name"));
      such = such->weiter[(unsigned int)*gDatBei++];
    }
    return such->blatt[(unsigned int)*gDatBei++];
  }

  void loesch(int tiefe) {
    if (tiefe-->1)
      for (int i=0; i<256; i++)
        if (weiter[i]) {
	  weiter[i]->loesch(tiefe);
          delete weiter[i];
        }
  }
};






/* Versucht die Datei na zu laden.
   Versucht außerdem, die Datei na.gz zu laden.
   Liefert 0, wenn keine der Dateien existiert.
   Throwt, wenn's beim Laden einen Fehler gibt.
   (Falls die SDL-Lad-Routine verwendet wird, kann nicht versucht werden,
   die .gz-Datei zu laden.) */
SDL_Surface * ladXPM(Str na, RohMaske & maske) {
  SDL_Surface * s;

  gDatAnfang = 0;
  
  gDateiGezippt = false;
  
  /* Das nachfolgende try-catch ist 1. um evtl Speicher freizugeben
     und zweitens, weil wir es dann nochmal mit der SDL-Laderoutine
     versuchen wollen. */
  try {

    /* Datei laden. Dabei werden gDatAnfang und gDatEnde gesetzt. */
    if (!ladeDatei(na)) return false;


    gDatBei = gDatAnfang;

    /* XPM-Kommentar-Zeile lesen */
    leerWeg();
    erwarte("/*");
    leerWeg();
    erwarte("XPM");
    leerWeg();
    erwarte("*/");
    /* Alles bis zur ersten { entfernen */
    liesBis('{');

    /* OK, jetzt sind wir im interessanten Bereich. */
    /* "groesse_x groesse_y farbzahl charpp" parsen. */
    leerUndKommentarWeg();
    erwarte('"');

    int groesse_x, groesse_y, farb, tiefe;
    groesse_x = getInt();
    groesse_y = getInt();
    farb = getInt();
    tiefe = getInt();
    bool monochrom = tiefe==0;
    leerWeg();
    erwarte('"');

    s = SDLTools::createSurface32(groesse_x, groesse_y);
    SDL_PixelFormat * pf = s->format;

    maske.init(groesse_x,groesse_y);

    /* Farben parsen */

    suchbaum farben(tiefe);

    for (int i = 0; i < farb; i++) {
      leerUndKommentarWeg();
      erwarte(',');
      leerUndKommentarWeg();
      erwarte('"');

      info* index = farben.rein(tiefe);
      leerWeg();
      char typ = *gDatBei++;
      CASSERT(typ == 'c');
      leerWeg();

      index->hintergrund=false;
      if (kommtString("None")) {
        // durchsichtig
	index->farbcode = SDL_MapRGBA(pf, 0, 0, 0, 0);
      } else if (kommtString("black")) {
        index->farbcode = SDL_MapRGBA(pf, 0, 0, 0, 255);
      } else if (kommtString("white")) {
        index->farbcode = SDL_MapRGBA(pf, 255, 255, 255, 255);
      } else if (kommtString("Background")) {
        // Farbe vom Level-Hintergrund (für Explosion)
        index->farbcode = ld->mHintergrundFarbe.getPixel(pf);
        index->hintergrund = true;
//       } else if (kommtString("FontDark")) {
//         // Farbe der Schrift in dem Level (für Punkt-Ziffern)
//         *index = ld->mSchriftFarbe[schrift_dunkel].getPixel(pf);
//       } else if (kommtString("Font")) {
//         // Farbe der Schrift in dem Level (für Punkt-Ziffern)
//         *index = ld->mSchriftFarbe[schrift_normal].getPixel(pf);
//       } else if (kommtString("FontLight")) {
//         // Farbe der Schrift in dem Level (für Punkt-Ziffern)
//         *index = ld->mSchriftFarbe[schrift_hell].getPixel(pf);
      } else {
        erwarte('#');
	int f_r = getHex();
	int f_g = getHex();
	int f_b = getHex();
	index->farbcode = SDL_MapRGBA(pf, f_r, f_g, f_b, 255);
      }
      liesBis('"');
    }

    SDL_LockSurface(s); /* Damit wir Pixel direkt bearbeiten duerfen */

    if (monochrom) {
      if (farb) {
        info farbe = farben.raus(tiefe);
        for (int y=0; y<groesse_y; y++)
          for (int x=0; x<groesse_x; x++)
            SDLTools::getPixel32(s,x,y) = farbe.farbcode;
        maske.fill(farbe.hintergrund);
      };
    } else
      for (int y = 0; y < groesse_y; y++) {
        leerUndKommentarWeg();
	erwarte(',');
	leerUndKommentarWeg();
	erwarte('"');
	for (int x = 0; x < groesse_x; x++) {
          info farbe = farben.raus(tiefe);
	  SDLTools::getPixel32(s, x, y) = farbe.farbcode;
          maske.set_pixel(x,y,farbe.hintergrund);
        };
	erwarte('"');
     }

    SDL_UnlockSurface(s);

    farben.loesch(tiefe);

    /* Angeblich dürfen jetzt noch Extensionen von xpm kommen. Da kümmern wir
       uns nicht weiter drum. */

    free(gDatAnfang);

  } catch (Fehler f) {
    if (gDatAnfang) free(gDatAnfang);

    if (gDebug)
      print_to_stderr(_sprintf(_("Fast xpm loading did not work for \"%s\":\n%s\n"),
			       na.data(), f.getText().data()));
    
    /* Ich habe im Nachfolgenden IMG_Load-Verwendung auskommentiert:
       IMG_Load scheint nicht zu funkionieren (segmentation fault).
       Oder vielleicht funktioniert es auch, aber es reagiert mit segfault
       auf Dateien, die es nicht lesen kann. Deshalb: Wenn wir die Datei
       nicht selbst lesen koennen, dann lieber den Fehler ausgeben als
       IMG_Load aufrufen. */
//    if (gDateiGezippt) {
      /* Bei einer gezippten Datei geht fallBackLaden nicht... */
      // TRANSLATORS: This is to prepend an error message
      throw Fehler(_("File \"%s.gz\": %s\n"), na.data(), f.getText().data());
//    } else {
      /* Die sdl-Laderoutine verwenden. Waere eh zu pruefen ob die nicht besser ist als
         unsere eigene */
//      return IMG_Load(na.data());
//    }
  }
  
  
  return s;
}
