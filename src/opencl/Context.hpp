#ifndef OPENCL_CONTEXT_H_
#define OPENCL_CONTEXT_H_

#include <vector> // TODO remove vector import
#include <iostream>  // TODO remove iostream import ?
#include "CL/opencl.h"
#include "Kernel.hpp"

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
  cl_device_type type;
  char name[MAX_INFO_STRING_LEN];
  cl_ulong global_mem_size;
  cl_uint address_bits;
  size_t max_work_group_size;
  size_t work_items_for_dims[3];
  cl_bool image_support;
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
  Kernel* create_kernel(char const *file_path,
                        char const *cmp_opt=nullptr, char const *main_f="main");
  cl_event read_buffer(MemoryHandler*, size_t offset, size_t size, void *dst,
                       bool block, cl_event* es=nullptr, int event_count=0);
  cl_event write_buffer(MemoryHandler*, size_t offset, size_t size, void *src,
                      bool block, cl_event* es=nullptr, int event_count=0);
  // info
  void display_opencl_info();

  // get&set
  bool was_initialized(){ return initialized; }
  DeviceInfo device(){ return _device; }
  PlatformInfo platform(){ return _platform; }
  cl_command_queue* command_queue(){ return &_clcommand_queue; }

private:
  void _cleanup();
  void platform_info(cl_platform_id platform_id, PlatformInfo& platform_info, std::vector<DeviceInfo>* devices=nullptr);
  void device_info(cl_device_id, DeviceInfo&);

private:
  bool initialized;
  int argc;
  char **argv;

  cl_device_id _cldevice;
  cl_context _clcontext;
  cl_command_queue _clcommand_queue;

  DeviceInfo _device;
  PlatformInfo _platform;

  Kernel _kernels[MAX_KERNEL_COUNT];
  size_t _kernel_count;
  MemoryHandler _allocations[MAX_ALLOCATIONS_COUNT];
  size_t _allocation_count;
};
}

std::ostream& operator<< (std::ostream&, const opencl::PlatformInfo&);
std::ostream& operator<< (std::ostream&, const opencl::DeviceInfo&);

#endif // OPENCL_CONTEXT_H_
