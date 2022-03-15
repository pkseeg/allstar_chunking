/***************************************************************************
                          code.cpp  -  description
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
 *   the Free Software Foundation; either version 2 of thef License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "cuyointl.h"
#include "global.h"
#include "code.h"
#include "variable.h"
#include "fehler.h"
#include "blop.h"
#include "knoten.h"
#include "aufnahme.h"
#include "ort.h"
#include "sound.h"
#include "cuyo.h"





//tCodeSpeicher Code::gCodeSpeicher;


/* Die ganzen (normalen) Konstruktoren brauchen alle ein paar Standard-
   Parameter, die ich nicht jedes mal tippen will... */
     
#define STDPAR DefKnoten * knoten, Str datna, int znr, CodeArt art
#define STDINIT \
  mArt(art), \
  mBool1Nr(art == folge_code || art == bedingung_code ? \
           knoten->neueBoolVariable() : -1), \
  mBool2Nr(art == bedingung_code ? \
           knoten->neueBoolVariable() : -1), \
  mDateiName(datna), \
  mZeilenNr(znr)


Code::Code(STDPAR): STDINIT,
  mF1(0), mF2(0), mF3(0), mVar1(0), mVar2(0), mOrt(0)
{
}



Code::Code(STDPAR, int zahl, int zahl2 /* = 0 */, int zahl3 /* = 0 */):
    STDINIT,
  mF1(0), mF2(0), mF3(0), mVar1(0), mVar2(0),
  mZahl(zahl), mZahl2(zahl2), mZahl3(zahl3), mOrt(0)
{
}


Code::Code(STDPAR, Variable * v1): STDINIT,
  mF1(0), mF2(0), mF3(0), mVar1(v1), mVar2(0), mOrt(0)
{
}


Code::Code(STDPAR, Variable * v1, Variable * v2): STDINIT,
  mF1(0), mF2(0), mF3(0), mVar1(v1), mVar2(v2), mOrt(0)
{
}


Code::Code(STDPAR, Variable * v1, int zahl): STDINIT,
  mF1(0), mF2(0), mF3(0), mVar1(v1), mVar2(0), mZahl(zahl), mOrt(0)
{
}


Code::Code(STDPAR, Code * f1, Variable * v1): STDINIT,
  mF1(f1), mF2(0), mF3(0), mVar1(v1), mVar2(0), mOrt(0)
{
}


Code::Code(STDPAR, Code * f1, Code * f2 /* =0 */,
           Code * f3 /* =0 */, int zahl /* = 0 */): STDINIT,
  mF1(f1), mF2(f2), mF3(f3), mVar1(0), mVar2(0),
  mZahl(zahl), mOrt(0)
{
}


Code::Code(STDPAR, Code * f1, Code * f2, Variable * v1): STDINIT,
  mF1(f1), mF2(f2), mF3(0), mVar1(v1), mVar2(0), mOrt(0)
{
}


Code::Code(STDPAR, Ort * ort, int zahl /* = 0*/): STDINIT,
  mF1(0), mF2(0), mF3(0), mVar1(0), mVar2(0),
  mZahl(zahl),mOrt(ort)
{
}


Code::Code(STDPAR, Str str): STDINIT,
  mF1(0), mF2(0), mF3(0), mVar1(0), mVar2(0),
  mString(str), mOrt(0)
{
}  


#undef STDPAR
#undef STDINIT
  
Code::Code(DefKnoten * knoten, const Code & f, bool neueBusyNummern):
       Definition() {
  kopiere(knoten, f, neueBusyNummern);
}




void Code::deepLoesch() {
  if (mF1) delete mF1;
  if (mF2) delete mF2;
  if (mF3) delete mF3;
  if (mVar1) delete mVar1;
  if (mVar2) delete mVar2;
  if (mOrt) delete mOrt;
}




