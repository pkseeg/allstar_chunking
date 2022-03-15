%option noyywrap
%option never-interactive

%{

/***************************************************************************
                          scanner.ll  -  description
                             -------------------
    begin                : Mit Jul 12 22:54:51 MEST 2000
    copyright            : (C) 2000 by Immi
    email                : cuyo@pcpool.mathematik.uni-freiburg.de

Modified 2002,2003,2005-2008,2010,2011 by the cuyo developers

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
#include <cstdlib>

#include "inkompatibel.h"
#include "cuyointl.h"
#include "code.h"
#include "knoten.h"
#include "ort.h"
/* parser.h darf erst hier included werden, weil es gemeinerweise
   ohne selbst zu includen auf code.h bis ort.h zugreift */
#include "parser.h"

/* Um nach include-Dateien zu suchen...: */
#include "pfaditerator.h"




/***** lex-Einstellungen *****/


/* _Nicht_ so parsen, dass man interaktiv die level.descr eingeben könnte,
   während sie geparst wird. */
#define YY_NEVER_INTERACTIVE 1

/* Wenn bison reentrant sein soll (d. h. nur lokale Variablen benutzen),
   wird die an die Lex-Hauptroutine yylex() noch ein Pointer übergeben
   auf die Variable, in die man den lval schreiben kann. */
#define YY_DECL int yylex(YYSTYPE * lvalPtr)



extern Str gDateiName;
extern int gZeilenNr;
extern bool gGabFehler;




Str * expandiereBackslash(char * a) {
  Str r;
  a++; // erstes " überspringen
  while (*(a + 1)) { // Letztes Zeichen auch auslassen
    if (*a == '\\') {
      a++;
      switch (*a) {
      case 'n':
        r += '\n';
        break;
      case '"':
        r += '"';
        break;
      case '\\':
        r += '\\';
        break;
      default:
        throw Fehler("%s",_("Unknown escape sequence"));
      }
    } else
      r += *a;
    a++;
  }
  return new Str(r);
}





/** Wechselt die Lex-Datei temporär. Liefert was zurück, was an popLex
    übergeben werden muss, um wieder zurückzuschalten. Throwt evtl.
    setzdefault wird an den Pfaditerator übergeben. */
void * pushLex(const char * na, bool setzdefault = false) {
  /* Alte Datei merken */
  YY_BUFFER_STATE merkBuf = YY_CURRENT_BUFFER;

  /* Neue Datei suchen und oeffnen. */
  PfadIterator pi(na, false, setzdefault);
  FILE * neudat;
  for (; !(neudat = fopen(pi.pfad().data(), "r")); ++pi) {}
  
  /* Auf neue Datei umschalten */
  yy_switch_to_buffer(yy_create_buffer(neudat, YY_BUF_SIZE));
  
  return merkBuf;
}


void popLex(void * merkBuf) {
  /* Aktuelle Datei schließen */
  FILE * f = yyin;
  yy_delete_buffer( YY_CURRENT_BUFFER );
  /* Die Datei lieber erst nach dem yy_delete_buffer() schließen.
     Man weiß nicht so recht, ob yy_delete_buffer() noch was mit
     der Datei macht... */
  fclose(f);

  /* Auf ursprüngliche Datei zurückschalten */
  yy_switch_to_buffer((YY_BUFFER_STATE) merkBuf);
  /* Hoffentlich kümmert sich lex drum, die neu geöffnete Datei
     yyin wieder zu schließen und yyin zurückzusetzen. */
}




%}

