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

#define DIMENSIONS 5
#define LIGHT_ROW 4
const uint32 k_entitiesNumber = DIMENSIONS * DIMENSIONS;


float accumTime;
void UserMain::init()
{
  accumTime = 0.0f;
  ResourceManager* rm = ResourceManager::Get();
  float offset = 3.0f;


  PtrAlloc<Entity> skybox;
  PtrAlloc<Material> skybox_material;
  PtrAlloc<Texture> skybox_texture;
  PtrAlloc<Geometry> skybox_geometry;

  skybox_texture.alloc();
  skybox_texture->loadTexture("./../../data/textures/cubemaps/newport_loft.ktx", 
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



  PtrAlloc<Entity> massiveTest[k_entitiesNumber];
  PtrAlloc<Geometry> geo;
  geo.alloc();
  geo->initWithPrimitive(PrimitiveType::kPrimitiveType_Sphere);
  for (size_t i = 0; i < DIMENSIONS; i++) {
    for (size_t j = 0; j < DIMENSIONS; ++j) {
      PtrAlloc<Transform> tr;
      tr.alloc();
      tr->setPosition(j * offset, i * offset, -3.0f);
      PtrAlloc<Material> pbrTest;
      pbrTest.alloc();
      pbrTest->setMaterialType(MaterialType::kMaterialType_PBRIBL);
      pbrTest->setMaterialColor({ 0.549585f, 0.556114f, 0.554256f });
      float jdiv = (float)j / ((float)DIMENSIONS - 1.0f);
      float idiv = (float)i / ((float)DIMENSIONS - 1.0f);
      pbrTest->setRoughness(glm::clamp(jdiv, 0.05f, 1.0f));
      pbrTest->setMetallic(glm::clamp(idiv, 0.1f, 1.0f));
      pbrTest->setExposure(4.5f);
      pbrTest->setGammaCorrection(1.0f);
      rm->createMaterial(pbrTest.get());

      massiveTest[i * DIMENSIONS + j].alloc();
      massiveTest[i * DIMENSIONS + j]->addComponent(tr.get());
      massiveTest[i * DIMENSIONS + j]->addComponent(geo.get());
      massiveTest[i * DIMENSIONS + j]->setMaterial(pbrTest.get());

      rm->createEntity(massiveTest[i * DIMENSIONS + j].get());
    }
  }

  offset = 8.0f;
  PtrAlloc<Entity> lightTest[LIGHT_ROW * LIGHT_ROW];
  for (size_t i = 0; i < LIGHT_ROW; i++) {
    for (size_t j = 0; j < LIGHT_ROW; j++) {
      PtrAlloc<PointLight> pLight;
      pLight.alloc();
      pLight->setPosition({ j * offset, i * offset, -2.0f });
      pLight->setLightColor({ 1.0f, 1.0f, 0.4f });
      lightTest[i * LIGHT_ROW + j].alloc();
      lightTest[i * LIGHT_ROW + j]->addComponent(pLight.get());
      rm->createEntity(lightTest[i * LIGHT_ROW + j].get());
    }
  }


  //PtrAlloc<Texture> crate_tex;
  //crate_tex.alloc();
  //crate_tex->loadTexture("./../../data/textures/crate.png", TextureType::kTextureType_2D, TextureFormat::kTextureFormat_RGBA8_SRGB);
  //rm->createTexture(crate_tex.get());

  //PtrAlloc<Transform> crate_tr;
  //crate_tr.alloc();
  //crate_tr->setPosition(-4.0f, 0.0f, -0.5f);
  //PtrAlloc<Geometry> crate_geo;
  //crate_geo.alloc();
  //crate_geo->initWithPrimitive(PrimitiveType::kPrimitiveType_Cube);

  //PtrAlloc<Material> crate_material;
  //crate_material.alloc();
  //crate_material->setMaterialType(MaterialType::kMaterialType_TextureSampler);
  //crate_material->setMaterialTexture(*crate_tex);
  //rm->createMaterial(crate_material.get());
  //PtrAlloc<Entity> crate;
  //crate.alloc();
  //crate->addComponent(crate_geo.get());
  //crate->addComponent(crate_tr.get());
  //crate->setMaterial(crate_material.get());

  //rm->createEntity(crate.get());
}

void UserMain::run(float delta_time)
{
  //Transform* tr = testEntity.getComponent<Transform>(kComponentType_Transform);
  //tr->rotateX(accumTime);
  accumTime += delta_time;
}

void UserMain::clear()
{

}

