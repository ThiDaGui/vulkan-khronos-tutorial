struct VertexInput {
    float2 position;
    float3 color;
    float2 tex_coordinate;
};

struct MVPUniformBuffer {
    float4x4 model;
    float4x4 view;
    float4x4 proj;
};
ConstantBuffer<MVPUniformBuffer> mvp;

struct VertexOutput {
    float4 sv_position : SV_Position;
    float3 frag_color;
    float2 frag_tex_coordinate;
};

[shader("vertex")]
VertexOutput vertMain(VertexInput in) {
    VertexOutput output;
    output.sv_position = mul(mvp.proj, mul(mvp.view, mul(mvp.model, float4(in.position, 0.0, 1.0))));
    output.frag_color = in.color;
    output.frag_tex_coordinate = in.tex_coordinate;
    return output;
}

Sampler2D texture;

[shader("fragment")]
float4 fragMain(VertexOutput inVert) : SV_Target
{
    return texture.Sample(inVert.frag_tex_coordinate);
}