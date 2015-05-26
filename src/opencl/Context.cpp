#include "Context.hpp"

#include <iostream>
#include <stdexcept>

#include "utils.h"

char const* device_type_str[] = {
  "-",
  "default", // 1
  "CPU", // 2
  "-",
  "GPU", // 4
  "-", "-", "-",
  "Accelerator", // 8
};

namespace opencl {

//
// KernelHandler
//

KernelHandler::KernelHandler()
    : kernel_id(nullptr),
      program_id(nullptr),
      arg_stack_size(0),
      context(nullptr){
}

void KernelHandler::push_arg(size_t arg_size, const void *arg_value){
  cl_int ciErr1 = clSetKernelArg(kernel_id, arg_stack_size, arg_size, arg_value);
  context->check_error(ciErr1, "Could not push kernel argument");
  ++arg_stack_size;
}


//
// MemoryHandler
//

MemoryHandler::MemoryHandler()
    : handle(nullptr),
      released(false){
}

void MemoryHandler::release(){
  if(!released && handle)
    clReleaseMemObject(handle);
  released = true;
}

//
// Context
//

// init/core functions

Context::Context(int argc, char **argv)
    : initialized(false),
      argc(argc),
      argv(argv),
      _kernel_count(0),
      _allocation_count(0){
}

Context::~Context() {
  this->_cleanup();
}

void Context::init() {
  // TODO ad ability to select platform & device
  cl_int ciErr1;

  // Get an OpenCL platform
  cl_platform_id platform_id;
  ciErr1 = clGetPlatformIDs(1, &platform_id, nullptr);
  check_error(ciErr1, "Error in clGetPlatformID");

  // Get the devices
  ciErr1 = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &_cldevice, nullptr);
  check_error(ciErr1, "Error in clGetDeviceIDs");

  // Create the context
  _clcontext = clCreateContext(0, 1, &_cldevice, nullptr, nullptr, &ciErr1);
  check_error(ciErr1, "Error in clCreateContext");

  // Create a command-queue
  _clcommand_queue = clCreateCommandQueue(_clcontext, _cldevice, 0, &ciErr1);
  check_error(ciErr1, "Error in clCreateCommandQueue");

  initialized = true;
}

void Context::_cleanup(){
  // kernels
  for(size_t i = 0; i < _kernel_count; i++){
    KernelHandler* k = _kernels + i;
    if (k->kernel_id)
      clReleaseKernel(k->kernel_id);
    if (k->program_id)
      clReleaseProgram(k->program_id);
  }

  // memory
  for(size_t i = 0; i < _allocation_count; i++){
    MemoryHandler* k = _allocations + i;
    k->release();
  }

  // other
  if (_clcommand_queue)
    clReleaseCommandQueue(_clcommand_queue);
  if (_clcontext)
    clReleaseContext(_clcontext);
}

void Context::check_error(cl_int errCode, char const *msg) {
  // std::cout << "CHECK: " << errCode << ": " << msg << std::endl;
  if (errCode != CL_SUCCESS) {
    std::cout << msg << "; status: " << errCode << std::endl;
    this->_cleanup(); // TODO does not clean up gpu buffers
    throw std::runtime_error("opencl error"); // TODO better error msg
  }
}


// execution

MemoryHandler* Context::allocate(cl_mem_flags flags, size_t size, void * host_ptr){
  check_error(_allocation_count < MAX_ALLOCATIONS_COUNT ? CL_SUCCESS : CL_INVALID_PROGRAM,
    "Wrapper hit allocations limit, increase MAX_ALLOCATIONS_COUNT");

  cl_int ciErr1;
  MemoryHandler* k = _allocations + _allocation_count;
  ++_allocation_count;
  k->handle = clCreateBuffer(_clcontext, flags, size, host_ptr, &ciErr1);
  check_error(ciErr1, "Error in clCreateBuffer");
  return k;
}

