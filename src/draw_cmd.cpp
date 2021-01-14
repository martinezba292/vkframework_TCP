#include "draw_cmd.h"
#include "resource_manager.h"
#include "dev/internal.h"


void DrawCmd::Execute(VkCommandBuffer cmd_buffer, DrawCallData draw_call, uint32 index, int64_t buffer_padding)
{
  Resources* intResources = ResourceManager::Get()->getResources();
  InternalMaterial* internalMat = &intResources->internalMaterials[draw_call.materialType];
  VkDeviceSize offsets[] = { 0 };
  VkBuffer vertexBuffers[] = { intResources->vertexBuffer.buffer_ };

  vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, internalMat->matPipeline);
  vkCmdBindVertexBuffers(cmd_buffer, 0, 1, vertexBuffers, offsets);
  vkCmdBindIndexBuffer(cmd_buffer, intResources->indicesBuffer.buffer_, 0, VK_INDEX_TYPE_UINT32);
  uint32 offset = draw_call.offset * static_cast<uint32>(buffer_padding);
  vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
    intResources->layouts[internalMat->layout].pipeline, 0, 1,
    &internalMat->matDescriptorSet[index], 1, &offset);

  InternalVertexData vertex_data = intResources->vertex_data[draw_call.geometry];
  uint32 first_vertex = vertex_data.offset;
  uint32 first_index = vertex_data.index_offset;
  vkCmdDrawIndexed(cmd_buffer, static_cast<uint32>(vertex_data.indices.size()), 1, first_index, first_vertex, 0);
}
