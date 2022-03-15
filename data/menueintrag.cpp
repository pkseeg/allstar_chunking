/***************************************************************************
                          menueintrag.cpp  -  description
                             -------------------
    begin                : Mit Jul 12 22:54:51 MEST 2000
    copyright            : (C) 2006 by Immi
    email                : cuyo@pcpool.mathematik.uni-freiburg.de

Modified 2006-2008,2010,2011 by the cuyo developers

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>


#include "sdltools.h"

#include "font.h"

#include "cuyointl.h"
#include "fehler.h"
#include "global.h"

#include "prefsdaten.h"

#include "layout.h"
#include "menueintrag.h"
#include "ui2cuyo.h"
#include "ui.h"

#include "sound.h"

#define HOT_ALIGN 1

#define hotkey_keins (-1)
#define hotkey_rahmen (-2)  // Rahmen um alles falls subBereich == subbereich_hyperaktiv


/*****************************************************************************/

DrawDing::DrawDing(const Str & text, int hotkey,
   	           int binSubBereich, /* Subbereich, zu dem dieses Drawding gehört */
                   int x, int y,
		   int align /*= AlignHCenter*/,   /* Akzeptiert nur waagerechtes Zeug,
				       senkrecht ist immer zentriert. */
                   Font * font, /*= NULL*/  /* Default hängt von aktSubBereich ab */
		   int * xmin /*= NULL*/, int * xmax /*= NULL*/
		   /* Wenn die !=0 sind, wird dort schon mal unsere Ausdehung
		      reingeschrieben. */) :
    mArt(dda_Text), mText(text),
    mHotkey(hotkey == hotkey_rahmen ? hotkey_keins : hotkey),
    mFont(font),
    //mDx(hotkey==hotkey_rahmen ? L_menu_rand_lr : 0),
    mRahmen(hotkey==hotkey_rahmen),
    mBinSubBereich(binSubBereich),
    mAbschneiden(false) {
  /* Irgend eine Schrift brauchen wir, um die Breite zu messen: */
  if (!font) font = Font::gMenu;
  
  int fh = font->getFontHeight();
  mY0 = y - fh / 2 - L_menu_rand_ou;
  mY1 = mY0 + fh + 2 * L_menu_rand_ou;
  
  int w = font->getLineWidth(mText.data()) + 2*L_menu_rand_lr;
  switch (align) {
    case AlignLeft: mX0=x; mX1=x+w; break;
    case AlignRight: mX0=x-w; mX1=x; break;
    case AlignHCenter:
      mX0=x-(w+1)/2; mX1=x+w/2;
      break;
    default: throw iFehler("%s",_("Invalid alignment"));
  }
  #if HOT_ALIGN
      if (hotkey >= 0) {
        int w2 = font->getLineWidth(mText.left(hotkey).data());
        w2 += font->getLineWidth(mText.mid(hotkey, 1).data()) / 2;
        mX0 = x - w2;
        mX1 = mX0 + w;
      }
  #endif
  mXPos = mX0 + L_menu_rand_lr;
  if (xmin) *xmin=mX0;
  if (xmax) *xmax=mX1;
}

DrawDing::DrawDing(int bild, int bildchen, int x, int y) :
    mArt(dda_Icon), mBild(bild), mBildchen(bildchen),
    mBinSubBereich(subbereich_default),
    mX0(x-gric/2), mX1(x+gric/2), mY0(y-gric/2), mY1(y+gric/2), mXPos(mX0),
    mAbschneiden(false) {}

/* Wenn man nicht möchte, dass das Bild zur Größe des Hintergrundrahmens
   beiträgt, wäre es vielleicht inzwischen sauberer, binsubbereich auf
   subbereich_keiner zu setzen (und in anzeigen() einbauen, dass das
   erlaubt ist) */
/* Folgende Funktion geht grad nicht: */
// DrawDing::DrawDing(int bild, int xbd, int ybd, int x, int y) :
//     mArt(dda_Bild),
//     mBild(bild), mRect(SDLTools::rect(xbd,ybd,2*gric,2*gric)),
//     mBinSubBereich(subbereich_default),
//     mX0(x), mX1(x), mY0(y),mY1(y), mXPos(mX0-gric),
//     mAbschneiden(false) {}
DrawDing::DrawDing(int bild, int x, int y) :
    mArt(dda_Bild),
    mBild(bild),
    mBinSubBereich(subbereich_default),
    mX0(x), mX1(x), mY0(y), mY1(y),
    mAbschneiden(false) {}



void DrawDing::abschneiden(int x0, int x1) {
  mX0 = x0;
  mX1 = x1;
  mAbschneiden = true;
}



