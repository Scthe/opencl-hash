#include <iostream>
#include <time.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "opencl\Context.hpp"

const char *cSourceFile = "src/kernel/open_cl_kernel.cl";
typedef unsigned long long ull;

// [C][C][C][_][_][_][_][G][G]
//
// C - iterate on CPU
// _ - based on problem's global id
// G - iterate on GPU
//
// each box represents letter position, there are 16 letters possible
// we have 4^9 combinations (4 bits/letter)

// crash:
// https://devtalk.nvidia.com/default/topic/471020/driver-crashs-while-opencl-app-is-running/

#define STRINGIFY2(X) #X
#define STRINGIFY(X) STRINGIFY2(X)

#define LETTER_COUNT 9
ull target_hash = 910897038977002;

int main(int argc, char **argv) {
  size_t letters_from_global_id = 4,
         letters_to_iter_over_in_kernel = 2;

  size_t global_work_size = 1 << (letters_from_global_id * 4), // 2^16
         local_work_size = 256; // TODO 1024

  // opencl context
  opencl::Context context(argc, argv);
  context.init();

  // memory allocation - both CPU & GPU
  char cpu_buf[LETTER_COUNT+1] = {0,0,0, 0,0,0, 0,0,0, 0};
  auto gpu_buf = context.allocate(CL_MEM_WRITE_ONLY, sizeof(cl_char) * LETTER_COUNT, nullptr);
  context.write_buffer(gpu_buf, 0, sizeof(cl_char) * LETTER_COUNT, cpu_buf, true);
  // cl_int flag_init = 0;
  // auto gpu_flag = context.allocate(CL_MEM_READ_WRITE, sizeof(cl_int), (void *)&flag_init);
  std::cout << "cpu/gpu buffers pair allocated" << std::endl;

  // kernel
  std::cout << "-D LETTER_COUNT=" STRINGIFY(LETTER_COUNT) << std::endl;
  auto kernel = context.create_kernel(cSourceFile, "-D LETTER_COUNT=" STRINGIFY(LETTER_COUNT));

  // calculate cpu iterations
  auto letters_to_iter_over_on_cpu = LETTER_COUNT - letters_from_global_id - letters_to_iter_over_in_kernel;
  ull repeatCnt = 1 << (letters_to_iter_over_on_cpu * 4);
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


    // clear flag ?
    // kernel args
    kernel->push_arg(sizeof(cl_mem), (void *)&gpu_buf->handle);
    // kernel->push_arg(sizeof(cl_mem), (void *)&gpu_flag->handle);
    kernel->push_arg(sizeof(cl_long), (void *)&target_hash);
    kernel->push_arg(sizeof(cl_int), (void *)&letters_from_global_id);
    kernel->push_arg(sizeof(cl_int), (void *)&i);
    kernel->push_arg(sizeof(cl_int), (void *)&letters_to_iter_over_in_kernel);

    // Launch kernel
    cl_event finish_token = kernel->execute(1, &global_work_size, &local_work_size);

    // Synchronous/blocking read of results
    context.read_buffer(gpu_buf, 0, sizeof(cl_char) * 9, (void *)cpu_buf, true, &finish_token, 1);
    // int flag = 0; // TODO use flag for proper thread sync
    // context.read_buffer(gpu_buf, 0, sizeof(cl_int), (void *)&flag, true, &finish_token, 1);

    // done
    // if(flag){
    char* result_buffer = (char *)cpu_buf;
    if(result_buffer[0]){
      result_buffer[LETTER_COUNT] = '\0';
      std::cout << std::endl << "found: '" << result_buffer << "'" << std::endl;
      break;
    }
    // }
  }

  exit(EXIT_SUCCESS);
}
