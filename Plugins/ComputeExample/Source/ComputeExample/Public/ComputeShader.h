#pragma once

#include "GlobalShader.h"
#include "UniformBuffer.h"
#include "RHICommandList.h"

BEGIN_UNIFORM_BUFFER_STRUCT(FComputeExampleConstantParameters, )
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(float, Speed)
END_UNIFORM_BUFFER_STRUCT(FComputeExampleConstantParameters)

BEGIN_UNIFORM_BUFFER_STRUCT(FComputeExampleVariableParameters, )
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(float, GlobalTime)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(float, NOISE_POSITION_SCALE)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(float, NOISE_TIME_SCALE)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(float, NOISE_SCALE)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(float, Turbulence)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(float, NoiseMult)
END_UNIFORM_BUFFER_STRUCT(FComputeExampleVariableParameters)

typedef TUniformBufferRef<FComputeExampleConstantParameters> FComputeExampleConstantParametersRef;
typedef TUniformBufferRef<FComputeExampleVariableParameters> FComputeExampleVariableParametersRef;

class FComputeShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FComputeShader, Global)

public:
	FComputeShader() {};
	explicit FComputeShader(const ShaderMetaType::CompiledShaderInitializerType& initializer);

	static bool ShouldCache(EShaderPlatform platform)
	{
		return IsFeatureLevelSupported(platform, ERHIFeatureLevel::SM5);
	}

	static void ModifyCompilationEnvironment(EShaderPlatform platform, FShaderCompilerEnvironment& outEnvironment);

	bool Serialize(FArchive& archive) override
	{
		bool parametersOutdated = FGlobalShader::Serialize(archive);
		archive << _boidData;
		return parametersOutdated;
	}

	void SetBoidData(FRHICommandList& rhiCmdList, FUnorderedAccessViewRHIRef boidDataUAV);
	void SetUniformBuffers(FRHICommandList& rhiCmdList, FComputeExampleConstantParameters& constantParameters, FComputeExampleVariableParameters& variableParameters);
	void UnbindBuffers(FRHICommandList& rhiCmdList);

private:
	FShaderResourceParameter _boidData;
};