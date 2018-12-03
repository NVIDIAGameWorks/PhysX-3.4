# NVIDIA PhysX SDK 3.4

## Introduction

Welcome to NVIDIA's PhysX and APEX SDK source code repository. This depot includes the PhysX SDK, the APEX SDK, and the Kapla Demo application.

NOTE: The APEX SDK is not needed to build either the PhysX SDK nor the demo and has been deprecated. It is provided for continued support of existing applications only. We recommend the following libraries as replacements:

For APEX Clothing: NvCloth - https://github.com/NVIDIAGameWorks/NvCloth

For APEX Destruction: Blast - https://github.com/NVIDIAGameWorks/Blast

For APEX Particles: Flex - https://github.com/NVIDIAGameWorks/FleX

For APEX Turbulence: Flow - https://github.com/NVIDIAGameWorks/Flow

## Documentation

Online Documentation:

http://gameworksdocs.nvidia.com/simulation.html

The documentation can also be found in the repository under
* \PhysX_3.4\Documentation
* \APEX_1.4\docs

Please also see the following readme files:
* \PhysX_3.4\readme_*.html,
* \APEX_1.4\readme.txt.

## Instructions

To begin, clone this repository onto your local drive.

To build PhysX and APEX SDKs:

(1) Build PhysX SDK by opening one of the solutions found under PhysX_3.4\Source\compiler.
Supported platforms: Windows, Linux, OSX, Android, iOS.

(2) The APEX SDK distribution contains pre-built binaries supporting GPU acceleration.
Re-building the APEX SDK removes support for GPU acceleration. The solutions can be found under APEX_1.4\compiler.
Supported platforms: Windows, Linux, Android.
______________________________________________________________
To build PhysX Snippets: open one of the solutions found under \PhysX_3.4\Snippets\compiler.

To build PhysX Samples (windows only): open one of the solutions found under \PhysX_3.4\Samples\compiler.

To build APEX Snippets: open one of the solutions found under \APEX_1.4\snippets\compiler.

To build APEX Samples: open one of the solutions found under \APEX_1.4\samples_v2\compiler.

To build and run the Kapla Demo (KaplaDemo\samples\compiler) please make sure to build the PhysX SDK with the same Visual Studio version and with the same build configuration **before** compiling the Kapla Demo. This will make sure the appropriate DLLs are copied to the KaplaDemo bin directory.

## Acknowledgements

This depot contains external third party open source software copyright their respective owners:

Alexander Chemeris,
The Android Open Source Project,
Brian Paul,
Brodie Thiesfield,
Electronic Arts,
Emil Mikulic,
FreeImage,
Hewlett-Packard Company,
Independent JPEG Group,
John W. Ratcliff,
Julio Jerez,
Kevin Bray,
The Khronos Group Inc.,
Kitware, Inc.,
Mark J. Kilgard,
Microsoft Corporation,
Open Dynamics Framework Group,
Python Software Foundation,
Univ. of Western Ontario,
and the ASSIMP, clang and glew development teams.
