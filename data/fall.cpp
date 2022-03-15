/***************************************************************************
                          fall.cpp  -  description
                             -------------------
    begin                : Sat Aug 18 2001
    copyright            : (C) 2001 by Immi
    email                : cuyo@karimmi.de

Modified 2001-2003,2005,2006,2008,2010,2011 by the cuyo developers

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "fall.h"
#include "spielfeld.h"
#include "aufnahme.h"
#include "fehler.h"
#include "sound.h"
#include "layout.h"


/* Anzahl der Pixel: Verschiebung einen Blops beim drehen... */
#define am_drehen_schieb_gross 28 /* gric * cos(30 Grad) */
#define am_drehen_schieb_klein 16 /* gric * sin(30 Grad) */
/* Anz. d. angezeigten Zwischenbildchen beim Drehen. (Das erste
   Zwischenbild sieht man allerdings nicht...) */
#define am_drehen_start 3


/* Anzahl der Pixel: Verschiebung des Falls beim verschieben... */
#define am_schieben_diff 8
/* Restverschiebung direkt nach dem Tastendruck. (Das erste
   Zwischenbild sieht man allerdings nicht...) */
#define am_schieben_start 16



/* Return-Werte für testBelegt() */
#define belegt_keins 0
#define belegt_0 1
#define belegt_1 2
#define belegt_beide (belegt_0 | belegt_1)


/***************************************************************************/


/** Liefert die Anzahl der Blops vom Fall zurück. */
int FallPos::getAnz() const {
  switch (r) {
    case richtung_keins: return 0;
    case richtung_einzel: return 1;
    default: return 2;
  }
}

int FallPos::getX(int a) const {
  return x + a * (r == richtung_waag);
}

/** ... hv = aktuelle Hochverschiebung... */
int FallPos::getY(int a, int hv) const {
  int y0 = yy + gric - 1 + hv;
  /* Korrektur bei ungeraden Spalten im Sechseck-Modus... */
  if (ld->mSechseck && (getX(a) & 1))
    y0 += gric / 2;

  return y0 / gric + a * (r == richtung_senk);
}


/***************************************************************************/


/** Konstruktor... */
Fall::Fall(Spielfeld * sp, bool re): BlopBesitzer(sp), mRechterSpieler(re),
       mRectW(0), mRectH(0) {
  mBlop[0].setBesitzer(this, ort_absolut(absort_fall, re, 0));
  mBlop[1].setBesitzer(this, ort_absolut(absort_fall, re, 1));
}

/**  Muß einmal aufgerufen werden */
void Fall::initialisiere() {
  /* Initialisiert werden müssen sie schon, bevor sich jemand bequemt,
     erzeug() aufzurufen:
     Mit @0 und @1 kann drauf zugegriffen werden.
     Nur halt noch nicht sinnvoll */
  mBlop[0] = Blop(blopart_keins);
  mBlop[1] = Blop(blopart_keins);
}

	
/** Erzeugt ein neues Fall. Liefert false, wenn dafür kein Platz ist */
bool Fall::erzeug() {
  if (ld->mFallPosZufaellig)
    mPos.x = Aufnahme::rnd(grx-1);
  else
    mPos.x = grx / 2 - 1;
  mPos.yy = mSpf->getHetzrandYPix() - gric;
  mPos.r = richtung_waag;
	
  /* Platz? */
  if (testBelegt(mPos)) {
    mPos.r = richtung_keins;
    return false;
  }

  for (int i=0; i<2; i++)
    mBlop[i]=ld->zufallsSorte(wv_farbe);

  mExtraDreh = 0;
  mExtraX = 0;
  mSchnell = false;
		
  return true;
}
	
	
/** Liefert true, wenn das Fallende senkrecht ist */
bool Fall::istSenkrecht() const {
  CASSERT(mPos.r == richtung_waag || mPos.r == richtung_senk);
  return mPos.r == richtung_senk;
}

	
/** Lässt nur noch Blop a übrig */
void Fall::halbiere(int a) {
  CASSERT(mPos.r == richtung_waag);
  CASSERT(a == 0 || a == 1);
  mPos.x += a;
  mPos.r = richtung_einzel;
  mExtraDreh = 0;
  mExtraX = 0;
}
	
