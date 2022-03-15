/***************************************************************************
                          sdltools.cpp  -  description
                             -------------------
    begin                : Fri Jul 21 2000
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

#include <cstdlib>

#include "stringzeug.h"
#include "sdltools.h"
#include "global.h"
#include "fehler.h"
#include "layout.h"
#include "bilddatei.h"


/* Right now, the picture scale funcition still has a fixed
   scale_base=4 build in. Apart from that, it should be possible
   to increase scale_base to 8 or 16 (so that more different scalings
   are possible) */

#define videomode_flags_w \
  (SDL_SWSURFACE/*|SDL_DOUBLEBUF*/|SDL_ANYFORMAT|SDL_VIDEORESIZE)
#define videomode_flags_f \
  (SDL_SWSURFACE/*|SDL_DOUBLEBUF*/|SDL_ANYFORMAT|SDL_FULLSCREEN)



namespace Area {
  void init();
}



namespace SDLTools {


/* A 32-Bit-surface, to copy the pixel format from */
SDL_Surface * gSampleSurface32 = 0;

/* The pixel format used for maskedDisplayFormat() */
SDL_PixelFormat gMaskedFormat;
/* The replacement colour for the ColourKey, if necessary */
Uint8 gColkeyErsatz;

/* gScale == scale_base means original size */
int gScale;
int gShiftX, gShiftY;

/* Virtual Window-size, i.e. size how it looks for the cuyo program
   (In contrast to screen size, which may be scaled).
   Start with sensible default size. */
int gVirtualWidth = L_usual_width;
int gVirtualHeight = L_usual_height;

bool gFullScreen = false;


void myQuitSDL() {
  if (gSampleSurface32)
    SDL_FreeSurface(gSampleSurface32);
  if (gMaskedFormat.palette) {
    delete[] gMaskedFormat.palette->colors;
    delete gMaskedFormat.palette;
  }
  SDL_Quit();
}



/* Compute the value of gScale for a window of size w x h */
int getScale(int w, int h) {
  int sc_w = w / (gVirtualWidth / scale_base);
  int sc_h = h / (gVirtualHeight / scale_base);
  int sc = sc_w < sc_h ? sc_w : sc_h;
  if (sc < 1) sc = 1;
  return sc;
}


void computeScaleAndShift() {
  SDL_Surface * s = SDL_GetVideoSurface();
  gScale = getScale(s->w, s->h);
  gShiftX = (s->w - gVirtualWidth * gScale / scale_base) / 2;
  gShiftY = (s->h - gVirtualHeight * gScale / scale_base) / 2;

  /* Rescale all icons */
  Bilddatei::resizeEvent();
}



/* In fullscreen mode: find out optimal resolution to choose;
   (this is also used to get an idea on the optimal window size
   in windowed mode);
   returns false, if there seem to be no fullscreen mode available */
bool fullscreenSize(int & w, int & h) {
  SDL_Rect **modes;
  /* format = 0: search for best video mode */
  modes = SDL_ListModes(0, videomode_flags_f);
  
  /* No size possible ?!? Let's hope that this is just because there
     is no fullscreen mode available. (As fullscreenSize() is called
     even in windowed mode, it is important that we do not just crash.) */
  if (modes == (SDL_Rect**) 0) {
    /* In case we just tried to guess a good window size, let's
       just return a value which we hope to be good. */
    w = L_preferred_width;
    h = L_preferred_height;
    return false;
  }
  
  /* Any size possible in fullscreen mode... */
  if (modes == (SDL_Rect**) -1) {
    w = L_preferred_width;
    h = L_preferred_height;
  }
  
  /* Return the smallest resolution which is as least as
     big as L_usual_width x L_usual_height,
     or otherwise the biggest existing one */
  bool found = false;
  for (int i = 0; modes[i]; i++) {
    if (modes[i]->w >= L_usual_width && modes[i]->h >= L_usual_height  ||  !found) {
      w = modes[i]->w;
      h = modes[i]->h;
      found = true;
    }
  }
  return true;
}


/* Set/change (screen, i.e. real) window size */
void setVideoMode(int w, int h) {

  /* In fullscreen mode or if no window size was given, find
     a good default window size */
  if (gFullScreen || w == -1) {
    if (!fullscreenSize(w, h)) {
      /* No fullscreen mode available? Then fall back to windowed
         mode. */
      gFullScreen = false;
      
    } else if (!gFullScreen) {
      /* Not fullscreen? Then use the resolution which would be optimal
         in fullscreen mode, but shrink the window to get rid of the
         black borders */
      int sc = getScale(w, h);
      int pref_w = L_preferred_width * sc / scale_base;
      int pref_h = L_preferred_height * sc / scale_base;
      if (w > pref_w) w = pref_w;
      if (h > pref_h) h = pref_h;
    }
  }
  
  /* Initialize the display
     requesting a software surface
     BitsPerPixel = 0: Take the current BitsPerPixel
     SDL_ANYFORMAT: Other pixel depths are ok, too. */
  SDL_Surface * s = SDL_SetVideoMode(w, h, 0,
        gFullScreen ? videomode_flags_f : videomode_flags_w);
  SDLASSERT(s);
  
  computeScaleAndShift();
}


void computeMaskedFormat() {

  CASSERT(gSampleSurface32);

  SDL_Surface * s = SDL_GetVideoSurface();
  
  gMaskedFormat = *(s->format);
  if (gMaskedFormat.palette) {
    int ncolours = gMaskedFormat.palette->ncolors;

    /* Erst mal eine Kopie der Palette anlegen. Böse Dinge könnten passieren,
       wenn das displaysurface sie ändert. */
    SDL_Color * neufarben = new SDL_Color[ncolours];
    memcpy((void*) neufarben, (void*) gMaskedFormat.palette->colors,
	   ncolours * sizeof(SDL_Color));
    SDL_Palette * neupal = new SDL_Palette;
    neupal->ncolors = ncolours;
    neupal->colors = neufarben;
    gMaskedFormat.palette = neupal;

    if (ncolours < 255) {
      gMaskedFormat.colorkey = ncolours + 1;
      gColkeyErsatz = 0;  // Wird eh nicht benutzt...
    }
    else {
      /* Jetzt müssen wir eine entbehrliche Farbe suchen */
      int dmin = 1000;
      int c1best=0, c2best=0;
        // Initialisierung ist unnötig, spart aber Warnungen

      SDL_Color * colours = gMaskedFormat.palette->colors;

      #ifdef DIST
        #error Vorsicht, DIST gibt es schon!
      #endif
      #define DIST(dim) (colours[c1].dim<colours[c2].dim \
        ? colours[c2].dim-colours[c1].dim                \
	: colours[c1].dim-colours[c2].dim)

      for (int c1=0; c1<ncolours; c1++)
	for (int c2=c1+1; c2<ncolours; c2++) {
	  int d = DIST(r)+DIST(g)+DIST(b);
	  if (d<dmin) {
	    dmin=d;
	    c1best=c1;
	    c2best=c2;
	  }
	}

      #undef DIST

      gMaskedFormat.colorkey = c2best;
      gColkeyErsatz = c1best;
    }
  }
  else
    gMaskedFormat = *(gSampleSurface32->format);
}


