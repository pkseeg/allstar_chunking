
%{

/***************************************************************************
                          parser.yy  -  description
                             -------------------
    begin                : Mit Jul 12 22:54:51 MEST 2000
    copyright            : (C) 2000 by Immi
    email                : cuyo@pcpool.mathematik.uni-freiburg.de

Modified 2002,2003,2005,2006,2008,2010,2011 by the cuyo developers

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

#include "cuyointl.h"
#include "code.h"
#include "fehler.h"
#include "knoten.h"
#include "variable.h"
#include "ort.h"
#include "version.h"
#include "global.h"
#include "leveldaten.h"
#include "sound.h"

#define YYMALLOC malloc
#define YYFREE free


/***********************************************************************/
/* Globale Parse-Variablen */

/** Für Fehlerausgabe: Aktueller Dateiname */
Str gDateiName;
/** Für Fehlerausgabe: Aktuelle Zeilen-Nummer */
int gZeilenNr;

/** True, wenn es während des Parsens (mindestens) einen Fehler gab. */
bool gGabFehler;


/** Wenn der Parser aufgerufen wird, muss in DefKnoten schon ein DefKnoten
    stehen, an den alles geparste angefügt wird. Normalerweise erzeugt man
    einen neuen Defknoten. Beim Includen will man da aber schon was drin
    haben.
    Siehe auch %type <defknoten> alles */
static DefKnoten * gAktDefKnoten;

//#define MAX_INCLUDE_TIEFE 16
/** YY_BUFFER_STATE ist ein flex-Datentyp für eine Datei, an der grade
    geparst wird. */
//static YY_BUFFER_STATE gIncludeStack[MAX_INCLUDE_TIEFE];
/** Aktuelle Include-Tiefe. (0 bei Hauptdatei) */
//static int gIncludeTiefe;

//static DefKnoten * gIncludeMerk;


/* Beim Erzeugen eines Codes müssen einige Variablen jedes Mal übergeben
   werden:
   - der zugehörige DefKnoten, damit ggf. noch Variablen reserviert werden
     können (genauer: die Busy-Variable).
   - Dateiname und Zeilennummer, damit der Code schönere Fehlermeldungen
     ausgeben kann.
   Damit ich das aber nicht jedes Mal eintippen muss, hier ein paar Macros:
 */
#define newCode0(ART) new Code(\
  gAktDefKnoten, gDateiName, gZeilenNr, ART)
#define newCode1(ART, X1) new Code(\
  gAktDefKnoten, gDateiName, gZeilenNr, ART, X1)
#define newCode2(ART, X1, X2) new Code(\
  gAktDefKnoten, gDateiName, gZeilenNr, ART, X1, X2)
#define newCode3(ART, X1, X2, X3) new Code(\
  gAktDefKnoten, gDateiName, gZeilenNr, ART, X1, X2, X3)
#define newCode4(ART, X1, X2, X3, X4) new Code(\
  gAktDefKnoten, gDateiName, gZeilenNr, ART, X1, X2, X3, X4)



/***********************************************************************/


#define VIEL 32767


/* PBEGIN_TRY und PEND_TRY() fangen Fehler ab und geben sie aus, damit
   weitergeparst werden kann und mehrere Fehler gleichzeitig ausgegeben
   werden können. Sie werden in fehler.h definiert. */


#define YYDEBUG 1


/* Bug in Bison? Er scheint den Stack nicht automatisch vergrößern zu wollen,
   wenn er voll ist. Es sieht so aus, als hätte er angst, dass da c++-Variablen
   vorkommen, wo er (wegen constructor und so) nicht einfach so rumalloziieren
   kann. (Ich weiß nicht, was der LTYPE ist, von dem er nicht festgestellt hat,
   das er trivial ist.) */
#define YYLTYPE_IS_TRIVIAL 1

%}



%union {
  Code * code;
  Code * codepaar[2];
  Str * str;
  int zahl;
  int zahlpaar[2];
  Knoten * knoten;
  DefKnoten * defknoten;
  ListenKnoten * listenknoten;
  WortKnoten * wortknoten;
  Variable * variable;
  Ort * ort;
  Version * version;
  CodeArt codeart;
  OrtHaelfte haelfte;
}



