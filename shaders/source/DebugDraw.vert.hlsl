struct VertexData
{
  float3 Position : TEXCOORD0;
  float4 Color : TEXCOORD1;
};

struct Interpolator
{
  float4 Color : TEXCOORD0;
  float4 Position : SV_Position;
};

Interpolator main(VertexData input)
{
  Interpolator output;
  output.Position = float4(input.Position, 1.0f);
  output.Color = input.Color;
  return output;
}