/* opt_w, opt_h: size of window, as given by command line option;
   or -1,-1 to automatically choose window size */
void initSDL(int opt_w, int opt_h) {

  /* Initialize the SDL library */
  /* Audio will be initialized by our sound system */
  SDLASSERT(SDL_Init(SDL_INIT_VIDEO) == 0);
  
  /* Clean up on exit */
  atexit(myQuitSDL);

  /* Set the name of the window (and the icon) */
  setMainTitle();
  
  setVideoMode(opt_w, opt_h);
  
  gSampleSurface32 = createSurface32(1, 1);
  SDLASSERT(gSampleSurface32);

  computeMaskedFormat();
  
 
  SDL_EventState(SDL_KEYUP, SDL_IGNORE);
  SDL_EventState(SDL_ACTIVEEVENT, SDL_IGNORE);
  
  /* We need the characters corresponding to key-events for the menus:
     For example, on a French keyboard, shift-& is 1 */
  SDL_EnableUNICODE(1);

  Area::init();
}




/* Call while cuyo is already running is not yet supported
   (but not difficult to implement) */
void setFullscreen(bool fs) {
  gFullScreen = fs;
}


/* Change the size of the window from the view of the cuyo program.
   Does *not* change the real window size; instead, scaling may
   change.
   I propose that this should only be used in such a way that when
   the real window size is the preferred one (L_preferred_xxx),
   then scaling should never change.
   Note that changing the scaling always takes some time.
*/
void setVirtualWindowSize(int w, int h) {
  gVirtualWidth = w;
  gVirtualHeight = h;
  computeScaleAndShift();
}


