#include "common.hpp"

#include <cassert>
#include <iostream>
#include <filesystem>
#include <variant>

// SDL_Surface *LoadImage(const Context *Context, const char *imageFilename,
//                        int desiredChannels) {
//   char fullPath[256];
//   // ReSharper disable once CppJoinDeclarationAndAssignment
//   SDL_Surface *result;
//   SDL_PixelFormat format;
//
//   SDL_snprintf(fullPath, sizeof(fullPath), "%sassets/%s.msl", Context->BasePath,
//                imageFilename);
//
//   result = SDL_LoadBMP(fullPath);
//   if (result == nullptr) {
//     std::cerr << "Failed to load BMP: \n" << SDL_GetError();
//     return nullptr;
//   }
//
//   if (desiredChannels == 4) {
//     format = SDL_PIXELFORMAT_ABGR8888;
//   } else {
//     SDL_assert(!"Unexpected desiredChannels");
//     // NOLINT(*-avoid-do-while, *-else-after-return)
//     SDL_DestroySurface(result);
//     return nullptr;
//   }
//   if (result->format != format) {
//     SDL_Surface *next = SDL_ConvertSurface(result, format);
//     SDL_DestroySurface(result);
//     result = next;
//   }
//
//   return result;
// }

// Vector Math

float VecMagnitude(Vector v) {
  switch (v.index()) {
  case VEC_2: {
    auto [v1, v2] = std::get<Vector2>(v);
    return sqrt(pow(v1, 2) + pow(v2, 2));
  }
  case VEC_3: {
    auto [v1, v2, v3] = std::get<Vector3>(v);
    return sqrt(pow(v1, 2) + pow(v2, 2) + pow(v3, 2));
  }
  case VEC_4: {
    auto [v1, v2, v3, v4] = std::get<Vector4>(v);
    return sqrt(pow(v1, 2) + pow(v2, 2) + pow(v3, 2) + pow(v4, 2));
  }
  default: {
    throw std::runtime_error(
        std::format("Unexpected Vector Variant. Cannot get magnitude."));
  }
  }
}

Vector VecNormalize(Vector v) {
  const auto mag = VecMagnitude(v);
  switch (v.index()) {
  case VEC_2: {
    const auto [v1, v2] = std::get<Vector2>(v);
    return Vector2{v1 / mag, v2 / mag};
  }
  case VEC_3: {
    const auto [v1, v2, v3] = std::get<Vector3>(v);
    return Vector3{v1 / mag, v2 / mag, v3 / mag};
  }
  case VEC_4: {
    const auto [v1, v2, v3, v4] = std::get<Vector4>(v);
    return Vector4{v1 / mag, v2 / mag, v3 / mag, v4 / mag};
  }
  default: {
    throw std::runtime_error(
        std::format("Unexpected Vector Variant. Cannot normalize."));
  }
  }
}

Vector VecScalarMultiply(const float s, Vector v) {
  switch (v.index()) {
  case VEC_2: {
    const auto [v1, v2] = std::get<Vector2>(v);
    return Vector2{v1 * s, v2 * s};
  }
  case VEC_3: {
    const auto [v1, v2, v3] = std::get<Vector3>(v);
    return Vector3{v1 * s, v2 * s, v3 * s};
  }
  case VEC_4: {
    const auto [v1, v2, v3, v4] = std::get<Vector4>(v);
    return Vector4{v1 * s, v2 * s, v3 * s, v4 * s};
  }
  default: {
    throw std::runtime_error(
        std::format("Unexpected Vector Variant. Cannot multiply."));
  }
  }
}

