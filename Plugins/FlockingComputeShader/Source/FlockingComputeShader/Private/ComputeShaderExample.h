#pragma once

#include "CoreMinimal.h"
#include "FlockingComputeShader.h"


class FComputeShaderExample
{
public:
	static void RunComputeShader_RenderThread(FRHICommandListImmediate& RHICmdList, const FShaderUsageExampleParameters& DrawParameters, FUnorderedAccessViewRHIRef VelocityOutpuUAV, FTextureRHIRef VelocityInputRT, FUnorderedAccessViewRHIRef PositionOutputUAV, FTextureRHIRef PositionInputRT);
};