void Code::kopiere(DefKnoten * knoten, const Code & f, bool neueBusyNummern) {
  mArt = f.mArt;
  
  mDateiName = f.mDateiName;
  mZeilenNr = f.mZeilenNr;

  if (f.mF1)
    mF1 = new Code(knoten, *f.mF1, neueBusyNummern);
  else
    mF1 = 0;

  if (f.mF2)
    mF2 = new Code(knoten, *f.mF2, neueBusyNummern);
  else
    mF2 = 0;

  if (f.mF3)
    mF3 = new Code(knoten, *f.mF3, neueBusyNummern);
  else
    mF3 = 0;

  if (f.mVar1)
    mVar1 = new Variable(*f.mVar1);
  else
    mVar1 = 0;

  if (f.mVar2)
    mVar2 = new Variable(*f.mVar2);
  else
    mVar2 = 0;

  mZahl = f.mZahl;
  mZahl2 = f.mZahl2;
  mZahl3 = f.mZahl3;

  mString = f.mString;

  if (f.mOrt)
    mOrt = new Ort(knoten, *f.mOrt, neueBusyNummern);
  else
    mOrt = 0;

  
  mBool1Nr = f.mBool1Nr;
  mBool2Nr = f.mBool2Nr;
  if (neueBusyNummern) {
    if (mBool1Nr != -1)
      mBool1Nr = knoten->neueBoolVariable();
    if (mBool2Nr != -1)
      mBool2Nr = knoten->neueBoolVariable();
  }
}








/** Liefert einen String zurück, der angibt, wo dieser Code
    definiert wurde (für Fehlermeldungen) */
Str Code::getDefString() const {
  return _sprintf("%s:%d", mDateiName.data(), mZeilenNr);
}





/** Liefert zurück, wie viele Bilder dieser Code höchstens gleichzeitig
    malt. Dabei wird (im Moment) der Einfachheit halber davon ausgegangen,
    dass Ausdrücke nix malen können; dementsprechend darf getStapelHoehe()
    dafür auch nicht aufgerufen werden. nsh wird um die Anzahl der
    Nachbarstapel-Malungen erhöht. */
int Code::getStapelHoehe(int & nsh) const {
  switch (mArt) {
    case weiterleit_code:
      return mF1->getStapelHoehe(nsh);
    case stapel_code:
      return mF1->getStapelHoehe(nsh) + mF2->getStapelHoehe(nsh);
    case push_code:
      return mF2->getStapelHoehe(nsh);
    case set_code:
    case add_code:
    case sub_code:
    case mul_code:
    case div_code:
    case mod_code:
    case nop_code:
    case busy_code:
    case buchstabe_code:
    case zahl_code:
    case bonus_code:
    case message_code:
    case explode_code:
    case sound_code:
    case verlier_code:
    case bitset_code:
    case bitunset_code:
      return 0;
    case mal_code:
      return 1;
    case mal_code_fremd:
      nsh++;
      return 0;
    case folge_code: {
      int a1 = mF1->getStapelHoehe(nsh);
      int a2 = mF2->getStapelHoehe(nsh);
      return a1 > a2 ? a1 : a2;
    }
    case bedingung_code: {
      int a2 = mF2->getStapelHoehe(nsh);
      int a3 = mF3->getStapelHoehe(nsh);
      return a2 > a3 ? a2 : a3;
    }
    /* Für die ganzen Ausdruck-Codes darf getStapelHoehe() nicht aufgerufen
       werden. */
    case variable_acode:
    case zahl_acode:
    case manchmal_acode:
    case nachbar_acode:
    case intervall_acode:
    case und_acode:
    case oder_acode:
    case not_acode:
    case rnd_acode:
    case add_acode:
    case sub_acode:
    case mul_acode:
    case div_acode:
    case mod_acode:
    case neg_acode:
    case eq_acode:
    case ne_acode:
    case gt_acode:
    case lt_acode:
    case ge_acode:
    case le_acode:
    case ggt_acode:
    case bitand_acode:
    case bitor_acode:
  //case bitset_acode:
    case bitunset_acode:
    case bittest_acode:
    case undefiniert_code:
      /* TRANSLATORS: "Code", "getStapelHoehe", and "CodeArt" are programming
	 keywords that should not be translated.
	 The %s describes, which part of a program triggered the error. */
      throw iFehler(_("%s: Internal error in Code::getStapelHoehe(): Call illegal for CodeArt %d"),
                   getDefString().data(), mArt);
  }
  /* *Kein* default im case verwenden; so erhaelt man gleich beim Compilieren eine
     Warnung, wenn man vergessen hat, eine neue CodeArt hier anzugeben */
  /* TRANSLATORS: "Code", "getStapelHoehe", and "CodeArt" are programming
     keywords that should not be translated.
     The %s describes, which part of a program triggered the error. */
  throw iFehler(_("%s: Internal error in Code::getStapelHoehe(): Unknown CodeArt %d"),
                   getDefString().data(), mArt);
}