void DrawDing::anzeigen(int subBereich, int x, int y) const {
  if (mAbschneiden)
    Area::setClip(SDLTools::rect(x + mX0, 0, mX1 - mX0, L_fenster_hoehe));
  switch (mArt) {
  case dda_Nichts:
    CASSERT(false); break;
  case dda_Text: {
    int xh=0;
    Str hotkey="";
    bool bright = subBereich == subbereich_hyperaktiv || subBereich == mBinSubBereich;
    Font * font = mFont;
    if (subBereich == subbereich_keinStrom)
      font = Font::gDimmed;
    if (!font)
      font = bright ? Font::gBright : Font::gMenu;

    if (mRahmen && subBereich == subbereich_hyperaktiv) {
      Color colour = Color(130,130,220);
      Area::fillRect(x+mX0,y+mY0,
		     mX1-mX0, 2, colour);
      Area::fillRect(x+mX0,y+mY1-2,
		     mX1-mX0, 2, colour);
      Area::fillRect(x+mX0,y+mY0,
		     2, mY1-mY0, colour);
      Area::fillRect(x+mX1-2,y+mY0,
		     2, mY1-mY0, colour);
    }

    int hot = mHotkey;
    if (bright)
      hot = hotkey_keins;
	
    if (hot>=0) {
      xh = mXPos + font->getLineWidth(mText.left(mHotkey).data());
      hotkey = mText.mid(mHotkey,1);

      if (subBereich != subbereich_keinStrom) {
	/* Einen Highlightkreis um den Hotkey malen */
	int x_ = x + xh+font->getLineWidth(hotkey.data())/2;
	int y_ = y + (mY0+mY1)/2;
	Bilddatei & bild = *Blatt::gBlattPics[blattpic_highlight];
	bild.malBildchen(x_-gric,y_-gric,0);
	bild.malBildchen(x_,y_-gric,2);
	bild.malBildchen(x_-gric,y_,6);
	bild.malBildchen(x_,y_,8);
      }
    }

    font->drawText(mText, x + mXPos, y + mY0 + L_menu_rand_ou,AlignTopLeft);

    if (hot>=0) {
      (subBereich == subbereich_keinStrom ? Font::gBrightDimmed : Font::gBright)->
         drawText(hotkey, x + xh, y + mY0 + L_menu_rand_ou, AlignTopLeft);
    }

    } break;
  case dda_Icon:
    Blatt::gBlattPics[mBild]->malBildchen(x+mXPos,y+mY0,mBildchen);
    break;
  case dda_Bild: {
      Bilddatei & bi = *Blatt::gBlattPics[mBild];
      bi.malBild(x+mX0-bi.getBreite() / 2,y+mY0 - bi.getHoehe() / 2);
      break;
    }
  }
  if (mAbschneiden)
    Area::noClip();
}




/*****************************************************************************/


MenuEintrag::MenuEintrag(BlattMenu * papi,
			 Str na /*= ""*/,
			 void(*doret)() /*= 0*/,
			 int accel /*= 0*/,
			 int hoehe /*= L_menueintrag_defaulthoehe*/):
        mHoehe(hoehe), mPapi(papi),
	mName(na), mAccel(accel), mAccIndex(hotkey_keins), mDoReturn(doret),
        mGetStrom(NULL),
	mArt(Art_normal), mX0(-11111), mUpdaten(false), mSubBereich(subbereich_nichtInitialisiert) {
  /* Hotkey suchen */
  if (mAccel==0) {
    for (int i=0; i<mName.length(); i++)
      if (mName[i]=='~')
	mAccIndex=i;
    if (mAccIndex!=hotkey_keins) {
      mName = mName.left(mAccIndex)+mName.right(mAccIndex+1);
      mAccel = mName[mAccIndex];
      if (mAccel>='a' && mAccel<='z')
	mAccel += 'A' - 'a';
    }
  }
}


MenuEintrag::MenuEintrag(BlattMenu * papi, Str na, Art ea,
			 int hoehe /*= L_menueeintrag_defaulthoehe*/):
        mHoehe(hoehe), mPapi(papi), mName(na),
	mAccel(0), mAccIndex(hotkey_keins), mDoReturn(NULL),
        mGetStrom(NULL),
	mArt(ea), mX0(-11111), mUpdaten(false), mSubBereich(subbereich_nichtInitialisiert) {}


bool nieStrom() {return false;}

void MenuEintrag::setNieStrom() {
  setGetStrom(nieStrom);
}


void MenuEintrag::setSubBereich(int subBereich) {
  int alt_sb = mSubBereich;
  mSubBereich = subBereich;
  if (!getStrom()) mSubBereich = subbereich_keinStrom;
  if (mSubBereich != alt_sb)
    updateDrawDinge();
}

