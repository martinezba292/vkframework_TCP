#include "resource_manager.h"
#include "vertex_buffer.h"
#include "internal.h"
#include <iterator>
#include "entity.h"
#include "Components/geometry.h"
#include "material.h"
#include "camera.h"
#include <array>
#include "Components/texture.h"

Geometry PrimitiveGeometry::triangle;
Geometry PrimitiveGeometry::quad;
Geometry PrimitiveGeometry::cube;
Geometry PrimitiveGeometry::sphere;

ResourceManager* ResourceManager::instance_ = nullptr;
Camera Scene::camera;
uint32 Scene::entitiesCount = 0;
std::array<Entity, kMaxInstance> Scene::sceneEntities;
uint32 Scene::materialCount = 0;
std::array<Material, kMaxInstance> Scene::sceneMaterials;
uint32 Scene::textureCount = 0;
std::array<Texture, kMaxTexture> Scene::userTextures;
std::chrono::steady_clock::time_point Scene::lastTime;
//std::vector<Entity> Scene::sceneEntities;
//std::vector<Material> Scene::sceneMaterials;


ResourceManager::~ResourceManager()
{
  for (auto& entity : Scene::sceneEntities) {
    entity.removeComponents();
  }

  for (auto& materials : Scene::sceneMaterials) {
    delete(materials.settings_);
  }

  delete(resources_);
}

ResourceManager* ResourceManager::Get()
{
   if (!instance_) {
     instance_ = new ResourceManager();
   }
 
   return instance_;
}


Resources* ResourceManager::getResources() const
{
  return resources_;
}

