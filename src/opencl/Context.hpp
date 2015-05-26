#ifndef OPENCL_CONTEXT_H_
#define OPENCL_CONTEXT_H_

#include "CL/opencl.h"
#include <vector>


namespace opencl {

class Context;

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

// TODO add const
struct KernelHandler{
    KernelHandler(Context*, cl_kernel, cl_program);
    ~KernelHandler();
    void push_arg(size_t arg_size, const void *);
    cl_kernel* kernel(){ return &kernel_id;}

  private:
    Context* context;
    cl_kernel kernel_id;
    cl_program program_id;
  public:
    int arg_stack_size = 0;
};

class Context {

public:
  Context(int argc, char **argv);
  ~Context();
  void init();

  void display_opencl_info();
  KernelHandler* create_kernel(char const *);
  cl_context* raw_context(){return &_clcontext;}
  void check_error(cl_int, char const *);
  cl_event execute_kernel(KernelHandler*, cl_event* es=nullptr, int event_count=0);
  cl_event read_buffer(cl_mem, size_t offset, size_t size, void *dst,
                       bool block, cl_event* es=nullptr, int event_count=0);

private:
  void _cleanup();
  void platform_info(cl_platform_id platform_id, PlatformInfo& platform_info, std::vector<DeviceInfo>& devices);
  DeviceInfo device_info(cl_device_id);

private:
  int argc = 0;
  char **argv = nullptr;

  cl_device_id _cldevice;           // OpenCL device
  cl_context _clcontext;         // OpenCL context
  cl_command_queue _clcommand_queue; // OpenCL command queue
  std::vector<KernelHandler> _kernels;
};
}

#endif // OPENCL_CONTEXT_H_