void setWindowTitle(const char * title) {
  char * title_ = convert_for_window_title(title);
  char * icon = convert_for_window_title("Cuyo");
  SDL_WM_SetCaption(title_,icon);
  free(title_);
  free(icon);
}

void setMainTitle() {
  // TRANSLATORS: "cuyo" is the program's name
  setWindowTitle(gDebug ? _("Cuyo - debug mode") : "Cuyo");
}

void setLevelTitle(const Str & levelname) {
  // TRANSLATORS: This is a window title
  setWindowTitle(_sprintf(_("Cuyo - level %s"),levelname.data()).data());
}

/* Convert Qt-Key into SDL-Key; don't use Qt constants: we don't want to depend on
   Qt just to be able to read old .cuyo files. */
SDLKey qtKey2sdlKey(int qtk) {

  /* Letters are uppercase in Qt and lowercase in SDL */
  if (qtk >= 'A' && qtk <= 'Z')
    return (SDLKey) (qtk - 'A' + 'a');
  
  /* Don't change other Ascii Characters.
     (Maybe Ä, Ö, Ü, etc are a problem) */
  if (qtk <= 255)
    return (SDLKey) qtk;

  /* Other important keys */
  switch (qtk) {
    case 0x1000: return SDLK_ESCAPE;
    case 0x1001: return SDLK_TAB;
    case 0x1003: return SDLK_BACKSPACE;
    case 0x1004: return SDLK_RETURN;
    case 0x1006: return SDLK_INSERT;
    case 0x1007: return SDLK_DELETE;
    case 0x1008: return SDLK_PAUSE;
    case 0x1009: return SDLK_PRINT;
    case 0x100a: return SDLK_SYSREQ;
    case 0x100b: return SDLK_CLEAR;
    case 0x1010: return SDLK_HOME;
    case 0x1011: return SDLK_END;
    case 0x1012: return SDLK_LEFT;
    case 0x1013: return SDLK_UP;
    case 0x1014: return SDLK_RIGHT;
    case 0x1015: return SDLK_DOWN;
    case 0x1016: return SDLK_PAGEUP;
    case 0x1017: return SDLK_PAGEDOWN;

    case 0x1030: return SDLK_F1;
    case 0x1031: return SDLK_F2;
    case 0x1032: return SDLK_F3;
    case 0x1033: return SDLK_F4;
    case 0x1034: return SDLK_F5;
    case 0x1035: return SDLK_F6;
    case 0x1036: return SDLK_F7;
    case 0x1037: return SDLK_F8;
    case 0x1038: return SDLK_F9;
    case 0x1039: return SDLK_F10;
    case 0x103a: return SDLK_F11;
    case 0x103b: return SDLK_F12;
    case 0x103c: return SDLK_F13;
    case 0x103d: return SDLK_F14;
    case 0x103e: return SDLK_F15;

    case 0x1055: return SDLK_MENU;
    case 0x1058: return SDLK_HELP;
    //case : return SDLK_BREAK;
    //case : return SDLK_POWER;
    //case : return SDLK_EURO;
    //case : return SDLK_UNDO;
    default: return SDLK_UNKNOWN;
  }
  
  /* Modifier keys are missing */
}



SDL_Rect rect(int x, int y, int w, int h) {
  SDL_Rect ret;
  ret.x = x; ret.y = y; ret.w = w; ret.h = h;
  return ret;
}

bool intersection(const SDL_Rect & a, const SDL_Rect & b, SDL_Rect & ret) {
  ret.x = a.x > b.x ? a.x : b.x;
  ret.y = a.y > b.y ? a.y : b.y;
  int xplusw = a.x + a.w < b.x + b.w ? a.x + a.w : b.x + b.w;
  int yplush = a.y + a.h < b.y + b.h ? a.y + a.h : b.y + b.h;
  if (xplusw<=ret.x || yplush<=ret.y)
    return false;
  ret.w = xplusw-ret.x;
  ret.h = yplush-ret.y;
  return true;
}