%pure_parser

%left ';'
%nonassoc ']'
%nonassoc IF_PREC
%nonassoc ELSE_TOK
%left ','
%left OR_TOK
%left AND_TOK
%left EQ_TOK NE_TOK '<' '>' GE_TOK LE_TOK
%nonassoc BIS_TOK
  // x==-3..-2 bedeutet *nicht* (x==-3..)-2
  // und x==y==2..3 bedeutet nicht (x==y)==2..3, weil das konstant 0 wäre
  // und willkürlicher auch nicht x==(y==2)..3
%nonassoc '!'
%left '+' '-'
%nonassoc ':'
%left '*' '/' '%'
%left '&' '|' BITSET_ATOK BITUNSET_ATOK
%nonassoc NEG_PREC
%nonassoc '.'
%nonassoc error

%token INCLUDE_TOK
%token BEGIN_CODE_TOK END_CODE_TOK
%token SWITCH_TOK IF_TOK BIS_TOK VAR_TOK BUSY_TOK
%token ADD_TOK SUB_TOK MUL_TOK DIV_TOK MOD_TOK BITSET_TOK BITUNSET_TOK
%token RND_TOK GGT_TOK BONUS_TOK MESSAGE_TOK SOUND_TOK EXPLODE_TOK VERLIER_TOK
%token DEFAULT_TOK DA_KIND_TOK
%token FREMD_TOK BITSET_ATOK BITUNSET_ATOK
%token <str> REINWORT_TOK WORT_TOK NACHBAR8_TOK NACHBAR6_TOK
%token <zahl> NULLEINS_TOK ZAHL_TOK HALBZAHL_TOK BUCHSTABE_TOK PFEIL_TOK

%type <codeart> zuweisungs_operator
%type <zahl> zahl halbzahl vorzeichen_zahl
%type <str> wort punktwort proc_def_wort var_def_wort versionsmerkmal
%type <code> code code_1 stern_at buch_stern auswahl_liste set_zeile
%type <code> ausdruck halbort
%type <codepaar> intervall
%type <ort> ort absort_klammerfrei absort_geklammert absort
%type <ort> relort_klammerfrei relort_geklammert relort
/*%type <bed> bedingung*/
%type <haelfte> haelften_spez

%type <zahl> ld_konstante konstante
%type <zahlpaar> echter_default unechter_default
%type <knoten> rechts_von_def def_liste_eintrag
%type <listenknoten> def_liste

%type <version> version versionierung

/* Eigentlich wäre es naheliegend und schön, wenn alles einen DefKnoten
   zurückliefern würde. Statt dessen wird das über die globale Variable
   gAktDefKnoten gemanaget. Das hat mehrere Gründe:
   - Am Ende muss sowieso das Parse-Ergebnis irgendwo weggepeichert werden,
     damit man es bekommt.
   - <<>>-Definitionen müssen auf den aktuellen DefKnoten zugreifen können.
     Dazu müsste er sowieso in einer globalen Variable stehen.
   - Beim Includen soll das neu geparste an einen schon vorhandenen DefKnoten
     angehängt werden. Das ginge anders auch nicht. */
/*%type <defknoten> alles*/

%type <variable> variable lokale_variable

/*
Erklaerung zu den Vorrangsregeln:
Bei einem Vorrangskonflikt muss entschieden werden zwischen eine Regel
R anwenden oder ein Token T shiften. Sowohl Regeln alsauch token haben
Prioritaeten. Dasjenige mit hoeherer Prioritaet (R oder T) wird
gemacht. Bei gleichstand entscheidet %left oder %right

Beispiel: Stack = "exp exp", gefunden: "(". "exp exp -> exp" und "("
haben gleiche Prioritaet; wegen right wird "(" geshiftet.
 */


%{
int yyerror(const char * s);
int yylex(YYSTYPE * lvalPtr);

/** Wechselt die Lex-Datei temporär. Liefert was zurück, was an popLex
    übergeben werden muss, um wieder zurückzuschalten. Throwt evtl.
    setzdefault wird an den Pfaditerator übergeben.
    Wird in lex.ll definiert, weil da die nötigen Lex-Dinge definiert sind. */
void * pushLex(const char * na, bool setzdefault = false);
void popLex(void * merkBuf);
%}


