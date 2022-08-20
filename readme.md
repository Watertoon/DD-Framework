# DD Framework
My public repository for the DD framework.

# Current Features
* A Vulkan 1.3 wrapper library to simplify using Vulkan while taking advantage of graphics pipeline libraries, dynamic rendering, buffer device addresses, and descriptor indexing
* A simd math library
* Many utility data structures
* Basic keyboard and mouse support
* Basic window support
* Basic file support
* A test application based on learnopengl.com

# Pending integration
* A user mode scheduler for Win32 Fibers to create a cooperative threading environment
* A CPU memory manager based on a couple of heap structures
* Hash algorithms, Murmur64, Murmur32, CRC32 using pclmul, SHA256
* Graphics pipeline library system

# Future Features (in active development, or pending integration)
* A parameter system based on the BYAML file format
* A job queue system for graphics, actors, and modules
* An environment built on the Vulkan wrapper library for managing 3d models and graphics techniques

# Project Stipulations
* I wish to limit the usage of external dependencies when possible. I currently rely on Win32 and Vulkan 1.3. I plan to integrate cURL and OpenSSL in the future.
* The project should be simple to build beyond setting up MSYS2, installing the relevant MSYS2 packages, and running make.
* The project's Vulkan implementation must conform to all the limits and features defined in vk::Context.
* The project curently requires an x86_64 CPU with AVX2, PCLMUL, RDTSC, and POPCNT.

# Footnote
Built for Windows with GCC 12.1 and gnu make from MSYS2. To build install MSYS2. From MSYS2 install the standard GCC package. Run the newly installed Make program from the command line. Run the relevant project exes.

Only the code and resources I've personally made are subject to copyright under GPLv2 and are marked as such. Third party content is found within "third_party" folders and are to be respected as they are licensed.

I'm informed mainly by learnopengl.com, reverse engineering, and documentation. I am not an expert on liscensing.

If there are any problems of any kind please file an issue or contact me and I'll do my best to resolve it. At the moment this project is not intended for commercial use and might never be.
