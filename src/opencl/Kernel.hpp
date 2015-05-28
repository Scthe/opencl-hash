#ifndef __KERNEL_H
#define __KERNEL_H

#include "CL/opencl.h"

namespace opencl {

// forward declaration
class Context;

class Kernel {
 public:
  void init(Context *, cl_kernel, cl_program, size_t max_work_group_size);
  void cleanup();

  void push_arg(size_t arg_size, const void *);

  /**
   * run the kernel
   * @param  work_dim           [description]
   * @param  global_work_size   [description]
   * @param  local_work_size    [may be NULL]
   * @param  es                 [description]
   * @param  event_count        [description]
   * @return                    [description]
   */
  cl_event execute(cl_uint work_dim,                //
                   const size_t *global_work_size,  //
                   const size_t *local_work_size,   //
                   cl_event *events_to_wait_for = nullptr, int event_count = 0);

 private:
  /**
   * https://www.khronos.org/registry/cl/sdk/1.0/docs/man/xhtml/clEnqueueNDRangeKernel.html
   * @return                  if work parameters fulfill the constraints
   */
  void check_work_parameters(cl_uint work_dim,  //
                             const size_t *global_work_size,
                             const size_t *local_work_size);

 private:
  cl_kernel kernel_id;
  cl_program program_id;
  Context *context;
  size_t max_work_group_size;
  int arg_stack_size;
  bool initialized = false;
};

//
}

#endif /* __KERNEL_H   */
