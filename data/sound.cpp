/***************************************************************************
                          sound.cpp  -  description
                             -------------------
    begin                : Fri Jul 21 2000
    copyright            : (C) 2000 by Immi
    email                : cuyo@pcpool.mathematik.uni-freiburg.de

Modified 2003-2006,2008,2010,2011 by the cuyo developers

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
#include <vector>

#include <SDL.h>
#include <SDL_mixer.h>

#include "cuyointl.h"
#include "sound.h"
#include "pfaditerator.h"
#include "fehler.h"

/* Um rauszufinden, ob's ein oder zwei Spieler gibt, für Stereo */
#include "cuyo.h"

#include "prefsdaten.h"

const char *sample_namen[fix_sample_anz] = {
  "leftright.wav",
  "turn.wav",
  "down.wav",
  "land.wav",
  "explode.wav",
  "menuclick.wav",
  "menuscroll.wav",
  "levelwin.wav",
  "levelloose.wav"
};

#define sound_chunk_size 512
#define sound_freq MIX_DEFAULT_FREQUENCY
#define sound_format MIX_DEFAULT_FORMAT  // AUDIO_S16
#define sound_music_volume (MIX_MAX_VOLUME / 2)


/* Globale Variable mit dem Sound-Objekt */

namespace Sound {

  bool mSoundInitialisiert;
  bool mSoundGeht;
  Mix_Music * mAktMod;
  std::vector<Str> mSampleNamen;
  std::vector<Mix_Chunk *> mSamples;



/***************************************************************************/





void init() {

  mSoundInitialisiert = false;

  mSoundGeht = false;

  /* Sound wird nur dann jetzt schon initialisiert, wenn
     Sound in den Prefs angeschaltet ist. Auf Computern,
     auf denen Sound nicht geht, braucht SDL nämlich manchmal
     ziemlich lang um zu merken, dass es nicht geht. */  
  checkePrefsStatus();
}


void destroy() {
  if (!mSoundGeht)
    return;

  if (mAktMod)
    Mix_FreeMusic(mAktMod);
    
  for (int i = 0; i < (int) mSamples.size(); i++)
    Mix_FreeChunk(mSamples[i]);
}



/** Sollte aufgerufen werden, wenn sich Pref->sound
    möglicherweise geändert hat */
void checkePrefsStatus() {

  /* Möglicherweise muss die Musik gestoppt werden */
  if (mSoundGeht && !PrefsDaten::getSound())
    setMusic("");
    

  /* Möglicherweise sound-Initialisierung nachholen */
  if (mSoundInitialisiert)
    return;
  if (!PrefsDaten::getSound())
    return;
    
  /* Nicht initialisiert. Soll aber. */
  
  mSoundInitialisiert = true;
  
  if (Mix_OpenAudio(sound_freq, sound_format, 2, sound_chunk_size) < 0) {
    // TRANSLATORS: The %s is a (hopefully) more specific error message
    print_to_stderr(_sprintf(_("Sound does not work: %s\n"), SDL_GetError()));
    return;
  } 
  Mix_VolumeMusic(sound_music_volume);
  
  
   mSoundGeht = true;
 

  mAktMod = 0;


  /* Basis-Samples laden */
  for (int i = 0; i < fix_sample_anz; i++) {
    ladSample(sample_namen[i]);
  }

  
  
}





/** Spielt die angegebene Mod-Datei immer wieder ab.
    Bei na = "" wird nix abgespielt. */
void setMusic(Str na) {

  if (!mSoundGeht || !PrefsDaten::getSound())
    return;
  

  if (mAktMod)
    Mix_FreeMusic(mAktMod);
  mAktMod = 0;
  
  if (na == "")
    return;

  try {
    for (PfadIterator pi(_sprintf("sounds/%s", na.data()));
       !(mAktMod = Mix_LoadMUS(pi.pfad().data())); ++pi) {}
  } catch (Fehler fe) {
    print_to_stderr(fe.getText()+"\n");
    return;
  }

  /* -1 = Lied soll im Kreis laufen: */
  Mix_PlayMusic(mAktMod, -1);
}



/** Lädt den angegebenen Sample und liefert eine Nummer zurück,
    mit dem man ihn abspielen kann. */
int ladSample(Str na) {

  if (!mSoundGeht)
    return sample_nix;

  /* Haben wir das schon geladen? */
  for (int i = 0; i < (int) mSampleNamen.size(); i++)
    if (mSampleNamen[i] == na)
      return i;

  int ret = mSamples.size();
  mSamples.resize(ret + 1);
  mSampleNamen.push_back(na);
  
  for (PfadIterator pi(_sprintf("sounds/%s", na.data()));
       !(mSamples[ret] = Mix_LoadWAV(pi.pfad().data())); ++pi) {}

  return ret;
}
  
  
  
/** Gibt alle Samples wieder frei, die mit ladSample geladen worden
    sind, außer die, die init() geladen hat. Sollte nach Levelende
    aufgerufen werden, wenn die Levelsounds nicht mehr gebraucht werden.
    (Aber erst, wenn die ld-Dateien einzeln geladen werden.) */
void loescheUserSamples() {
  for (int i = fix_sample_anz; i < (int) mSamples.size(); i++)
    Mix_FreeChunk(mSamples[i]);
  mSamples.resize(fix_sample_anz);
}



/* Akustisches Layout */

#define felder_spalte 64
#define einzel_rand 0


/** Spielt das Sample mit der angegebenen Nummer (die entweder eine
    der obigen Konstanten ist oder von ladSample zurückgeliefert wurde).
    so,xz,xn bestimmen die x-Position für Stereo-Effekte.
    Dabei ist xz/xn ein Bruch, 0 für den linken und 1 für den rechten
    Rand von so. Bei so=so_global werden xz und xn ignoriert. */
void playSample(int nr, SoundOrt so, int xz /*=1*/, int xn /*=2*/) {
  if (!mSoundGeht || !PrefsDaten::getSound())
    return;
    
  if (nr == sample_nix)
    return;

  CASSERT(nr >= 0 && nr < (int) mSamples.size());
  CASSERT(mSamples[nr]);

  int links,rechts;
  switch (so) {
    case so_fenster: links=0; rechts=255; break;
    case so_lfeld:
      if (Cuyo::getSpielerZahl()==2)
	{links=0; rechts=127-felder_spalte/2;}
      else
        {links=einzel_rand; rechts=255-einzel_rand;};
      break;
    case so_rfeld: links=128+felder_spalte/2; rechts=255; break;
    case so_lsemi:
      if (Cuyo::getSpielerZahl()==2)
        {links=0; rechts=0;}
      else
        {links=127; rechts=128;};
      break;
    case so_rsemi: links=255; rechts=255; break;
    case so_global: links=127; rechts=128; break;
    default:
      throw iFehler("%s",_("Data corruption."));
  }

  int pan = (xz*rechts + (xn-xz)*links + xn/2) / xn;
  if (pan<0) pan=0;
  if (pan>255) pan=255;
    
  /* Abspielen starten. -1: Im ersten freien channel; 0: nicht wiederholen */
  int ch = Mix_PlayChannel(-1, mSamples[nr], 0);
  /* Stereo-Einstellung */
  Mix_SetPanning(ch, 255-pan, pan);
}



} // namespace Sound