Vector VecAdd(Vector a, Vector b) {
  assert(a.index() == b.index());
  switch (a.index()) {
  case VEC_2: {
    const auto [a1, a2] = std::get<Vector2>(a);
    const auto [b1, b2] = std::get<Vector2>(b);
    return Vector2{a1 + b1, a2 + b2};
  }
  case VEC_3: {
    const auto [a1, a2, a3] = std::get<Vector3>(a);
    const auto [b1, b2, b3] = std::get<Vector3>(b);
    return Vector3{a1 + b1, a2 + b2, a3 + b3};
  }
  case VEC_4: {
    const auto [a1, a2, a3, a4] = std::get<Vector4>(a);
    const auto [b1, b2, b3, b4] = std::get<Vector4>(b);
    return Vector4{a1 + b1, a2 + b2, a3 + b3, a4 + b4};
  }
  default: {
    throw std::runtime_error(
        std::format("Unexpected Vector Variant. Cannot add."));
  }
  }
}

float VecDotProduct(Vector a, Vector b) {
  assert(a.index() == b.index());
  switch (a.index()) {
  case VEC_2: {
    const auto [a1, a2] = std::get<Vector2>(a);
    const auto [b1, b2] = std::get<Vector2>(b);
    return (a1 * b1) + (a2 * b2);
  }
  case VEC_3: {
    const auto [a1, a2, a3] = std::get<Vector3>(a);
    const auto [b1, b2, b3] = std::get<Vector3>(b);
    return (a1 * b1) + (a2 * b2) + (a3 * b3);
  }
  case VEC_4: {
    const auto [a1, a2, a3, a4] = std::get<Vector4>(a);
    const auto [b1, b2, b3, b4] = std::get<Vector4>(b);
    return (a1 * b1) + (a2 * b2) + (a3 * b3) + (a4 * b4);
  }
  default: {
    throw std::runtime_error(
        std::format("Unexpected Vector Variant. Cannot compute dot product."));
  }
  }
}

float VecScalarProjection(Vector a, Vector b) {
  assert(a.index() == b.index());
  return VecDotProduct(a, b) / VecMagnitude(b);
}

Vector VecVectorProjection(Vector a, Vector b) {
  return VecScalarMultiply(VecScalarProjection(a, b), VecNormalize(b));
}

Vector3 VectorCrossProduct(Vector3 a, Vector3 b) {
  return Vector3{.v1 = a.v2 * b.v3 - a.v3 * b.v2,
                 .v2 = -(a.v1 * b.v3 - a.v3 * b.v1),
                 .v3 = a.v1 * b.v2 - a.v2 - b.v1,};
}

// Matrix Math

Matrix MatAdd(Matrix a, Matrix b) {
  assert(a.index() == b.index());
  switch (a.index()) {
  case MAT_2x2: {
    const auto [a11, a12, a21, a22] = std::get<Matrix2x2>(a);
    const auto [b11, b12, b21, b22] = std::get<Matrix2x2>(b);
    return Matrix2x2{.m11 = a11 + b11, .m12 = a12 + b12, .m21 = a21 + b21,
                     .m22 = a22 + b22};
  }
  case MAT_3x3: {
    const auto [a11, a12, a13, a21, a22, a23, a31, a32, a33] = std::get<
      Matrix3x3>(a);
    const auto [b11, b12, b13, b21, b22, b23, b31, b32, b33] = std::get<
      Matrix3x3>(b);
    return Matrix3x3{.m11 = a11 + b11, .m12 = a12 + b12, .m13 = a13 + b13,
                     .m21 = a21 + b21, .m22 = a22 + b22, .m23 = a23 + b23,
                     .m31 = a31 + b31, .m32 = a32 + b32, .m33 = a33 + b33};
  }
  case MAT_4x4: {
    const auto [a11, a12, a13, a14, a21, a22, a23, a24, a31, a32, a33, a34, a41,
      a42, a43, a44] = std::get<Matrix4x4>(a);
    const auto [b11, b12, b13, b14, b21, b22, b23, b24, b31, b32, b33, b34, b41,
      b42, b43, b44] = std::get<Matrix4x4>(b);
    return Matrix4x4{.m11 = a11 + b11, .m12 = a12 + b12, .m13 = a13 + b13,
                     .m14 = a14 + b14, .m21 = a21 + b21, .m22 = a22 + b22,
                     .m23 = a23 + b23, .m24 = a24 + b24, .m31 = a31 + b31,
                     .m32 = a32 + b32, .m33 = a33 + b33, .m34 = a34 + b34,
                     .m41 = a41 + b41, .m42 = a42 + b42, .m43 = a43 + b43,
                     .m44 = a44 + b44};
  }
  default: {
    throw std::runtime_error(
        std::format("Unexpected Matrix Variant. Cannot add."));
  }
  }
}

