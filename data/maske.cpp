/*
   Copyright 2011 by Mark Weyer

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stddef.h>
#include <SDL.h>

#include "maske.h"
#include "sdltools.h"
#include "fehler.h"

RohMaske::RohMaske() : mWidth(0), mHeight(0), mData(NULL) {}

RohMaske::RohMaske(const RohMaske & src) : mWidth(src.mWidth),
					   mHeight(src.mHeight) {
  if (src.mData) {
    size_t size=mWidth*mHeight;
    mData = new bool[size];
    memcpy((void*) &mData, (void *) &src.mData, size*sizeof(bool));
  } else mData=NULL;
}

RohMaske::~RohMaske() {
  if (mData) delete mData;
}

void RohMaske::init(int w, int h) {
  mWidth=w;
  mHeight=h;
  if (mData) delete mData;
  mData=NULL;
}

void RohMaske::set_pixel(int x, int y, bool pixel) {
  if (pixel && !mData) {
    int size=mWidth*mHeight;
    mData = new bool[size];
    for (int i=0; i<size; i++) mData[i]=false;
  };
  if (mData) mData[x+y*mWidth]=pixel;
}

void RohMaske::fill(bool pixel) {
  int size=mWidth*mHeight;
  if (pixel && !mData) mData = new bool[size];
  if (mData) for (int i=0; i<size; i++) mData[i]=pixel;
}

bool RohMaske::is_empty() const {return !mData;}



Maske::Maske() : mWidth(0), mHeight(0), mData(NULL) {}

Maske::Maske(const Maske & src) : mWidth(src.mWidth), mHeight(src.mHeight) {
  if (src.mData) {
    size_t size=mWidth*mHeight;
    mData = new Sint8[size];
    memcpy((void*) &mData, (void *) &src.mData, size*sizeof(Sint8));
  } else mData=NULL;
}

Maske::~Maske() {
  if (mData) delete mData;
}

void Maske::scale(const RohMaske & src, int scale) {
  mWidth = src.mWidth * scale / scale_base;
  mHeight = src.mHeight * scale / scale_base;

  if (src.mData) {
    mData = new Sint8[mWidth*mHeight];

    /* First, just the scaling */
    if (scale>=scale_base)
      for (int yd=0; yd<mHeight; yd++) {
        int ys=yd*scale_base/scale;
        for (int xd=0; xd<mWidth; xd++)
	  mData[yd*mWidth+xd] = (src.mData[ys*src.mWidth+xd*scale_base/scale]
				 ? 1
				 : -1);
      }
    else
      for (int yd=0; yd<mHeight; yd++) {
        int ys0=yd*scale_base/scale;
        int ys1=(yd*scale_base+scale_base-1)/scale;
        for (int xd=0; xd<mWidth; xd++) {
          int xs0=xd*scale_base/scale;
          int xs1=(xd*scale_base+scale_base-1)/scale;
          Sint8 pixel=-1;
          for (int ys=ys0; ys<ys1; ys++)
            for (int xs=xs0; xs<xs1; xs++)
              if (src.mData[ys*src.mWidth+xs]) pixel=1;
          mData[yd*mWidth+xd]=pixel;
	};
      };

    /* Then the optimization */
    for (int yd=0; yd<mHeight; yd++) {
      Sint8 next=0;
      for (int xd=mWidth-1; xd>=0; xd--) {
	size_t pos=yd*mWidth+xd;
        if ((mData[pos]>0 && next>0) || (mData[pos]<0 && next<0)) {
	  mData[pos]+=next;
	  /* Test for overflow */
	  if ((mData[pos]>0 && next<0) || (mData[pos]<0 && next>0))
	    mData[pos]-=next;
	};
	next=mData[pos];
      }
    }
  }
  else {
    mData=NULL;
  };
}

bool Maske::is_empty() const {return !mData;}

void Maske::masked_blit(SDL_Surface * src, int srcx, int srcy,
		 SDL_Rect & mask,
		 SDL_Surface * dst, SDL_Rect & dstr) const {
  if (mData) {
    SDL_LockSurface(src);
    SDL_LockSurface(dst);

    size_t Bpp=src->format->BytesPerPixel;
    CASSERT(Bpp==dst->format->BytesPerPixel);
    size_t srcpitch=src->pitch;
    size_t dstpitch=dst->pitch;
    int w = (mask.x+mask.w>mWidth ? mWidth-mask.x : mask.w);
    int h = (mask.y+mask.h>mHeight ? mHeight-mask.y : mask.h);
    for (int y=0; y<h; y++)
      for (int x=0; x<w; ) {
	int datum = (int) mData[(y+mask.y)*mWidth + x+mask.x];
	if (datum>0) {
          if (x+datum>w) datum=w-x;
	  memcpy(((Uint8*) dst->pixels) + (y+dstr.y)*dstpitch + (x+dstr.x)*Bpp,
		 ((Uint8*) src->pixels) + (y+srcy)*srcpitch + (x+srcx)*Bpp,
		 datum*Bpp);
	  x+=datum;
	} else x-=datum;
      };

    SDL_UnlockSurface(dst);
    SDL_UnlockSurface(src);
  }
}