/* Aufrufen, wenn sich möglicherweise der Stromstatus geändert hat */
void MenuEintrag::updateStrom() {
  bool neustrom = getStrom();
  if (neustrom != (mSubBereich != subbereich_keinStrom))
    setSubBereich(neustrom ? subbereich_keiner : subbereich_keinStrom);
}

void MenuEintrag::updateDrawDinge() {
  CASSERT(mSubBereich != subbereich_nichtInitialisiert);
  mAnzDraw = 0;
  updateDDIntern();

  mX0 = L_fenster_breite_menus;
  mX1 = -L_fenster_breite_menus;
  for (int i=0; i<mAnzDraw; i++)
    if (mDraw[i].mBinSubBereich == subbereich_default) {
      if (mDraw[i].mX0<mX0) mX0=mDraw[i].mX0;
      if (mDraw[i].mX1>mX1) mX1=mDraw[i].mX1;
    }

  setUpdateFlag();
  UI::nachEventAllesAnzeigen();
}


void MenuEintrag::deactivateAccel() {
  mAccel=0;
  mAccIndex=hotkey_keins;
}

void MenuEintrag::anzeigen(int x, int y, bool graue) {

  /* Unschön, könnte irgendwann mal ein Bug werden, tut aber im Moment:
     Eigentlich müsste ein MenüEintrag informiert werden, wenn es sich
     ändert, ob die Grauen neben ihm sitzen oder nicht, damit er ein
     Graphik-Update machen kann (mit setUpdateFlag()). Im Moment werden
     die grauen aber immer nur dann verschoben, wenn auch was anderes am
     Eintrag verändert wird, so dass die erforderlichen Graphik-Updates
     sowieso geschehen */

  CASSERT(mSubBereich != subbereich_nichtInitialisiert);

  if (!mUpdaten) return;
  mUpdaten = false;
  
  //print_to_stderr(_sprintf("%s\n", mName.data()));
  
  if (mAnzDraw == 0)
    return;

  /* Normaler Hintergrund */
  Area::fillRect(0, y, L_fenster_breite_menus, mHoehe, Color(30, 30, 70));
  Area::updateRect(0, y, L_fenster_breite_menus, mHoehe);

  /* Position von Highlight-Hintergrund und Grauen bestimmen */

  int hx0 = L_fenster_breite_menus, hx1 = 0; /* Hintergrund */
  int hy0 = L_fenster_hoehe, hy1 = 0;

  int gx0 = L_fenster_breite_menus, gx1 = 0; /* Graue */

  for (int i=0; i<mAnzDraw; i++) {
    if (mDraw[i].mX0<gx0) gx0=mDraw[i].mX0;
    if (mDraw[i].mX1>gx1) gx1=mDraw[i].mX1;
    if (mDraw[i].mBinSubBereich == mSubBereich || mSubBereich == subbereich_hyperaktiv) {
      if (x+mDraw[i].mX0<hx0) hx0=x+mDraw[i].mX0;
      if (x+mDraw[i].mX1>hx1) hx1=x+mDraw[i].mX1;
      if (y+mDraw[i].mY0<hy0) hy0=y+mDraw[i].mY0;
      if (y+mDraw[i].mY1>hy1) hy1=y+mDraw[i].mY1;
    }
  }

  /* Hintergrund malen */
  if (hx1 > 0) {
    Bilddatei & bild = *Blatt::gBlattPics[blattpic_highlight];
    int w = hx1-hx0;
    int h = hy1-hy0;
    int w1,w2,w3, h1,h2,h3;
    if (w>=2*L_menueintrag_highlight_rad) {
      w1 = L_menueintrag_highlight_rad;
      w2 = w-2*L_menueintrag_highlight_rad;
      w3 = L_menueintrag_highlight_rad;
    } else {
      w1 = w/2;
      w2 = 0;
      w3 = w-w1;
    }
    if (h>=2*L_menueintrag_highlight_rad) {
      h1 = L_menueintrag_highlight_rad;
      h2 = h-2*L_menueintrag_highlight_rad;
      h3 = L_menueintrag_highlight_rad;
    } else {
      h1 = h/2;
      h2 = 0;
      h3 = h-h1;
    }
    // Ecken
    bild.malBildAusschnitt(hx0,hy0,SDLTools::rect(gric-w1,gric-h1,w1,h1));
    bild.malBildAusschnitt(hx1-w3,hy0,SDLTools::rect(2*gric,gric-h1,w3,h1));
    bild.malBildAusschnitt(hx0,hy1-h3,SDLTools::rect(gric-w1,2*gric,w1,h3));
    bild.malBildAusschnitt(hx1-w3,hy1-h3,SDLTools::rect(2*gric,2*gric,w3,h3));
    // Waagerechte Kanten
    bild.malStreifenH(hx0+w1, hy0, w2, SDLTools::rect(gric,gric-h1,gric,h1));
    bild.malStreifenH(hx0+w1, hy1-h3, w2, SDLTools::rect(gric,2*gric,gric,h3));
    // Senkrechte Kanten
    bild.malStreifenV(hx0, hy0+h1, h2, SDLTools::rect(gric-w1,gric,w1,gric));
    bild.malStreifenV(hx1-w3, hy0+h1, h2, SDLTools::rect(2*gric,gric,w3,gric));
    // Fläche
    if (w2>0 && h2>0)
      Area::fillRect(hx0+w1, hy0+h1, w2, h2, Color(50, 50, 120));
        /* Wenn diese Farbe geändert wird, muß das auch in
	   some_pic_sources/highlight.pov geschehen. */
  }

  /* Eigentliches Zeug malen */
  for (int i=0; i<mAnzDraw; i++)
    mDraw[i].anzeigen(mSubBereich, x, y);
  
  /* Graue malen */
  if (graue) {
    Blatt::gBlattPics[blattpic_pfeile]->malBildchen(x+gx0-L_grausep-gric,
						     y+mHoehe/2-gric/2,
						     0);
    Blatt::gBlattPics[blattpic_pfeile]->malBildchen(x+gx1+L_grausep,
						     y+mHoehe/2-gric/2,
						     1);
  }
}


