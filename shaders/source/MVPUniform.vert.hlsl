#pragma pack_matrix(row_major)

cbuffer UniformData : register(b0, space1) {
    float4x4 model_matrix : packoffset(c0);
    float4x4 view_matrix : packoffset(c4);
    float4x4 proj_matrix : packoffset(c8);
}

struct VS_Input {
    float3 Position : TEXCOORD0;
    float4 Color : TEXCOORD1;
    float3 Normal : TEXCOORD2;
};

struct VS_Output {
    float4 Color : TEXCOORD0;
    float4 Position : SV_Position;
};

VS_Output main(VS_Input input) {
    VS_Output output;

    output.Color = float4((input.Normal.xyz + float3(1.0f, 1.0f, 1.0f))  * 0.5f, 1.0f);

    float4 affine_position = float4(input.Position, 1.0f);
    float4x4 mvp_matrix = mul(mul(model_matrix, view_matrix), proj_matrix);
    output.Position = mul(affine_position, mvp_matrix);

    return output;
}