%%
/*****************************************************************************/


/***** Normaler Level-Def-Bereich *****/



alles:                 { /* Nix zu tun. */ }
  | alles punktwort version '=' rechts_von_def   {
	
      PBEGIN_TRY
        gAktDefKnoten->fuegeEin(*$2, *$3, $5);
        delete $2;
        delete $3;
      PEND_TRY(;);
    }
  | alles BEGIN_CODE_TOK code_modus END_CODE_TOK  {
			   
      /* Nix zu tun; die Codes speichern sich von alleine nach
         gAktDefKnoten */
    }
/* Wenn man nach Fehlern sinnvoll weitermachen zu können glaubt,
   kann man das hier wieder dekommentieren. Aber wenn man recht hat,
   ist vielleicht die Produktion "code_zeile: error" ein besserer
   Kandidat.
  | alles BEGIN_CODE_TOK error END_CODE_TOK
*/
  | alles INCLUDE_TOK punktwort {
      PBEGIN_TRY
      
        /* Lex auf neue Datei umschalten */
	void * merkBuf = pushLex($3->data());


        /***** Bison-Aufruf für include-Datei *****/
        /* Hier muss man aufpassen, dass mit allen globalen Variablen
	   das richtige passiert. Nichts zu tun ist bei:
	   - gGabFehler (Fehler in include-Datei ist halt auch Fehler)
	   - gAktDefKnoten (Die Include-Datei speichert ihre Ergebnisse
	     auch einfach da mit rein.)
	   */

	
	/* Datei und Zeilennummer zwischenspeichern. */
	Str merkDat = gDateiName;
	gDateiName = *$3;
	int merkZNr = gZeilenNr;
	gZeilenNr = 1;
	
	/* Der rekursive Aufruf! Hier! Live! (Die Ergebnisse werden in
	   gAktDefKnoten eingefügt.) */
	if ((yyparse()) && !gGabFehler) {
	  print_to_stderr(_("Unknown error during file inclusion!\n"));
	  gGabFehler = true;
	}
	
	gDateiName = merkDat;
	gZeilenNr = merkZNr;
		
	/* Lex auf alte Datei zurückschalten */
	popLex(merkBuf);
	
      PEND_TRY(;);
    }
;




versionsmerkmal:
    wort { $$ = $1; }
  | zahl { $$ = new Str(_sprintf("%d",$1)); }   /* Etwas häßlich, aber wir
                                                        brauchen's für die
                                                        Versionen 1 und 2. */
  ;

versionierung:
    versionsmerkmal                   {
      $$ = new Version();
      $$->nochEinMerkmal(*$1);
      delete $1;
    }
  | versionsmerkmal ',' versionierung {
      $$ = $3;
      $$->nochEinMerkmal(*$1);
      delete $1;
    }
;

version:
                          { $$ = new Version(); }
  | '[' versionierung ']' { $$ = $2; }
;

ld_konstante:
    vorzeichen_zahl    { $$ = $1; }
  | '<' konstante '>'  { $$ = $2; }
;

rechts_von_def: '{' {
      /* OK, hier wird ein neuer Defknoten eröffnet. Der alte wird
         auf dem Bison-Stack zwischengespeichert... */
      DefKnoten * merk = gAktDefKnoten;
      /* Neuen Defknoten erzeugen, mit dem alten als Vater */
      gAktDefKnoten = new DefKnoten(gDateiName, gZeilenNr, merk);
      $<defknoten>$ = merk;

                    } alles '}'  {
		    
      /* Jetzt wurde gAktDefKnoten mit Inhalt gefüllt, den wir
         zurückliefern */
      $$ = gAktDefKnoten;
      /* POP DefKnoten */
      gAktDefKnoten = $<defknoten>2;
    }
  | def_liste                   { $$ = $1; }
;

def_liste: def_liste_eintrag    {
      $$ = new ListenKnoten(gDateiName, gZeilenNr);
      $$->fuegeEin($1);
    }
  | def_liste ',' def_liste_eintrag   {
      $$ = $1;
      $$->fuegeEin($3);
    }