/* Returns true if a is contained in b */
bool contained(const SDL_Rect & a, const SDL_Rect & b) {
  return a.x >= b.x && a.y >= b.y &&
      a.x + a.w <= b.x + b.w && a.y + a.h <= b.y + b.h;
}


/* Creates a 32-bit-surface with alpha. After filling it with your
   data, you should convert it to screen format */
SDL_Surface * createSurface32(int w, int h) {

  union { Uint32 f; Uint8 k[4];} rmask, gmask, bmask, amask;
  
  /* Die richtigen Bits der Masken auf 1 setzen; das Problem ist, dass
     welches die richtigen Bits sind von der Endianness abhaengen.
     Das folgende macht's richtig: */
  rmask.f = gmask.f = bmask.f = amask.f = 0;
  rmask.k[0] = gmask.k[1] = bmask.k[2] = amask.k[3] = 0xff;

  SDL_Surface * s = SDL_CreateRGBSurface(SDL_HWSURFACE, w, h, 32, rmask.f, gmask.f, bmask.f, amask.f);
  SDLASSERT(s);
  return s;
}


/* Converts a surface to a 32-bit-surface with alpha. The original surface
   is deleted. */  
void convertSurface32(SDL_Surface *& s) {
  /* Silly: The only way I know to create an SDL_PixelFormat is using
     SDL_CreateRGBSurface; so we need a "sample surface" to get the
     format from... */
  SDL_Surface * tmp = SDL_ConvertSurface(s, gSampleSurface32->format, SDL_SWSURFACE);
  SDLASSERT(tmp);
  SDL_FreeSurface(s);
  s = tmp;
}


/* Return a reference to the pixel at (x, y);
   assumes that the surface is 32-Bit.
   NOTE: The surface must be locked before calling this! */
Uint32 & getPixel32(SDL_Surface *surface, int x, int y) {
  int bpp = surface->format->BytesPerPixel;
  return *(Uint32 *) ((Uint8 *)surface->pixels + y * surface->pitch + x * bpp);
}


/* Converts the surface to a format suitable for fast blitting onto the
   display surface. Contrary to SDL_DisplayFormat, transparency is
   respected, at least where it is full transparency. Contrary to
   SDL_DisplayFormatAlpha, it uses ColourKey for paletted surfaces. */
SDL_Surface * maskedDisplayFormat(SDL_Surface * src) {
  SDL_Surface * ret = SDL_ConvertSurface(src,&gMaskedFormat,0);
  SDLASSERT(ret);

  if (gMaskedFormat.palette) {
    /* SDL sieht nicht vor, daß Alpha zu ColourKey konvertiert wird.
       Seufz. Also selber nachbearbeiten. */
    SDLASSERT(!SDL_MUSTLOCK(src) && !SDL_MUSTLOCK(ret));

    ret->flags |= SDL_SRCCOLORKEY;
    Uint8 colkey = gMaskedFormat.colorkey;
    ret->format->colorkey = colkey;
    Uint32 amask = src->format->Amask;
    Uint8 * srcrow = (Uint8 *) src->pixels;
    Uint8 * retrow = (Uint8 *) ret->pixels;
    int w = src->w;
    int p1 = src->pitch;
    int p2 = ret->pitch;
    for (int i=src->h; i; i--, srcrow+=p1, retrow+=p2) {
      Uint32 * srcpix = (Uint32 *) srcrow;
      Uint8 * retpix = retrow;
      for (int j=w; j; j--, srcpix++, retpix++)
	if (((*srcpix) & amask)==0)
	  *retpix = colkey;
	else if ((*retpix)==colkey)
	  *retpix = gColkeyErsatz;
    }
  }
  return ret;
}

SDL_Surface * createMaskedDisplaySurface(int w, int h) {
  return SDL_CreateRGBSurface(0,w,h,gMaskedFormat.BitsPerPixel,
			      gMaskedFormat.Rmask,gMaskedFormat.Gmask,
			      gMaskedFormat.Bmask,gMaskedFormat.Amask);
}


/* Scales the surface according to our window size;
   s must be 32 bit.
   Warning: It is possible that a new surface is created and
   returned, but is is also possible that just a pointer to s
   is returned. */
