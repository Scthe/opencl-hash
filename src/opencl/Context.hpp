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

/**
 * base information about platform
 */
struct PlatformInfo{
  char name[MAX_INFO_STRING_LEN];
  char vendor[MAX_INFO_STRING_LEN];
  char version[MAX_INFO_STRING_LEN];
};

/**
 * base information about device
 */
struct DeviceInfo{
  cl_device_type type;
  char name[MAX_INFO_STRING_LEN];
  cl_ulong global_mem_size;
  cl_uint address_bits;
  size_t max_work_group_size;
  size_t work_items_for_dims[3];
  cl_bool image_support;
};

/**
 * opencl memory handle
 */
struct MemoryHandler{
  MemoryHandler();
  void release();

  cl_mem handle;
  size_t size = 0;

  private:
    bool released;
};


/**
 * Base class for interaction with opencl.
 * Remember to call init!
 */
class Context {

public:
  Context(int argc, char **argv);
  ~Context();
  void init(); // TODO int clDeviceType = CL_DEVICE_TYPE_GPU, int deviceNumber = -1
  void check_error(bool, char const *);
  void check_error(cl_int, char const *);

  //
  // execution
  //

  void block();

  /**
   * Allocate memory on opencl device
   * https://www.khronos.org/registry/cl/sdk/1.1/docs/man/xhtml/clCreateBuffer.html
   *
   * @param  flags    opencl flags
   * @param  size     bytest to allocate. Use f.e. sizeof(cl_char) * COUNT
   * @return          handler used by context
   */
  MemoryHandler* allocate(cl_mem_flags, size_t);

  /**
   * Create kernel from file
   *
   * @param  file_path path to .cl file that contains source code
   * @param  cmp_opt   [OPT] compilation options f.e. macros
   * @param  main_f    [OPT] name of main kernel function
   * @return           kernel object
   */
  Kernel* create_kernel(char const *file_path,
                        char const *cmp_opt=nullptr, char const *main_f="main");

  /**
   * Read buffer from opencl device and copy it to host memory
   *
   * @param  gpu_buffer               source buffer
   * @param  offset                   buffer offset
   * @param  size                     how much to read
   * @param  dst                      destination buffer
   * @param  block                    blocking/nonblocking operation switch
   * @param  events_to_wait_for       [OPT]wait for other operations to finish
   * @param  events_to_wait_for_count [OPT]
   * @return                          opencl event object
   */
  cl_event read_buffer(MemoryHandler*, size_t offset, size_t size, void *dst,
                       bool block, cl_event* es=nullptr, int event_count=0);

  /**
   * Read buffer from opencl device and copy it to host memory
   *
   * @param  gpu_buffer               source buffer
   * @param  dst                      destination buffer
   * @param  block                    blocking/nonblocking operation switch
   * @param  events_to_wait_for       [OPT]wait for other operations to finish
   * @param  events_to_wait_for_count [OPT]
   * @return                          opencl event object
   */
  cl_event read_buffer(MemoryHandler*, void *dst, bool block,
                       cl_event* es=nullptr, int event_count=0);

 /**
  * Copy data from host memory to opencl device
  *
  * @param  gpu_buffer               destination buffer
  * @param  offset                   buffer offset
  * @param  size                     how much to read
  * @param  src                      source buffer
  * @param  block                    blocking/nonblocking operation switch
  * @param  events_to_wait_for       [OPT]wait for other operations to finish
  * @param  events_to_wait_for_count [OPT]
  * @return                          opencl event object
  */
  cl_event write_buffer(MemoryHandler*, size_t offset, size_t size, void *src,
                      bool block, cl_event* es=nullptr, int event_count=0);

  /**
   * Copy data from host memory to opencl device
   *
   * @param  gpu_buffer               destination buffer
   * @param  src                      source buffer
   * @param  block                    blocking/nonblocking operation switch
   * @param  events_to_wait_for       [OPT]wait for other operations to finish
   * @param  events_to_wait_for_count [OPT]
   * @return                          opencl event object
   */
  cl_event write_buffer(MemoryHandler*, void *src, bool block,
                        cl_event* es=nullptr, int event_count=0);

  /**
   * Fill with zero values
   *
   * @param  gpu_buffer               destination buffer
   * @param  block                    blocking/nonblocking operation switch
   * @param  events_to_wait_for       [OPT]wait for other operations to finish
   * @param  events_to_wait_for_count [OPT]
   * @return                          opencl event object
   */
  cl_event zeros_float(MemoryHandler*, bool block,
                        cl_event* es=nullptr, int event_count=0);


  //
  // info
  //

  /**
   * print to stdout platform and devices information
   */
  void display_opencl_info();

  //
  // get&set
  //

  /**
   * was context initialized ? AKA can I use any cmds beside info gathering ?
   */
  bool was_initialized(){ return initialized; }

  /**
   * device that this context is bound to
   */
  DeviceInfo device(){ return _device; }

  /**
   * platform that this context is bound to
   */
  PlatformInfo platform(){ return _platform; }


  /**
   * command queue. This may be called leaky abstraction, but it's not like
   * we don't expose more advanced stuff (f.e. max_work_group_size).
   * Also You probably will not have any use of raw command_queue.
   */
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
