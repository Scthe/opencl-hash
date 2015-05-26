#include <iostream>
#include <time.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "opencl\Context.hpp"

const char *cSourceFile = "src/kernel/open_cl_kernel.cl";
typedef unsigned long long ull;

// main
int main(int argc, char **argv) {
  std::cout << "start" << std::endl;

  ull iter_count = ((ull)1) << (4 * 9); // 16^9

  // opencl context
  opencl::Context context(argc, argv);
  context.init();

  // memory allocation - both CPU & GPU
  void* cpu_buf = (void *)malloc(sizeof(cl_char) * 1024);
  auto gpu_buf = context.allocate(CL_MEM_READ_WRITE, sizeof(cl_char) * 1024, nullptr);
  std::cout << "cpu/gpu buffers pair allocated" << std::endl;

  // kernel
  opencl::KernelHandler* kernel = context.create_kernel(cSourceFile, "HashKernel");

  size_t szGlobalWorkSize = 256*256; // TODO ??

  auto repeatCnt = iter_count / szGlobalWorkSize;
  int percent_done = 0;
  for (ull i = 0; i < repeatCnt; i++) {
    // report progress
    if (i % (repeatCnt / 100) == 0) {
      std::cout << "\r[";
      for(auto i=0; i<10;i++){
        std::cout << (percent_done > i * 10 ? "=":" ");
      }
      std::cout << "] " << percent_done << "%";
      ++percent_done;
    }

    kernel->push_arg(sizeof(cl_mem), (void *)&gpu_buf->handle);
    kernel->push_arg(sizeof(cl_int), (void *)&i);

    // Launch kernel
    //cl_event finish_token = context.execute_kernel(kernel);
    context.execute_kernel(kernel);

    // Synchronous/blocking read of results
    // context.read_buffer(cmDevDst,0,sizeof(cl_char) * 1024, dst, true,finish_token,1);
    context.read_buffer(gpu_buf, 0, sizeof(cl_char) * 1024, cpu_buf, true);

    // done
    char* result_buffer = (char *)cpu_buf;
    if (*result_buffer == 'f') {
      std::cout << std::endl << result_buffer << std::endl;
      break;
    }
  }

  free(cpu_buf);
  std::cout << "--end--" << std::endl;

  system("pause");

  exit(EXIT_SUCCESS);
}