Matrix MatScalarMultiply(float s, Matrix m) {
  switch (m.index()) {
  case MAT_2x2: {
    const auto [m11, m12, m21, m22] = std::get<Matrix2x2>(m);
    return Matrix2x2{.m11 = (s * m11), .m12 = (s * m12), .m21 = (s * m21),
                     .m22 = (s * m22)};
  }
  case MAT_3x3: {
    const auto [m11, m12, m13, m21, m22, m23, m31, m32, m33] = std::get<
      Matrix3x3>(m);
    return Matrix3x3{.m11 = (s * m11), .m12 = (s * m12), .m13 = (s * m13),
                     .m21 = (s * m21), .m22 = (s * m22), .m23 = (s * m23),
                     .m31 = (s * m31), .m32 = (s * m32), .m33 = (s * m33),};
  }
  case MAT_4x4: {
    const auto [m11, m12, m13, m14, m21, m22, m23, m24, m31, m32, m33, m34, m41,
      m42, m43, m44] = std::get<Matrix4x4>(m);
    return Matrix4x4{.m11 = (s * m11), .m12 = (s * m12), .m13 = (s * m13),
                     .m14 = (s * m14), .m21 = (s * m21), .m22 = (s * m22),
                     .m23 = (s * m23), .m24 = (s * m24), .m31 = (s * m31),
                     .m32 = (s * m32), .m33 = (s * m33), .m34 = (s * m34),
                     .m41 = (s * m41), .m42 = (s * m42), .m43 = (s * m43),
                     .m44 = (s * m44),};
  }
  default: {
    throw std::runtime_error(
        std::format("Unexpected Matrix Variant. Cannot multiply."));
  }
  }
}

Matrix MatMultiply(Matrix a, Matrix b) {
  assert(a.index() == b.index());
  switch (a.index()) {
  case MAT_2x2: {
    const auto [a11, a12, a21, a22] = std::get<Matrix2x2>(a);
    const auto [b11, b12, b21, b22] = std::get<Matrix2x2>(b);
    return Matrix2x2{.m11 = (a11 * b11) + (a12 * b21),
                     .m12 = (a11 * b12) + (a12 * b22),
                     .m21 = (a21 * b11) + (a22 * b21),
                     .m22 = (a21 * b12) + (a22 * b22),};
  }
  case MAT_3x3: {
    const auto [a11, a12, a13, a21, a22, a23, a31, a32, a33] = std::get<
      Matrix3x3>(a);
    const auto [b11, b12, b13, b21, b22, b23, b31, b32, b33] = std::get<
      Matrix3x3>(b);
    return Matrix3x3{.m11 = (a11 * b11) + (a12 * b21) + (a13 * b31),
                     .m12 = (a11 * b12) + (a12 * b22) + (a13 * b32),
                     .m13 = (a11 * b13) + (a12 * b23) + (a13 * b33),
                     .m21 = (a21 * b11) + (a22 * b21) + (a23 * b31),
                     .m22 = (a21 * b12) + (a22 * b22) + (a23 * b32),
                     .m23 = (a21 * b13) + (a22 * b23) + (a23 * b33),
                     .m31 = (a31 * b11) + (a32 * b21) + (a33 * b31),
                     .m32 = (a31 * b12) + (a32 * b22) + (a33 * b32),
                     .m33 = (a31 * b13) + (a32 * b23) + (a33 * b33),};
  }
  case MAT_4x4: {
    const auto [a11, a12, a13, a14, a21, a22, a23, a24, a31, a32, a33, a34, a41,
      a42, a43, a44] = std::get<Matrix4x4>(a);
    const auto [b11, b12, b13, b14, b21, b22, b23, b24, b31, b32, b33, b34, b41,
      b42, b43, b44] = std::get<Matrix4x4>(b);
    return Matrix4x4{
        .m11 = (a11 * b11) + (a12 * b21) + (a13 * b31) + (a14 * b41),
        .m12 = (a11 * b12) + (a12 * b22) + (a13 * b32) + (a14 * b42),
        .m13 = (a11 * b13) + (a12 * b23) + (a13 * b33) + (a14 * b43),
        .m14 = (a11 * b14) + (a12 * b24) + (a13 * b34) + (a14 * b44),
        .m21 = (a21 * b11) + (a22 * b21) + (a23 * b31) + (a24 * b41),
        .m22 = (a21 * b12) + (a22 * b22) + (a23 * b32) + (a24 * b42),
        .m23 = (a21 * b13) + (a22 * b23) + (a23 * b33) + (a24 * b43),
        .m24 = (a21 * b14) + (a22 * b24) + (a23 * b34) + (a24 * b44),
        .m31 = (a31 * b11) + (a32 * b21) + (a33 * b31) + (a34 * b41),
        .m32 = (a31 * b12) + (a32 * b22) + (a33 * b32) + (a34 * b42),
        .m33 = (a31 * b13) + (a32 * b23) + (a33 * b33) + (a34 * b43),
        .m34 = (a31 * b14) + (a32 * b24) + (a33 * b34) + (a34 * b44),
        .m41 = (a41 * b11) + (a42 * b21) + (a43 * b31) + (a44 * b41),
        .m42 = (a41 * b12) + (a42 * b22) + (a43 * b32) + (a44 * b42),
        .m43 = (a41 * b13) + (a42 * b23) + (a43 * b33) + (a44 * b43),
        .m44 = (a41 * b14) + (a42 * b24) + (a43 * b34) + (a44 * b44),};
  }
  default: {
    throw std::runtime_error(
        std::format("Unexpected Matrix Variant. Cannot multiply."));
  }
  }
}