STRINGBUCH [ !#-\[\]-\xff]|(\\[n"\\])



%%


<INITIAL>var            { return VAR_TOK; }
<INITIAL>busy           { return BUSY_TOK; }
<INITIAL>switch         { return SWITCH_TOK; }
<INITIAL>if             { return IF_TOK; }
<INITIAL>else           { return ELSE_TOK; }
<INITIAL>rnd            { return RND_TOK; }
<INITIAL>gcd            { return GGT_TOK; }
<INITIAL>include        { return INCLUDE_TOK; }
<INITIAL>bonus          { return BONUS_TOK; }
<INITIAL>message        { return MESSAGE_TOK; }
<INITIAL>sound          { return SOUND_TOK; }
<INITIAL>lose           { return VERLIER_TOK; }
<INITIAL>explode        { return EXPLODE_TOK; }
<INITIAL>default        { return DEFAULT_TOK; }
<INITIAL>reapply        { return DA_KIND_TOK; }
<INITIAL>"<<"           { return BEGIN_CODE_TOK; }
<INITIAL>">>"           { return END_CODE_TOK; }
<INITIAL>".."           { return BIS_TOK; }
<INITIAL>[01]           { lvalPtr->zahl = yytext[0]-'0';
                          return NULLEINS_TOK;
                        }
<INITIAL>[A-Za-z]       {
                            if (yytext[0] >= 'a')
                              lvalPtr->zahl = yytext[0] - 'a' + 26;
                            else
                              lvalPtr->zahl = yytext[0] - 'A';
                            return BUCHSTABE_TOK;
                        }
<INITIAL>[A-Za-z]+      {
                            lvalPtr->str = new Str(yytext);
                            return REINWORT_TOK;
                        }
<INITIAL>[A-Za-z_][A-Za-z_0-9]*  {
                            lvalPtr->str = new Str(yytext);
                            return WORT_TOK;
                        }
<INITIAL>[01?][01?][01?][01?][01?][01?][01?][01?] {
                          lvalPtr->str = new Str(yytext);
                          return NACHBAR8_TOK;
                        }
<INITIAL>[01?][01?][01?][01?][01?][01?] {
                          lvalPtr->str = new Str(yytext);
                          return NACHBAR6_TOK;
                        }
<INITIAL>([0-9]+)|(0[Xx][0-9A-Fa-f]+) {
                          sscanf(yytext, "%i", &lvalPtr->zahl);
                          return ZAHL_TOK;
                        }
<INITIAL>[0-9]*\.5      {
                          /* Falls der Benutzer ".5" schreibt,
			     wird sscanf nix scannen... */
               	          lvalPtr->zahl = 0;
			  
                          /* sscanf mit %d liest nur alles vor dem "." */
                          sscanf(yytext, "%d", &lvalPtr->zahl);
                          return HALBZAHL_TOK;
                        }
<INITIAL>"->"           {
                          lvalPtr->zahl = ohne_merk_pfeil;  // siehe code.h
                          return PFEIL_TOK;
                        }
<INITIAL>"=>"           {
                          lvalPtr->zahl = mit_merk_pfeil;  // siehe code.h
                          return PFEIL_TOK;
                        }
<INITIAL>"=="           { return EQ_TOK; }
<INITIAL>"!="           { return NE_TOK; }
<INITIAL>"<="           { return LE_TOK; }
<INITIAL>">="           { return GE_TOK; }
<INITIAL>"&&"           { return AND_TOK; }
<INITIAL>"||"           { return OR_TOK; }
<INITIAL>"+="           { return ADD_TOK; }
<INITIAL>"-="           { return SUB_TOK; }
<INITIAL>"*="           { return MUL_TOK; }
<INITIAL>"/="           { return DIV_TOK; }
<INITIAL>"%="           { return MOD_TOK; }
<INITIAL>".+="          { return BITSET_TOK; }
<INITIAL>".-="          { return BITUNSET_TOK; }
<INITIAL>".+"           { return BITSET_ATOK; }
<INITIAL>".-"           { return BITUNSET_ATOK; }
<INITIAL>"@@"           { return FREMD_TOK; }
<INITIAL>\"({STRINGBUCH})*\"   {
                           PBEGIN_TRY
                             lvalPtr->str = expandiereBackslash(yytext);
                             return WORT_TOK;
                           PEND_TRY(lvalPtr->str = new Str())
                        }
<INITIAL>[=+,;(){}*@/~!%:<>&.] {return yytext[0];}
<INITIAL>"-"             {return yytext[0];}
<INITIAL>"["             {return yytext[0];}
<INITIAL>"]"             {return yytext[0];}
<INITIAL>[ \t]+           /* Leerzeug */
<INITIAL>"#"+[^\n]*       /* Kommentar */
<INITIAL>\n               gZeilenNr++;
<INITIAL>.                { PBEGIN_TRY
                              throw Fehler(_sprintf(_("Wrong character '%s'"),
                                yytext));
                            PEND_TRY(;);
                          }
                            
%%

