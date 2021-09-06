#include "resource_manager.h"
#include "vertex_buffer.h"
#include "dev/internal.h"
#include "entity.h"
#include "Components/geometry.h"
#include "material.h"
#include "camera.h"
#include "Components/texture.h"
#include <unordered_map>
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"


struct PrimitiveGeometry {
  PtrAlloc<Geometry> triangle;
  PtrAlloc<Geometry> quad;
  PtrAlloc<Geometry> cube;
  PtrAlloc<Geometry> sphere;
  PtrAlloc<Geometry> terrain;
}Primitives;

ResourceManager* ResourceManager::instance_ = nullptr;


Camera Scene::camera;
uint32 Scene::entitiesCount = 0;
std::array<PtrAlloc<Entity>, kMaxInstance> Scene::sceneEntities;
uint32 Scene::materialCount = 0;
std::array<PtrAlloc<Material>, kMaxMaterial> Scene::sceneMaterials;
uint32 Scene::textureCount = 0;
std::array<PtrAlloc<Texture>, kMaxTexture> Scene::userTextures;
std::chrono::steady_clock::time_point Scene::lastTime;


ResourceManager::~ResourceManager()
{
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

  uint32 triangle_indices[] = { 0, 2, 1 };

  Primitives.triangle.alloc();
  Primitives.triangle->loadGeometry(triangle, 3, triangle_indices, 3);
  Primitives.triangle->create();
  
  /*****QUAD******/
  Vertex quad[] = {
    {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
    {{ 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
    {{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}
  };

  uint32 quad_indices[] = {
    0, 1, 2, 2, 3, 0
  };

  Primitives.quad.alloc();
  Primitives.quad->loadGeometry(quad, 4, quad_indices, 6);
  Primitives.quad->create();


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

    uint32 cube_indices[] = { 1,  2,  0,  3,  0,  2,
                              6,  5,  4,  7,  6,  4,
                              9,  10, 8,  11,  9,  8,
                              13, 14, 12, 14, 15, 12,
                              19, 17, 16, 18, 17, 19,
                              20, 21, 23, 22, 23, 21 };

  Primitives.cube.alloc();
  Primitives.cube->loadGeometry(cube, 24, cube_indices, 36);
  Primitives.cube->create();

  /*****SPHERE*****/
  sphereGenerator(50.0f, 30.0f);


  /*****TERRAIN MESH*****/
  terrainGenerator(64, 64);
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
  Scene::sceneEntities[Scene::entitiesCount] = new_entity;
  ++Scene::entitiesCount;
}

void ResourceManager::createMaterial(Material* material)
{
  if (!material || material->materialId_ >= 0) return;
  if (Scene::materialCount >= kMaxMaterial) return;

  material->materialId_ = Scene::materialCount;
  Scene::sceneMaterials[Scene::materialCount] = material,
  ++Scene::materialCount;
}

void ResourceManager::createTexture(Texture* texture)
{
  if (!texture || texture->id_ >= 0) return;
  if (Scene::textureCount >= kMaxTexture) return;

  texture->id_ = Scene::textureCount;
  Scene::userTextures[Scene::textureCount] = texture;
  ++Scene::textureCount;
}

Camera& ResourceManager::getCamera()
{
  return Scene::camera;
}


void ResourceManager::terrainGenerator(uint32 w, uint32 h)
{
  float divw = 1.0f / (float)w;// - 1.0f;
  float divh = 1.0f / (float)h;// - 1.0f;
  std::vector<Vertex> vertices;
  for (size_t i = 0; i < h; i++) {
    for (size_t j = 0; j < w; j++) {
      // v0-----v1
      // |  \    |
      // |   \   |
      // |    \  |
      // v3-----v2
      float nextw = (float)j + 1.0f;
      float nexth = (float)i + 1.0f;
      vertices.push_back({{ j - 0.5f, 0.0f, i - 0.5f }, { 0.0f, 0.0f, 1.0f }, {(float)j * divw, (float)i * divh}});
      vertices.push_back({{ j + 0.5f, 0.0f, i - 0.5f }, { 0.0f, 0.0f, 1.0f }, {nextw    * divw, (float)i * divh}});
      vertices.push_back({{ j + 0.5f, 0.0f, i + 0.5f }, { 0.0f, 0.0f, 1.0f }, {nextw    * divw, nexth    * divh}});
      vertices.push_back({{ j - 0.5f, 0.0f, i + 0.5f }, { 0.0f, 0.0f, 1.0f }, {(float)j * divw, nexth    * divh}});

      //vertices.push_back({{ j, 0.0f, i }, { 0.0f, 0.0f, 1.0f }, {(float)j / w, (float)i / h}});


    }
  }

  std::vector<uint32> indices;
  for (size_t i = 0; i < h; i++) {
    uint32 index1 = w * i;
    for (size_t j = 0; j < w; j++) {
      uint32 index = index1 + j;
      ////v0--v1
      //// \  |
      ////  \ |
      ////   v2
      indices.push_back(4 * index);
      indices.push_back(4 * index + 1);
      indices.push_back(4 * index + 2);

      //// v0
      //// | \ 
      //// |  \
      //// v2--v1
      indices.push_back(4 * index);
      indices.push_back(4 * index + 2);
      indices.push_back(4 * index + 3);
    }
  }

  Primitives.terrain.alloc();
  Primitives.terrain->loadGeometry(vertices.data(), vertices.size(), indices.data(), indices.size());
  Primitives.terrain->create();
}


void ResourceManager::sphereGenerator(float longitude_rev, float latitude_rev)
{
  float xy;   //amplitude

  std::vector<Vertex>sphere;
  float sectorStep = 2 * PI / longitude_rev;
  float stackStep = PI / latitude_rev;
  float sectorAngle, stackAngle;

  for (size_t i = 0; i <= latitude_rev; ++i)
  {
    stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
    xy = cosf(stackAngle);             // r * cos(u)
    Vertex vert;
    vert.vertex.z = sinf(stackAngle);              // r * sin(u)

    // add (longitudeRev+1) vertices per stack
    // the first and last vertices have same position and normal, but different tex coords
    for (size_t j = 0; j <= longitude_rev; ++j)
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
      vert.uv.x = (float)j / longitude_rev;
      vert.uv.y = (float)i / latitude_rev;

      sphere.push_back(vert);
    }
  }

  std::vector<uint32>sphere_indices;
  int32 k1, k2;
  for (size_t i = 0; i < latitude_rev; ++i)
  {
    k1 = i * ((uint32)(longitude_rev)+1);     // beginning of current stack
    k2 = k1 + (uint32)(longitude_rev)+1;      // beginning of next stack

    for (size_t j = 0; j < longitude_rev; ++j, ++k1, ++k2)
    {
      // 2 triangles per sector excluding first and last stacks
      // k1 => k2 => k1+1
      if (i != 0)
      {
        sphere_indices.push_back(k2);
        sphere_indices.push_back(k1 + 1);
        sphere_indices.push_back(k1);
      }

      // k1+1 => k2 => k2+1
      if (i != (latitude_rev - 1))
      {
        sphere_indices.push_back(k2);
        sphere_indices.push_back(k2 + 1);
        sphere_indices.push_back(k1 + 1);
      }
    }
  }

  Primitives.sphere.alloc();
  Primitives.sphere->loadGeometry(sphere.data(), sphere.size(), sphere_indices.data(), sphere_indices.size());
  Primitives.sphere->create();
}

std::list<PtrAlloc<Geometry>> ResourceManager::loadObj(std::string path)
{
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;

  assert(tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()));

  std::vector<Vertex> vertexobj;
  std::vector<uint32> indices;
  std::list<PtrAlloc<Geometry>> geometries;
  std::unordered_map<Vertex, uint32> unique_vertices;
  for (const auto& shape : shapes) {
    PtrAlloc<Geometry> new_shape;
    new_shape.alloc();
    for (const auto& indice : shape.mesh.indices) {
      Vertex vertices{};
      vertices.vertex = {
        attrib.vertices[3 * indice.vertex_index],
        attrib.vertices[3 * indice.vertex_index + 1],
        attrib.vertices[3 * indice.vertex_index + 2],
      };

      vertices.normal = {
        attrib.normals[3 * indice.normal_index],
        attrib.normals[3 * indice.normal_index + 1],
        attrib.normals[3 * indice.normal_index + 2],
      };

      if (attrib.texcoords.size()) {
        vertices.uv = {
          attrib.texcoords[2 * indice.texcoord_index],
          1.0f - attrib.texcoords[2 * indice.texcoord_index + 1],
        };
      }

      if (!unique_vertices.count(vertices)) {
        unique_vertices[vertices] = static_cast<uint32>(vertexobj.size());
        vertexobj.push_back(vertices);
      }

      indices.push_back(unique_vertices[vertices]);
    }
    new_shape->loadGeometry(vertexobj.data(), vertexobj.size(), indices.data(), indices.size());
    geometries.push_back(new_shape);
  }

  return geometries;
}

ResourceManager::ResourceManager()
{
  resources_ = new Resources();
}