/** Entfernt das Fall ganz */
void Fall::zerstoere() {
  mPos.r = richtung_keins;
}


/** Macht alles von spielSchritt ausser Grafik-Update  markieren */
void Fall::spielSchrittIntern() {
  /* Bei nicht-Existenz nichts tun. */
  if (mPos.r == richtung_keins)
    return;

  /* Wenn der Stein von einem früheren Tastendruck noch nicht fertig
     gedreht ist, dann weiterdrehen */
  if (mExtraDreh > 0)
    mExtraDreh--;
  /* Wenn der Stein noch nicht fertig X-verschoben ist, dann jetzt fertig
     X-verschieben */
  if (mExtraX > 0) mExtraX -= am_schieben_diff;
  if (mExtraX < 0) mExtraX += am_schieben_diff;


 	
  /* Wenn am Platzen, dann nicht mehr nach unten bewegen */
  if (getAmPlatzen())
    return;
 		
  /* Neue y-Koordinate... */
  FallPos fp2 = mPos;
  if (mSchnell || mPos.r == richtung_einzel)
    fp2.yy += mSpf->getSemiglobal().getVariable(spezvar_falling_fast_speed);
  else
    fp2.yy += mSpf->getSemiglobal().getVariable(spezvar_falling_speed);
  /* Geschwindigkeit könnte ja negativ sein. Also clippen. */
  if (fp2.yy < mSpf->getHetzrandYPix() - gric)
    fp2.yy = mSpf->getHetzrandYPix() - gric;

  /* Gibt's Platz um weiterzufallen? */
  int beleg = testBelegt(fp2);
  if (!beleg) {
    mPos = fp2;
    return;
  }
 	
  /* OK, wir kommen irgendwo auf. */
  playSample(sample_land);
 	
  /* Einzelblop? */
  if (mPos.r == richtung_einzel) {
    Blop * b1 = festige(0);
    if (b1) b1->execEvent(event_land);
    zerstoere();
    return;
  }

  /* Senkrecht? */
  if (istSenkrecht()) {
    /* Wichtig: Erst den unteren Blop festigen; es könnte sein, dass die Blops
       weiter oben gefestigt werden müssen, als geplant (wg. Hochverschiebung
       oder sogar wegen richtiger Spielfeldänderung am Ende von einer Zeilen-
       übergabe)... */
    Blop * b1 = festige(1);
    Blop * b0 = festige(0);
    if (b1) b1->execEvent(event_land);
    if (b0) b0->execEvent(event_land);
    zerstoere();
    return;
  }

  /* OK, waagrecht. Welche Teile? */
  //Blop::beginGleichzeitig();
  Blop * b0 = 0, * b1 = 0;
  if (beleg & belegt_0) b0 = festige(0);
  if (beleg & belegt_1) b1 = festige(1);
  if (b0) b0->execEvent(event_land);
  if (b1) b1->execEvent(event_land);
  //Blop::endGleichzeitig();


  /* Beide Hälften aufgekommen? */ 		
  if (beleg == belegt_beide) {
    zerstoere();
    return;
  }
 			
  /* Nur eine Hälfte aufgekommen. */
  mPos = fp2;
  if (beleg == belegt_0) {
    // linke Hälfte aufgekommen =>
    // übriges rechtes nach links schieben
    mBlop[0] = mBlop[1];
  }
  /* Nur noch die Hälfte vom Fall ist übrig; wenn das linke fest ist,
     dann die rechte Hälfte. */
  halbiere(beleg == belegt_0);
}  // spielSchrittIntern




/** Bewegt das Fall ggf. nach unten und animiert es ggf.
 */