;

def_liste_eintrag: punktwort   {
      $$ = new WortKnoten(gDateiName, gZeilenNr, *$1); delete $1;
    }
  | ld_konstante          {
      $$ = new ZahlKnoten(gDateiName, gZeilenNr, $1);
    }
  | punktwort '*' ld_konstante {
      $$ = new VielfachheitKnoten(gDateiName, gZeilenNr, *$1, $3); delete $1;
    }
;

konstante:
    zahl                          { $$ = $1; }
  | wort                          {
      Knoten * def = gAktDefKnoten->getVerwandten(*$1, ld->mVersion, false);
      const DatenKnoten * datum = 0;
      switch (def->type()) {
        case type_DatenKnoten:
          datum=(const DatenKnoten *) def;
          break;
        case type_ListenKnoten:
          datum=((ListenKnoten*) def)->getEinzigesDatum();
          break;
        default: throw Fehler(_("%s not a number"),$1->data());
      }
      $$ = datum->assert_datatype(type_ZahlDatum)->getZahl();
      delete $1;
    }
  | '(' konstante ')'             { $$ = $2; }
  | '-' konstante %prec NEG_PREC  { $$ = -$2; }
  | konstante '+' konstante       { $$ = $1 + $3; }
  | konstante '-' konstante       { $$ = $1 - $3; }
  | konstante '*' konstante       { $$ = $1 * $3; }
  | konstante '/' konstante       { $$ = divv($1,$3); }
  | konstante '%' konstante       { $$ = modd($1,$3); }
;




/***** Bereich in << >> *****/


code_modus: 
  | code_modus code_zeile
;

code_zeile: proc_def_wort version '=' code_1 ';'    {
      gAktDefKnoten->speicherDefinition(namespace_prozedur, *$1,
                                        *$2, $4);
      delete $1; delete $2;
    }
  | VAR_TOK var_liste ';'
  | DEFAULT_TOK default_liste ';'
/* Wenn man nach Fehlern sinnvoll weitermachen zu können glaubt,
   kann man das hier wieder dekommentieren.
  | error ';'
*/
;


/* Der nachfolgende Zustand existiert nur, um die Fehlermeldung
   zu verbessern. */
proc_def_wort: punktwort    { $$ = $1; }
    | BUCHSTABE_TOK {
      /* Wie gibt man einen Fehler möglichst umständlich aus?
         (Aber so, dass er genauso aussieht wie die anderen Fehler. */
      PBEGIN_TRY
        throw Fehler("%s",_("Procedure names can't be single letters."));
      PEND_TRY($$ = new Str());
    }
;


var_liste: var_def
  | var_liste ',' var_def
;

echter_default:
    konstante                 { $$[0]=$1; $$[1]=da_init; }
  | konstante ':' DA_KIND_TOK { $$[0]=$1; $$[1]=da_kind; }
  ;

unechter_default:
                       { $$[0]=0;  $$[1]=da_init; }
  | '=' echter_default { $$[0]=$2[0];  $$[1]=$2[1];}
  ;

var_def:
    var_def_wort version unechter_default    {
      PBEGIN_TRY
        gAktDefKnoten->neueVarDefinition(*$1, *$2, $3[0], $3[1]);
        delete $1; delete $2;
      PEND_TRY(;)
    }
  ;

/* Der nachfolgende Zustand existiert nur, um die Fehlermeldung
   zu verbessern. */
var_def_wort: wort    { $$ = $1; }
    | BUCHSTABE_TOK {
      /* Wie gibt man einen Fehler möglichst umständlich aus?
         (Aber so, dass er genauso aussieht wie die anderen Fehler. */
      PBEGIN_TRY
        throw Fehler("%s",_("Variable names can't be single letters."));
      PEND_TRY($$ = new Str());
    }
;


default_liste:
    default_def
  | default_liste ',' default_def
  ;

