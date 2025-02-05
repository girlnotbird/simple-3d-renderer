cbuffer VS_Camera_Data : register( b0, space1 ) {
    float4x4 View_Matrix : packoffset( c0 );
    float4x4 Proj_Matrix : packoffset( c4 );
};

cbuffer VS_Model_Data : register( b1, space1 ) {
    float4x4 Model_Matrix : packoffset( c0 );
};

struct VS_Input {
    float3 Position : TEXCOORD0;
};

struct VS_Output {
    float4 Position : SV_Position;
    float4 Color : TEXCOORD0;
};

VS_Output main(VS_Input input) {
    VS_Output output;
    float4 affine_position = float4(input.Position.x, input.Position.y, input.Position.z, 0.0f);
    output.Position = mul(Model_Matrix, affine_position);
    output.Position = mul(View_Matrix, affine_position);
    output.Position = mul(Proj_Matrix, affine_position);
    output.Color = float4(1.0f, 1.0f, 1.0f, 1.0f);

    return output;
}