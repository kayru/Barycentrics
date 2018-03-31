# Barycentrics

This demo implements a cross-platform approach for computing barycentric coordinates in the pixel shader without using geometry shaders or vendor extensions.

Geometry is rendered using non-indexed draws. Vertex shader explicitly loads indices and vertices from index and vertex buffers and writes out barycentric coordinates. This approach is similar to using a geometry shader that outputs per-vertex barycentrics.

This is likely to be much slower than using a vendor extension, since post transform cache is not utilized due to non-indexed drawing. It serves as a proof-of-concept / experiment only.
