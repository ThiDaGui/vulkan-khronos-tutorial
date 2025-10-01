struct VertexInput {
    float2 position;
    float3 color;
};

struct MVPUniformBuffer {
    float4x4 model;
    float4x4 view;
    float4x4 proj;
};
ConstantBuffer<MVPUniformBuffer> mvp;

struct VertexOutput {
    float3 color;
    float4 sv_position : SV_Position;
};

[shader("vertex")]
VertexOutput vertMain(VertexInput in) {
    VertexOutput output;
    output.sv_position = mul(mvp.proj, mul(mvp.view, mul(mvp.model, float4(in.position, 0.0, 1.0))));
    output.color = in.color;
    return output;
}

[shader("fragment")]
float4 fragMain(VertexOutput inVert) : SV_Target
{
    float3 color = inVert.color;
    return float4(color, 1.0);
}