SDL_Surface * scaleSurface(SDL_Surface * s) {

  if (!gScale == scale_base)
    return s;

  int w2 = s->w * gScale / scale_base;
  int h2 = s->h * gScale / scale_base;
  SDL_Surface * ret = SDLTools::createSurface32(w2+1, h2+1);

  SDL_LockSurface(s);
  SDL_LockSurface(ret);

  /* Fill the safety margin (which is there to buffer rounding errors in
     Area::blitSurface().). */
  Uint32 trans=SDL_MapRGBA(ret->format,0,0,0,0);
  SDL_Rect r=rect(w2,0,1,h2+1);
  SDL_FillRect(ret,&r,trans);
  r=rect(0,h2,w2,1);
  SDL_FillRect(ret,&r,trans);

  for (int y = 0; y < h2; y++) {
    for (int x = 0; x < w2; x++) {
      Uint8 * dst = (Uint8 *)ret->pixels + ret->pitch * y + 4 * x;
      Uint8 * src = (Uint8 *)s->pixels + s->pitch * (y * 4 / gScale) + 4 * (x * 4 / gScale);
      
      Uint32 srcpix[16];
      Uint32 anteil[16];
      int numpix;
      
      /* Scaling is not yet very beautiful; in particular not enlarging */
      if (gScale >= 4) {
        numpix = 1;
        srcpix[0] = * (Uint32*) src;
        anteil[0] = 16;
      } else if (gScale == 3) {
        numpix = 4;
	srcpix[0] = * (Uint32*) src;
	srcpix[1] = * (Uint32*) (src + 4);
	srcpix[2] = * (Uint32*) (src + s->pitch);
	srcpix[3] = * (Uint32*) (src + s->pitch + 4);
	anteil[0] = (3 - x % 3) * (3 - y % 3);
	anteil[1] = (1 + x % 3) * (3 - y % 3);
	anteil[2] = (3 - x % 3) * (1 + y % 3);
	anteil[3] = (1 + x % 3) * (1 + y % 3);
      } else if (gScale == 2) {
        numpix = 4;
	srcpix[0] = * (Uint32*) src;
	srcpix[1] = * (Uint32*) (src + 4);
	srcpix[2] = * (Uint32*) (src + s->pitch);
	srcpix[3] = * (Uint32*) (src + s->pitch + 4);
	anteil[0] = anteil[1] = anteil[2] = anteil[3] = 4;
      } else {/* 1 */
        numpix = 16;
        for (int j = 0; j < 16; j++) {
	  srcpix[j] = * (Uint32*) (src + s->pitch * (j / 4) + 4 * (j % 4));
	  anteil[j] = 1;
	}
      }
      /* Hier ist erstmal Summe der Anteile = 16 */
      
      /* Alpha-Kanal einrechnen */
      Uint32 gesAlpha = 0;
      for (int j = 0; j < numpix; j++) {
	Uint32 sp = srcpix[j];
	anteil[j] *= ((Uint8 *) &sp)[3];
	gesAlpha += anteil[j];
      }
      
      Uint8 dstpix[4];
      dstpix[3] = gesAlpha / 16;
      if (dstpix[3] > 0) {
	for (int i = 0; i < 3; i++) {
          Uint32 komponente = 0;
	  for (int j = 0; j < numpix; j++) {
	    Uint32 sp = srcpix[j];
            komponente += anteil[j] * ((Uint8 *) &sp)[i];
	  }
	  komponente /= gesAlpha;
	  dstpix[i] = komponente;
	}
      }
      memcpy(dst,dstpix,4);
    }
  }

  SDL_UnlockSurface(s);
  SDL_UnlockSurface(ret);

  return ret;
}


int getScale() {
  return gScale;
}


/* Like SDL_PollEvent, but the event is already scaled.
   And for resize events, the window and scaling is already
   prepared. */