void Fall::spielSchritt() {

  /* Das meiste passiert hier */
  spielSchrittIntern();
  
  /* Altes Rechteck zum updaten markieren,
     neues Rechteck bestimmen,
     neues Rechteck zum updaten markieren.
     
     Achtung: Altes Rechteck kann auch vor spielSchrittIntern()
     nicht mehr berechnet werden, da Tastendruecke das Fall verschoben
     haben koennen.
     */
  setUpdateFallRect();
  calcFallRect();
  setUpdateFallRect();
  
}  // spielSchritt




/** Führt die Animationen durch. Innerhalb einer Gleichzeit aufrufen. */
void Fall::animiere() {
  //CASSERT(gGleichZeit);
  for (int i = 0; i < getAnz(); i++)
    mBlop[i].animiere();
}


/** kopiert einen fallenden Blop nach mDaten und liefert den
    Zielblop in mDaten zurück (als Referenz), damit man einen
    land-Event senden kann. Sendet den land-Event nicht selbst,
    weil Cual-Code erwarten könnte, dass erst beide Blops
    gefestigt werden und dann erst die Events kommen.
    Kann 0 zurückliefern (wenn der Blop keinen Platz auf
    dem Bildschirm hat). */
Blop * Fall::festige(int n) {
  int x = getX(n);
  int y = getY(n);

  /* Evtl. ist dieses Feld schon belegt. Dann so lange weiter oben probieren,
     bis wir ein freies Feld finden. */
  while (mSpf->getDatenPtr()->getFeldArt(x, y) != blopart_keins) {
    y--;
    /* Sollte tatsächlich (auf welche Art auch immer) plötzlich die ganze Spalte
       voll sein, dann verschwindet der Blop halt. */
    if (y < 0)
      return 0;
  }
  Blop & dst = mSpf->getDatenPtr()->getFeld(x, y);
  dst = mBlop[n];
  return &dst;
}



/** Bewegt das Fall eins nach links */
void Fall::tasteLinks() {
  mBlop[0].execEvent(event_keyleft);
  mBlop[1].execEvent(event_keyleft);
  if (steuerbar()) {
    FallPos fp2 = mPos;
    fp2.x--;
    if (!testBelegt(fp2)) {
      mPos = fp2;
      /* Auf dem Bildschirm soll der Fall noch nicht fertigverschoben
	 erscheinen */
      mExtraX = am_schieben_start;
    }
  }
}

/** Bewegt das Fall eins nach rechts */
void Fall::tasteRechts(){
  mBlop[0].execEvent(event_keyright);
  mBlop[1].execEvent(event_keyright);
  if (steuerbar()) {
    FallPos fp2 = mPos;
    fp2.x++;
    if (!testBelegt(fp2)) {
      mPos = fp2;
      /* Auf dem Bildschirm soll der Fall noch nicht fertigverschoben
	 erscheinen */
      mExtraX = -am_schieben_start;
    }
  }
}

/** Dreht das Fall */
void Fall::tasteDreh1(){
  mBlop[0].execEvent(event_keyturn);
  mBlop[1].execEvent(event_keyturn);
}

/** Dreht das Fall */
void Fall::tasteDreh2(){
  if (steuerbar()) {
    FallPos fp2 = mPos;
    fp2.r = fp2.r == richtung_waag ? richtung_senk : richtung_waag;
    if (!testBelegt(fp2)) {
      mPos = fp2;
      /* Drehrichtung bei senkrecht gespiegeltem Level andersrum, damit
	 es für den Spieler gleich erscheint */
      if (ld->mSpiegeln ? fp2.r == richtung_senk : fp2.r == richtung_waag) {
	Blop b = mBlop[0];
	mBlop[0] = mBlop[1];
	mBlop[1] = b;
      }
      /* Für Level, bei denen sich die Teile beim Drehen verändern... */
      Blop::beginGleichzeitig();
      mBlop[0].execEvent(event_turn);
      mBlop[1].execEvent(event_turn);
      Blop::endGleichzeitig();

      /* Auf dem Bildschirm soll der Fall noch nicht fertiggedreht
	 erscheinen */
      mExtraDreh = am_drehen_start;
    }
  }
}