Matrix MatTranspose(Matrix m) {
  switch (m.index()) {
  case MAT_2x2: {
    const auto [m11, m12, m21, m22] = std::get<Matrix2x2>(m);
    return Matrix2x2{.m11 = m11, .m12 = m21,
                        .m21 = m12, .m22 = m22};
  }
  case MAT_3x3: {
    const auto [m11, m12, m13, m21, m22, m23, m31, m32, m33] = std::get<
      Matrix3x3>(m);
    return Matrix3x3{.m11 = m11, .m12 = m21, .m13 = m31,
                        .m21 = m12, .m22 = m22, .m23 = m32,
                        .m31 = m13, .m32 = m23, .m33 = m33};
  }
  case MAT_4x4: {
    const auto [m11, m12, m13, m14, m21, m22, m23, m24, m31, m32, m33, m34, m41,
      m42, m43, m44] = std::get<Matrix4x4>(m);
    return Matrix4x4{.m11 = m11, .m12 = m21, .m13 = m31, .m14 = m41,
                        .m21 = m12, .m22 = m22, .m23 = m32, .m24 = m42,
                        .m31 = m13, .m32 = m23, .m33 = m33, .m34 = m43,
                        .m41 = m14, .m42 = m24, .m43 = m34, .m44 = m44};
  }
  default: {
    throw std::runtime_error(
        std::format("Unexpected Matrix Variant. Cannot transpose."));
  }
  }
}

Matrix MatSquare(Matrix m) {
  return MatMultiply(m, m);
}

