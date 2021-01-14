#ifndef __DRAW_CMD__
#define __DRAW_CMD__ 1

#include "common_def.h"
#include "vulkan/vulkan.h"

struct DrawCallData {
  int32 geometry;
  int32 materialType;
  int32 offset;
};

class DrawCmd {
public:
  DrawCmd(){}
  ~DrawCmd(){}
  DrawCmd(const DrawCmd&) {}
  void Execute(VkCommandBuffer cmd_buffer, DrawCallData draw_call, uint32 index, int64_t buffer_padding);

};

#endif // __DRAW_CMD__
