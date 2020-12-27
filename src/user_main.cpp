#include "user_main.h"
#include "vertex_buffer.h"
#include "resource_manager.h"
#include "entity.h"
#include "Components/transform.h"
#include "Components/geometry.h"

Entity cubeEntity;
Entity triangleEntity;
float accumTime;

void UserMain::init()
{
  accumTime = 0.0f;
  ResourceManager* rm = ResourceManager::Get();
  Transform* myTr = new Transform();
  myTr->setPosition(1.0f, 1.0f, -1.0f);
  //myTr->setScale(2.0f, 1.0f, 1.0f);
  cubeEntity.addComponent(myTr);

  Transform* tr = cubeEntity.getComponent<Transform>(kComponentType_Transform);

  Vertex cube[] = { 
    ///FRONT
    VertexBuffer::VertexInitializer({-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}),
    VertexBuffer::VertexInitializer({ 0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}),
    VertexBuffer::VertexInitializer({ 0.5f,  0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}),
    VertexBuffer::VertexInitializer({-0.5f,  0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}),
    
    ///REAR
    VertexBuffer::VertexInitializer({-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}),
    VertexBuffer::VertexInitializer({ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}),
    VertexBuffer::VertexInitializer({ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}),
    VertexBuffer::VertexInitializer({-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}),
    
    ///RIGHT
    VertexBuffer::VertexInitializer({ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}),
    VertexBuffer::VertexInitializer({ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}),
    VertexBuffer::VertexInitializer({ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}),
    VertexBuffer::VertexInitializer({ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}),
    
    ///LEFT
    VertexBuffer::VertexInitializer({ -0.5f, -0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}),
    VertexBuffer::VertexInitializer({ -0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}),
    VertexBuffer::VertexInitializer({ -0.5f,  0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}),
    VertexBuffer::VertexInitializer({ -0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}),
    
    ///DOWN
    VertexBuffer::VertexInitializer({ -0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}),
    VertexBuffer::VertexInitializer({ -0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}),
    VertexBuffer::VertexInitializer({  0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}),
    VertexBuffer::VertexInitializer({  0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}),
    
    ///UP
    VertexBuffer::VertexInitializer({ -0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}),
    VertexBuffer::VertexInitializer({ -0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}),
    VertexBuffer::VertexInitializer({  0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}),
    VertexBuffer::VertexInitializer({  0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f})
  };

  uint32 cube_indices[] = { 0,  1,  2,  2,  3,  0, 
                            4,  6,  5,  4,  7,  6,
                            8,  9,  10, 8,  11, 9, 
                            12, 13, 14, 12, 14, 15,
                            16, 19, 17, 19, 18, 17,
                            23, 20, 21, 21, 22, 23 };

  Geometry* quad_geometry = new Geometry();
  quad_geometry->loadGeometry(cube, 24, cube_indices, 36);
  quad_geometry->create();

  cubeEntity.addComponent(quad_geometry);

  rm->createEntity(&cubeEntity);


  Transform* triangleTr = new Transform();
  triangleTr->setPosition(-1.0f, 0.0f, 0.0f);
  triangleEntity.addComponent(triangleTr);

  Vertex triangle[] = { VertexBuffer::VertexInitializer({-0.5f,0.5f, 0.0f}),
                        VertexBuffer::VertexInitializer({0.0f, -0.5f, 0.0f}),
                        VertexBuffer::VertexInitializer({0.5f, 0.5f, 0.0f})
  };

  uint32 triangle_indices[] = { 0, 1, 2 };
  Geometry* triangle_geometry = new Geometry();
  triangle_geometry->loadGeometry(triangle, 3, triangle_indices, 3);
  triangle_geometry->create();

  triangleEntity.addComponent(triangle_geometry);

  rm->createEntity(&triangleEntity);
}

void UserMain::run(float delta_time)
{
  Transform* quadTr = cubeEntity.getComponent<Transform>(kComponentType_Transform);
  Transform* triangleTr = triangleEntity.getComponent<Transform>(kComponentType_Transform);

  quadTr->rotateY(accumTime);
  quadTr->rotateX(accumTime);
  triangleTr->rotateZ(accumTime);
  accumTime += delta_time;
}

void UserMain::clear()
{

}