default_def:
    var_def_wort version '=' echter_default  {
      PBEGIN_TRY
        gAktDefKnoten->neuerDefault(
          ((VarDefinition*)
              (gAktDefKnoten->getDefinition(namespace_variable,*$1,*$2,false)))
            -> mNummer,
          $4[0], $4[1]);
        delete $1; delete $2;
      PEND_TRY(;)
    }
  ;



/***** Code *****/
/* Bemerkung: newCode*() (siehe oben) ruft new Code auf und übergibt
   noch ein paar Parameter, die jedes Mal übergeben werden müssen
   (gAktDefKnoten, gDateiName, gZeilenNr) */

code: code_1              { $$ = $1; }
  | code_1 ';' code       { $$ = newCode2(stapel_code, $1, $3);}
;

/* Code_1 darf kein Semikolon enthalten. ({} verwenden.) */
code_1: SWITCH_TOK '{' auswahl_liste '}' { $$ = $3; }
  | IF_TOK ausdruck PFEIL_TOK code_1   %prec IF_PREC {
      $$ = newCode4(bedingung_code, $2, $4,
                     newCode0(nop_code),
                     $3 + 2 * ohne_merk_pfeil);
    }
  | IF_TOK ausdruck PFEIL_TOK code_1 ELSE_TOK code_1   %prec IF_PREC {
      if ($3==ohne_merk_pfeil)
        $$ = newCode4(bedingung_code, $2, $4,
                      $6,
                      3*ohne_merk_pfeil);
      else
	/* TRANSLATORS: The text in the literal strings should not be translated. */
        throw Fehler("%s",_("Please specify \"else ->\" or \"else =>\""));
    }
  | IF_TOK ausdruck PFEIL_TOK code_1 ELSE_TOK PFEIL_TOK code_1  %prec IF_PREC {
      /* Nach else kann, muss aber kein Pfeil stehen.
         (Kein Pfeil will man vermutlich, wenn dann gleich das
	 nächste if kommt.) */
      $$ = newCode4(bedingung_code, $2, $4,
                     $7,
                     $3 + 2 * $6);
    }
  | '{' code '}'          { $$ = $2; }
  | code_1 ',' code_1     { $$ = newCode2(folge_code, $1, $3);}
  | zahl                  { $$ = newCode1(zahl_code, $1); }
  | buch_stern            { $$ = $1; }
  | zahl buch_stern       {
      $$ = newCode2(stapel_code, newCode1(zahl_code, $1), $2);
    }
  | punktwort                  {
      PBEGIN_TRY
        /* Kopie erzeugen...) */
        $$ = new Code(gAktDefKnoten, * (Code*)
               gAktDefKnoten->getDefinition(namespace_prozedur, *$1,
                                            ld->mVersion, false), true);
        delete $1;
      PEND_TRY($$ = newCode0(undefiniert_code))
    }
  | '&' punktwort              {
      PBEGIN_TRY
        /* Kopie erzeugen...) */
        $$ = newCode1(weiterleit_code,
	      new Code(gAktDefKnoten, * (Code*)
                gAktDefKnoten->getDefinition(namespace_prozedur, *$2,
                                             ld->mVersion, false), false));
        delete $2;
      PEND_TRY($$ = newCode0(undefiniert_code))
    }
  |                       { $$ = newCode0(nop_code); }
  | set_zeile             { $$ = $1; }
  | '[' lokale_variable '=' ausdruck ']' code_1   {
      PBEGIN_TRY
        if ($2->istKonstante())
          throw Fehler(_sprintf(_("%s is a constant. (Variable expected.)"),
                     $2->getName().data()));
        $$ = newCode3(push_code, $4, $6, $2);
        
      PEND_TRY($$ = newCode0(undefiniert_code))
    }
  | BUSY_TOK              { $$ = newCode0(busy_code); }
  | BONUS_TOK '(' ausdruck ')'  {
      $$ = newCode1(bonus_code, $3);
    }
  | MESSAGE_TOK '(' punktwort ')' {
      $$ = newCode1(message_code, _($3->data()));
      delete $3;
    }
  | SOUND_TOK '(' punktwort ')' {
      $$ = newCode1(sound_code, Sound::ladSample(*$3));
      delete $3;
    }
  | VERLIER_TOK {
      $$ = newCode0(verlier_code);
    }
  | EXPLODE_TOK {
      $$ = newCode0(explode_code);
    }
