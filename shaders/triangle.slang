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
    float4x4 Rv = float4x4(
        float4(vp.view[0].xyz, 0.0),
        float4(vp.view[1].xyz, 0.0),
        float4(vp.view[2].xyz, 0.0),
        float4(0.0, 0.0, 0.0, 1.0)
    );
    float4 t = mul(transpose(Rv), vp.view._m03_m13_m23_m33);
    float4x4 Tv = float4x4(
        float4(1.0, 0.0, 0.0, t.x),
        float4(0.0, 1.0, 0.0, t.y),
        float4(0.0, 0.0, 1.0, t.z),
        float4(0.0, 0.0, 0.0, 1.0)
   );
    float4x4 transform_matrix = mul(vp.proj, mul(Rv, Tv));
    output.sv_position = mul(transform_matrix, float4(vertices[vertexID].position, 1.));// mul(vp.proj, mul(vp.view, float4(vertices[vertexID].position, 1.0)));
    output.frag_color = vertices[vertexID].color;
    return output;
}

[shader("fragment")]
float4 fragMain(VertexToFrag inVert) : SV_Target
{
    return float4(inVert.frag_color, 1.0);
}