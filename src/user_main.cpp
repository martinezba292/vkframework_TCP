#include "user_main.h"
#include "vertex_buffer.h"
#include "resource_manager.h"
#include "entity.h"
#include "Components/transform.h"
#include "Components/geometry.h"
#include "material.h"
#include "Components/texture.h"
#include "Components/point_light.h"

#define DIMENSIONS 10
const uint32 k_entitiesNumber = DIMENSIONS * DIMENSIONS;


float accumTime;
//Entity massiveTest[k_entitiesNumber];
Entity testEntity;
Entity lightTestEntity;

Entity crate;

void UserMain::init()
{
  accumTime = 0.0f;
  ResourceManager* rm = ResourceManager::Get();

  Transform* tr = new Transform();
  tr->setPosition(0.0f, 0.0f, -4.0f);
  Geometry* geo = new Geometry();
  geo->initWithPrimitive(kPrimitiveType_Sphere);

  Material pbrTest;
  pbrTest.setMaterialType(kMaterialType_BasicPBR);
  pbrTest.setMaterialColor({ 1.0f, 0.7655f, 0.3360f });
  //pbrTest.setMaterialColor({ 0.0f, 1.f, 0.0f });
  pbrTest.setRoughness(0.5f);
  pbrTest.setMetallic(1.0f);
  rm->createMaterial(&pbrTest);

  testEntity.addComponent(tr);
  testEntity.addComponent(geo);
  testEntity.setMaterial(&pbrTest);

  rm->createEntity(&testEntity);


  Transform* lightTr = new Transform();
  lightTr->setPosition(0.0f, -2.0f, -4.0f);
  PointLight* pLight = new PointLight();

  lightTestEntity.addComponent(lightTr);
  lightTestEntity.addComponent(pLight);
  rm->createEntity(&lightTestEntity);


  Texture crate_tex;
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

  rm->createEntity(&crate);
}

void UserMain::run(float delta_time)
{
  Transform* tr = testEntity.getComponent<Transform>(kComponentType_Transform);
  tr->rotateX(accumTime);
  accumTime += delta_time;
}

void UserMain::clear()
{

}