KernelHandler* Context::create_kernel(char const *file_path, char const *main_f){
  cl_int ciErr1;
  std::cout << "Reading kernel function from '" << file_path << "'" << std::endl;

  // Read the OpenCL kernel from source file
  size_t kernel_len = 0;
  char* kernel_source = load_file(file_path, "", &kernel_len);
  std::cout << "Kernel length: " << kernel_len << std::endl;
  check_error(kernel_len > 0 ? CL_SUCCESS : CL_INVALID_PROGRAM, "Could not read file");

  check_error(_kernel_count < MAX_KERNEL_COUNT ? CL_SUCCESS : CL_INVALID_PROGRAM,
    "Wrapper hit kernel limit, increase MAX_KERNEL_COUNT");
  KernelHandler* k = _kernels + _kernel_count;
  ++_kernel_count;
  k->context = this;


  // create program
  k->program_id = clCreateProgramWithSource(_clcontext, 1, (const char **)&kernel_source, &kernel_len, &ciErr1);
  check_error(ciErr1, "Error in clCreateProgramWithSource");
  free(kernel_source); // TODO better take care of shader source free

  // build program
  ciErr1 = clBuildProgram(k->program_id, 1, &_cldevice, nullptr, nullptr, nullptr);
  if (ciErr1 == CL_BUILD_PROGRAM_FAILURE) {
    size_t length;
    char buffer[2048];
    clGetProgramBuildInfo(k->program_id, _cldevice, CL_PROGRAM_BUILD_LOG,
                          sizeof(buffer), buffer, &length);
    std::cout << "--- Build log ---" << std::endl << buffer << std::endl;
  }
  check_error(ciErr1, "Error in clBuildProgram");

  // Create the kernel
  k->kernel_id = clCreateKernel(k->program_id, main_f, &ciErr1);
  check_error(ciErr1, "Error in clCreateKernel");

  // std::cout << "kernel created(f) :" <<k->kernel_id<<":"<<k->program_id<< std::endl;
  return k;

}

cl_event Context::execute_kernel(KernelHandler* kernel,
                                 cl_event* events_to_wait_for,
                                 int events_to_wait_for_count){
   check_error(kernel->context == this ? CL_SUCCESS : CL_INVALID_PROGRAM,
    "Kernel context mismatch: tried to execute kernel from context A using context B");

  // TODO change work size
  size_t szLocalWorkSize = 256;
  size_t szGlobalWorkSize = 256 * 256;

  kernel->arg_stack_size = 0; // succes or not we have to prepare for next invoke

  cl_event finish_token;
  cl_int ciErr1 = clEnqueueNDRangeKernel(
      _clcommand_queue, kernel->kernel_id,             // what and where to execute
      1, nullptr, &szGlobalWorkSize, &szLocalWorkSize, // work dim, exec groups
      events_to_wait_for_count, events_to_wait_for, &finish_token);  // sync events
  check_error(ciErr1, "Error in clEnqueueNDRangeKernel");
  return finish_token;
}

cl_event Context::read_buffer(MemoryHandler* gpu_buffer,
                              size_t offset, size_t size, void *dst,
                              bool block,
                              cl_event* events_to_wait_for,
                              int events_to_wait_for_count){
  cl_event finish_token;
  cl_bool clblock = block? CL_TRUE : CL_FALSE;
  cl_mem gpu_memory_pointer = gpu_buffer->handle;
  cl_int ciErr1 = clEnqueueReadBuffer(
      _clcommand_queue, gpu_memory_pointer, // what and where to execute
      clblock,           // block or not
      offset, size, dst, // read params: offset, size and target
      events_to_wait_for_count, events_to_wait_for, &finish_token); // sync events
  check_error(ciErr1, "Error in clEnqueueReadBuffer");
  return finish_token;
}


// info