/** Ändert die Fallgeschwindigkeit vom Fall */
void Fall::tasteFall(){
  mBlop[0].execEvent(event_keyfall);
  mBlop[1].execEvent(event_keyfall);
  if (steuerbar()) {
    mSchnell = !mSchnell;
  }
}


/** Liefert true, wenn das Fall (noch) am Platzen ist
    (wg. Spielende) */
bool Fall::getAmPlatzen() const {
  if (mPos.r == richtung_keins)
    return false;

  /* Wenn einer platzt, dann beide. */
  CASSERT(mPos.r == richtung_einzel ||
         mBlop[0].getAmPlatzen() == mBlop[1].getAmPlatzen());

  return mBlop[0].getAmPlatzen();
}

/** Lässt alle Blops vom Fall platzen (Spielende). */
void Fall::lassPlatzen() {
  CASSERT(mPos.r != richtung_keins);
  for (int i = 0; i < getAnz(); i++)
    mBlop[i].lassPlatzen();
}

/** Liefert einen Pointer auf die Blops zurück. Wird vom
    KIPlayer und von @0 und @1 benötigt. */
const Blop * Fall::getBlop() const {
  return mBlop;
}

Blop * Fall::getBlop() {
  return mBlop;
}


/** Malt das Fall. */
void Fall::malen() const {
  /* Wenn kein Fallendes unterwegs ist, liefert getAnz() 0 */
  for (int i = 0; i < getAnz(); i++)
    mBlop[i].malen(getXX(i), getYY(i));
}


int Fall::testBelegt(FallPos p) const {
  int i = 0;
  int ret = belegt_keins;

  for (; i < p.getAnz(); i++)
    if (!mSpf->getDatenPtr()->testPlatzSpalte(p.getX(i), p.getY(i, mSpf->getHochVerschiebung())))
      ret |= (i == 0 ? belegt_0 : belegt_1);

  /* Sonderfall: Wenn im Senkrecht-Modus Blop 1 belegt ist,
     dann per Definition auch Blop 0.
     (Das wird zwar (im Moment) nirgends verwendet, macht aber Sinn.) */
  if (mPos.r == richtung_senk && ret) ret = belegt_beide;
	
  return ret;
}
/** Liefert true, wenn das Fall existiert.
    Mit Argument: Wenn diese Hälfte existiert. */
bool Fall::existiert(int a/*=0*/) const {
  return mPos.r != richtung_keins && !(mPos.r == richtung_einzel && a == 1);
}



int Fall::getX(int a) const {
  if (!existiert(a)) return blop_pos_nix;
  return mPos.getX(a);
}

int Fall::getY(int a) const {
  if (!existiert(a)) return blop_pos_nix;
  return mPos.getY(a, mSpf->getHochVerschiebung());
}



int Fall::getXX(int a) const {
  if (!existiert(a)) return blop_pos_nix;

  /*           Gedreht              Ungedreht */
  /*           Normal    Spiegel              */
  /*           Waag Senk Waag Senk            */
  int drehx = "2215 2243 2215 4322  2"[getDrehIndex(a)] - '1';
  /*           1*>01>0* 0*>01 10>0*           */
  /*           0  *  1  1  *  *  1            */

  /* 1: -1 + sin   2: 0   3: sin  4: cos  5: -1 + cos   (bei 30 Grad) */
  int wandel[] = {-gric + am_drehen_schieb_klein, 0,
    am_drehen_schieb_klein, am_drehen_schieb_gross,
    -gric + am_drehen_schieb_gross};

  return (mPos.x + a*(mPos.r==richtung_waag))*gric + mExtraX + wandel[drehx];
}

int Fall::getYY(int a) const {
  if (!existiert(a)) return blop_pos_nix;

  /*           Gedreht              Ungedreht */
  /*           Normal    Spiegel              */
  /*           Waag Senk Waag Senk            */
  int drehy = "4322 2215 2243 2215  2"[getDrehIndex(a)] - '1';
  /*           1*>01>0* 0*>01 10>0*           */
  /*           0  *  1  1  *  *  1            */

  /* 1: -1 + sin   2: 0   3: sin  4: cos  5: -1 + cos   (bei 30 Grad) */
  int wandel[] = {-gric + am_drehen_schieb_klein, 0,
    am_drehen_schieb_klein, am_drehen_schieb_gross,
    -gric + am_drehen_schieb_gross};

  return mPos.yy + a*(mPos.r==richtung_senk)*gric + wandel[drehy];
}