;


set_zeile: variable zuweisungs_operator ausdruck  {
      PBEGIN_TRY
        if ($1->istKonstante())
          throw Fehler(_sprintf(_("%s is a constant. (Variable expected.)"),
                     $1->getName().data()));
        $$ = newCode2($2, $3, $1);
      PEND_TRY($$ = newCode0(undefiniert_code))
    }
;

zuweisungs_operator: '=' { $$ = set_code; }
  | ADD_TOK              { $$ = add_code; }
  | SUB_TOK              { $$ = sub_code; }
  | MUL_TOK              { $$ = mul_code; }
  | DIV_TOK              { $$ = div_code; }
  | MOD_TOK              { $$ = mod_code; }
  | BITSET_TOK           { $$ = bitset_code; }
  | BITUNSET_TOK         { $$ = bitunset_code; }
;

/* * oder *@(x,y) oder @(x,y)* */
stern_at:
    '*'       { $$ = newCode0(mal_code); }
  | '*' ort   { $$ = newCode2(mal_code_fremd, $2, 1); }
  | ort '*'   { $$ = newCode2(mal_code_fremd, $1, -1); }
;

/* Buchtabe oder Buchstabe* oder *; und evtl. @(bla, blub) */
buch_stern: BUCHSTABE_TOK { $$ = newCode1(buchstabe_code, $1); }
  | BUCHSTABE_TOK stern_at    {
      $$ = newCode2(stapel_code, newCode1(buchstabe_code, $1),
                     $2);
    }
  | stern_at                   { $$ = $1; }
;



auswahl_liste: ausdruck PFEIL_TOK code_1 ';'   {
      $$ = newCode4(bedingung_code, $1, $3,
                     newCode0(nop_code),
                     $2 + 2 * ohne_merk_pfeil);
    }
  | ausdruck PFEIL_TOK code_1 ';' PFEIL_TOK code_1 ';'     {
      $$ = newCode4(bedingung_code, $1, $3,
                     $6,
                     $2 + 2 * $5);
    }
  | ausdruck PFEIL_TOK code_1 ';' auswahl_liste     {
      $$ = newCode4(bedingung_code, $1, $3,
                     $5,
                     $2 + 2 * mit_merk_pfeil);
    }
;




punktwort:
    wort                         { $$ = $1; }
  | punktwort '.' wort           {
      *$1 += '.';  *$1 += *$3;  $$ = $1;
      delete $3;
    }
  | punktwort '.' BUCHSTABE_TOK  {
      *$1 += '.';  *$1 += ($3>=26 ? 'a'+$3-26 : 'A'+$3);  $$ = $1;
    }
;


wort: WORT_TOK     { $$ = $1; } 
  | REINWORT_TOK   { $$ = $1; }
;



/***** Ausdrücke *****/

ausdruck: variable            {
      if ($1->istKonstante()) {
        /* Wenn die Variable in Wirklichkeit eine Konstante ist,
           dann gleich die Konstante einsetzen. */
        $$ = newCode1(zahl_acode, $1->getDefaultWert());
        delete $1;
      } else
        $$ = newCode1(variable_acode, $1);
    }
  | zahl                      { $$ = newCode1(zahl_acode, $1); }
  | '(' ausdruck ')'          { $$ = $2; }
  | NACHBAR8_TOK              {
      $$ = newNachbarCode(gAktDefKnoten, gDateiName, gZeilenNr, $1);
    }
  | NACHBAR6_TOK              {
      $$ = newNachbarCode(gAktDefKnoten, gDateiName, gZeilenNr, $1);
    }
