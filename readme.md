# DD Framework
My public repository for the DD application framework.

# Current Features
* A cooperative M:N threading environment powered by a usermode fiber scheduler
* A custom CPU heap memory manager
* A Vulkan 1.3 wrapper library to simplify using Vulkan while taking advantage of dynamic rendering, buffer device addresses, and descriptor indexing
* A SIMD math library (Vectors, Matrices, Camera's, Projections)
* Many utility data structures (trees, intrusive lists, time, result codes)
* A custom error handling system
* A file device and resource manager
* Basic keyboard and mouse support
* Basic window support
* A 3D test application based on learnopengl.com (test_program1 not currently building due to refactor)

# Pending integration (finished/draft held back)
* Non-cryptographically secure hash algorithms (crc32, murmur32, murmur64)
* A more cryptograhically secure hash algorithm (SHA256)
* A red-black tree
* A dependency based multithreaded job queue system
* A file decompressor
* Support for multiple reverse engineered Nintendo file formats (See https://github.com/Watertoon/sarc-differ for early headers)

# Future Features (in early development)
* A parameter system based on Nintendo's BYAML file format
* An asynchronous file loader
* An environment built on the Vulkan wrapper library for managing 3d models and graphics techniques

# Project Stipulations
* I wish to limit the usage of external dependencies when possible. I currently rely on Win32 and Vulkan 1.3. I plan to integrate Zstd, cURL, and OpenSSL in the future.
* The project should be simple to build beyond setting up MSYS2, installing the relevant MSYS2 packages, and running make.
* The project's Vulkan implementation must conform to all the limits and features defined in vk::Context.
* The project curently requires an x86_64 CPU with AVX2, PCLMUL, RDTSC, and POPCNT.

# Footnote
Built for Windows with GCC 12.2 and gnu make from MSYS2. To build install MSYS2. From MSYS2 install the standard GCC package. Run the newly installed Make program from the command line. Run the relevant project exes.

Only the code and resources I've personally created are subject to copyright under GPLv2 and are marked as such. Third party content is found within "third_party" folders and are to be respected as they are licensed.

I'm informed by learnopengl.com, reverse engineering, text books, free online resources, and documentation. I am not an expert on liscensing but I believe to be conformant.

If there are any problems of any kind please file an issue or contact me and I'll do my best to resolve it.
