#include <SFML/Audio.hpp>
#include <string>

#include "util.hpp"

sf::SoundBuffer _buffers[256];
sf::Sound _sounds[256];
bool _didInit = false;

void loadSound(std::string filename, unsigned index) {
  if (!_buffers[index].loadFromFile(filename.c_str())) {
    utilLogError("[loadSound] Couldn't load sound %s", filename.c_str());
    exit(EXIT_FAILURE);
  }
  _sounds[index].setBuffer(_buffers[index]);
}

void soundPlay(unsigned index, unsigned volume) {
  if (!_didInit) {
    utilLogError("[soundPlay] Sounds not initalised, refusing to play");
    return;
  }
  _sounds[index].setVolume(volume);
  _sounds[index].play();
}

void soundInit() {
  loadSound("audio/piano/C3sus2.wav", 10);
  loadSound("audio/piano/C3sus2-1.wav", 11);
  for (unsigned offset = 0; offset <= 50; offset += 10) {
    loadSound("audio/cello/C1.wav", offset + 100);
    loadSound("audio/cello/E1.wav", offset + 101);
    loadSound("audio/cello/G1.wav", offset + 102);
    loadSound("audio/cello/C2.wav", offset + 103);
    loadSound("audio/cello/E2.wav", offset + 104);
    loadSound("audio/cello/G2.wav", offset + 105);
    loadSound("audio/cello/C3.wav", offset + 106);
    loadSound("audio/cello/E3.wav", offset + 107);
    loadSound("audio/cello/G3.wav", offset + 108);
    loadSound("audio/cello/C4.wav", offset + 109);
  }
  _didInit = true;
}
