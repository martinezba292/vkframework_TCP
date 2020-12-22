#include "user_main.h"
#include "vertex_buffer.h"
#include "resource_manager.h"

void UserMain::init()
{
  VertexBuffer bufferTest;
  glm::vec3 quad[] = { {-0.2f, -0.3f, 0.0f},
                       {-0.1f, -0.3f, 0.0f},
                       {-0.1f, 0.3f, 0.0f},
                       {-0.2f, 0.3f, 0.0f}
  };

  uint32 quad_indices[] = { 0, 1, 2, 2, 3, 0 };

  bufferTest.loadPoints(quad, 4);
  bufferTest.loadIndices(quad_indices, 6);
  ResourceManager::Get()->createVertexBuffer(&bufferTest);

  glm::vec3 triangle[] = { {0.0f, -0.3f, 0.0f},
                           {0.4f, 0.0f, 0.0f},
                           {0.0f, 0.3f, 0.0f}
  };

  uint32 triangle_indices[] = { 0, 1, 2 };

  bufferTest.loadPoints(triangle, 3);
  bufferTest.loadIndices(triangle_indices, 3);
  ResourceManager::Get()->createVertexBuffer(&bufferTest);
}

void UserMain::run()
{

}

void UserMain::clear()
{

}

