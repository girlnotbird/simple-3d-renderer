struct VertexData
{
  uint VertexIndex : SV_VertexID;

};

struct Interpolator
{
  float4 Color : TEXCOORD0;
  float4 Position : SV_Position;
};

Interpolator main(VertexData input)
{
  Interpolator output;
  float2 pos;
  // NOTE: 0,0 is the center of the viewport. Position coords are passed to
  //       which assigns the vertices to clip coordinates.

  if (input.VertexIndex == 0)
  {
    pos = (-1.0f).xx;
	// NOTE: Same as pos = float2(-1.0f, -1.0f). Syntax called swizzling
	// NOTE: pos = float2(-1.0f, -1.0f) is the bottom left corner.
    output.Color = float4(1.0f, 0.0f, 0.0f, 1.0f); // red
  }
  else
  {
    if (input.VertexIndex == 1)
    {
      pos = float2(1.0f, -1.0f);
	  // NOTE: pos = float2(1.0f, -1.0f) is the bottom right corner.
      output.Color = float4(0.0f, 1.0f, 0.0f, 1.0f); // green
    }
    else
    {
      if (input.VertexIndex == 2)
      {
        pos = float2(0.0f, 1.0f);
		// NOTE: pos = float2(0.0f, 1.0f) is the top center.
        output.Color = float4(0.0f, 0.0f, 1.0f, 1.0f); // blue
      }
    }
  }
  output.Position = float4(pos, 0.0f, 1.0f);
  return output;
}