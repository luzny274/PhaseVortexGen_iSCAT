SET LLVM_PATH=C:\Program Files\LLVM\bin

set my_libs=-lkernel32 -luser32 -lgdi32 -lshell32 -lcomctl32 

set my_path="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.27.29110\lib\x64"

"%LLVM_PATH%\clang++" main.cpp -o convert.exe -m64


pause