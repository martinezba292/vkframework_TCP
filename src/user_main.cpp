#include "user_main.h"
#include "vertex_buffer.h"
#include "resource_manager.h"
#include "entity.h"
#include "Components/transform.h"
#include "Components/geometry.h"

Entity quadEntity;
float accumTime;

void UserMain::init()
{
  accumTime = 0.0f;
  ResourceManager* rm = ResourceManager::Get();
  Transform* myTr = new Transform();
  myTr->setPosition(1.0f, 1.0f, -1.0f);
  myTr->setScale(1.0f, 4.0f, 1.0f);
  quadEntity.addComponent(myTr);

  Transform* tr = quadEntity.getComponent<Transform>(kComponentType_Transform);

  glm::vec3 quad[] = { {-0.1f, -0.1f, 0.0f},
                       { 0.1f, -0.1f, 0.0f},
                       { 0.1f,  0.1f, 0.0f},
                       {-0.1f,  0.1f, 0.0f}
  };

  uint32 quad_indices[] = { 0, 1, 2, 2, 3, 0 };

  Geometry* quad_geometry = new Geometry();
  quad_geometry->loadGeometry(quad, 4, quad_indices, 6);
  quad_geometry->create();

  quadEntity.addComponent(quad_geometry);

  rm->createEntity(&quadEntity);


  Transform* triangleTr = new Transform();
  triangleTr->setPosition(0.0f, 0.0f, 0.0f);
  Entity triangleEntity;
  triangleEntity.addComponent(triangleTr);

  glm::vec3 triangle[] = { {0.0f, -0.3f, 0.0f},
                           {0.4f, 0.0f, 0.0f},
                           {0.0f, 0.3f, 0.0f}
  };

  uint32 triangle_indices[] = { 0, 1, 2 };
  Geometry* triangle_geometry = new Geometry();
  triangle_geometry->loadGeometry(triangle, 3, triangle_indices, 3);
  triangle_geometry->create();

  triangleEntity.addComponent(triangle_geometry);

  rm->createEntity(&triangleEntity);

  /*bufferTest.loadVertices(triangle, 3);
  bufferTest.loadIndices(triangle_indices, 3);
  ResourceManager::Get()->createVertexBuffer(&bufferTest);*/
}

void UserMain::run(float delta_time)
{
  Transform* quadTr = quadEntity.getComponent<Transform>(kComponentType_Transform);

  quadTr->rotateY(accumTime);
  quadTr->rotateZ(accumTime);
  accumTime += delta_time;
}

void UserMain::clear()
{

}

