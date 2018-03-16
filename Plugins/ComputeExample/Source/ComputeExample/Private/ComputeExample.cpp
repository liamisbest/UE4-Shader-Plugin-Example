// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ComputeExample.h"
#include "ComputeShader.h"


//#define NUM_BOIDS_COUNT 1024
#define NUM_THREADS_PER_GROUP_DIMENSION 32

UComputeExample::UComputeExample(const class FObjectInitializer& initializer)
{
	// Some shit we need!
	outer = GetOuter();
	if (outer && outer->ImplementsGetWorld())
	{
		_featureLevel = outer->GetWorld()->Scene->GetFeatureLevel();
	}

	// Setup our params
	_constantParameters.Speed = 1;
	_variableParameters = FComputeExampleVariableParameters();
	_variableParameters.NOISE_POSITION_SCALE = NoisePosScale;
	_variableParameters.NOISE_TIME_SCALE = NoiseTimeScale;
	_variableParameters.NOISE_SCALE = NoiseScale;
	_variableParameters.Turbulence = NoiseTurbulence;
	_variableParameters.NoiseMult = NoiseMult;

	// Setup buffers
	_boidData.Init(FBoid(), BoidCount);

	// This is the default buffer that gets sent to the GPU
	TResourceArray<FBoid> bufferData;
	for (int32 i = 0; i < BoidCount; i++) {
		FVector4 startVel = FVector4(FMath::RandRange(-.5f, 0.5f), FMath::RandRange(-.5f, 0.5f), FMath::RandRange(-.5f, 0.5f), 1.0f);
		FVector4 startPos = FVector4(FMath::RandRange(-100.f, 100.0f), FMath::RandRange(-100.f, 100.0f), FMath::RandRange(-100.f, 100.0f), 1.0f);
		bufferData.Add(FBoid(startVel, startPos));
	}
	bufferData.SetAllowCPUAccess(true);
	
	// Assign our data to a RHI (Rendering Hardware Interface) resource
	FRHIResourceCreateInfo createInfo;
	createInfo.ResourceArray = &bufferData;

	// Create a RHI Structured Buffer (our way to access the data on CPU)
	_boidDataBuffer = RHICreateStructuredBuffer(sizeof(FBoid), sizeof(FBoid) * BoidCount, BUF_UnorderedAccess | BUF_ShaderResource, createInfo);
	// Our UAV - What the GPU is gonna be writing to
	_boidDataUAV = RHICreateUnorderedAccessView(_boidDataBuffer, true, false);
}

UComputeExample::~UComputeExample()
{
	bIsUnloading = true;
}

void UComputeExample::ExecuteShader()
{
	if (bIsUnloading || bIsExecuting)
	{
		return;
	}

	// Update our variable shader attributes here
	if (IsValid(outer)) {
		_variableParameters.GlobalTime = UGameplayStatics::GetRealTimeSeconds(outer->GetWorld());
	}
	else {
		_variableParameters.GlobalTime += 1.0;
	}
	_variableParameters.NOISE_POSITION_SCALE = NoisePosScale;
	_variableParameters.NOISE_TIME_SCALE = NoiseTimeScale;
	_variableParameters.NOISE_SCALE = NoiseScale;
	_variableParameters.Turbulence = NoiseTurbulence;
	_variableParameters.NoiseMult = NoiseMult;

	// Flag our shit that it's executing
	bIsExecuting = true;

	// Let the magic begin -> Tell unreal that we wanna execute our shader on that juicy render thread
	ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
		FComputeShaderRunner,
		UComputeExample*,
		computeShader,
		this,
		{
			computeShader->ExecuteShaderInternal();
		}
	)
}

void UComputeExample::ExecuteShaderInternal()
{
	check(IsInRenderingThread());

	// Clean up shit, or else GPU drivers will probably crash
	if (bIsUnloading)
	{
		if (_boidDataUAV != NULL)
		{
			_boidDataUAV.SafeRelease();
			_boidDataUAV = NULL;
		}
		return;
	}

	// I don't really know but we need it - I can only assume its a call stack of render commands to run
	FRHICommandListImmediate& rhiCmdList = GRHICommandList.GetImmediateCommandList();
	TShaderMapRef<FComputeShader> computeShader(GetGlobalShaderMap(_featureLevel));
	rhiCmdList.SetComputeShader(computeShader->GetComputeShader());

	// Feed the beast it's food to crunch on
	computeShader->SetBoidData(rhiCmdList, _boidDataUAV);
	// Set our uniform buffers
	computeShader->SetUniformBuffers(rhiCmdList, _constantParameters, _variableParameters);
	// Make the compute shader execute
	DispatchComputeShader(rhiCmdList, *computeShader, (sizeof(FBoid) * BoidCount)/ NUM_THREADS_PER_GROUP_DIMENSION, 1, 1);
	// Unbind our uniforms and data buffer
	computeShader->UnbindBuffers(rhiCmdList);

	// Lets get our data
	RetrieveDataInternal();
}

void UComputeExample::RetrieveDataInternal()
{
	check(IsInRenderingThread());

	// Lock our struct buffer, or suffer death
	uint8* srcPtr = (uint8*)RHILockStructuredBuffer(_boidDataBuffer, 0, sizeof(FBoid) * BoidCount, EResourceLockMode::RLM_ReadOnly);
	// Reference pointer to first element for our destination
	uint8* dstPtr = (uint8*)_boidData.GetData();
	// Copy from GPU to main memory
	FMemory::Memcpy(dstPtr, srcPtr, sizeof(FBoid) * BoidCount);
	//Unlock the struct buffer
	RHIUnlockStructuredBuffer(_boidDataBuffer);

	// Queue a task on the CPU call stack to set executing to false, and to let our blueprints friends know we care
	FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([&]()
	{
		bIsExecuting = false;
		//if (!bIsUnloading && !bIsExecuting)
		//{
			OnShaderExecuted.Broadcast();
		//}
	}, TStatId(), NULL, ENamedThreads::GameThread);
}