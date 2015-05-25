#include "CL/opencl.h"
#include "OpenCL_utils.h"

#include <iostream>
#include <time.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace hash_open_cl{

	// const char* cSourceFile = "./open_cl_kernel.cl";
	const char* cSourceFile = "/src/kernel/open_cl_kernel.cl";
	typedef unsigned long long ull;

	// OpenCL Vars
	cl_context cxGPUContext;        // OpenCL context
	cl_command_queue cqCommandQueue;// OpenCL command queue
	cl_device_id cdDevice;          // OpenCL device
	cl_program cpProgram;           // OpenCL program
	cl_kernel ckKernel;             // OpenCL kernel
	char* cSourceCL = nullptr;      // Buffer to hold source for compilation

	// forward declaration
	void Cleanup(int argc, char **argv, int iExitCode);
	void checkError(int argc, char **argv, cl_int ciErr, char const * msg);


	// Problem variables
	void *dst;        // Host buffers for OpenCL test
	cl_mem cmDevDst;  // OpenCL device destination buffer


	// main
	int main(int argc, char **argv){
		std::cout << "start" << '\n';
		ull iter_count = pow(16, 9);

		// set and log Global and Local work size dimensions
		size_t szLocalWorkSize = 256;  // 1D var for # of work items in the work group
		// 1D var for Total # of work items
		//( rounded up to the nearest multiple of the LocalWorkSize)
		size_t szGlobalWorkSize = 256 * 256;// 16^4

		cl_int ciErr1, ciErr2;

		// Allocate and initialize host< c++> arrays
		dst = (void *)malloc(sizeof(cl_char)* 1024);

		//Get an OpenCL platform
		cl_platform_id cpPlatform;
		ciErr1 = clGetPlatformIDs(1, &cpPlatform, nullptr);
		checkError(argc, argv, ciErr1, "Error in clGetPlatformID");

		//Get the devices
		ciErr1 = clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, 1, &cdDevice, nullptr);
		checkError(argc, argv, ciErr1, "Error in clGetDeviceIDs");

		//Create the context
		cxGPUContext = clCreateContext(0, 1, &cdDevice, nullptr, nullptr, &ciErr1);
		checkError(argc, argv, ciErr1, "Error in clCreateContext");

		// Create a command-queue
		cqCommandQueue = clCreateCommandQueue(cxGPUContext, cdDevice, 0, &ciErr1);
		checkError(argc, argv, ciErr1, "Error in clCreateCommandQueue");

		// Allocate the OpenCL buffers
		// for source and result on the device GMEM
		cmDevDst = clCreateBuffer(cxGPUContext, CL_MEM_READ_WRITE, sizeof(cl_char)* 1024, nullptr, &ciErr1);
		checkError(argc, argv, ciErr1, "Error in clCreateBuffer");

		// Read the OpenCL kernel in from source file
		size_t szKernelLength;
		std::cout << "Reading kernel function from '" << cSourceFile << "'" << '\n';
		cSourceCL = oclLoadProgSource(cSourceFile, "", &szKernelLength);

		// Create & build the program
		cpProgram = clCreateProgramWithSource(cxGPUContext, 1, (const char **)&cSourceCL, &szKernelLength, &ciErr1);
		checkError(argc, argv, ciErr1, "Error in clCreateProgramWithSource");
		// http://www.khronos.org/registry/cl/sdk/2.0/docs/man/xhtml/clBuildProgram.html
		ciErr1 = clBuildProgram(cpProgram, 0, nullptr, nullptr, nullptr, nullptr);
		if (ciErr1 == CL_BUILD_PROGRAM_FAILURE){
			size_t length;
			char buffer[2048];
			clGetProgramBuildInfo(cpProgram, cdDevice, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &length);
			std::cout << "--- Build log ---\n " << buffer << '\n';
		}
		checkError(argc, argv, ciErr1, "Error in clBuildProgram");

		// Create the kernel
		//cl_kernel clCreateKernel(cl_program  program, const char *kernel_name, cl_int *errcode_ret)
		ckKernel = clCreateKernel(cpProgram, "HashKernel", &ciErr1);
		checkError(argc, argv, ciErr1, "Error in clCreateKernel");

		auto repeatCnt = iter_count / szGlobalWorkSize;
		//void* nullBuffer = malloc(sizeof(cl_char)* 1024);
		char nullBuffer[sizeof(cl_char)* 1024];
		memset(nullBuffer, 0, 1024);
		int per = 0;
		for (ull i = 0; i < repeatCnt; i++){
			if (i % (repeatCnt / 100) == 0){
				std::cout << per << " %" << '\n';
				++per;
			}

			// Set the Argument values
			//cl_int clSetKernelArg(cl_kernel kernel, cl_uint arg_index, size_t arg_size, const void *arg_value)
			ciErr1 = clSetKernelArg(ckKernel, 0, sizeof(cl_mem), (void*)&cmDevDst);
			ciErr1 |= clSetKernelArg(ckKernel, 1, sizeof(cl_int), (void*)&i);
			checkError(argc, argv, ciErr1, "Error in clSetKernelArg");

			// Enqueues a command to execute a kernel on a device.
			ciErr1 = clEnqueueWriteBuffer(cqCommandQueue,	// cl_command_queue command_queue
										  cmDevDst,	// cl_mem buffer
										  CL_FALSE,		// cl_bool blocking_write
										  0,			// size_t offset
										  sizeof(cl_char)* 1024, // size_t cb
										  memset,			// const void *ptr
										  0,			// cl_uint num_events_in_wait_list
										  nullptr,		// const cl_event *event_wait_list
										  nullptr);		// cl_event *event

			// Launch kernel
			ciErr1 = clEnqueueNDRangeKernel(cqCommandQueue,	// cl_command_queue command_queue
											ckKernel,		// cl_kernel kernel
											1,				// cl_uint work_dim
											nullptr,		// const size_t *global_work_offset
											&szGlobalWorkSize,	// const size_t *global_work_size
											&szLocalWorkSize,	// const size_t *local_work_size
											0,				// cl_uint num_events_in_wait_list
											nullptr,		// const cl_event *event_wait_list
											nullptr);		// cl_event *event
			checkError(argc, argv, ciErr1, "Error in clEnqueueNDRangeKernel");

			// Synchronous/blocking read of results, and check accumulated errors
			ciErr1 = clEnqueueReadBuffer(cqCommandQueue,// cl_command_queue command_queue
										 cmDevDst,		// cl_mem buffer
										 CL_TRUE, 		// cl_bool blocking_write
										 0,		  		// size_t offset
										 sizeof(cl_char)* 1024, // size_t cb
										 dst,		// const void *ptr
										 0, 	  	// cl_uint num_events_in_wait_list
										 nullptr, 	// const cl_event *event_wait_list
										 nullptr);	// cl_event *event
			checkError(argc, argv, ciErr1, "Error in clEnqueueReadBuffer");


			// done
			auto dst_c = (char*)dst;
			if (*dst_c == 'f'){
				std::cout << dst_c << '\n';
				break;
			}
		}

		system("pause");

		// Cleanup and leave
		Cleanup(argc, argv, EXIT_SUCCESS);

		return 0;
	}

	void Cleanup(int argc, char **argv, int iExitCode){
		// Cleanup allocated objects
		std::cout << "Starting Cleanup" << '\n';
		if (cSourceCL)free(cSourceCL);
		if (ckKernel)clReleaseKernel(ckKernel);
		if (cpProgram)clReleaseProgram(cpProgram);
		if (cqCommandQueue)clReleaseCommandQueue(cqCommandQueue);
		if (cxGPUContext)clReleaseContext(cxGPUContext);
		if (cmDevDst)clReleaseMemObject(cmDevDst);

		// Free host memory
		free(dst);

		__shrQAFinish(argc, argv, (iExitCode == EXIT_SUCCESS) ? QA_PASSED : QA_FAILED);
		exit(iExitCode);
	}

	void checkError(int argc, char **argv, cl_int ciErr, char const * msg){
		if (ciErr != CL_SUCCESS)	{
			std::cout << msg << "; status: " << ciErr << '\n';
			Cleanup(argc, argv, EXIT_FAILURE);
		}
	}
}
