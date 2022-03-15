/***************************************************************************
                          global.cpp  -  description
                             -------------------
    begin                : Mit Jul 12 22:54:51 MEST 2000
    copyright            : (C) 2000 by Immi
    email                : cuyo@pcpool.mathematik.uni-freiburg.de

Modified 2002,2006,2008,2010,2011 by the cuyo developers

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "global.h"
#include "stringzeug.h"





/** True, wenn -d übergeben wurde, d. h. im Debug-Modus. */
bool gDebug;


/** Pfad, wo das Cuyo-Programm liegt, das aufgerufen wurde (aus
    argv[0] extrahiert. Wird vom PfadIterator gebraucht, um nach
    Leveln zu suchen. */
Str gCuyoPfad;


/** True, wenn der Benutzer den Namen einer ld-Datei übergeben hat. */
bool gDateiUebergeben;


/** Wenn ein Level-Datei-Name als Parameter übergeben wurde... */
Str gLevelDatei;







/** Entfernt von p alles, was nach dem ersten Punkt kommt. Ist dazu da
    um aus einem "bla.xpm", was unter pics steht, den Namen für Programme
    zu extrahieren. */
Str picsEndungWeg(const Str & p) {
  for (int i = 0; i < (int) p.length(); i++) {
    /* Etwas brutal: Alles ab dem ersten Punkt weg. */
    if (p[i] == '.')
      return p.left(i);
  }
  /* Keine Endung... */
  return p;
}



/** d sollte ein Pfad mit Dateiname sein. Liefert nur den Pfad-Anteil
    zurück (d. h. alles vor dem letzten "/". Liefer "./", falls d keinen
    "/" enthält. */
Str nimmPfad(Str d) {
  /* Letzten Slash suchen */
  for (int i = d.length(); i--; ) {
    if (d[i] == '/') {
      /* Letzten Slash gefunden. Das Links davon ist der Pfad */
      return d.left(i + 1);
    }
  }
  /* Kein "/" gefunden, also offenbar im aktuellen Verzeichnis. */
  return "./";
}


Str vergissPfad(Str d) {
  /* Letzten Slash suchen */
  for (int i = d.length(); i--; ) {
    if (d[i] == '/') {
      /* Letzten Slash gefunden. Das rechts davon ist, was wir wollen. */
      return d.right(i + 1);
    }
  }
  /* Kein "/" gefunden, also machen wir eine Kopie. */
  Str kopie = d;
  return kopie;
}