void ResourceManager::initPrimitiveGeometries()
{
  /*****TRIANGLE******/
  Vertex triangle[] = {
    VertexBuffer::VertexInitializer({ 0.5f,  0.5f, 0.0f}),
    VertexBuffer::VertexInitializer({ 0.0f, -0.5f, 0.0f}),
    VertexBuffer::VertexInitializer({-0.5f,  0.5f, 0.0f})
  };

  uint32 triangle_indices[] = { 0, 1, 2 };

  PrimitiveGeometry::triangle.loadGeometry(triangle, 3, triangle_indices, 3);
  PrimitiveGeometry::triangle.create();
  
  /*****QUAD******/
  Vertex quad[] = {
    {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
    {{ 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
    {{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}
  };

  uint32 quad_indices[] = {
    0, 2, 1, 2, 0, 3
  };

  PrimitiveGeometry::quad.loadGeometry(quad, 4, quad_indices, 6);
  PrimitiveGeometry::quad.create();


  /******CUBE*******/
  Vertex cube[] = {
    ///FRONT
    {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
    {{ 0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
    {{ 0.5f,  0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f,  0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},

    ///REAR
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
    {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
    {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
    {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},

    ///RIGHT
    {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
    {{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
    {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},

    ///LEFT
    {{ -0.5f, -0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{ -0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{ -0.5f,  0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
    {{ -0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},

    ///DOWN
    {{ -0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{ -0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{  0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
    {{  0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},

    ///UP
    {{ -0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
    {{ -0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
    {{  0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
    {{  0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}}
  };

  uint32 cube_indices[] = { 0,  2,  1,  2,  0,  3,
                            4,  5,  6,  4,  6,  7,
                            8,  10, 9,  8,  9,  11,
                            12, 14, 13, 12, 15, 14,
                            16, 17, 19, 19, 17, 18,
                            23, 21, 20, 21, 23, 22 };

  PrimitiveGeometry::cube.loadGeometry(cube, 24, cube_indices, 36);
  PrimitiveGeometry::cube.create();

  

  /*****SPHERE*****/
  float longitudeRev = 50.0f;
  float latitudeRev = 30.0f;
  //if (longitudeRev <= 1.0f) return;
  //if (latitudeRev <= 1.0f) return;

  float x, y, z, xy;                  // vertex position
  float nx, ny, nz;										// vertex normal
  float tx, ty;                       // vertex texCoord

  float PI = 3.141596f;

  std::vector<Vertex>sphere;
  float sectorStep = 2 * PI / longitudeRev;
  float stackStep = PI / latitudeRev;
  float sectorAngle, stackAngle;


  for (size_t i = 0; i <= latitudeRev; ++i)
  {
    stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
    xy = cosf(stackAngle);             // r * cos(u)
    Vertex vert;
    vert.vertex.z = sinf(stackAngle);              // r * sin(u)

    // add (longitudeRev+1) vertices per stack
    // the first and last vertices have same position and normal, but different tex coords
    for (size_t j = 0; j <= longitudeRev; ++j)
    {
      sectorAngle = j * sectorStep;           // starting from 0 to 2pi

      // vertex position (x, y, z)

      vert.vertex.x = (float)(xy * cosf(sectorAngle));             // r * cos(u) * cos(v)
      vert.vertex.y = (float)(xy * sinf(sectorAngle));             // r * cos(u) * sin(v)


      // normalized vertex normal (nx, ny, nz)
      vert.normal.x = vert.vertex.x;
      vert.normal.y = vert.vertex.y;
      vert.normal.z = vert.vertex.z;


      // vertex tex coord (s, t) range between [0, 1]
      vert.uv.x = (float)j / longitudeRev;
      vert.uv.y = (float)i / latitudeRev;

      sphere.push_back(vert);
    }
  }

  std::vector<uint32>sphere_indices;
  int32 k1, k2;
  for (size_t i = 0; i < latitudeRev; ++i)
  {
    k1 = i * (longitudeRev + 1);     // beginning of current stack
    k2 = k1 + longitudeRev + 1;      // beginning of next stack

    for (size_t j = 0; j < longitudeRev; ++j, ++k1, ++k2)
    {
      // 2 triangles per sector excluding first and last stacks
      // k1 => k2 => k1+1
      if (i != 0)
      {
        sphere_indices.push_back(k1);
        sphere_indices.push_back(k1 + 1);
        sphere_indices.push_back(k2);
      }

      // k1+1 => k2 => k2+1
      if (i != (latitudeRev - 1))
      {
        sphere_indices.push_back(k1 + 1);
        sphere_indices.push_back(k2 + 1);
        sphere_indices.push_back(k2);
      }
    }
  }

  PrimitiveGeometry::sphere.loadGeometry(sphere.data(), sphere.size(), sphere_indices.data(), sphere_indices.size());
  PrimitiveGeometry::sphere.create();
}


void ResourceManager::createVertexBuffer(VertexBuffer* buffer)
{
  buffer->buffer_id_ = resources_->vertex_data.size();
  InternalVertexData newVertexData;
  newVertexData.vertex.resize(buffer->getVertexNumber());

  const Vertex* vertices = buffer->getVertexArray();
  std::copy(vertices,
            vertices + buffer->getVertexNumber(),
            newVertexData.vertex.data());

  uint32 indices_number = buffer->indices_array_.size();
  newVertexData.indices.resize(indices_number);
  const uint32* indices = buffer->indices_array_.data();
  std::copy(indices, indices + indices_number, newVertexData.indices.data());

  resources_->vertex_data.push_back(newVertexData);
}


void ResourceManager::createEntity(Entity* new_entity)
{
  if (!new_entity || new_entity->id_ >= 0) return;
  if (Scene::entitiesCount >= kMaxInstance) return;

  new_entity->id_ = Scene::entitiesCount;
  Scene::sceneEntities[Scene::entitiesCount] = *new_entity;
  ++Scene::entitiesCount;
}

void ResourceManager::createMaterial(Material* material)
{
  if (!material || material->materialId_ >= 0) return;
  if (Scene::materialCount >= kMaxMaterial) return;

  material->materialId_ = Scene::materialCount;
  Scene::sceneMaterials[Scene::materialCount] = *material;
  ++Scene::materialCount;
}

void ResourceManager::createTexture(Texture* texture)
{
  if (!texture || texture->id_ >= 0) return;
  if (Scene::textureCount >= kMaxTexture) return;

  texture->id_ = Scene::textureCount;
  Scene::userTextures[Scene::textureCount] = *texture;
  ++Scene::textureCount;
}

ResourceManager::ResourceManager()
{
  resources_ = new Resources();
}