void MenuEintrag::doReturn(bool) {
  if (mDoReturn) {
    Sound::playSample(sample_menuclick,so_fenster);
    mDoReturn();
  }
}

bool MenuEintrag::getStrom() const {
  if (mGetStrom)
    return mGetStrom();
  else
    return true;
}


int MenuEintrag::getMausPos(int x, int /*y*/) {
  CASSERT(mSubBereich != subbereich_nichtInitialisiert);

  return x >= mX0 && x < mX1 ? subbereich_default : subbereich_keiner;
}



bool MenuEintrag::getWaehlbar() const {
  return mArt != Art_deko && getStrom();
}

bool MenuEintrag::getAktiv() const {
  return mArt==Art_aktiv || mArt==Art_hyperakt;
}

bool MenuEintrag::getHyper() const {
  return mArt==Art_hyper || mArt==Art_hyperakt;
}


void MenuEintrag::doPapiEscape() {
  mPapi->doEscape();
}

void MenuEintrag::doPapiNavigiere(int d) {
  mPapi->navigiere(d);
}

void MenuEintrag::updateDDIntern() {
  neuDraw() = DrawDing(mName, mAccIndex, subbereich_default, 0, mHoehe / 2,
		       AlignHCenter,
		       mArt == Art_deko ? Font::gTitle : NULL);
}

/*****************************************************************************/

MenuEintragBild::MenuEintragBild(BlattMenu * papi, int nr):
   MenuEintrag(papi, "", MenuEintrag::Art_deko,
	       Blatt::gBlattPics[nr]->getHoehe()),
   mBildNr(nr) {
}


void MenuEintragBild::updateDDIntern() {
  neuDraw() = DrawDing(mBildNr, 0, mHoehe / 2);
}



/*****************************************************************************/

MenuEintragEscape::MenuEintragEscape(BlattMenu * papi) :
    // TRANSLATORS: "Done" is a menu entry for quitting a submenu.
    MenuEintrag(papi,_("Done"),NULL,SDLK_ESCAPE) {
}

void MenuEintragEscape::doReturn(bool) {
  /* Sound wird gespielt, wenn das Menü sich zumacht. */
  doPapiEscape();
}



/*****************************************************************************/

MenuEintragSubmenu::MenuEintragSubmenu(BlattMenu * papi,
				       const Str & name, BlattMenu * menu,
				       int accel /*=0*/,
				       int hoehe
				         /*= L_menueintrag_defaulthoehe*/) :
    MenuEintrag(papi,name,NULL,accel,hoehe), mSub(menu) {
}

MenuEintragSubmenu::~MenuEintragSubmenu() {
  delete mSub;
}

void MenuEintragSubmenu::doReturn(bool durchMaus) {
  Sound::playSample(sample_menuclick,so_fenster);
  mSub->oeffnen(durchMaus);
}



/*****************************************************************************/

