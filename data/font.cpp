/***************************************************************************
                          font.cpp  -  description
                             -------------------
    begin                : Fri Jul 21 2000
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

#include <cstdlib>

#include "fehler.h"
#include "sdltools.h"
#include "font.h"
#include "pfaditerator.h"

Font * Font::gMenu;
Font * Font::gBright;
Font * Font::gTitle;
Font * Font::gDimmed;
Font * Font::gBrightDimmed;
Font * Font::gData;
Font * Font::gGame;
Font * Font::gDbg;


/* Returns the number of non-transparent pixels in the specyfied column */
int pixelInCol(SDL_Surface * s, int x, int y, int h) {
  int ret = 0;
  for (int i = 0; i < h; i++) {
    Uint8 r, g, b, a;
    SDL_GetRGBA(SDLTools::getPixel32(s, x, y + i), s->format, &r, &g, &b, &a);
    if (a > 30) ret++;
  }
  return ret;
}




FontStr::FontStr(const Str & text) {
  char * s = convert_for_font(text.data());
  mText = s;
  free(s);
}

bool FontStr::operator == (const FontStr & fs) const {
  return mText==fs.mText;
}



Font::Font(const Str & filename, int w, int h, int (*c2p)(char),
       bool varWidth, int addToWidth /*= 0*/):
  mWidth(w), mHeight(h), mChar2Pos(c2p)
{
  mChars.laden(filename);
  mCharsPerLine = mChars.getBreite() / w;
  int numLines = mChars.getHoehe() / h;
  
  if (varWidth) {
    SDL_Surface * s = mChars.getSurface();
    SDL_LockSurface(s);
    for (int i = 0; i < mCharsPerLine * numLines; i++) {
      int xx = w * (i % mCharsPerLine);
      int yy = h * (i / mCharsPerLine);
      
      /* Search for left border */
      int x = 0;
      while (pixelInCol(s, xx + x, yy, h) == 0) {
        x++;
	if (x == w) {
	  mCharLeft[i] = 0;
	  mCharWidth[i] = w / 3;
	  goto space;
	}
      }
      mCharLeft[i] = x;

      /* Search for right border */
      x = w - 1;
      while (pixelInCol(s, xx + x, yy, h) == 0) {
        x--;
	CASSERT(x >= 0);
      }
      mCharWidth[i] = x + 1 - mCharLeft[i] + addToWidth;
      
     space:;
    }
    SDL_UnlockSurface(s);
    
  } else {
    for (int i = 0; i < mCharsPerLine * numLines; i++) {
      mCharLeft[i] = 0;
      mCharWidth[i] = w;
    }
  }
}

Font::Font(Font * quelle, const Color & faerbung) :
    mChars(&(quelle->mChars), faerbung), mCharsPerLine(quelle->mCharsPerLine),
    mWidth(quelle->mWidth), mHeight(quelle->mHeight),
    mChar2Pos(quelle->mChar2Pos) {
  for (int i = 0; i < 256; i++) {
    mCharLeft[i]= quelle->mCharLeft[i];
    mCharWidth[i]= quelle->mCharWidth[i];
  }
}


int Font::getFontHeight() const {
  return mHeight;
}




void Font::drawText(const FontStr & s, int x, int y, TextAlign align) const {
  switch (align & AlignVMask) {
    case AlignBottom: y -= getTextHeight(s); break;
    case AlignVCenter: y -= getTextHeight(s) / 2; break;
  }

  const char * bei=s.mText.data();
  while (1) {
  
    int xx = x;
    switch (align & AlignHMask) {
      case AlignRight: xx -= getLineWidth(bei); break;
      case AlignHCenter: xx -= getLineWidth(bei) / 2; break;
    }
  
    while (*bei != '\n') {
      if (*bei == 0) goto finished;
      drawChar(*bei, xx, y);
    
      bei++;
    }
    bei++;
  
    y += mHeight;
  }
  finished:;
}


/* Increments x by charWidth */
void Font::drawChar(char c, int & x, int y) const {
  int p = (*mChar2Pos)(c);
  
  mChars.malBildAusschnitt(x - mCharLeft[p], y,
       SDLTools::rect((p % mCharsPerLine) * mWidth, (p / mCharsPerLine) * mHeight,
                      mWidth, mHeight));
  x += mCharWidth[p];
}




void Font::wordBreak(FontStr & text, int width) const {
  int l = text.mText.length();
  int w = 0;
  int last_break_pos = 0;
  for (int i = 0; i < l; i++) {
    char c = text.mText[i];
    if (c == '\n') {
      w = 0;
      last_break_pos = 0;
    } else {
      if (c == ' ')
        last_break_pos = i;
      w += mCharWidth[(*mChar2Pos)(c)];
      
      if (w > width) {
        /* Line is too long now */
	if (last_break_pos == 0) {
          text.mText = text.mText.left(i)+"\n"+text.mText.right(i);
          last_break_pos = i;
        }
	else text.mText[last_break_pos] = '\n';
	i = last_break_pos;
	w = 0;
	last_break_pos = 0;
      }
    }
  }
}




/* Returns only the width of the first line of text */
int Font::getLineWidth(const Str & t) const {
  FontStr t_ = t;
  const char * text = t_.mText.data();
  int w = 0;
  while (*text != 0 && *text != '\n') {
    w += mCharWidth[(*mChar2Pos)(*text)];
    text++;
  }
  return w;
}





int Font::getTextHeight(const FontStr & t) const {
  const char * text = t.mText.data();
  int h = mHeight;
  while (*text) {
    if (*text == '\n')
      h += mHeight;
    text++;
  }
  return h;
}



int c2pBig(char c) {
  //if ((unsigned char) c < 32) c = '_';
  //int r = c - 32;
  //r = (r & 0x000f) + (0xd0 - (r & 0x00f0));
  //return r;
  return (unsigned char) c;
}

int c2pDbg(char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  else if (c == '-')
    return 10;
  else
    return 11;
}


void Font::init() {
  gGame = new Font("font-big.xpm", L_font_width, L_font_height,
                   c2pBig, true, 1);
  gBright = new Font(gGame,Color(255,255,128));
  gTitle = new Font(gGame,Color(200,60,240));
  gDimmed = new Font(gGame,Color(80,80,145)); // Mittelwert aus 30,30,70 und 130,130,220
  gBrightDimmed = new Font(gGame,Color(143,143,99)); // Mittelwert aus 30,30,70 und 255,255,128
  gData = new Font(gGame,Color(200,200,255));
  gMenu = new Font(gGame,Color(130,130,220));
  gDbg = new Font("dbgZiffern.xpm", 8, 16, c2pDbg, false);
  
  /* Automatically recolor the Game font */
  AutoColor::gGame.addUser(&(gGame->mChars));
}


void Font::destroy() {
  delete gGame;
  delete gDbg;
  delete gMenu;
  delete gData;
  delete gBrightDimmed;
  delete gDimmed;
  delete gTitle;
  delete gBright;
}
