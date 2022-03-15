/***************************************************************************
                          configdatei.cpp  -  description
                             -------------------
    begin                : Sun Jul 1 2001
    copyright            : (C) 2001 by Immi
    email                : cuyo@karimmi.de

Modified 2001,2002,2006,2008,2010,2011 by the cuyo developers

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

#include "configdatei.h"

#include "cuyointl.h"


#define zeilentyp_leer 0
#define zeilentyp_abschnitt 1
#define zeilentyp_zuweisung 2


/* Beste Zahl fuer buffer_size:
   - So, dass die meisten Zeilen kuerzer sind
   - Aber nicht unnoetig gross
   - Und so, dass es sicher irgendwo eine Zeile gibt, die laenger ist,
     damit man's mitkriegt, wenn der Code buggy ist, der die Zeile in
     mehreren Stuecken liest */
#define buffer_size 100

/* Liefert eine Zeile zurueck ohne das \n am Ende */
Str readLine(FILE * f) {
  Str ret;
  char buf[buffer_size];
  bool weiter = true;
  while (weiter) {
    /* fgets liest bis Dateiende oder \n */
    if (!fgets(buf, buffer_size, f))
      break; // eof
    int nlpos = strlen(buf) - 1;
    if (buf[nlpos] == '\n') {
      buf[nlpos] = 0;
      weiter = false;
    }
    ret = ret + buf;
  }
  return ret;
}

#undef buffer_size



ConfigDatei::ConfigDatei(const Str & name):
  mName(name.data())
{
  mDatei = fopen(name.data(), "r");
  setAbschnitt();
}

ConfigDatei::~ConfigDatei() {
  if (mDatei)
    fclose(mDatei);
}


/** Liefert den aktuellen Abschnitt */
Str ConfigDatei::getAbschnitt() const {
  return mAbschnitt;
}


/** Wechselt zum angegebenen Abschnitt. (Abschnitte werden durch
[bla] eingeleitet.) Liefert false, wenn der Abschnitt nicht existiert. */
bool ConfigDatei::setAbschnitt(Str na /*= ""*/) {
  if (na.isEmpty()) {
    /* Anfangs-Abschnitt */
    mAbschnitt = na;
    mAbschnittPos = 0;
    return true;
		
  } else {
    /* Datei überhaupt offen? */
    if (!mDatei)
      return false;
	
    /* Abschnitt suchen */
    rewind(mDatei);
    Str z;
    while (!feof(mDatei)) {
      z = readLine(mDatei);
      int a, b;
      if (getZeilenTyp(z, a, b) == zeilentyp_abschnitt) {
	if (na == z.mid(a, b - a)) {
	  /* Abschnit gefunden */
	  mAbschnitt = na;
	  mAbschnittPos = ftell(mDatei);
	  return true;
	}
					
      }
    } // while Datei nicht zu Ende
		
    /* Abschnitt nicht gefunden */
    return false;
  }
}


/** Liefert zurück, was für ein Zeilentyp die Zeile ist:
    leer, abschnitt, zuweisung. In a und b werden interessante
    Positionen abgespeichert:
    Bei abschnitt: a = Pos. nach "["; b = Pos von "]"
    Bei zuweisung: a = Anfangspos; b = Pos nach "=" */
int ConfigDatei::getZeilenTyp(const Str & z, int & a, int & b) const {
  int p = 0;
  if (z.isEmpty())
    return zeilentyp_leer;
  /* Leertasten am Anfang weg */
  while (z[p] == ' ' || z[p] == '\t')
    p++;
	
  switch (z[p]) {
  case '#':
  case 0:
    /* Leer-Zeile */
    return zeilentyp_leer;
		
  case '[':
    /* Abschnitt-Zeile */
    a = p + 1;
    while (z[p] != ']') {
      if (z[p] == 0) {
	fehlerZeile(z);
	return zeilentyp_leer;
      }
      p++;
    }
    b = p;
		
    /* Hier könnte man jetzt noch testen, ob danach auch wirklich nichts mehr
       kommt... */
		
    return zeilentyp_abschnitt;
		
  default:
    /* Zuweisungs-Zeile */
    a = p;
    while (z[p] != '=') {
      if (z[p] == 0) {
	fehlerZeile(z);
	return zeilentyp_leer;
      }
      p++;
    }
    b = p + 1;
		
    return zeilentyp_zuweisung;
  }
	
}