/*  | wort '~' NACHBAR_TOK      {
      print_to_stderr(_("~ geht noch nicht!!!"));
      $$ = newNachbarCode(gAktDefKnoten, $3);
    }*/
  | ausdruck ':' ausdruck       { $$ = newCode2(manchmal_acode, $1, $3);}
  | ausdruck '+' ausdruck       { $$ = newCode2(add_acode, $1, $3);}
  | '-' ausdruck %prec NEG_PREC { $$ = newCode1(neg_acode, $2);}
  | ausdruck '-' ausdruck       { $$ = newCode2(sub_acode, $1, $3);}
  | ausdruck '*' ausdruck       { $$ = newCode2(mul_acode, $1, $3);}
  | ausdruck '/' ausdruck       { $$ = newCode2(div_acode, $1, $3);}
  | ausdruck '%' ausdruck       { $$ = newCode2(mod_acode, $1, $3);}
  | ausdruck BITSET_ATOK ausdruck   { $$ = newCode2(bitset_acode, $1, $3);}
  | ausdruck BITUNSET_ATOK ausdruck { $$ = newCode2(bitunset_acode, $1, $3);}
  | ausdruck '.' ausdruck       { $$ = newCode2(bittest_acode, $1, $3);}
  | ausdruck EQ_TOK ausdruck    { $$ = newCode2(eq_acode, $1, $3);}
  | ausdruck NE_TOK ausdruck    { $$ = newCode2(ne_acode, $1, $3);}
  | ausdruck GE_TOK ausdruck    { $$ = newCode2(ge_acode, $1, $3);}
  | ausdruck LE_TOK ausdruck    { $$ = newCode2(le_acode, $1, $3);}
  | ausdruck '>' ausdruck       { $$ = newCode2(gt_acode, $1, $3);}
  | ausdruck '<' ausdruck       { $$ = newCode2(lt_acode, $1, $3);}
  | ausdruck '&' ausdruck       { $$ = newCode2(bitand_acode, $1, $3);}
  | ausdruck '|' ausdruck       { $$ = newCode2(bitor_acode, $1, $3);}
  | '!' ausdruck                { $$ = newCode1(not_acode, $2);}
  | ausdruck AND_TOK ausdruck   { $$ = newCode2(und_acode, $1, $3);}
  | ausdruck OR_TOK ausdruck    { $$ = newCode2(oder_acode, $1, $3);}
  | ausdruck EQ_TOK intervall {
      $$ = newCode3(intervall_acode, $1, $3[0], $3[1]);
    }
  | RND_TOK '(' ausdruck ')' {
      $$ = newCode1(rnd_acode, $3);
    }
  | GGT_TOK '(' ausdruck ',' ausdruck ')' {
      $$ = newCode2(ggt_acode, $3, $5);
    }
;


intervall:
    ausdruck BIS_TOK { $$[0]=$1; $$[1]=newCode1(zahl_acode, VIEL); }
  | BIS_TOK ausdruck { $$[0]=newCode1(zahl_acode, -VIEL); $$[1]=$2; }
  | ausdruck BIS_TOK ausdruck { $$[0]=$1; $$[1]=$3; }
  ;


lokale_variable:  wort       {
      PBEGIN_TRY
        $$ = new Variable(//gDateiName, gZeilenNr,
               (VarDefinition*) gAktDefKnoten->
                     getDefinition(namespace_variable, *$1,
                                   ld->mVersion, false),
               0
             );
      PEND_TRY($$ = new Variable())
      delete $1;
    }
    | BUCHSTABE_TOK {
      /* Wie gibt man einen Fehler möglichst umständlich aus?
         (Aber so, dass er genauso aussieht wie die anderen Fehler. */
      PBEGIN_TRY
        throw Fehler("%s",_("Variable names can't be single letters."));
      PEND_TRY($$ = new Variable());
    }
;

variable:
    lokale_variable   { $$ = $1; }
  | wort ort   {
      PBEGIN_TRY
        $$ = new Variable(//gDateiName, gZeilenNr,
               (VarDefinition*) gAktDefKnoten->
                     getDefinition(namespace_variable, *$1,
                                   ld->mVersion, false),
               $2);
      PEND_TRY($$ = new Variable())
      delete $1;
    }
;

halbort:
    ausdruck   { $$ = $1; }
  | halbzahl   { $$ = newCode1(zahl_acode, $1); }
;

haelften_spez:
    '=' { $$ = haelfte_hier; }
  | '!' { $$ = haelfte_drueben; }
  | '<' { $$ = haelfte_links; }
  | '>' { $$ = haelfte_rechts; }
;

