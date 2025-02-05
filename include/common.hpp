#ifndef TBD_COMMON_HPP
#define TBD_COMMON_HPP

#include <SDL3/SDL.h>
#include <string>

// Basic Application-Level Enums & Containers
enum ApplicationStatus : Sint8 {
    APP_SUCCESS = 0,
    APP_FAILURE = -1,
    APP_CONTINUE = 1,
};
// SDL_Surface* LoadImage(Context* Context, const char* imageFilename, int desiredChannels);

// Vector Formats
using Vector2 = struct {float v1, v2;};
using Vector3 = struct {float v1, v2, v3;};
using Vector4 = struct {float v1, v2, v3, v4;};
using Vector = std::variant<Vector2, Vector3, Vector4>;
enum VecTypes: Uint8 { // variant.index() values for type Vector
    VEC_2 = 0,
    VEC_3 = 1,
    VEC_4 = 2
};

// Matrix Formats
using Matrix2x2 = struct {
  float m11, m12;
  float m21, m22;
};
using Matrix3x3 = struct {
  float m11, m12, m13;
  float m21, m22, m23;
  float m31, m32, m33;
};
using Matrix4x4 = struct {
  float m11, m12, m13, m14;
  float m21, m22, m23, m24;
  float m31, m32, m33, m34;
  float m41, m42, m43, m44;
};
using Matrix = std::variant<Matrix2x2, Matrix3x3, Matrix4x4>;
enum MatTypes: Uint8 { // variant.index() values for type Matrix
    MAT_2x2 = 0,
    MAT_3x3 = 1,
    MAT_4x4 = 2
};

// Quaternion Format
using Quaternion = struct {float q1, q2, q3, q4;};

// Vector Math
float VecMagnitude(Vector v);
Vector VecNormalize(Vector v);
Vector VecScalarMultiply(float s, Vector v);
Vector VecAdd(Vector a, Vector b);
float VecDotProduct(Vector a, Vector b);
float VecScalarProjection(Vector a, Vector b);
Vector VecVectorProjection(Vector a, Vector b);
Vector3 VecCrossProduct(Vector3 a, Vector3 b);

// Matrix Math
Matrix MatAdd(Matrix a, Matrix b);
Matrix MatScalarMultiply(float s, Matrix m);
Matrix MatMultiply(Matrix a, Matrix b);
Matrix MatTranspose(Matrix m);
Matrix MatSquare(Matrix m);
float MatDeterminant(Matrix m);

// Quaternion Math
Quaternion QuatMultiply(Quaternion a, Quaternion b);
Quaternion QuatInvert(Quaternion q);

// Transformation Math
Vector4 MultiplyMatrixByVector(Matrix4x4 m, Vector4 v);
Vector4 MultiplyQuaternionByVector(Quaternion q, Vector4 v);
Vector4 RotateVectorByQuaternion(Quaternion q, Vector4 v);
Matrix4x4 CreateScalingMatrix(float scale_x, float scale_y, float scale_z);
Quaternion CreateRotationQuaternion(Vector4 axis, float theta);
Matrix4x4 CreateTranslationMatrix(float x, float y, float z);

#endif // TBD_COMMON_HPP