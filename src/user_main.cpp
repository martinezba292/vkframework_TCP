#include "user_main.h"
#include "vertex_buffer.h"
#include "resource_manager.h"
#include "entity.h"
#include "Components/transform.h"
#include "Components/geometry.h"
#include "material.h"
#include "Components/texture.h"
#include "Components/point_light.h"
#include "dev/ptr_alloc.h"
#include <array>

//NUMBER OF ENTITIES AND LIGHTS IN SCENE
#define DIMENSIONS 5
#define LIGHT_ROW 5
const uint32 k_entitiesNumber = DIMENSIONS * DIMENSIONS;

PtrAlloc<Entity> massiveTest[k_entitiesNumber];
PtrAlloc<Material> terrainmat;



float accumTime;
float offset = 5.0f;
void UserMain::init()
{
  accumTime = 0.0f;
  ResourceManager* rm = ResourceManager::Get();

  /// <summary>
  /// SKYBOX CREATION
  /// </summary>
  PtrAlloc<Entity> skybox;
  PtrAlloc<Material> skybox_material;
  PtrAlloc<Texture> skybox_texture;
  PtrAlloc<Geometry> skybox_geometry;

  skybox_texture.alloc();
  skybox_texture->loadTexture("./../../data/textures/cubemaps/highwayhdr.ktx", 
                               TextureType::kTextureType_Cubemap, 
                               TextureFormat::kTextureFormat_RGBA16_FLOAT);
  rm->createTexture(skybox_texture.get());

  skybox_material.alloc();
  skybox_material->setMaterialType(MaterialType::kMaterialType_Skybox);
  skybox_material->setTextureCubemap(*skybox_texture, rm->getCamera());
  rm->createMaterial(skybox_material.get());
  skybox_geometry.alloc();
  skybox_geometry->initWithPrimitive(PrimitiveType::kPrimitiveType_Cube);

  skybox.alloc();
  skybox->addComponent(skybox_geometry.get());
  skybox->setMaterial(skybox_material.get());
  rm->createEntity(skybox.get());


  /// <summary>
  /// ENTITY CREATION
  /// </summary>
  //PtrAlloc<Geometry> geo;
  //geo.alloc();
  //geo->initWithPrimitive(PrimitiveType::kPrimitiveType_Sphere);
  //for (size_t i = 0; i < DIMENSIONS; i++) {
  //  for (size_t j = 0; j < DIMENSIONS; ++j) {
  //    PtrAlloc<Transform> tr;
  //    tr.alloc();
  //    tr->setPosition(j * offset, i * offset, -2.0f);
  //    PtrAlloc<Material> pbrTest;
  //    pbrTest.alloc();
  //    pbrTest->setMaterialType(MaterialType::kMaterialType_PBRIBL);
  //    pbrTest->setMaterialColor({ 1.00, 0.71, 0.29 });
  //    float jdiv = (float)j / ((float)DIMENSIONS - 1.0f);
  //    float idiv = (float)i / ((float)DIMENSIONS - 1.0f);
  //    pbrTest->setRoughness(glm::clamp(jdiv, 0.05f, 1.0f));
  //    pbrTest->setMetallic(glm::clamp(idiv, 0.1f, 1.0f));
  //    pbrTest->setExposure(4.5f);
  //    pbrTest->setGammaCorrection(1.0f);
  //    rm->createMaterial(pbrTest.get());

  //    massiveTest[i * DIMENSIONS + j].alloc();
  //    massiveTest[i * DIMENSIONS + j]->addComponent(tr.get());
  //    massiveTest[i * DIMENSIONS + j]->addComponent(geo.get());
  //    massiveTest[i * DIMENSIONS + j]->setMaterial(pbrTest.get());

  //    rm->createEntity(massiveTest[i * DIMENSIONS + j].get());
  //  }
  //}

  /// <summary>
  /// OBJ LOADER
  /// </summary>
  //PtrAlloc<Material> objmat;
  //PtrAlloc<Transform> objtr;
  //std::list<PtrAlloc<Geometry>> objgeo = rm->loadObj("./../../data/models/venus.obj");
  //objtr.alloc();
  //objtr->setPosition(7.0f, 5.0f, -3.0f);
  //objtr->setScale(2.0f, 2.0f, 2.0f);
  //objtr->rotateZ(3.14f);
  //objmat.alloc();
  //objmat->setMaterialType(MaterialType::kMaterialType_BasicPBR);
  //objmat->setMaterialColor({ 1.00, 0.86, 0.24 });
  //objmat->setRoughness(0.1f);
  //objmat->setMetallic(0.5f);
  ////objmat->setExposure(4.5f);
  ////objmat->setGammaCorrection(1.0f);
  //rm->createMaterial(objmat.get());

  //for (auto& geo : objgeo) {
  //  PtrAlloc<Entity> objtest;
  //  objtest.alloc();
  //  geo->create();
  //  objtest->addComponent(objtr.get());
  //  objtest->addComponent(geo.get());
  //  objtest->setMaterial(objmat.get());
  //  rm->createEntity(objtest.get());
  //}

  /// <summary>
  /// POINT LIGHTS
  /// </summary>
  PtrAlloc<Entity> lightTest[LIGHT_ROW * LIGHT_ROW];
  for (size_t i = 0; i < LIGHT_ROW; i++) {
    for (size_t j = 0; j < LIGHT_ROW; j++) {
      PtrAlloc<PointLight> pLight;
      pLight.alloc();
      pLight->setPosition({ j * offset, i * offset, 0.0f });
      pLight->setLightColor({ 1.0f, 1.0f, 1.0f });
      lightTest[i * LIGHT_ROW + j].alloc();
      lightTest[i * LIGHT_ROW + j]->addComponent(pLight.get());
      rm->createEntity(lightTest[i * LIGHT_ROW + j].get());
    }
  }


  PtrAlloc<Entity> terrain;
  PtrAlloc<Transform> terraintr;
  PtrAlloc<Geometry> terraingeo;

  terraingeo.alloc();
  terraingeo->initWithPrimitive(PrimitiveType::kPrimitiveType_Terrain);
  terraintr.alloc();
  terraintr->setPosition(0.0f, 5.0f, 0.0f);

  terrainmat.alloc();
  terrainmat->setMaterialType(MaterialType::kMaterialType_Noise);
  terrainmat->setRandomNoise(45.5f);
  terrainmat->setNoiseAmplification(64.0f);
  rm->createMaterial(terrainmat.get());

  terrain.alloc();
  terrain->addComponent(terraintr.get());
  terrain->addComponent(terraingeo.get());
  terrain->setMaterial(terrainmat.get());
  rm->createEntity(terrain.get());
}



void UserMain::run(float delta_time)
{
  accumTime += delta_time;
  //float p = (sin(accumTime));
  //terrainmat->setNoiseAmplification(p * 100.0f);
}




void UserMain::clear()
{

}