/** Fuehrt diesen Code aus auf den Variablen von Blop b.
    In busy wird zurueckgeliefert, ob dieser Code gerade Busy ist */
int Code::eval(Blop & b, bool & busy) const {
  /* Fehlermeldung verbessern */
  try {

    busy = false;
    bool busy1;
    switch (mArt) {
      case weiterleit_code:
        mF1->eval(b, busy);
        return 0;
      case stapel_code:
	mF1->eval(b, busy);
	mF2->eval(b, busy1); busy |= busy1;
	return 0;
      case push_code: {
	int merk = b.getVariable(*mVar1);
	b.setVariable(*mVar1, mF1->eval(b), set_code);
	mF2->eval(b, busy);
	b.setVariable(*mVar1, merk, set_code);
	return 0;
      }
      case set_code:
      case add_code:
      case sub_code:
      case mul_code:
      case div_code:
      case mod_code:
      case bitset_code:
      case bitunset_code: {
	int w;
	w = mF1->eval(b);
        
	if (mArt == div_code && w == 0)
          throw Fehler("%s",_("Division by zero"));

	if (mArt == mod_code && w == 0)
          throw Fehler("%s",_("Modulo zero"));

        /* setVariable() entscheidet anhand von mArt, was zu tun ist.
	   Insbesondere wird die Operation vielleicht erst in der Zukunft
	   ausgeführt. */
	b.setVariable(*mVar1, w, mArt);

	return 0;
      }
      case mal_code:
	b.speichereBild();
	return 0;
      case mal_code_fremd:
	b.speichereBildFremd(*mOrt, mZahl);
	return 0;
      case nop_code:
	return 0;
      case busy_code:
	busy = true;
	return 0;
      case folge_code: {
	/* Ablauf einer Folge:
	 - bool1 <=> Kind2 ausfuehren (sonst Kind1)
	 - Wenn das kind busy ist, busy zurueckliefern und fertig. Sonst:
	 - bool1 wechseln. busy zurueckliefern <=> Wechsel 1->2

         (Alte Version:)
	 A - Wenn niemand beschäftigt ist (weder ich noch Kinder), dann fängt
             die Folge frisch an => Kind1, selbst auf beschäftigt schalten
	 B - Wenn ich beschäftigt bin, schaue ob Kind1 beschäftigt.
	 C   - Wenn ja, sind wir noch in der Kind1 Folge => Kind1
	 D   - Wenn nein, sind wir mit Kind1 fertig => Kind2
               Außerdem selbst auf unbeschäftigt schalten, wenn Kind2 fertig ist
               (also auf unbeschäftigt geschaltet hat)
	   - Wenn ich selbst unbeschäftigt bin, aber ein Kind, ist irgend was
             schiefgelaufen. Das kann eigentlich noch nicht mal durch Programm-
             unterbrechung passieren.
	*/
	bool kind2dran = b.getBoolVariable(mBool1Nr);
	if (kind2dran)
	  mF2->eval(b, busy1);
	else
	  mF1->eval(b, busy1);
	if (busy1) {
	  busy = true;
	} else {
	  kind2dran = !kind2dran;
	  busy = kind2dran;
	  b.setBoolVariable(mBool1Nr, kind2dran);
	}

	return 0;
      }
      case buchstabe_code: {
	b.setVariable(spezvar_pos, mZahl, set_code);
	return 0;
      }
      case zahl_code: {
	b.setVariable(spezvar_file, mZahl, set_code);
	return 0;
      }
      case bedingung_code: {
	/* Funktionsweise von
	     switch {
	       bed_1 pfeil_1 code_1;
	       bed_2 pfeil_2 code_2;
	       ...
	     }
	   
	   - bool1 gibt an, ob beim letzten mal Ast 1 ausgefuehrt wurde und
	     der Ast busy war
	   - bool2 entsprechend fuer Ast 2
	   - Wenn eins davon wahr ist und der entsprechende Ast ein "=>" hat,
	     Ast nochmal waehlen, sonst if-Bedingung checken
	   - Wenn eins wahr ist (aber der Ast kein "=>" hat) und jetzt nicht
	     nochmal ausgefuehrt wird, dann busy-Reset senden
	   - Ich selbst bin busy, falls der ausgefuehrte Ast busy ist und
	     "=>" hat
         */
	bool vast1 = b.getBoolVariable(mBool1Nr);
	bool vast2 = b.getBoolVariable(mBool2Nr);
	bool wahl1;
	if (vast1 && (mZahl & 1)) {
	  wahl1 = true;
	} else if (vast2 && (mZahl & 2)) {
	  wahl1 = false;
	} else wahl1 = mF1->eval(b);

        if (vast1 && !wahl1) mF2->busyReset(b);
        if (vast2 && wahl1) mF3->busyReset(b);

	if (wahl1) {
	  mF2->eval(b, busy);
	  b.setBoolVariable(mBool1Nr, busy);
	  b.setBoolVariable(mBool2Nr, false);
	  busy &= !!(mZahl & 1);
	} else {
	  mF3->eval(b, busy);
	  b.setBoolVariable(mBool1Nr, false);
	  b.setBoolVariable(mBool2Nr, busy);
	  busy &= !!(mZahl & 2);
	}

	
	return 0;
      }
       case bonus_code: {
         b.bekommPunkte(mF1->eval(b));
        return 0;
      }
      case message_code: {
        b.zeigMessage(mString);
	return 0;
      }
      case explode_code: {
        if (b.getSpezConst(spezconst_falling)) {
          if (gDebug)
            print_to_stderr(
		    /* TRANSLATORS: "explode" is a programming keyword
		       that should not be translated. */
		    _("Warning: Can't use 'explode' in falling blob.\n"));
	} else
          b.lassPlatzen();
        return 0;
      }
      case sound_code: {
        b.playSample(mZahl);
        return 0;
      }
      case verlier_code: {
        Cuyo::spielerTot();
        return 0;
      }
      
      /***** Ab hier: Ausdruck-Codes *****/
      
      case zahl_acode: {
	return  mZahl;
      }
      case variable_acode: {
	return b.getVariable(*mVar1);
      }
      case manchmal_acode: {
	int ret = mF2->eval(b);
	if (ret == 0)
	  /* TRANSLATORS: The colon in "x:0" is a programming keyword
	     that should not be translated. */
	  throw Fehler("%s",_("Probability x:0 in the animation program"));
	/* Vermutlich sollte auch "long" oder so statt double gehen; aber
	   ich bin mir grad nicht ganz sicher, ob long wirklich immer long
	   genug ist. */
	ret = Aufnahme::rnd(ret) < mF1->eval(b);
	return ret;
      }
      case nachbar_acode: {
	return  (b.getVariable(spezconst_connect) & mZahl) == mZahl2;
      }
      case eq_acode: {
	return mF1->eval(b) == mF2->eval(b);
      }
      case ne_acode: {
	return mF1->eval(b) != mF2->eval(b);
      }
      case gt_acode: {
	return mF1->eval(b) > mF2->eval(b);
      }
      case lt_acode: {
	return mF1->eval(b) < mF2->eval(b);
      }
      case ge_acode: {
	return mF1->eval(b) >= mF2->eval(b);
      }
      case le_acode: {
	return mF1->eval(b) <= mF2->eval(b);
      }
      case not_acode: {
	return ! mF1->eval(b);
      }
      case rnd_acode: {
	int w = mF1->eval(b);
	if (w <= 0)
	  /* TRANSLATORS: "rnd" is a programming keyword
	     that should not be translated. */
	  throw Fehler("%s",_("rnd(<=0) in the animation program"));
	return Aufnahme::rnd(w);
      }
      case und_acode: {
	return mF1->eval(b) && mF2->eval(b);
      }
      case oder_acode: {
	return mF1->eval(b) || mF2->eval(b);
      }
      case add_acode: {
	return mF1->eval(b) + mF2->eval(b);
      }
      case sub_acode: {
	return mF1->eval(b) - mF2->eval(b);
      }
      case mul_acode: {
	return mF1->eval(b) * mF2->eval(b);
      }
      case div_acode: {
	int ret = mF2->eval(b);
	if (ret == 0)
	  throw Fehler("%s",_("Division by zero"));
	ret = divv(mF1->eval(b), ret);
	return ret;
      }
      case mod_acode: {
	int ret = mF2->eval(b);
	if (ret == 0)
	  throw Fehler("%s",_("Modulo zero"));
	ret = modd(mF1->eval(b), ret);
	return ret;
      }
      case neg_acode: {
        return -mF1->eval(b);
      }
      case intervall_acode: {
	int z1 = mF1->eval(b);
	int z2 = mF2->eval(b);
	int z3 = mF3->eval(b);
	return (z1 >= z2 && z1 <= z3);
      }
      case ggt_acode: {
        int a_ = mF1->eval(b);
        int b_ = mF2->eval(b);
        while (b_!=0) {
          int c_ = a_%b_;
          a_=b_;
          b_=c_;
        }
        return a_;
      }
      case bitand_acode: {
        return mF1->eval(b) & mF2->eval(b);
      }
    //case bitset_acode:
      case bitor_acode: {
        return mF1->eval(b) | mF2->eval(b);
      }
      case bitunset_acode: {
        return mF1->eval(b) & (-1 - mF2->eval(b));
      }
      case bittest_acode: {
        return (mF1->eval(b) & mF2->eval(b)) != 0;
      }
      case undefiniert_code:
	/* TRANSLATORS: "Code", "eval", "CodeArt", and "undefined_code" are
	   programming keywords that should not be translated. */
	throw iFehler("%s",_("Internal error in Code::eval(): CodeArt undefined_code"));
	break;
    }
    /* Kein default im switch; so erhaelt man gleich beim Compilieren eine Warnung,
       wenn man vergisst, eine Codeart hier anzugeben. */
    /* TRANSLATORS: "Code", "eval", and "CodeArt" are programming
       keywords that should not be translated. */
    throw iFehler(_("Internal error in Code::eval(): Unknown CodeArt %d"), mArt);

  } catch (Fehler fe) {
    /* Fehlermeldung verbessern */
    if (!fe.mMitZeile) {
      /* TRANSLATORS: Add line number information (first %s)
	 to an error message (second %s). */
      Fehler f2 = Fehler(_("%s: %s"), getDefString().data(), fe.getText().data());
      f2.mMitZeile = true;
      throw f2;
    } else
      throw fe;
  }
}



