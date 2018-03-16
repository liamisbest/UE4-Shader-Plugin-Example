// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ComputeShader.h"

#include "CoreUObject.h"
#include "Engine.h"

#include "ComputeExample.h"

#include "ShaderParameterUtils.h"
#include "RHIStaticStates.h"

IMPLEMENT_UNIFORM_BUFFER_STRUCT(FComputeExampleConstantParameters, TEXT("CSConstants"))
IMPLEMENT_UNIFORM_BUFFER_STRUCT(FComputeExampleVariableParameters, TEXT("CSVariables"))

FComputeShader::FComputeShader(const ShaderMetaType::CompiledShaderInitializerType& initializer) : FGlobalShader(initializer)
{
	_boidData.Bind(initializer.ParameterMap, TEXT("BoidData"));
}

void FComputeShader::ModifyCompilationEnvironment(EShaderPlatform platform, FShaderCompilerEnvironment& outEnvironment)
{
	FGlobalShader::ModifyCompilationEnvironment(platform, outEnvironment);
	outEnvironment.CompilerFlags.Add(CFLAG_StandardOptimization);
}

void FComputeShader::SetBoidData(FRHICommandList& rhiCmdList, FUnorderedAccessViewRHIRef boidDataUAV)
{
	FComputeShaderRHIParamRef computeShaderRHI = GetComputeShader();

	if (_boidData.IsBound())
	{
		rhiCmdList.SetUAVParameter(computeShaderRHI, _boidData.GetBaseIndex(), boidDataUAV);
	}
}

void FComputeShader::SetUniformBuffers(FRHICommandList& rhiCmdList, FComputeExampleConstantParameters& constantParameters, FComputeExampleVariableParameters& variableParameters)
{
	FComputeExampleConstantParametersRef constantParametersBuffer = FComputeExampleConstantParametersRef::CreateUniformBufferImmediate(constantParameters, UniformBuffer_SingleDraw);
	FComputeExampleVariableParametersRef variableParametersBuffer = FComputeExampleVariableParametersRef::CreateUniformBufferImmediate(variableParameters, UniformBuffer_SingleDraw);

	SetUniformBufferParameter(rhiCmdList, GetComputeShader(), GetUniformBufferParameter<FComputeExampleConstantParameters>(), constantParametersBuffer);
	SetUniformBufferParameter(rhiCmdList, GetComputeShader(), GetUniformBufferParameter<FComputeExampleVariableParameters>(), variableParametersBuffer);
}

void FComputeShader::UnbindBuffers(FRHICommandList& rhiCmdList)
{
	FComputeShaderRHIParamRef computeShaderRHI = GetComputeShader();

	if (_boidData.IsBound())
		rhiCmdList.SetUAVParameter(computeShaderRHI, _boidData.GetBaseIndex(), FUnorderedAccessViewRHIRef());
}

IMPLEMENT_SHADER_TYPE(, FComputeShader, TEXT("/Plugin/ComputeExample/ComputeShader.usf"), TEXT("MainComputeShader"), SF_Compute);

IMPLEMENT_MODULE(FDefaultModuleImpl, FComputeShader)