bool pollEvent(SDL_Event & evt) {
  if (!SDL_PollEvent(&evt))
    return false;
  
  if (evt.type == SDL_VIDEORESIZE) {
    setVideoMode(evt.resize.w, evt.resize.h);
  } else if (evt.type == SDL_MOUSEMOTION) {
    evt.motion.x -= gShiftX;
    evt.motion.y -= gShiftY;
    /* We have to ensure:
         motion.xy = old-motion.xy + motion.xyrel
       Without rounding errors. Thus we cannot just
       multiply motion.xyrel by scale_base/gScale */
    Uint16 xold, yold;
    xold = evt.motion.x - evt.motion.xrel;
    yold = evt.motion.y - evt.motion.yrel;
    evt.motion.x = evt.motion.x * scale_base / gScale;
    evt.motion.y = evt.motion.y * scale_base / gScale;
    xold = xold * scale_base / gScale;
    yold = yold * scale_base / gScale;
    evt.motion.xrel = evt.motion.x - xold;
    evt.motion.yrel = evt.motion.y - yold;
  } else if (evt.type == SDL_MOUSEBUTTONDOWN ||
             evt.type == SDL_MOUSEBUTTONUP) {
    evt.button.x -= gShiftX;
    evt.button.y -= gShiftY;
    evt.button.x = evt.button.x * scale_base / gScale;
    evt.button.y = evt.button.y * scale_base / gScale;
  }
  
  return true;
}


}  // namespace SDLTools

/**************************************************************************/

#define max_area_depth 5

namespace Area {

  /* When drawing, three coordinate transformation are done:
  
       Area coordinates
          translate according to the origin of the current Area
       virtual coordinates
          scale coordinates
          Translate to get the black border around the window
       screen coordinates

     The two translation cannot really well be merged because one is in
     virtual pixels and the other one in screen pixels
  
     To avoid rounding errors during scaling:
     The virtual coordinate x corresponds to the screen coordinate
     floor(x * gScale / scale_base); in screen coordinates, translations
     aren't exactly linear anymore. So instead of
         (a) scale(x) + scale(w)
     always do
         (b) scale(x+w)
     There is one exception in which you may use (a): When w is
     divisible by scale_base; this means that drawing Surfaces or
     parts of surfaces with size divisible by scale_base is allowed
  */



  /* The bounds of each area in the area stack.
     mBounds[0] is always the whole virtual window.
     mBounds is in virtual coordinates */
  SDL_Rect mBounds[max_area_depth];
  /* mBounds[mActDepth] are the present bounds */
  int mActDepth;
  
  /* I don't use an stl-vector for the updateRects, because:
     a) I don't want the vector to be resized to a small one
        each time the updateRects are cleared
     b) I need direct access to the memory area with the rects
        to pass them to SDL
     The mUpdateRects are in screen coordinates
  */
  SDL_Rect * mUpdateRects;
  int mNumUpdateRects;
  int mReservedUpdateRects;
  bool mUpdateAll;

  SDL_Surface * mBackground;
  SDL_Rect mBackgroundRect; /* screen coordinates */
  int mBackgroundContext; /* value -1 for "undefined" */

  /*** Private functions ***/


  void needMoreUpdateRects() {
    mReservedUpdateRects *= 2;
    mUpdateRects = (SDL_Rect *) realloc(mUpdateRects,
                   sizeof(SDL_Rect) * mReservedUpdateRects);
  }
  
    
  void scale(int & x, int & y) {
    x = x * SDLTools::gScale / scale_base;
    y = y * SDLTools::gScale / scale_base;
  }
  void scale(Sint16 & x, Sint16 & y) {
    x = x * SDLTools::gScale / scale_base;
    y = y * SDLTools::gScale / scale_base;
  }


  /* Area coordinates to Screen coordinates */
  void transformA2S(int & x, int & y) {
    x += mBounds[mActDepth].x;
    y += mBounds[mActDepth].y;
    scale(x, y);
    x += SDLTools::gShiftX;
    y += SDLTools::gShiftY;
  }
  
  
  /* Area coordinates to Virtual coordinates */
  void transformA2V(SDL_Rect & r) {
    r.x += mBounds[mActDepth].x;
    r.y += mBounds[mActDepth].y;
  }

  void scale(SDL_Rect & r) {
    int right = r.x + r.w;
    int bot = r.y + r.h;
    scale(r.x, r.y);
    scale(right, bot);
    r.w = right - r.x;
    r.h = bot - r.y;
  }

  /* Virtual coordinates to Screen coordinates */
  void transformV2S(SDL_Rect & r) {
    scale(r);
    r.x += SDLTools::gShiftX;
    r.y += SDLTools::gShiftY;
  }


  /* Area coordinates to Screen coordinates */
  void transformA2S(SDL_Rect & r) {
    transformA2V(r);
    transformV2S(r);
  }


