project('misracpp', 'cpp')

cxx = meson.get_compiler('cpp')

dep_llvm = dependency('llvm')
llvm_libdir = dep_llvm.get_configtool_variable('libdir')
dep_libs = [
    cxx.find_library('clang-cpp', dirs: llvm_libdir),
    dep_llvm,
]

library('misracpp', 'misracpp.cpp', dependencies: dep_libs)