/** Wenn ein Parse-Fehler in Zeile z aufgetreten ist... */
void ConfigDatei::fehlerZeile(const Str & z) const {
  /* TRANSLATORS: The first %s is the file name,
     the second %s is the erroneous line. */
  print_to_stderr(_sprintf(_("Parse error in file %s: %s\n"),
			   mName.data(), z.data()));
}


/** Liefert true, wenn der Eintrag existiert; schreibt ihn ggf. nach ret */
bool ConfigDatei::getEintragIntern(const Str & schluessel,
				  Str & ret) const {
	
  /* Datei überhaupt offen? */
  if (!mDatei)
    return false;

  fseek(mDatei, mAbschnittPos, SEEK_SET);
	
  while (!feof(mDatei)) {
    Str z = readLine(mDatei);
    int a, b, t;
    t = getZeilenTyp(z, a, b);
    if (t == zeilentyp_abschnitt) break;
		
    if (t == zeilentyp_zuweisung) {
      /* OK, hier ist eine Zuweisung. Ist es die richtige? */
      if (schluessel == z.mid(a, b - 1 - a)) {
				/* Ja */
        ret = z.right(b);
	return true;
      }
    }
  }
	
  return false;
}




/** Liefert den Eintrag, wenn er existiert, sonst den default-String
    (der per default "" ist). */
Str ConfigDatei::getEintrag(const Str & schluessel,
				  Str def /* = "" */) const {
	
  /* Datei überhaupt offen? */
  if (!mDatei)
    return def;

  fseek(mDatei, mAbschnittPos, SEEK_SET);
	
  while (!feof(mDatei)) {
    Str z = readLine(mDatei);
    int a, b, t;
    t = getZeilenTyp(z, a, b);
    if (t == zeilentyp_abschnitt) break;
		
    if (t == zeilentyp_zuweisung) {
      /* OK, hier ist eine Zuweisung. Ist es die richtige? */
      if (schluessel == z.mid(a, b - 1 - a)) {
				/* Ja */
	return z.right(b);
      }
    }
  }
	
  return def;
}


/** Gibt's den Eintrag? */
bool ConfigDatei::hatEintrag(const Str & schluessel) const {
  Str muell;
  return getEintragIntern(schluessel, muell);
}


/** Liefert den Eintrag als Zahl, wenn er existiert, sonst die default-Zahl. */
int ConfigDatei::getZahlEintrag(const Str & schluessel,
				int def /*= 0*/) const {
  Str e;
  if (getEintragIntern(schluessel, e)) {
    int ret = def;
    sscanf(e.data(), "%d", &ret);
    return ret;
  } else
    return def;
}

// /** Liefert den Eintrag als Farbe, wenn er existiert,
//     sonst die default-Farbe. */
// Color ConfigDatei::getFarbEintrag(const Str & schluessel,
// 				   const Color & def /*= black*/) const {
//   Str e;
//   if (getEintragIntern(schluessel, e)) {
//     int r, g, b;
//     sscanf(e.nichtNull().data(), "%d,%d,%d", &r, &g, &b);
//     return Color(r, g, b);
//   } else
//     return def;
// }

/** Liefert einen Eintrag als Komma-getrennte Liste */
int ConfigDatei::getMengenEintrag(const Str & schluessel,
				  std::set<Str> & menge) const {

  Str e;
  if (!getEintragIntern(schluessel, e))
    return 0;

  /* Menge löschen */
  menge.clear();
	
  /* Leere Liste? */
  if (e.isEmpty())
    return 0;
		
  /* Evtl. Schlusskomma anhängen */
  if (e[e.length() - 1] != ',')
    e += ',';
	
  /* String zerlegen */
  int anz = 0;
  int p = 0, vp = 0;
  while (p<e.length()) {
    if (e[p] == ',') {
      menge.insert(e.mid(vp, p - vp));
      vp = p + 1;
      anz++;
    }
    p++;
  }
  return anz;
}