/** Kodiert alle Informationen des Drehens in eine Zahl.
    Ist für getXX und getYY da */
int Fall::getDrehIndex(int a) const {
/* Bit 21: gar nicht gedreht | */
/* Bit 10: Spiegel? | Bit 5: wirdSenk? | */
/* Bit 2: Blob1? | Bit 1: Schritt1? */

  if (mExtraDreh != 0) {

    /* 3 bedeutet eigentlich: Noch gar nicht gedreht. Normalerweise wird
       nach einem Tastendruck erst mal ein spielSchritt() aufgerufen (mit
       mExtraDreh--) und dann das Fall erst neu gemalt. Bei einem außer-
       ordentlichen Update könnte allerdings auch schon früher neu gemalt
       werden. Der Einfachheit halber malen wir da schon angedreht. */
    int ed = mExtraDreh;
    if (ed == 3) ed = 2;

    return 10 * ld->mSpiegeln + 5 * istSenkrecht() + 2 * a + (ed == 1);
  }
  else return 21;
}


/** Bestimm mFallRect neu (Fall-ueberdeckendes Rechteck) */
void Fall::calcFallRect() {
  if (mPos.r == richtung_keins)
    mRectW = mRectH = 0;
  else {
    /* Faule Variante... ohne Beachtung der Drehposition und der genauen
       y-Position */
    int li = mPos.x;
    int re = mPos.x + 2;
    if (mExtraX < 0) li--;
    if (mExtraX > 0) re++;
    mRectX = li * gric;
    mRectW = (re - li) * gric;
    mRectY = mPos.yy;
    mRectH = 3 * gric;
  }
}


/** Setzt den Bereich, der durch mRect* angegeben ist, auf
    upzudaten */
void Fall::setUpdateFallRect() {
  if (mRectW != 0) {
    mSpf->setUpdateRect(mRectX, mRectY, mRectW, mRectH);
  }
}



/** Liefert true, wenn das Fall aus grade am zerfallen ist
    (d. h. existiert, aber aus nur noch einem Blop besteht).
    In dieser Zeit darf nämlich keine Explosion gezündet
    werden. (Erst warten, bis der andere Blop auch angekommen
    ist.) */
bool Fall::istEinzel() const {
  return mPos.r == richtung_einzel;
}


void Fall::playSample(int nr) const {
  Sound::playSample(nr, mRechterSpieler ? so_rfeld : so_lfeld,
		    2*mPos.x+1+(mPos.r==richtung_waag), 2*grx);
}


int Fall::getSpezConst(int vnr, const Blop * wer) const {
  /* Wie nett: Man hat uns mitgeteilt, welcher der beiden
     Blops anfragt. */
  bool bin_1 = wer->getOrt().x == 1;
  
  switch (vnr) {
  case spezconst_loc_x:
    return getX(bin_1);
  case spezconst_loc_y:
    return getY(bin_1);
  case spezconst_loc_xx:
    return getXX(bin_1);
  case spezconst_loc_yy:
    return getYY(bin_1);
  case spezconst_turn:
    /* Potentieller Bug:
       Hier stand mal < 3 statt < 4 (und "021" statt "0211").
       Jene assertion wurde nichtreproduzierbar verletzt.
       Die Verletzung war schon beim ersten Bildaufbau (vorm Spiel),
       aber nicht im ersten Spiel. Sollte man mal bei Gelegenheit verfolgen. */
    CASSERT(mExtraDreh < 4);
    return "0211"[mExtraDreh] - '0';
  case spezconst_falling:
    return 1;
  case spezconst_falling_fast:
    return mSchnell;
  }

  /* Wir wissen von nix; Blop::getSpezConst() soll den Default-Wert
     zurückliefern. */
  return spezconst_defaultwert;
}

