#ifndef __UTILS_OPENCL_H
#define __UTILS_OPENCL_H

#include "CL/opencl.h"

namespace opencl {
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
 * convert error code to string
 *
 * @param  cl_int :cl_int, error code
 * @return        :string
 */
const char* get_opencl_error_str(cl_int);
}
}

#endif /* __UTILS_OPENCL_H   */
