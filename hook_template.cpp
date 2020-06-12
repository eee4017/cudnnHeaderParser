#include <stdio.h>
#include <cuda.h>
#include <cudnn.h>
#include <dlfcn.h>
#include <string.h>

#include "fromCudnn.h"
#include "fromCuda.h"
#include "prefetch.h"
#include "global.h"

void *libcudnnHandle;
void *libcudaHandle;
void *libcudartHandle;
void *libdlHandle;

extern "C" {
  void *__libc_dlsym(void *map, const char *name);
  void *__libc_dlopen_mode(const char *name, int mode);
}

__attribute__((constructor))
void libcudnnInit(void) {
  libcudnnHandle = __libc_dlopen_mode("libcudnn.so", RTLD_LAZY);
  libcudaHandle = __libc_dlopen_mode("libcuda.so", RTLD_LAZY);
  libcudartHandle = __libc_dlopen_mode("libcudart.so", RTLD_LAZY);
  libdlHandle = __libc_dlopen_mode("libdl.so", RTLD_LAZY);
}

void *actualDlsym(void *handle, const char *symbol) {
  typedef decltype(&dlsym) funcType;
  funcType func = (funcType) __libc_dlsym(libdlHandle, "dlsym");

  void *ret = (*func)(handle, symbol);
  if (!ret) {
    PRINT("Error: Cannot load %s\n", symbol);
  }
  return ret;
}

#define STRINGIFY(x) #x

void *dlsym(void *handle, const char *symbol) {
  if (strcmp(symbol, STRINGIFY(cuLaunchKernel)) == 0) {
    PRINT("dlsym() hook %s\n", STRINGIFY(cuLaunchKernel));
    return (void*)(&cuLaunchKernel);
  } else if (strcmp(symbol, STRINGIFY(cuMemAlloc)) == 0) {
    PRINT("dlsym() hook %s\n", STRINGIFY(cuMemAlloc));
    return (void*)(&cuMemAlloc);
  }

  PRINT("dlsym() pass %s\n", symbol);
  return actualDlsym(handle, symbol);
}

/* CUDA Runtime API part */

cudaError_t CUDAAPI cudaMallocManaged(void** devPtr, 
                                      size_t size, 
                                      unsigned int flags) {
  if (size == 0) {
    return cudaSuccess;
  }

  PRINT("Enter cudaMallocManaged(), size = %llu\n", (unsigned long long int) size);

  cudaError_t (*func)(void**, size_t, unsigned int);
  func = (cudaError_t(*)(void**, size_t, unsigned int)) actualDlsym(libcudartHandle, STRINGIFY(cudaMallocManaged));

  cudaError_t ret = func(devPtr,
                         size,
                         flags);

  if (ret != cudaSuccess) {
    PRINT("cudaMallocManaged() returns %d\n", (int) ret);
  } else {
    PRINT("cudaMallocManaged(), devPtr = %llx, size = %llu\n", (unsigned long long int) *devPtr, (unsigned long long int) size);
  }

  return ret;
}

cudaError_t CUDAAPI cudaMalloc(void** devPtr, 
                               size_t size) {
  return cudaMallocManaged(devPtr, size, cudaMemAttachGlobal);
}

/* CUDA Driver API part */
