/*
struct VertexInput {
    float3 position;
    float3 color;
    float2 tex_coordinate;
};
*/
struct VPUniformBuffer {
    float4x4 view;
    float4x4 proj;
};

ParameterBlock<VPUniformBuffer> vp;

static float3 positions[3] = float3[](
	float3(0.0, 0.0, -0.5),
	float3(0.5, 0.0, 0.5),
	float3(-0.5, 0.0, 0.5)
);


static float3 colors[3] = float3[](
	float3(1.0, 0.0, 0.0),
	float3(0.0, 1.0, 0.0),
	float3(0.0, 0.0, 1.0)
);

struct VertexOutput {
    float4 sv_position : SV_Position;
    float3 frag_color;
};

[shader("vertex")]
VertexOutput vertMain(uint vertexID : SV_VertexID) {
    VertexOutput output;
    output.sv_position = mul(vp.proj, mul(vp.view, float4(positions[vertexID], 1.0)));
    output.frag_color = colors[vertexID];
    return output;
}

[shader("fragment")]
float4 fragMain(VertexOutput inVert) : SV_Target
{
    return float4(inVert.frag_color, 1.0);
}