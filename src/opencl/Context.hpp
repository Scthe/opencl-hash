#ifndef OPENCL_CONTEXT_H_
#define OPENCL_CONTEXT_H_

#include "CL/opencl.h"
#include <vector> // TODO remove vector import

#define MAX_KERNEL_COUNT 32
#define MAX_ALLOCATIONS_COUNT 32
#define MAX_INFO_STRING_LEN 256

namespace opencl {

class Context;

struct PlatformInfo{
  char name[MAX_INFO_STRING_LEN];
  char vendor[MAX_INFO_STRING_LEN];
  char version[MAX_INFO_STRING_LEN];
};

struct DeviceInfo{
  cl_ulong global_mem_size;
  cl_bool image_support;
  size_t max_work_group_size;
  cl_device_type type;
  char name[MAX_INFO_STRING_LEN];
};

struct KernelHandler{
    KernelHandler();
    void push_arg(size_t arg_size, const void *);

    cl_kernel kernel_id;
    cl_program program_id;
    int arg_stack_size = 0;
    Context* context;
};

struct MemoryHandler{
  MemoryHandler();
  void release();

  cl_mem handle;

  private:
    bool released;
};

class Context {

public:
  Context(int argc, char **argv);
  ~Context();
  void init(); // TODO int clDeviceType = CL_DEVICE_TYPE_GPU, int deviceNumber = -1
  void check_error(bool, char const *);
  void check_error(cl_int, char const *);

  // execution
  MemoryHandler* allocate(cl_mem_flags, size_t, void *);
  KernelHandler* create_kernel(char const * file_path, char const *main_f="main");
  cl_event execute_kernel(KernelHandler*, cl_event* es=nullptr, int event_count=0);
  cl_event read_buffer(MemoryHandler*, size_t offset, size_t size, void *dst,
                       bool block, cl_event* es=nullptr, int event_count=0);
  // info
  void display_opencl_info();

private:
  void _cleanup();
  void platform_info(cl_platform_id platform_id, PlatformInfo& platform_info, std::vector<DeviceInfo>& devices);
  DeviceInfo device_info(cl_device_id);

private:
  bool initialized;
  int argc;
  char **argv;

  cl_device_id _cldevice;
  cl_context _clcontext;
  cl_command_queue _clcommand_queue;
  KernelHandler _kernels[MAX_KERNEL_COUNT];
  size_t _kernel_count;
  MemoryHandler _allocations[MAX_ALLOCATIONS_COUNT];
  size_t _allocation_count;
};
}

#endif // OPENCL_CONTEXT_H_