MenuEintragAuswahl::MenuEintragAuswahl(BlattMenu * papi,
				       const Str & na, const Str & info,
				       void(*doretint)(int), int arg,
				       int accel /*=0*/) :
    MenuEintrag(papi,na,NULL,accel), mArg(arg), mInfo(info),
    mDoReturnInt(doretint) {
}

void MenuEintragAuswahl::doReturn(bool) {
  /* Sound nicht abspielen; das tut schon doPapiEscape() */
  //Sound::playSample(sample_menuclick,so_fenster);
  /* Vorsicht: Im Moment *muss* das folgende in dieser Reihenfolge
     passieren wegen des levelpack-menus: Bei doPapiEscape() wird
     die Graphik des Levelpack-Menüeintrags geupdatet. Der neue
     Levelpack muss davor schon ausgewählt worden sein. */
  mDoReturnInt(mArg);
  doPapiEscape();
}



/*****************************************************************************/

BlattMenu * auswahlmenu(MenuEintragSubmenu * obereintrag, BlattMenu * obermenu,
                        const std::vector<Str> & moeglichkeiten,
			const std::vector<Str> & infos,
			void (*doret)(int),
			const Str & titel, Str info,
			int & vorlauf) {
  BlattMenu * ret = new BlattMenu(true);  // true = scrollleiste immer da... und esc-button
  ret->setObermenu(obermenu);
  ret->setObereintrag(obereintrag);
  if (titel=="")
    vorlauf = 0;
  else {
    /* Erstmal alle '~' rauslöschen */
    Str titel_ = titel;
    for (int i=0; i<titel_.length(); i++)
      if (titel_[i]=='~')
	titel_ = titel_.left(i)+titel_.right(i+1);
    ret->neuerEintrag(new MenuEintrag(ret, titel_, MenuEintrag::Art_deko));
    ret->neuerEintrag(new MenuEintrag(ret, "",
				      MenuEintrag::Art_deko, L_medskip));
    vorlauf = 2;
  }
  for (size_t i=0; i<moeglichkeiten.size(); i++)
    ret->neuerEintrag(new MenuEintragAuswahl(ret, moeglichkeiten[i],
					     i<infos.size() ? infos[i] : "",
					     doret, i));
  if (info!=Str())
    ret->neuerEintrag(new MenuEintrag(ret, "",
				      MenuEintrag::Art_deko, L_medskip));
  while (info!=Str()) {
    // Zeilenende suchen
    int i;
    for (i=0; i<info.length() ? (info[i]!='\n') : false; i++) {}
    ret->neuerEintrag(new MenuEintrag(ret, info.left(i),
				      MenuEintrag::Art_deko));
    if (i<info.length())
      i++;
    info = info.right(i);
  }

  return ret;
}


MenuEintragAuswahlmenu::MenuEintragAuswahlmenu(BlattMenu * papi,
					       const Str & name,
                                               const std::vector<Str> *const
					         auswahlen,
					       const std::vector<Str> *const
					         infos,
					       int (*getakt) (),
					       void (*doret)(int),
					       const Str & info /*= Str()*/,
					       int accel /*=0*/) :
    MenuEintragSubmenu(papi, name,
		       auswahlmenu(this, papi, *auswahlen, *infos,
				   doret, name, info, mVorlauf),
		       accel,
		       L_menueintrag_defaulthoehe+L_font_height),
    mAuswahlen(auswahlen), mGetAktuell(getakt),
    mEintragDoReturn(doret),
    mAnimation(0), mAnimationDX(0) {
  mArt = Art_aktiv;
}

Str MenuEintragAuswahlmenu::getInfo() {
  return mSubBereich == subbereich_default
    ? _("Press `return' or click for a full list")
    : "";
}

void MenuEintragAuswahlmenu::doHyperaktiv(const SDL_keysym & key, int taste) {
  switch (taste) {
    case SDLK_RIGHT:
      doPfeil(1);
      break;
    case SDLK_LEFT:
      doPfeil(-1);
      break;
    default:
      MenuEintragSubmenu::doHyperaktiv(key,taste);
      break;
  }
}


int MenuEintragAuswahlmenu::getMausPos(int x, int y) {

  /* Manuel zusammengeflicktes befindet-sich-die-Maus-über-einem-Pfeil.
     Wenn wir irgendwann mehr solche SubBereiche haben, will man das
     schöner machen. */
    
  if (y >= mHoehe/2) {
    if (x >= mPfeil1X0 && x < mPfeil1X1)
      return 1;
    if (x >= mPfeil2X0 && x < mPfeil2X1)
      return 2;
  }
  
  return MenuEintrag::getMausPos(x, y);
}



void MenuEintragAuswahlmenu::zeitSchritt() {
  if (mAnimation != 0) {
    mAnimation += mAnimation > 0 ? -1 : 1;
    updateDrawDinge();
  }
}



