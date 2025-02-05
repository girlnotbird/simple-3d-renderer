struct VertexData
{
  float3 Position : Position;
};

struct Interpolator
{
  float4 Color : TEXCOORD0;
  float4 Position : SV_Position;
};

Interpolator main(VertexData input)
{
  Interpolator output;
  output.Position = float4(input.Position.x/2.0f, input.Position.y/2.0f, input.Position.z/2.0f, 1.0f);
  output.Color = float4(1.0f, 1.0f, 1.0f, 1.0f);
  return output;
}