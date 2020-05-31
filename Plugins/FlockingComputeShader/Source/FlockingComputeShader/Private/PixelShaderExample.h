#pragma once

#include "CoreMinimal.h"
#include "FlockingComputeShader.h"


class FPixelShaderExample
{
public:
	static void DrawToRenderTarget_RenderThread(FRHICommandListImmediate& RHICmdList, const FShaderUsageExampleParameters& DrawParameters, FTextureRHIRef ComputeShaderVelocityOutput, FTextureRHIRef ComputeShaderPositionOutput);
};