absort_klammerfrei:
                  { $$ = new Ort(absort_semiglobal); }
  | NULLEINS_TOK  { $$ = new Ort(absort_fall, newCode1(zahl_acode, $1)); }
;

absort_geklammert:
                         { $$ = new Ort(absort_semiglobal); }
  | ausdruck             { $$ = new Ort(absort_fall, $1); }
  | halbort ',' halbort  { $$ = new Ort(absort_feld, $1, $3); }
;

absort:
    absort_klammerfrei                           { $$ = $1; }
  | '(' absort_geklammert ')'                    { $$ = $2; }
  | '(' absort_geklammert ';' haelften_spez ')'  {
      $2->setzeHaelfte($4);
      $$ = $2;
    }
;

relort_klammerfrei:
                  { $$ = new Ort(absort_global); }
  | NULLEINS_TOK  { $$ = new Ort(newCode1(zahl_acode, $1)); }
;

relort_geklammert:
                         { $$ = new Ort(absort_global); }
  | ausdruck             { $$ = new Ort($1); }
  | halbort ',' halbort  { $$ = new Ort($1, $3); }
;

relort:
    relort_klammerfrei                           { $$ = $1; }
  | '(' relort_geklammert ')'                    { $$ = $2; }
  | '(' relort_geklammert ';' haelften_spez ')'  {
      $2->setzeHaelfte($4);
      $$ = $2;
    }
;

ort:
    '@' relort         { $$ = $2; }
  | FREMD_TOK absort   { $$ = $2; }
;





zahl: ZAHL_TOK      { $$ = $1; }
   | NULLEINS_TOK   { $$ = $1; }
;

/* Bei relativen Koordinaten für Variablenangaben dürfen in Y-Richtung
   auch Halbganze Zahlen angegeben werden... (für Hex-Level) */
halbzahl:
     HALBZAHL_TOK                      {
       /* Halbzahlen sollen intern aufgerundet gespeichert werden... */
       $$ = $1 + 1;
     }
   | '-' HALBZAHL_TOK                  { $$ = -$2; }
;

vorzeichen_zahl: zahl           { $$ = $1; }
   | '-' zahl                   { $$ = -$2; }
;


%%
/*****************************************************************************/






extern FILE * yyin;
int yyparse();
//void initLex();



int yyerror (const char * s)  /* Called by yyparse on error */
{
  PBEGIN_TRY
   throw Fehler(Str(s));
  PEND_TRY(;)
  return 0;
}


/* Öffnet die Datei mit dem angegebenen Namen und parst sie. Das
   Ergebnis wird in den Defknoten erg geschrieben. */
/** Komplettbeschreibung vom Parse-Vorgang siehe leveldaten.h */
void parse(const Str & name, DefKnoten * erg) {

  /* Datei öffnen, Lex initialisieren. Eigentlich bräuchte man
     kein pushLex und popLex ganz außen; aber es ist irgendwie
     sauberer. Vor allem ist dann sicher, dass ein Fehler in einem
     früheren Parsen keine Auswirkungen auf ein späteres Parsen
     hat.
     true = Default-Pfad merken für die Includes. (wird an den
            Pfaditerator weitergegeben.) */
  void * merkBuf = pushLex(name.data(), true);

  gDateiName = name;
  gZeilenNr = 1;
  gGabFehler = false;  /* Wenn es denn mal ein bison-Fehler-recovery gibt,
                         sollte gGabFehler dort auf true gesetzt werden */

  /* Das Parse-Ergebnis soll in den Knoten erg geschrieben werden. */
  gAktDefKnoten = erg;
  
  /* Hier findet das Parsen statt. Man beachte: Um Flex und Bison nicht
     zu verwirren, kann yyparse() nicht mit throw verlassen werden.
     Deshalb brauchen wir nix zu catchen, um alles wieder aufräumen zu
     können. */
  int perg = yyparse();
  
  /* Datei schließen, lex zurücksetzen. */
  popLex(merkBuf);
  
  
  /* Hier werden vermutlich mehr Bedingungen getestet als nötig. */
  if (perg || gGabFehler)
    throw Fehler("%s",_("There have been errors parsing the level description files."));
  
}

