#ifndef OPENCL_CONTEXT_H_
#define OPENCL_CONTEXT_H_

#include "CL/opencl.h"

#include <vector> // TODO remove this


namespace opencl {

// 1D var for # of work items in the work group
// size_t szLocalWorkSize;

// 1D var for Total # of work items
//( rounded up to the nearest multiple of the LocalWorkSize)
// size_t szGlobalWorkSize;

struct PlatformInfo{
  char name[1024];
  char vendor[1024];
  char version[1024];
};

struct DeviceInfo{
  cl_ulong global_mem_size;
  cl_bool image_support;
  size_t max_work_group_size;
  cl_device_type type;
  char name[1024];
};

typedef cl_kernel KernelHandler;

class Context {

public:
  Context(int argc, char **argv);
  ~Context();
  void display_opencl_info();
  void init();
  KernelHandler create_kernel(char const *);

private:
  void platform_info(cl_platform_id platform_id, PlatformInfo& platform_info, std::vector<DeviceInfo>& devices);
  DeviceInfo device_info(cl_device_id);
  void check_error(cl_int, char const *);

private:
  int argc = 0;
  char **argv = nullptr;

  cl_device_id _cldevice;           // OpenCL device
  cl_context _clcontext;         // OpenCL context
  cl_command_queue _clcommand_queue; // OpenCL command queue
};
}

#endif // OPENCL_CONTEXT_H_
