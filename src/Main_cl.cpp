#include <iostream>
#include <time.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "opencl\Context.hpp"

// const char* cSourceFile = "./open_cl_kernel.cl";
const char *cSourceFile = "src/kernel/open_cl_kernel.cl";
typedef unsigned long long ull;

// forward declaration
// void Cleanup(int argc, char **argv, int iExitCode);
// void checkError(int argc, char **argv, cl_int ciErr, char const *msg);

// Problem variables
void *dst;       // Host buffers for OpenCL test
cl_mem cmDevDst; // OpenCL device destination buffer

// main
int main(int argc, char **argv) {
  std::cout << "start" << std::endl;
  ull iter_count = ((ull)1) << (4 * 9); // 16^9

  cl_int ciErr1;
  opencl::Context context(argc, argv);
  context.init();
  cl_context* gpu_context = context.raw_context();

  // Allocate and initialize host< c++> arrays
  dst = (void *)malloc(sizeof(cl_char) * 1024);

  // Allocate the OpenCL buffers
  // for source and result on the device GMEM
  // TODO allocate through custom class for better cleanup
  cmDevDst = clCreateBuffer(*gpu_context, CL_MEM_READ_WRITE,
                            sizeof(cl_char) * 1024, nullptr, &ciErr1);
  context.check_error(ciErr1, "Error in clCreateBuffer");
  std::cout << "cpu/gpu buffers pair allocated" << std::endl;

  // kernel
  opencl::KernelHandler* kernel = context.create_kernel(cSourceFile);
  std::cout << "kernel created" << std::endl;

  size_t szGlobalWorkSize = 256*256; // TODO ??
  auto repeatCnt = iter_count / szGlobalWorkSize;
  char nullBuffer[sizeof(cl_char) * 1024];
  memset(nullBuffer, 0, 1024);
  int percent_done = 0;

/*
  for (ull i = 0; i < repeatCnt; i++) {

    // report progress
    if (i % (repeatCnt / 100) == 0) {
      std::cout << percent_done << "%" << std::endl;
      ++percent_done;
    }

    // Set the Argument values
    // cl_int clSetKernelArg(cl_kernel kernel, cl_uint arg_index, size_t
    // arg_size, const void *arg_value)
    kernel->push_arg(sizeof(cl_mem), (void *)&cmDevDst);
    kernel->push_arg(sizeof(cl_int), (void *)&i);


    // Launch kernel
    //cl_event finish_token = context.execute_kernel(kernel);
    context.execute_kernel(kernel);

    // Synchronous/blocking read of results
    // context.read_buffer(cmDevDst,0,sizeof(cl_char) * 1024, dst, true,finish_token,1);
    context.read_buffer(cmDevDst,0,sizeof(cl_char) * 1024, dst, true);

    /*
    ciErr1 = clEnqueueNDRangeKernel(
        cqCommandQueue,    // cl_command_queue command_queue
        ckKernel,          // cl_kernel kernel
        1,                 // cl_uint work_dim
        nullptr,           // const size_t *global_work_offset
        &szGlobalWorkSize, // const size_t *global_work_size
        &szLocalWorkSize,  // const size_t *local_work_size
        0,                 // cl_uint num_events_in_wait_list
        nullptr,           // const cl_event *event_wait_list
        nullptr);          // cl_event *event
    cl_context->check_error(ciErr1, ciErr1, "Error in clEnqueueNDRangeKernel");

    // Synchronous/blocking read of results, and check accumulated errors
    ciErr1 =
        clEnqueueReadBuffer(cqCommandQueue, // cl_command_queue command_queue
                            cmDevDst,       // cl_mem buffer
                            CL_TRUE,        // cl_bool blocking_write
                            0,              // size_t offset
                            sizeof(cl_char) * 1024, // size_t cb
                            dst,                    // const void *ptr
                            0,        // cl_uint num_events_in_wait_list
                            nullptr,  // const cl_event *event_wait_list
                            nullptr); // cl_event *event
    cl_context->check_error(ciErr1, ciErr1, "Error in clEnqueueReadBuffer");
    * /

    // done
    auto dst_c = (char *)dst;
    if (*dst_c == 'f') {
      std::cout << dst_c << '\n';
      break;
    }
  }
*/

  std::cout << "--end--" << std::endl;
  system("pause");

  clReleaseMemObject(cmDevDst);
  free(dst);
  // Cleanup and leave
  // Cleanup(argc, argv, EXIT_SUCCESS);

  exit(EXIT_SUCCESS);
  // return 0;
}

/*
void Cleanup(int argc, char **argv, int iExitCode) {
  // Cleanup allocated objects
  std::cout << "Starting Cleanup" << '\n';
  if (cSourceCL)
    free(cSourceCL);
  if (ckKernel)
    clReleaseKernel(ckKernel);
  if (cpProgram)
    clReleaseProgram(cpProgram);
  if (cqCommandQueue)
    clReleaseCommandQueue(cqCommandQueue);
  if (cxGPUContext)
    clReleaseContext(cxGPUContext);
  if (cmDevDst)
    clReleaseMemObject(cmDevDst);

  // Free host memory
  free(dst);

  __shrQAFinish(argc, argv,
                (iExitCode == EXIT_SUCCESS) ? QA_PASSED : QA_FAILED);
  exit(iExitCode);
}
*/