  /* Clip the (screen) coordinates so that they fit into the window. */
  void boundS(int & x, int & y) {
    if (x<0) x=0;
    if (y<0) y=0;
    SDL_Surface * screen = SDL_GetVideoSurface();
    if (x>screen->w) x=screen->w;
    if (y>screen->h) y=screen->h;
  }

  void boundS(SDL_Rect & r) {
    int x0=r.x;
    int y0=r.y;
    int x1=r.x+r.w;
    int y1=r.y+r.h;
    boundS(x0,y0);
    boundS(x1,y1);
    r.x=x0; r.y=y0; r.w=x1-x0; r.h=y1-y0;
  }
  
  /* r is in virtual coordinates */
  void setClipRectV(SDL_Rect r) {
    transformV2S(r);
    boundS(r);
    SDL_SetClipRect(SDL_GetVideoSurface(), &r);
  }
  
  
  /*** Public functions ***/
  
  
  /* This is called from initSDL() */
  void init() {
    /* 20 updateRects will never be enough, but I prefer that
       needMoreUpdateRects is called early enough so I notice if
       there's a bug inside. */
    mReservedUpdateRects = 20;
    mUpdateRects = (SDL_Rect *) malloc(sizeof(SDL_Rect) * mReservedUpdateRects);
    mNumUpdateRects = 0;
    mUpdateAll = false;

    mActDepth = 0;
    /* Only the top-left corner mBounds[0] should be used.
       Make this rectangle empty, in the hope that this will
       make it easier to detect bugs when it is falsely used */
    mBounds[mActDepth] = SDLTools::rect(0, 0, 0, 0);
    noClip();

    mBackgroundContext = -1;
  }
  
  void destroy() {
    free(mUpdateRects);
  }
  
  
  void enter(SDL_Rect r) {
    transformA2V(r);
    
    mActDepth++;
    CASSERT(mActDepth < max_area_depth);
    if (mActDepth > 1)
      CASSERT(SDLTools::contained(r, mBounds[mActDepth - 1]));
    mBounds[mActDepth] = r;
    noClip();
  }
  
  void leave() {
    CASSERT(mActDepth > 0);
    if (mBackgroundContext==mActDepth) mBackgroundContext=-1;
    mActDepth--;
    noClip();
  }

  void setClip(SDL_Rect r) {
    transformA2V(r);
    if (mActDepth == 0)
      setClipRectV(r);
    else {
      SDL_Rect inter;
      SDLTools::intersection(r,mBounds[mActDepth],inter);
      setClipRectV(inter);
    }
  }
  
  void noClip() {
    if (mActDepth == 0) {
      /* Outer most area is the only one which allows
         to draw outside */
      SDL_Surface * s = SDL_GetVideoSurface();
      SDL_Rect r = SDLTools::rect(0, 0, s->w, s->h);
      SDL_SetClipRect(s, &r);
    } else
      setClipRectV(mBounds[mActDepth]);
  }

  void setBackground(const SDL_Rect & r, SDL_Surface * s) {
    mBackground = s;
    mBackgroundRect = r;
    transformA2S(mBackgroundRect);
    mBackgroundContext = mActDepth;
  }

  void maskBackground(const Maske * mask, SDL_Rect mr, int x, int y) {
    if (mBackgroundContext!=-1 && !mask->is_empty()) {
      int x2 = x + mr.w;
      int y2 = y + mr.h;
      transformA2S(x,y);
      transformA2S(x2,y2);
      scale(mr.x, mr.y);
      SDL_Rect dr = SDLTools::rect(x,y,x2-x,y2-y);
      SDL_Rect ir;
      if (SDLTools::intersection(mBackgroundRect,dr,ir)) {
	boundS(ir);
	mr.x += ir.x-dr.x;
	mr.y += ir.y-dr.y;
        mr.w = ir.w;
        mr.h = ir.h;
	mask->masked_blit(mBackground,
			  ir.x-mBackgroundRect.x, ir.y-mBackgroundRect.y,
			  mr,
			  SDL_GetVideoSurface(),ir);
      }
    }
  }