/* Dito, wenn man an-busieness nicht interessiert ist */
int Code::eval(Blop & b) const {
  bool muell;
  return eval(b, muell);
}



/** Resettet den Busy-Status von diesem Baum. Ist etwas ineffizient:
    eigentlich braeuchte nicht so ein grosser Teil des Baums abgelaufen
    zu werden. Vielleicht sollte ein Code wissen, ob es unter ihm nix
    gibt mit busy-Status. */
void Code::busyReset(Blop & b) const {
  /* Ausdruck-Codes brauchen keinen busy-reset */
  if (mArt >= erster_acode)
    return;

  switch (mArt) {
    case folge_code:
      b.setBoolVariable(mBool1Nr, false);  // kind2dran = false
      break;
    case bedingung_code:
      b.setBoolVariable(mBool1Nr, false);  // Kein Ast hat grad Ausfuehrung
      b.setBoolVariable(mBool2Nr, false);  // fuer sich reserviert
      break;
    default:;
  }

  if (mF1) mF1->busyReset(b);
  if (mF2) mF2->busyReset(b);
  if (mF3) mF3->busyReset(b);
}



/************************************************************************/


/** Erzeugt einen Code, der prüft, ob es die
    gewünschten Nachbarn gibt (aus "01?"-String).
    Der "01?"-String kann Länge 6 oder 8 haben. */
Code * newNachbarCode(DefKnoten * knoten, Str datna, int znr, Str * str) {
  /* Getestet wird nachher: x & z1 == z2 */
  int l = str->length();
  CASSERT(l == 6 || l == 8);
  int z1 = 0, z2 = 0;
  int bit = 1;
  for (int i = 0; i < l; i++) {
    if ((*str)[i] != '?')
      z1 |= bit;
    if ((*str)[i] == '1')
      z2 |= bit;
    bit *= 2;
    /* Wenn die Länge 6 ist, dann intern noch Fragezeichen für
       die waagrechten Richtungen einfügen (nach den Hex-Zeichen
       1 und 4). */
    if (l == 6 && (i == 1 || i == 4))
      bit *= 2;
  }
  delete str;
  return new Code(knoten, datna, znr, nachbar_acode, z1, z2);
}




