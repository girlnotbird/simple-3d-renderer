struct FragBufOut {
    float4 Color : SV_Target0;
    float Depth: SV_Depth;
};

FragBufOut main(float4 Color : TEXCOORD0, float4 Position : SV_Position) {
    FragBufOut output;
    output.Color = Color;
    output.Depth = Position.w;

    return output;
}