  /* If the coordinates of srcrect are not a multiple of scale_base, then
     rounding is done in such a way that the result is correct in the
     *destination*, not in the source. */
  void blitSurface(SDL_Surface *src, SDL_Rect srcrect, int dstx, int dsty) {
    int dstx2 = dstx + srcrect.w;
    int dsty2 = dsty + srcrect.h;
    transformA2S(dstx, dsty);
    transformA2S(dstx2, dsty2);
    SDL_Rect dstrect = SDLTools::rect(dstx, dsty, dstx2-dstx, dsty2-dsty);
    boundS(dstrect);
    scale(srcrect.x, srcrect.y);
    srcrect.x+=dstrect.x-dstx;
    srcrect.y+=dstrect.y-dsty;
    srcrect.w = dstrect.w;
    srcrect.h = dstrect.h;

    SDL_BlitSurface(src, &srcrect, SDL_GetVideoSurface(), &dstrect);
  }
  
  void blitSurface(SDL_Surface *src, int dstx, int dsty) {
    transformA2S(dstx, dsty);
    SDL_Rect dstrect = SDLTools::rect(dstx,dsty,src->w,src->h);
    boundS(dstrect);
    SDL_Rect srcrect = SDLTools::rect(dstrect.x-dstx,dstrect.y-dsty,
				      dstrect.w,dstrect.h);
    SDL_BlitSurface(src, &srcrect, SDL_GetVideoSurface(), &dstrect);
  }


  void fillRect(SDL_Rect dst, const Color & c) {
    SDL_Surface * s = SDL_GetVideoSurface();
    transformA2S(dst);
    boundS(dst);
    if (((Sint16) dst.w) < 0 || ((Sint16) dst.h) < 0)
      print_to_stderr(_("Probably trying to fill rectangle of negative size; this causes an overflow.\n"));
    SDL_FillRect(s, &dst, c.getPixel(s->format));
    
  }
  
  void fillRect(int x, int y, int w, int h, const Color & c) {
    fillRect(SDLTools::rect(x, y, w, h), c);
  }

  /* Fills everything outside the current virtual window */
  void fillBorder(const Color & c) {
    SDL_Rect r = SDLTools::rect(0, 0,
                      SDLTools::gVirtualWidth, SDLTools::gVirtualHeight);
    transformV2S(r);
    
    SDL_Surface * s = SDL_GetVideoSurface();
    SDL_Rect dst;
    
    dst = SDLTools::rect(0, 0, r.x, s->h);
    boundS(dst);
    SDL_FillRect(s, &dst, c.getPixel(s->format));
    dst = SDLTools::rect(r.x + r.w, 0, s->w - (r.x + r.w), s->h);
    boundS(dst);
    SDL_FillRect(s, &dst, c.getPixel(s->format));
    dst = SDLTools::rect(r.x, 0, r.w, r.y);
    boundS(dst);
    SDL_FillRect(s, &dst, c.getPixel(s->format));
    dst = SDLTools::rect(r.x, r.y + r.h, r.w, s->h - (r.y + r.h));
    boundS(dst);
    SDL_FillRect(s, &dst, c.getPixel(s->format));
  }


  /* You have to call the following methods to make your drawing operations
     really visible on the screen. (However, the update will take place only
     at the next call to doUpdate) */
  void updateRect(SDL_Rect dst) {
    if (!mUpdateAll) {
      transformA2V(dst);
      if (mActDepth != 0) {
        SDL_Rect inter;
        if (!SDLTools::intersection(dst,mBounds[mActDepth],inter))
          return;
        dst = inter;
      }

      transformV2S(dst);
      boundS(dst);
      if (mNumUpdateRects >= mReservedUpdateRects)
	needMoreUpdateRects();
      mUpdateRects[mNumUpdateRects++] = dst;
    }
  }
  
  
  void updateRect(int x, int y, int w, int h) {
    updateRect(SDLTools::rect(x, y, w, h));
  }
  
  /* Better than calling updateRect(bigRect): Stops collecting small
     rectangles. All means really all, not only active area. */
  void updateAll() {
    mUpdateAll = true;
  }

  

  /* To be called only by ui.cpp */
  void doUpdate() {
    if (mUpdateAll)
      SDL_UpdateRect(SDL_GetVideoSurface(), 0, 0, 0, 0);
    else {
      //print_to_stderr(_sprintf("%d\n", mNumUpdateRects));
      SDL_UpdateRects(SDL_GetVideoSurface(), mNumUpdateRects, mUpdateRects);
    }
    mUpdateAll = false;
    mNumUpdateRects = 0;
  }

}

