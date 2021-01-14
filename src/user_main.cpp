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

#define DIMENSIONS 10
#define LIGHT_ROW 5
const uint32 k_entitiesNumber = DIMENSIONS * DIMENSIONS;


float accumTime;
void UserMain::init()
{
  accumTime = 0.0f;
  ResourceManager* rm = ResourceManager::Get();
  float offset = 3.0f;

  PtrAlloc<Entity> massiveTest[k_entitiesNumber];
  PtrAlloc<Entity> lightTest[LIGHT_ROW * LIGHT_ROW];
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
      pbrTest->setMaterialColor({ 1.0f, 0.71f, 0.29f });
      pbrTest->setMaterialType(MaterialType::kMaterialType_BasicPBR);
      float jdiv = (float)j / ((float)DIMENSIONS - 1.0f);
      float idiv = (float)i / ((float)DIMENSIONS - 1.0f);
      pbrTest->setRoughness(glm::clamp(jdiv, 0.05f, 1.0f));
      pbrTest->setMetallic(glm::clamp(idiv, 0.1f, 1.0f));
      rm->createMaterial(pbrTest.get());

      massiveTest[i * DIMENSIONS + j].alloc();
      massiveTest[i*DIMENSIONS+j]->addComponent(tr.get());
      massiveTest[i*DIMENSIONS+j]->addComponent(geo.get());
      massiveTest[i*DIMENSIONS+j]->setMaterial(pbrTest.get());

      rm->createEntity(massiveTest[i * DIMENSIONS + j].get());
    }
  }

  offset = 6.0f;
  for (size_t i = 0; i < LIGHT_ROW; i++) {
    for (size_t j = 0; j < LIGHT_ROW; j++) {
      PtrAlloc<PointLight> pLight;
      pLight.alloc();
      pLight->setPosition({ j * offset, i * offset, -1.0f });
      pLight->setLightColor({ 1.0f, 1.0f, 1.0f });
      lightTest[i * LIGHT_ROW + j].alloc();
      lightTest[i * LIGHT_ROW + j]->addComponent(pLight.get());
      rm->createEntity(lightTest[i * LIGHT_ROW + j].get());
    }
  }

  /*Texture crate_tex;
  crate_tex.loadTexture("./../../data/textures/crate.png");
  rm->createTexture(&crate_tex);

  Transform* crate_tr = new Transform();
  crate_tr->setPosition(1.0f, 0.0f, -0.5f);
  Geometry* crate_geo = new Geometry();
  crate_geo->initWithPrimitive(kPrimitiveType_Cube);

  Material crate_material;
  crate_material.setMaterialType(kMaterialType_TextureSampler);
  crate_material.setMaterialTexture(crate_tex);
  rm->createMaterial(&crate_material);
  crate.addComponent(crate_geo);
  crate.addComponent(crate_tr);
  crate.setMaterial(&crate_material);

  rm->createEntity(&crate);*/
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