void MenuEintragAuswahlmenu::doReturn(bool durchMaus) {
  if (mSubBereich == 1) {
    doPfeil(-1);
  } else if (mSubBereich == 2) {
    doPfeil(1);
  } else {
    Sound::playSample(sample_menuclick,so_fenster);
    mSub->oeffnen(durchMaus, mGetAktuell() + mVorlauf);
  }
}



void MenuEintragAuswahlmenu::doUntermenuSchliessen() {
  updateDrawDinge();
}


void MenuEintragAuswahlmenu::updateDDIntern() {
  int y = mHoehe/2-L_font_height/2;
  int x0,x1;
  neuDraw() = DrawDing(mName,mAccIndex, subbereich_default, 0, y, AlignHCenter,
		       NULL, &x0, &x1);

  int x_mitte = (x0 + x1) / 2;
  y += L_font_height;
  int aktuell = mGetAktuell();
  Str tmpstr = (*mAuswahlen)[aktuell];
  /*if (mSubBereich == subbereich_keiner)
    tmpstr = "(" + tmpstr + ")";
    */
  int anim_x = x_mitte + mAnimation * mAnimationDX / L_auswahlmenu_anim_schritte;
  DrawDing & dd = neuDraw();
  dd = DrawDing(tmpstr,hotkey_keins, subbereich_default,
			        anim_x, y,
				AlignHCenter, Font::gData, &x0, &x1);
  dd.abschneiden(x_mitte - L_auswahlmenu_pfeilsep,
	         x_mitte + L_auswahlmenu_pfeilsep);
  if (mAnimation != 0) {
    DrawDing & dd2 = neuDraw();
    dd2 = DrawDing(mAnimationWahlAlt,hotkey_keins, subbereich_default,
          anim_x + (mAnimation > 0 ? -mAnimationDX : mAnimationDX), y,
	  AlignHCenter, Font::gData, &x0, &x1);
    dd2.abschneiden(x_mitte - L_auswahlmenu_pfeilsep,
	            x_mitte + L_auswahlmenu_pfeilsep);
  }

  DrawDing & pf1 = neuDraw();
  pf1 = DrawDing("\010", hotkey_keins, 1,
		       x_mitte - L_auswahlmenu_pfeilsep, y,
		       AlignRight, NULL);
  mPfeil1X0 = pf1.mX0;
  mPfeil1X1 = pf1.mX1;

  DrawDing & pf2 = neuDraw();
  pf2 = DrawDing("\011", hotkey_keins, 2,
		       x_mitte + L_auswahlmenu_pfeilsep, y,
		       AlignLeft, NULL);
  mPfeil2X0 = pf2.mX0;
  mPfeil2X1 = pf2.mX1;


//   if (mSubBereich != subbereich_keiner) {
//     /* Erstmal deaktiviert, weil's mir nicht so gefällt
//     neuDraw() = DrawDing((*mAuswahlen)[schiebAktuell(-1)],-1,
// 			 0,
// 			 x0-L_auswahlsep,y,
// 			 AlignRight, &x0);
//     neuDraw() = DrawDing((*mAuswahlen)[schiebAktuell(1)],-1,
// 			 0,
// 			 x1+L_auswahlsep,y,
// 			 AlignLeft, NULL, &x1);
//     */
//     
//     /*
//     neuDraw() = DrawDing(blattpic_pfeile,2,x0-L_grausep-gric/2,y);
//     neuDraw() = DrawDing(blattpic_pfeile,2+1,x1+L_grausep+gric/2,y);
//     */
//   }
}


void MenuEintragAuswahlmenu::doPfeil(int d) {

  Sound::playSample(sample_menuscroll,so_fenster);

  mAnimationWahlAlt = (*mAuswahlen)[mGetAktuell()];
  mEintragDoReturn(schiebAktuell(d));

  mAnimation = L_auswahlmenu_anim_schritte * d;
  
  /* Abstand zwischen altem und neuem Text bestimmen. Soll eigentlich
     L_auswahlmenu_anim_dx, aber evtl. mehr, wenn sonst der neue Text
     schon vorher reingeguckt hätte oder umgekehrt */
  int w1 = Font::gMenu->getLineWidth(mAnimationWahlAlt.data()) / 2;
  int w2 = Font::gMenu->getLineWidth((*mAuswahlen)[mGetAktuell()].data()) / 2;
  if (w1 < L_auswahlmenu_pfeilsep - L_auswahlmenu_anim_dx)
    w1 = L_auswahlmenu_pfeilsep - L_auswahlmenu_anim_dx;
  if (w2 < L_auswahlmenu_pfeilsep - L_auswahlmenu_anim_dx)
    w2 = L_auswahlmenu_pfeilsep - L_auswahlmenu_anim_dx;
  mAnimationDX = w1 + w2 + L_auswahlmenu_anim_dx;
  
  updateDrawDinge();
}