void Context::display_opencl_info() {
  cl_int ciErr1;

  cl_uint platform_count = 0;
  ciErr1 = clGetPlatformIDs(0, nullptr, &platform_count);
  check_error(ciErr1, "Could not get platform count");
  std::cout << "platforms:" << std::endl;

  // prepare platform ids vector
  std::vector<cl_platform_id> platform_ids;
  platform_ids.reserve(platform_count);
  for (size_t i = 0; i < platform_count; i++) {
    platform_ids.push_back(nullptr);
  }

  ciErr1 = clGetPlatformIDs(platform_count, &platform_ids[0], nullptr);
  check_error(ciErr1, "Could not get platform ids");

  PlatformInfo platform_info;
  std::vector<DeviceInfo> devices;
  for (auto i = begin(platform_ids); i != end(platform_ids); ++i) {
    devices.clear();
    this->platform_info(*i, platform_info, devices);
    std::cout << "  " << platform_info.vendor
              << "::" << platform_info.name
              << ", version " << platform_info.version << std::endl;
    std::cout << "  devices:" << std::endl;
    // devices
    for (auto j = begin(devices); j != end(devices); ++j) {
      std::cout << "     "  << device_type_str[j->type]
                << "::" << j->name
                << ", memory: " << (j->global_mem_size / 1024 / 1024) << "MB"
                << ", image support: " << (j->image_support==CL_TRUE? "YES":"NO")
                << ", max work group size: " << j->max_work_group_size << std::endl;
    }
  }

  std::cout << "found " << platform_count << " opencl platforms" << std::endl;
}

void Context::platform_info(cl_platform_id platform_id, PlatformInfo& platform_info,
                                    std::vector<DeviceInfo>& devices) {
  size_t value_size = 0;
  cl_int ciErr1;
  // get base info
  ciErr1 = clGetPlatformInfo(platform_id, CL_PLATFORM_NAME, sizeof(platform_info.name), &platform_info.name, &value_size);
  platform_info.name[value_size] = '\0';
  ciErr1 |= clGetPlatformInfo(platform_id, CL_PLATFORM_VENDOR, sizeof(platform_info.vendor), &platform_info.vendor, &value_size);
  platform_info.vendor[value_size] = '\0';
  ciErr1 |= clGetPlatformInfo(platform_id, CL_PLATFORM_VERSION, sizeof(platform_info.version), &platform_info.version, &value_size);
  platform_info.version[value_size] = '\0';
  check_error(ciErr1, "Could not get platform details");

  // get device count
  cl_uint device_count = 0;
  ciErr1 = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_ALL, 0, nullptr, &device_count);
  check_error(ciErr1, "Could not get platform devices");
  // std::cout << "  found " << device_count << " devices" << std::endl;

  // device ids
  std::vector<cl_device_id> device_ids;
  device_ids.reserve(device_count);
  for (size_t i = 0; i < device_count; i++) {
    device_ids.push_back(0);
  }

  ciErr1 = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_ALL, device_count, &device_ids[0], nullptr);
  check_error(ciErr1, "Could not get device ids");

  for (auto i = begin(device_ids); i != end(device_ids); ++i) {
    devices.push_back(this->device_info(*i));
  }
}

DeviceInfo Context::device_info(cl_device_id device_id) {
// https://www.khronos.org/registry/cl/sdk/1.0/docs/man/xhtml/clGetDeviceInfo.html
  DeviceInfo info;
  cl_int ciErr1;
  size_t value_size = 0;
  ciErr1 =  clGetDeviceInfo(device_id, CL_DEVICE_GLOBAL_MEM_SIZE, 1024, &info.global_mem_size, nullptr);
  ciErr1 |= clGetDeviceInfo(device_id, CL_DEVICE_IMAGE_SUPPORT, 1024, &info.image_support, nullptr);
  ciErr1 |= clGetDeviceInfo(device_id, CL_DEVICE_MAX_WORK_GROUP_SIZE, 1024, &info.max_work_group_size, nullptr);
  ciErr1 |= clGetDeviceInfo(device_id, CL_DEVICE_TYPE, 1024, &info.type, nullptr);
  ciErr1 |= clGetDeviceInfo(device_id, CL_DEVICE_NAME, sizeof(info.name), &info.name, &value_size);
  info.name[value_size] = '\0';
  check_error(ciErr1, "Could not get device data");
  return info;
}

}

/*
int main(int argc, char **argv) {
  opencl::Context context(argc, argv);
  context.display_opencl_info();
}
*/
