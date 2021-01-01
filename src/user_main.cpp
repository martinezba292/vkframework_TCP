#include "user_main.h"
#include "vertex_buffer.h"
#include "resource_manager.h"
#include "entity.h"
#include "Components/transform.h"
#include "Components/geometry.h"

#define DIMENSIONS 10
const uint32 k_entitiesNumber = DIMENSIONS * DIMENSIONS;


float accumTime;
Entity massiveTest[k_entitiesNumber];
Entity textureTest;

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

  //Vertex quad[] = {
  //  {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
  //  {{ 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
  //  {{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
  //  {{-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}
  //};

  //uint32 quad_indices[] = {
  //  0, 2, 1, 2, 0, 3
  //};

  Transform* quad_tr = new Transform();
  quad_tr->setPosition(0.0f, 0.0f, -2.0f);
  //float scale = 2.0f;
  //quad_tr->setScale(scale, scale, 1.0f);
  Geometry* quad_geometry = new Geometry();
  quad_geometry->initWithPrimitive(kPrimitiveType_Quad);
  //quad_geometry->loadGeometry(quad, 4, quad_indices, 6);
  //quad_geometry->create();

  textureTest.addComponent(quad_tr);
  textureTest.addComponent(quad_geometry);

  rm->createEntity(&textureTest);
}

void UserMain::run(float delta_time)
{
  //for (size_t i = 0; i < k_entitiesNumber; i++) {
  //  Transform* tr = massiveTest[i].getComponent<Transform>(kComponentType_Transform);
  //  tr->rotateX(accumTime);
  //  tr->rotateZ(accumTime);
  //}



  accumTime += delta_time;
}

void UserMain::clear()
{

}

