#include <iostream>
#include <stdio.h>
#include <strings.h>

enum eQAstatus { QA_FAILED = 0, QA_PASSED = 1, QA_WAIVED = 2 };

size_t shrRoundUp(int group_size, int global_size) {
  int r = global_size % group_size;
  if (r == 0) {
    return global_size;
  } else {
    return global_size + group_size - r;
  }
}

// Helper function to init data arrays
void shrFillArray(float *pfData, int iSize) {
  int i;
  // const float fScale = 1.0f / (float)RAND_MAX;
  for (i = 0; i < iSize; ++i) {
    // pfData[i] = fScale * rand();
    pfData[i] = (float)i;
  }
}

//////////////////////////////////////////////////////////////////////////////
//! Loads a Program file and prepends the cPreamble to the code.
//!
//! @return the source string if succeeded, 0 otherwise
//! @param cFilename        program filename
//! @param cPreamble        code that is prepended to the loaded file, typically
//a set of #defines or a header
//! @param szFinalLength    returned length of the code string
//////////////////////////////////////////////////////////////////////////////
char *oclLoadProgSource(const char *cFilename, const char *cPreamble,
                        size_t *szFinalLength) {
  // locals
  FILE *pFileStream = NULL;
  size_t szSourceLength;

// open the OpenCL source code file
#ifdef _MSC_VER // Visual studio
  if (fopen_s(&pFileStream, cFilename, "rb") != 0) {
    return NULL;
  }
#else // Linux version
  pFileStream = fopen(cFilename, "rb");
  if (pFileStream == 0) {
    return NULL;
  }
#endif

  size_t szPreambleLength = strlen(cPreamble);

  // get the length of the source code
  fseek(pFileStream, 0, SEEK_END);
  szSourceLength = ftell(pFileStream);
  fseek(pFileStream, 0, SEEK_SET);

  // allocate a buffer for the source code string and read it in
  char *cSourceString = (char *)malloc(szSourceLength + szPreambleLength + 1);
  memcpy(cSourceString, cPreamble, szPreambleLength);
  if (fread((cSourceString) + szPreambleLength, szSourceLength, 1,
            pFileStream) != 1) {
    fclose(pFileStream);
    free(cSourceString);
    return 0;
  }

  // close the file and return the total length of the combined (preamble +
  // source) string
  fclose(pFileStream);
  if (szFinalLength != 0) {
    *szFinalLength = szSourceLength + szPreambleLength;
  }
  cSourceString[szSourceLength + szPreambleLength] = '\0';

  return cSourceString;
}

inline int findExeNameStart(const char *exec_name) {
  int exename_start = (int)strlen(exec_name);

  while ((exename_start > 0) && (exec_name[exename_start] != '\\') &&
         (exec_name[exename_start] != '/')) {
    exename_start--;
  }

  if (exec_name[exename_start] == '\\' || exec_name[exename_start] == '/') {
    return exename_start + 1;
  } else {
    return exename_start;
  }
}

#ifndef STRCASECMP
#ifdef _MSC_VER
#define STRCASECMP _stricmp
#else
// for some reason it can't find strcasecmp on mingw..
int __str_ccmp(const char *s1, const char *s2) {
  const unsigned char *p1 = (const unsigned char *)s1;
  const unsigned char *p2 = (const unsigned char *)s2;
  while (toupper(*p1) == toupper(*p2)) {
    if (*p1 == '\0')
      return 0;
    ++p1;
    ++p2;
  }
  return toupper(*p2) > toupper(*p1) ? -1 : 1;
}
#define STRCASECMP __str_ccmp
#endif
#endif

inline void __shrQAFinish(int argc, char **argv, int iStatus) {
  // By default QATest is disabled and NoPrompt is Enabled (times out at seconds
  // passed into __ExitInTime() )
  bool bQATest = false, bNoPrompt = true, bQuitInTime = true;
  const char *sStatus[] = {"FAILED", "PASSED", "WAIVED", NULL};

  for (int i = 1; i < argc; i++) {
    int string_start = 0;
    while (argv[i][string_start] == '-')
      string_start++;

    const char *string_argv = &argv[i][string_start];
    if (!STRCASECMP(string_argv, "qatest")) {
      bQATest = true;
    }
    // For SDK individual samples that don't specify -noprompt or -prompt,
    // a 3 second delay will happen before exiting, giving a user time to view
    // results
    if (!STRCASECMP(string_argv, "noprompt") ||
        !STRCASECMP(string_argv, "help")) {
      bNoPrompt = true;
      bQuitInTime = false;
    }
    if (!STRCASECMP(string_argv, "prompt")) {
      bNoPrompt = false;
      bQuitInTime = false;
    }
  }

  int exename_start = findExeNameStart(argv[0]);
  if (bQATest) {
    fprintf(stdout, "&&&& %s %s", sStatus[iStatus], &(argv[0][exename_start]));
    for (int i = 1; i < argc; i++)
      fprintf(stdout, " %s", argv[i]);
    fprintf(stdout, "\n");
  } else {
    fprintf(stdout, "[%s] test results...\n%s\n", &(argv[0][exename_start]),
            sStatus[iStatus]);
  }
  fflush(stdout);
  printf("\n");
  fflush(stdout);
}
