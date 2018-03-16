// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ComputeShader.h"
#include "CoreUObject.h"
#include "Engine.h"
#include "ComputeExample.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FShaderExecuted);

USTRUCT(BlueprintType)
struct FBoid
{
	GENERATED_BODY()

	FBoid() { Vel = FVector4(); Pos = FVector4(); }
	FBoid(FVector4 vel, FVector4 pos) { Vel = vel; Pos = pos; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector4 Pos;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector4 Vel;
};


UCLASS(Blueprintable, BlueprintType)
class COMPUTEEXAMPLE_API UComputeExample : public UObject
{
	GENERATED_UCLASS_BODY()

	public:
		~UComputeExample();

		// Queue our render thread
		UFUNCTION(BlueprintCallable)
		void ExecuteShader();

		// Render thread commands
		void ExecuteShaderInternal();
		void RetrieveDataInternal();

		// Getter for our computed data
		UFUNCTION(BlueprintCallable)
		TArray<FBoid> GetBoidData() { return TArray<FBoid>(_boidData); }

		// Delegate that fires when everything is computed
		UPROPERTY(BlueprintAssignable)
		FShaderExecuted OnShaderExecuted;

		// How many boids can you handle?
		UPROPERTY(BlueprintReadOnly)
		int32 BoidCount = 512;

		UPROPERTY(BlueprintReadWrite, EditAnywhere)
			float NoisePosScale = 0.005;

		UPROPERTY(BlueprintReadWrite, EditAnywhere)
			float NoiseTimeScale = 1.0;

		UPROPERTY(BlueprintReadWrite, EditAnywhere)
			float NoiseScale = 0.02;

		UPROPERTY(BlueprintReadWrite, EditAnywhere)
			float NoiseTurbulence = 0.8;

		UPROPERTY(BlueprintReadWrite, EditAnywhere)
			float NoiseMult = 500.0;

		
	private:
		// Some states we might need
		bool bIsExecuting = false;
		bool bIsUnloading = false;

		// Define our shader params
		FComputeExampleConstantParameters _constantParameters;
		FComputeExampleVariableParameters _variableParameters;
		ERHIFeatureLevel::Type _featureLevel;

		// Define our data
		TArray<FBoid> _boidData;
		FStructuredBufferRHIRef _boidDataBuffer;
		FUnorderedAccessViewRHIRef _boidDataUAV;

		// Dirt
		UObject* outer;
};