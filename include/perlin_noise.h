#ifndef  __PERLIN_NOISE__
#define  __PERLIN_NOISE__ 1

#include "common_def.h"

class PerlinNoise {
private:
  uint32 permutations[512];
  float fade(float t);
  float lerp(float t, float a, float b);
  float grad(int hash, float x, float y, float z);



public:
  PerlinNoise();
  float noise(float x, float y, float z);

};


class FractalNoise {
private:
  PerlinNoise perlinNoise;
  uint32_t octaves;
  float persistence;
public:

  FractalNoise(const PerlinNoise& perlinNoise);
  float noise(float x, float y, float z);

};

#endif // define  __PERLIN_NOISE__ 1


