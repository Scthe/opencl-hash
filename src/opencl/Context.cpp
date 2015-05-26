#include "Context.hpp"

#include <iostream>
#include <vector>

#include "../OpenCL_utils.h" // TODO move to opencl/utils.h

char const* device_type_str[] = {
  "-", // 0
  "default", // 1
  "CPU", // 2
  "-", // 3
  "GPU", // 4
  "-", // 5
  "-", // 6
  "-", // 7
  "Accelerator", // 8
};

namespace opencl {

// size_t szLocalWorkSize = 256;
// size_t szGlobalWorkSize = 256 * 256;

Context::Context(int argc, char **argv):argc(argc), argv(argv){}

Context::~Context() {
  // perform cleanup
}

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
  ciErr1 = clGetPlatformInfo(platform_id, CL_PLATFORM_NAME, 1024, &platform_info.name, &value_size);
  platform_info.name[value_size] = '\0';
  ciErr1 |= clGetPlatformInfo(platform_id, CL_PLATFORM_VENDOR, 1024, &platform_info.vendor, &value_size);
  platform_info.vendor[value_size] = '\0';
  ciErr1 |= clGetPlatformInfo(platform_id, CL_PLATFORM_VERSION, 1024, &platform_info.version, &value_size);
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
  ciErr1 |= clGetDeviceInfo(device_id, CL_DEVICE_NAME, 1024, &info.name, &value_size);
  info.name[value_size] = '\0';
  check_error(ciErr1, "Could not get device data");
  return info;
}

void Context::init() {
  cl_int ciErr1;

  // Get an OpenCL platform
  cl_platform_id platform_id;
  ciErr1 = clGetPlatformIDs(1, &platform_id, nullptr);
  check_error(ciErr1, "Error in clGetPlatformID");

  // Get the devices
  ciErr1 = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &_cldevice, nullptr);
  check_error(ciErr1, "Error in clGetDeviceIDs");

  // Create the context
  _clcontext = clCreateContext(0, 1, &_cldevice, nullptr, nullptr, &ciErr1); // TODO use pfn_notify
  check_error(ciErr1, "Error in clCreateContext");

  // Create a command-queue
  _clcommand_queue = clCreateCommandQueue(_clcontext, _cldevice, 0, &ciErr1);
  check_error(ciErr1, "Error in clCreateCommandQueue");
}

void Context::check_error(cl_int errCode, char const *msg) {
  if (errCode != CL_SUCCESS) {
    std::cout << msg << "; status: " << errCode << '\n';
    // Cleanup(argc, argv, EXIT_FAILURE);
  }
}

KernelHandler Context::create_kernel(char const *file_path){
  cl_int ciErr1;
  char const* main_function = "HashKernel";

  // Read the OpenCL kernel in from source file
  std::cout << "Reading kernel function from '" << file_path << "'" << '\n';
  size_t kernel_len = 0;
  char* kernel_source = oclLoadProgSource(file_path, "", &kernel_len);
  // TODO free kernel_source

  std::cout << "Kernel length: " << kernel_len << std::endl;
  check_error(kernel_len > 0 ? CL_SUCCESS : CL_INVALID_PROGRAM, "Error in clCreateProgramWithSource");

  // Create & build the program
  cl_program clprogram = clCreateProgramWithSource(_clcontext, 1, (const char **)&kernel_source, &kernel_len, &ciErr1);
  check_error(ciErr1, "Error in clCreateProgramWithSource");

  // http://www.khronos.org/registry/cl/sdk/2.0/docs/man/xhtml/clBuildProgram.html
  ciErr1 = clBuildProgram(clprogram, 1, &_cldevice, nullptr, nullptr, nullptr);
  if (ciErr1 == CL_BUILD_PROGRAM_FAILURE) {
    size_t length;
    char buffer[2048];
    clGetProgramBuildInfo(clprogram, _cldevice, CL_PROGRAM_BUILD_LOG,
                          sizeof(buffer), buffer, &length);
    std::cout << "--- Build log ---" << std::endl << buffer << std::endl;
  }
  check_error(ciErr1, "Error in clBuildProgram");

  // Create the kernel
  cl_kernel kernel = clCreateKernel(clprogram, main_function, &ciErr1);
  check_error(ciErr1, "Error in clCreateKernel");
  return kernel;

}

  //
}

int main(int argc, char **argv) {
  opencl::Context context(argc, argv);
  context.display_opencl_info();
}
