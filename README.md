# simple-3d-renderer

Written in C++ with [SDL3](https://github.com/libsdl-org/SDL)'s GPU API and [GLM](https://github.com/g-truc/glm) for matrix &amp; vector types. Shaders are cross-compiled offline to Metal Shader Langauge (MSL) from HLSL using [SDL_Shadercross](https://github.com/libsdl-org/SDL_shadercross).

This project currently only runs on MacOS using Metal. If you want to cross-compile for your own machine, you can use SDL_Shadercross but will have to update the source code.

This is an educational project available under the Apache 2.0 License. See LICENSE.md for more details.
