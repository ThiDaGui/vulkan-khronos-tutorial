struct VertexInput {
    float3 position;
    float3 color;
};

struct VPUniformBuffer {
    float4x4 view;
    float4x4 proj;
};

ParameterBlock<VPUniformBuffer> vp;


struct VertexToFrag {
    float4 sv_position : SV_Position;
    float3 frag_color;
};

[shader("vertex")]
VertexToFrag vertMain(uint vertexID : SV_VertexID, VertexInput *vertices) {
    VertexToFrag output;
    output.sv_position = mul(vp.proj, mul(vp.view, float4(vertices[vertexID].position, 1.0)));
    output.frag_color = vertices[vertexID].color;
    return output;
}

[shader("fragment")]
float4 fragMain(VertexToFrag inVert) : SV_Target
{
    return float4(inVert.frag_color, 1.0);
}