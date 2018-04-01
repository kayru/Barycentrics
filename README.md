# Barycentrics

This demo several approaches for computing barycentric coordinates in the pixel shader.


## Mode 1: Non-indexed geometry

Geometry is rendered using non-indexed draws. Vertex shader explicitly loads indices and vertices from index and vertex buffers and writes out barycentric coordinates. This approach is similar to using a geometry shader that outputs per-vertex barycentrics.

This approach results in geometry throughput ~2x slower than regular indexed rendering.

## Mode 2: Geometry shader

Geometry shader is used to output new triangles with explicit per-vertex barycentric coordinates. This approach does not require custom vertex fetching (unlike mode 1).

Performance is slightly worse than mode 1 on AMD Fury X, but better on NVIDIA 1080.
In general, we are still looking at ~2x slower rendering in geometry-bound scenes.

## Mode 3: Manual ray-triangle intersection in pixel shader

Primitive indices and vertices are loaded in the pixel shader based on primitive ID.
Positions are transformed into world space and resulting triangle is intersected with the eye ray to calculate barycentrics.

Despite doing quite a lot of work per pixel, this mode is much faster than modes 1 and 2.
On NVIDIA 1080 this runs ~25% slower than baseline "speed-of-light" shader that simply outpts texture coordinates in a geometry-bound scene. Interestingly, even with no geometry visible on screen (camera facing away, but still shading all vertices) performance is ~13% slower than "speed-of-light". It appears that simply using gl_PrimitiveID incurs an overhead.

## Mode 4: Passthrough geometry shader (NVIDIA)

This mode uses `VK_NV_geometry_shader_passthrough` extension. Fast / passthrough geometry shader is used to output world positions of triangles to the pixel shader, which then performs a ray-triangle intersection similar to mode 3.

Performance is slightly better than mode 3, averaging ~15% slowdown compared to "speed-of-light". With no geometry in view, performance matches the baseline (no primitive ID overhead, unlike mode 3).

## Mode 5: Native barycentrics (AMD) [TODO]

This mode uses `VK_AMD_shader_explicit_vertex_parameter` extension. This approach is described in [GPUOpen blog post](https://gpuopen.com/stable-barycentric-coordinates).
