# BMR(Bi-Manipulator Robot) gRPC Example

## Name
gRPC sample project including gRPC server and client applications.

## Description
This is an sample project to demonstrate communication between BMR control application(the RPC server) and perception system(the RPC client). The message and service protocol is defined based upon Googole Protocol buffer. The "bmr_rpc_server" is the RPC server application and others are the RPC client applications.

## Build
Before building this project, gRPC should be installed in the system before with its submodules such as Google's Probobuf, Abseil, etc.

dtProto and dtCore header files and static libraries are requred to be installed on the system.

It also requires GCC version 8.5 or above and CMake 3.12 or more recent version installed.

After getting the source project from GitLab repository, run the following shell commands.

> $ cd {path to source repository}
>  
> $ cmake -S . -B build
>
> $ cmake --build build -j8
>

If you have any problem while building this project, please try to modify library path in CMakeLists.txt. Especially, check the include and library path for dtProto library.

## Support
Please contact to the project maintainer of this project by email.
Maintainer: Sangyup Yi
Email address: sean.yi@hyundai.com

## License
It is free for anyone to use inside Hyundai Motor Company.