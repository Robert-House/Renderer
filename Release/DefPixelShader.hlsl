struct PS_INPUT
{
	float4 Position		: SV_POSITION;
	float2 Texture		: TEXCOORD;
	float3x3 TBNMatrix	: TBNMATRIX;
};

struct PS_OUTPUT
{
	float4 Color		: SV_TARGET0;
	float4 Normal		: SV_TARGET1;
	float4 Roughness	: SV_TARGET2;
};

Texture2D gDiffuse		: register(t0);
Texture2D gNormal		: register(t1);
Texture2D gRoughness	: register(t2);

SamplerState gSampleType : register(s0);

PS_OUTPUT PS(PS_INPUT In)
{
	PS_OUTPUT Output;

	// Grab normal map and normalize it
	float3 NormalMap = gNormal.Sample(gSampleType, In.Texture).rgb;
	NormalMap = normalize(2.0f * (NormalMap) - 1.0f);

	// Combine TVNMatrix and NormalMap. Normalize
	float3 WorldNormal = normalize(mul(NormalMap, In.TBNMatrix));

	// RoughnessMap
	float4 Roughness = gRoughness.Sample(gSampleType, In.Texture);
	Roughness = normalize(Roughness);

	// Output render targets
	Output.Color = gDiffuse.Sample(gSampleType, In.Texture);
	Output.Normal.rgb = WorldNormal;
	Output.Normal.a = normalize(gNormal.Sample(gSampleType, In.Texture).a);
	Output.Roughness.gb = In.Position.z / In.Position.w;
	Output.Roughness.r = Roughness;

	return Output;
}
