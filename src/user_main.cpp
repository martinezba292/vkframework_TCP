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
  myTr->setPosition(2.0f, 0.0f, -5.0f);
  myTr->rotateX(-2.0f);
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

  uint32 cube_indices[] = { 0,  2,  1,  2,  0,  3, 
                            4,  5,  6,  4,  6,  7,
                            8,  10,  9, 8,  9, 11, 
                            12, 14, 13, 12, 15, 14,
                            16, 17, 19, 19, 17, 18,
                            23, 21, 20, 21, 23, 22 };

  Geometry* quad_geometry = new Geometry();
  quad_geometry->loadGeometry(cube, 24, cube_indices, 36);
  quad_geometry->create();

  cubeEntity.addComponent(quad_geometry);

  rm->createEntity(&cubeEntity);


  Transform* triangleTr = new Transform();
  triangleTr->setPosition(-1.0f, 0.0f, -2.0f);
  triangleEntity.addComponent(triangleTr);

  Vertex triangle[] = { 
    VertexBuffer::VertexInitializer({0.5f, 0.5f, -2.0f}),
    VertexBuffer::VertexInitializer({0.0f, -0.5f,-2.0f}),
    VertexBuffer::VertexInitializer({-0.5f,0.5f, -2.0f})
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

