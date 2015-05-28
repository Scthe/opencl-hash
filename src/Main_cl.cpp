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
  cl_int flag_init = 0;
  auto gpu_flag = context.allocate(CL_MEM_READ_WRITE, sizeof(cl_int), (void *)&flag_init);
  std::cout << "cpu/gpu buffers pair allocated" << std::endl;

  unsigned int positions_to_fill = 2;

  // kernel
  auto kernel = context.create_kernel(cSourceFile);

  // [C][C][C][G][G][G][G][_][_]
  //
  // each box represents letter position, there are 16 letters possible
  // we have 4^9 combinations (4 bits/letter)
  // IF we will run this on szGlobalWorkSize=16^4 threads,
  // which effectively will itearate over first 4 letter combinations
  // we will iterate over last 5 letters on GPU

  // crash:
  // https://devtalk.nvidia.com/default/topic/471020/driver-crashs-while-opencl-app-is-running/

  size_t global_work_size = 256*256, // TODO ??
         local_work_size = 256;

  // 2^36 / 2^16 / 2**8 = 2**12 = 4096
  auto repeatCnt = iter_count / global_work_size / (1 << (positions_to_fill * 4) );
  // std::cout << "repeatCnt: " << repeatCnt << std::endl;
  int percent_done = 0, repeats_per_percent = repeatCnt / 100 + 1;
  for (ull i = 0; i < repeatCnt; i++) {
    // report progress
    if (i % repeats_per_percent == 0) {
      std::cout << "\r[";
      for(auto i=0; i<10;i++){
        std::cout << (percent_done > i * 10 ? "=":" ");
      }
      std::cout << "] " << percent_done << "%";
      ++percent_done;
    }

    // kernel args
    kernel->push_arg(sizeof(cl_mem), (void *)&gpu_buf->handle);
    kernel->push_arg(sizeof(cl_mem), (void *)&gpu_flag->handle);
    kernel->push_arg(sizeof(cl_int), (void *)&i);
    kernel->push_arg(sizeof(cl_int), (void *)&positions_to_fill);

    // Launch kernel
    cl_event finish_token = kernel->execute(1, &global_work_size, &local_work_size);

    // Synchronous/blocking read of results
    context.read_buffer(gpu_buf, 0, sizeof(cl_char) * 9, cpu_buf, true, &finish_token, 1);
    // int flag = 0; // TODO use flag for proper thread sync
    // context.read_buffer(gpu_buf, 0, sizeof(cl_int), (void *)&flag, true, &finish_token, 1);

    // done
    // if(flag){
    char* result_buffer = (char *)cpu_buf;
    if(result_buffer[0] == 'a'){
      result_buffer[10] = '\0';
      std::cout << "found: '" << result_buffer << "'" << std::endl;
      break;
    }
    // }
  }

  free(cpu_buf);
  std::cout << "--end--" << std::endl;

  system("pause");

  exit(EXIT_SUCCESS);
}