int MenuEintragAuswahlmenu::schiebAktuell(int d) {
  int neu = mGetAktuell()+d;
  if (neu<0)
    neu=mAuswahlen->size()-1;
  if (neu>=(int) mAuswahlen->size())
    neu=0;
  return neu;
}

/*****************************************************************************/

/* Wer hat zur Zeit das Kreuz? */
/* Wo ist diese Variable besser gekapselt? Wenn sie ein private Member von
   MenuEintragSpielerModus ist oder wenn sie nur hier in dieser Datei steht
   (und ein "static" hat, damit der Linker auch nicht verrät, dass es diese
   Variable gibt)? */
static MenuEintragSpielerModus * gMenuEintragSpielerModusAktuell = NULL;


void MenuEintragSpielerModus::doReturn(bool) {

  Sound::playSample(sample_menuclick,so_fenster);

  /* Keine neue Animation, wenn das bereits angekreuzt war */
  if (Cuyo::getSpielerModus() == mModus)
    return;
  Cuyo::setSpielerModus(mModus);
  mAnimation = 2; /* "ASCII"-Code des ersten Kreuzchen-Bildchens */

  if (gMenuEintragSpielerModusAktuell)
    gMenuEintragSpielerModusAktuell->updateDrawDinge();

  /* Vermutlich wird restart-last-level stromlos */
  //CASSERT(gMenuEintragRestartLastLevel);
  //gMenuEintragRestartLastLevel->updateDrawDinge();

  /* Da steht im Moment die Funktion drin, die Restart-last-level
     neu malt (wegen Stromänderung).
     Eigentlich könnte man hieraus auch gleich einen generischen
     Ankreuzlisten-MenüEintragTyp machen, im Stil der anderen Auswahlmenüs. */
  (*mDoWechsel)();

  updateDrawDinge();
}


void MenuEintragSpielerModus::updateDDIntern() {
#if HOT_ALIGN
  int xkreuz;
  neuDraw() = DrawDing(mName, mAccIndex, subbereich_default, 0, mHoehe/2,
		       AlignRight, NULL, NULL, &xkreuz);
  xkreuz += L_datensep;
#else
  neuDraw() = DrawDing(mName, mAccIndex, subbereich_default, 0, mHoehe/2,
		       AlignRight);
  int xkreuz = x+L_datensep;
#endif
  neuDraw() = DrawDing("\001", hotkey_keins, subbereich_default,
		       xkreuz, mHoehe/2, AlignLeft);

  if (Cuyo::getSpielerModus() == mModus) {
    CASSERT(mAnimation >= 2);
    Str tmpstr = _sprintf("%c", mAnimation);
    neuDraw() = DrawDing(tmpstr, hotkey_keins, subbereich_default,
		         xkreuz, mHoehe/2, AlignLeft, Font::gData);
    gMenuEintragSpielerModusAktuell = this;
  }
}


void MenuEintragSpielerModus::zeitSchritt() {
  if (mAnimation < 0 || mAnimation == 7)
    return;

  mAnimation++;
  if (mAnimation == 3 || mAnimation == 6)
    mAnimation++;
  updateDrawDinge();
}



/*****************************************************************************/

void MenuEintragTaste::updateDDIntern() {
  neuDraw() = DrawDing(mName, mAccIndex,subbereich_default, 0, mHoehe/2,
		       AlignRight);

  Str ts = SDL_GetKeyName(PrefsDaten::getTaste(mSpieler, mTaste));
  int x0,x1;
  neuDraw() = DrawDing(ts, hotkey_rahmen,subbereich_default, 0, mHoehe/2,
		       AlignLeft, Font::gData, &x0, &x1);
}

Str MenuEintragTaste::getInfo() {
  return mSubBereich == subbereich_hyperaktiv
    ? _("Type a new key")
    : _("To enter a new key, click or press `return' first");
}

void MenuEintragTaste::doHyperaktiv(const SDL_keysym & taste, int) {
  if (taste.sym!=SDLK_ESCAPE) {
    PrefsDaten::setTaste(mSpieler, mTaste, taste.sym);
    doPapiNavigiere(1);
  }
}



/*****************************************************************************/

