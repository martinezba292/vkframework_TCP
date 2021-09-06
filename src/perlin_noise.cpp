#include "perlin_noise.h"
#include <numeric>
#include <random>

float PerlinNoise::fade(float t)
{
  return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

float PerlinNoise::lerp(float t, float a, float b)
{
  return a + t * (b - a);
}

float PerlinNoise::grad(int hash, float x, float y, float z)
{
  // Convert LO 4 bits of hash code into 12 gradient directions
  int h = hash & 15;
  float u = h < 8 ? x : y;
  float v = h < 4 ? y : h == 12 || h == 14 ? x : z;
  return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

PerlinNoise::PerlinNoise()
{
  // Generate random lookup for permutations containing all numbers from 0..255
  std::vector<uint8> plookup;
  plookup.resize(256);
  std::iota(plookup.begin(), plookup.end(), 0);
  std::default_random_engine rndEngine(std::random_device{}());
  std::shuffle(plookup.begin(), plookup.end(), rndEngine);

  for (uint32_t i = 0; i < 256; i++)
  {
    permutations[i] = permutations[256 + i] = plookup[i];
  }
}

float PerlinNoise::noise(float x, float y, float z)
{
  // Find unit cube that contains point
  int32 X = (int32)floor(x) & 255;
  int32 Y = (int32)floor(y) & 255;
  int32 Z = (int32)floor(z) & 255;
  // Find relative x,y,z of point in cube
  x -= floor(x);
  y -= floor(y);
  z -= floor(z);

  // Compute fade curves for each of x,y,z
  float u = fade(x);
  float v = fade(y);
  float w = fade(z);

  // Hash coordinates of the 8 cube corners
  uint32 A = permutations[X] + Y;
  uint32 AA = permutations[A] + Z;
  uint32 AB = permutations[A + 1] + Z;
  uint32 B = permutations[X + 1] + Y;
  uint32 BA = permutations[B] + Z;
  uint32 BB = permutations[B + 1] + Z;

  // And add blended results for 8 corners of the cube;
  float res = lerp(w, lerp(v,
    lerp(u, grad(permutations[AA], x, y, z), grad(permutations[BA], x - 1, y, z)), lerp(u, grad(permutations[AB], x, y - 1, z), grad(permutations[BB], x - 1, y - 1, z))),
    lerp(v, lerp(u, grad(permutations[AA + 1], x, y, z - 1), grad(permutations[BA + 1], x - 1, y, z - 1)), lerp(u, grad(permutations[AB + 1], x, y - 1, z - 1), grad(permutations[BB + 1], x - 1, y - 1, z - 1))));
  return res;
}

FractalNoise::FractalNoise(const PerlinNoise& perlinNoise)
{
  this->perlinNoise = perlinNoise;
  octaves = 6;
  persistence = 0.5f;
}

float FractalNoise::noise(float x, float y, float z)
{
  float sum = 0;
  float frequency = 1.0f;
  float amplitude = 1.0f;
  float max = 0.0f;
  for (uint32_t i = 0; i < octaves; i++)
  {
    sum += perlinNoise.noise(x * frequency, y * frequency, z * frequency) * amplitude;
    max += amplitude;
    amplitude *= persistence;
    frequency *= 2.0f;
  }

  sum = sum / max;
  return (sum + 1.0f) / 2.0f;
}