float MatDeterminant(Matrix m) {
  switch (m.index()) {
  case MAT_2x2: {
    const auto [m11, m12, m21, m22] = std::get<Matrix2x2>(m);
    return (m11 * m22) - (m12 * m21);
  }
  case MAT_3x3: {
    const auto [m11, m12, m13, m21, m22, m23, m31, m32, m33] = std::get<
      Matrix3x3>(m);
    return (m11 * (m22 * m33 - m23 * m32)) -
           (m12 * (m21 * m33 - m23 * m31)) +
           (m13 * (m21 * m32 - m22 * m31));
  }
  case MAT_4x4: {
    const auto [m11, m12, m13, m14, m21, m22, m23, m24, m31, m32, m33, m34, m41,
      m42, m43, m44] = std::get<Matrix4x4>(m);
    return (m11 * (m22 * (m33 * m44 - m34 * m43)) -
                  (m23 * (m32 * m44 - m34 * m42)) +
                  (m24 * (m32 * m43 - m33 * m42))) -
           (m12 * (m21 * (m33 * m44 - m34 * m43)) -
                  (m23 * (m31 * m44 - m34 * m41)) +
                  (m24 * (m31 * m43 - m33 * m41))) +
           (m13 * (m21 * (m32 * m44 - m34 * m42)) -
                  (m22 * (m31 * m44 - m34 * m41)) +
                  (m24 * (m31 * m42 - m32 * m41))) -
           (m14 * (m21 * (m32 * m43 - m33 * m42)) -
                  (m22 * (m31 * m43 - m33 * m41)) +
                  (m23 * (m31 * m42 - m32 * m41)));
  }
  default: {
    throw std::runtime_error(
        std::format("Unexpected Matrix Variant. Cannot compute determinant."));
  }
  }
}

// Quaternion Math

Quaternion QuatMultiply(Quaternion a, Quaternion b) {
  auto [a1, a2, a3, a4] = a;
  auto [b1, b2, b3, b4] = b;
  return Quaternion {
    .q1 = (a1 * b1) - (a2 * b2) - (a3 * b3) - (a4 * b4),
    .q2 = (a1 * b2) + (a2 * b1) - (a3 * b4) + (a4 * b3),
    .q3 = (a1 * b3) + (a2 * b4) + (a3 * b1) - (a4 * b2),
    .q4 = (a1 * b4) - (a2 * b3) + (a3 * b2) + (a4 * b1),
  };
}

Quaternion QuatInvert(Quaternion q) {
  auto [q1, q2, q3, q4] = q;
  return Quaternion {
    .q1 = q1,
    .q2 = -q2,
    .q3 = -q3,
    .q4 = -q4
  };
}

// Useful Transformation Math

Vector4 MutiplyMatrixByVector(Matrix4x4 m, Vector4 v) {
  const auto [m11, m12, m13, m14,
              m21, m22, m23, m24,
              m31, m32, m33, m34,
              m41, m42, m43, m44] = m;
  const auto [v1, v2, v3, v4] = v;
  return Vector4 {
    .v1 = m11 * v1 + m12 * v2 + m13 * v3 + m14 * v4,
    .v2 = m21 * v1 + m22 * v2 + m23 * v3 + m24 * v4,
    .v3 = m31 * v1 + m32 * v2 + m33 * v3 + m34 * v4,
    .v4 = m41 * v1 + m42 * v2 + m43 * v3 + m44 * v4,
  };
}

Vector4 RotateVectorByQuaternion(Quaternion q, Vector4 v) {
  auto [v1, v2, v3, v4] = v;
  const auto p = Quaternion {0, v1, v2, v3};
  const auto res = QuatMultiply(QuatMultiply(QuatInvert(q), p), q);
  return Vector4 {res.q2, res.q3, res.q4, v4};
};

// Create Transform Matrices / Quaternions

Matrix4x4 CreateTranslationMatrix(float dx, float dy, float dz) {
  return Matrix4x4{
    1, 0, 0, dx,
    0, 1, 0, dy,
    0, 0, 1, dz,
    0, 0, 0, 1};
}

Matrix4x4 CreateScalingMatrix(float scale_x, float scale_y, float scale_z) {
  return Matrix4x4{
    scale_x, 0, 0, 0,
    0, scale_y, 0, 0,
    0, 0, scale_z, 0,
    0, 0, 0, 1};
}

Quaternion CreateRotationQuaternion(Vector4 axis, float theta) {
  auto [xhat, yhat, zhat, w] = axis;
  return Quaternion {
    .q1 = cos(theta/2),
    .q2 = xhat * sin(theta/2),
    .q3 = yhat * sin(theta/2),
    .q4 = zhat * sin(theta/2)
  };
};
