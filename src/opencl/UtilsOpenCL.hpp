#ifndef __UTILS_OPENCL_H
#define __UTILS_OPENCL_H

#include "CL/opencl.h"

namespace opencl {

class Kernel;

namespace utils {

/**
 * cl_device_type is a number so we will change it to string
 */
extern char const* device_type_str[9];

/**
 * Loads a Program file and prepends the cPreamble to the code.
 * @param  cFilename     program filename
 * @param  cPreamble     code that is prepended to the loaded file, typically \
a set of #defines or a header
 * @param  szFinalLength returned length of the code string
 * @return               the source string if succeeded, 0 otherwise
 */
char* load_file(const char* cFilename, const char* cPreamble,
                size_t* szFinalLength);

/**
 * Due too different possible resolutions we may have to recalculate this each
 * time.
 * Implementation note: we assume that device's address_bits can hold the
 * number of range w*h. For example if address_bits==32 then we would need
 * image bigger then 2^16 in width and height for this condition to fail.
 * There is appropriate check in Kernel class.
 *
 * @param global_work_size float array of size 2
 * @param local_work_size float array of size 2
 */
void work_sizes(const opencl::Kernel&, size_t* global_work_size,
                size_t* local_work_size, size_t w, size_t h);

/**
 * convert error code to string
 *
 * @param  cl_int :cl_int, error code
 * @return        :string
 */
const char* get_opencl_error_str(cl_int);
}
}

#endif /* __UTILS_OPENCL_H   */
