clang++ -fsyntax-only -Xclang -load -Xclang build/libmisracpp.so -Xclang -plugin -Xclang print-fns cudnn_v7.h > gen.cpp
cat hook_template.cpp gen.cpp > hook.cpp 

