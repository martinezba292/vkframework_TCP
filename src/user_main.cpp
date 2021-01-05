#include "user_main.h"
#include "vertex_buffer.h"
#include "resource_manager.h"
#include "entity.h"
#include "Components/transform.h"
#include "Components/geometry.h"
#include "material.h"
#include "Components/texture.h"

#define DIMENSIONS 10
const uint32 k_entitiesNumber = DIMENSIONS * DIMENSIONS;


float accumTime;
//Entity massiveTest[k_entitiesNumber];
Entity colorTest;

void UserMain::init()
{
  accumTime = 0.0f;
  ResourceManager* rm = ResourceManager::Get();

  //uint32 offset = 3.0f;
  //for (size_t i = 0; i < DIMENSIONS; i++) {
  //  for (size_t j = 0; j < DIMENSIONS; j++) {
  //    uint32 index = i * DIMENSIONS + j;

  //    Transform* newTR = new Transform();
  //    newTR->setPosition(j * offset, i * offset, -5.0f);
  //    Geometry* newGeo = new Geometry();
  //    newGeo->initWithPrimitive(kPrimitiveType_Sphere);

  //    massiveTest[index].addComponent(newTR);
  //    massiveTest[index].addComponent(newGeo);

  //    rm->createEntity(&massiveTest[index]);
  //  }
  //}

  Texture tex;
  tex.loadTexture("./../../data/textures/rock_texture.jpg");
  rm->createTexture(&tex);

  
  Transform* colorTr = new Transform();
  colorTr->setPosition(-3.0f, 2.0f, -2.0f);
  Geometry* colorGeo = new Geometry();
  colorGeo->initWithPrimitive(kPrimitiveType_Sphere);
  Material color_mat;
  color_mat.setMaterialType(kMaterialType_UnlitColor);
  glm::vec3 color1 = { 1.0f, 0.2f, 0.0f };
  color_mat.setMaterialColor(color1);
  //color_mat.setMaterialTexture(tex);
  rm->createMaterial(&color_mat);
  colorTest.addComponent(colorTr);
  colorTest.addComponent(colorGeo);
  colorTest.setMaterial(&color_mat);

  rm->createEntity(&colorTest);


  Texture tex2;
  tex2.loadTexture("./../../data/textures/wave.jpg");
  rm->createTexture(&tex2);

  Entity textureTest;
  Transform* quad_tr = new Transform();
  quad_tr->setPosition(0.0f, 0.0f, -2.0f);
  quad_tr->setScale(2.0f, 1.0f, 1.0f);
  Geometry* quad_geometry = new Geometry();
  quad_geometry->initWithPrimitive(kPrimitiveType_Quad);


  Material texture_mat;
  texture_mat.setMaterialType(kMaterialType_TextureSampler);
  texture_mat.setMaterialTexture(tex2);
  rm->createMaterial(&texture_mat);
  textureTest.setMaterial(&texture_mat);
  textureTest.addComponent(quad_tr);
  textureTest.addComponent(quad_geometry);

  rm->createEntity(&textureTest);
}

void UserMain::run(float delta_time)
{
  Transform* colortr = colorTest.getComponent<Transform>(kComponentType_Transform);
  colortr->rotateY(accumTime);
  accumTime += delta_time;
}

void UserMain::clear()
{

}