void MenuEintragAI::updateDDIntern() {
  int li, re;
  
  neuDraw() = DrawDing(mName, mAccIndex,subbereich_default, 0, mHoehe/2,
		       AlignRight, 0, &li, &re);

  Str ts = _sprintf("%d", PrefsDaten::getKIGeschwLog());
  neuDraw() = DrawDing(ts, hotkey_rahmen,subbereich_default, 0, mHoehe/2,
		       AlignLeft, Font::gData);

  DrawDing & pf1 = neuDraw();
  pf1 = DrawDing("\010", hotkey_keins, 1,
		       li - L_AI_pfeil_sep_li, mHoehe/2,
		       AlignRight, NULL);
  mPfeil1X0 = pf1.mX0;
  mPfeil1X1 = pf1.mX1;

  DrawDing & pf2 = neuDraw();
  pf2 = DrawDing("\011", hotkey_keins, 2,
		       re + L_AI_pfeil_sep_re, mHoehe/2,
		       AlignLeft, NULL);
  mPfeil2X0 = pf2.mX0;
  mPfeil2X1 = pf2.mX1;
}


void MenuEintragAI::doHyperaktiv(const SDL_keysym &, int taste) {
  if (taste==SDLK_RIGHT)
    doPfeil(1);
  else if (taste==SDLK_LEFT)
    doPfeil(-1);
}


void MenuEintragAI::doReturn(bool) {
  if (mSubBereich == 1) {
    doPfeil(-1);
  } else if (mSubBereich == 2) {
    doPfeil(1);
  }
}


void MenuEintragAI::doPfeil(int d) {
  int alt = PrefsDaten::getKIGeschwLog();
  int neu = alt + d;
  if (neu<0) neu=0;
  if (neu>9) neu=9;

  if (neu != alt) {
    PrefsDaten::setKIGeschwLog(neu);
    updateDrawDinge();
  }  
}


int MenuEintragAI::getMausPos(int x, int y) {

  /* Manuel zusammengeflicktes befindet-sich-die-Maus-über-einem-Pfeil.
     Wenn wir irgendwann mehr solche SubBereiche haben, will man das
     schöner machen. */
    
  if (x >= mPfeil1X0 && x < mPfeil1X1)
    return 1;
  if (x >= mPfeil2X0 && x < mPfeil2X1)
    return 2;
  
  return MenuEintrag::getMausPos(x, y);
}




/*****************************************************************************/

void MenuEintragSound::updateDDIntern() {
  neuDraw() = DrawDing(mName, mAccIndex,subbereich_default, 0, mHoehe/2,
		       AlignRight);

  Str ts = mBitteWarten ? "..." : PrefsDaten::getSound() ? _("On") : _("Off");
  neuDraw() = DrawDing(ts, hotkey_rahmen,subbereich_default, 0, mHoehe/2,
		       AlignLeft, Font::gData);
}


void MenuEintragSound::doReturn(bool) {
  /* Da es auf manchen Rechnern ziemlich lange dauert, bis SDL merkt, dass
     es den Sound doch nicht anschalten kann, für diese Wartezeit eine optische
     Rückkopplung */
  mBitteWarten = true;
  updateDrawDinge();
  UI::sofortAllesAnzeigen();
  mBitteWarten = false;

  setSound(!PrefsDaten::getSound());
  updateDrawDinge();
}


void MenuEintragSound::doHyperaktiv(const SDL_keysym &, int taste) {
  bool alt = PrefsDaten::getSound();
  bool neu = alt;

  if (taste=='0') neu = false;
  else if (taste=='1') neu = true;
  else if (taste==SDLK_RIGHT || taste==SDLK_LEFT || taste==SDLK_RETURN || taste==' ')
    neu = !alt;

  if (neu == alt)
    return;
    
  setSound(neu);
  updateDrawDinge();
}


void MenuEintragSound::setSound(bool neu) {
  if (neu != PrefsDaten::getSound()) {
    if (!neu)
      Sound::setMusic("");
    PrefsDaten::setSound(neu);
    Sound::checkePrefsStatus();
    if (neu)
      Sound::setMusic("cuyo.it");
  }
}


/*****************************************************************************/

MenuEintragLevel::MenuEintragLevel(BlattMenu * papi, Str na, bool gewonnen, bool strom):
   MenuEintrag(papi, na), mGewonnen(gewonnen)
{
  if (!strom)
    setNieStrom();
}



void MenuEintragLevel::updateDDIntern() {
  int xmin;
  neuDraw() = DrawDing(mName, hotkey_keins, subbereich_default, 0, mHoehe / 2,
                       AlignLeft, 0, &xmin);
  if (mGewonnen) {
    xmin -= L_datensep;
    neuDraw() = DrawDing("\013", hotkey_keins, subbereich_default, xmin, mHoehe / 2, AlignRight);
  }
}
