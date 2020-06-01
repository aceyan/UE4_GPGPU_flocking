#include "ComputeShaderExample.h"
#include "ShaderParameterUtils.h"
#include "RHIStaticStates.h"
#include "Shader.h"
#include "GlobalShader.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
#include "UniformBuffer.h"
#include "RHICommandList.h"

#define NUM_THREADS_PER_GROUP_DIMENSION 32


class FComputeShaderExampleCS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FComputeShaderExampleCS);
	SHADER_USE_PARAMETER_STRUCT(FComputeShaderExampleCS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_UAV(RWTexture2D<float4>, VelocityOutputTexture)
		SHADER_PARAMETER_TEXTURE(Texture2D<float4>, VelocityInput)
		SHADER_PARAMETER_UAV(RWTexture2D<float4>, PositionOutputTexture)
		SHADER_PARAMETER_TEXTURE(Texture2D<float4>, PositionInput)
		SHADER_PARAMETER(FVector2D, TextureSize) // Metal doesn't support GetDimensions(), so we send in this data via our parameters.
		SHADER_PARAMETER(float, Range)
		SHADER_PARAMETER(float, AlignScaler)
		SHADER_PARAMETER(float, CohesionScaler)
		SHADER_PARAMETER(float, SeparationScaler)
	END_SHADER_PARAMETER_STRUCT()

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	static inline void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_X"), NUM_THREADS_PER_GROUP_DIMENSION);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Y"), NUM_THREADS_PER_GROUP_DIMENSION);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Z"), 1);
	}
};

// This will tell the engine to create the shader and where the shader entry point is.
//                            ShaderType                            ShaderPath                     Shader function name    Type
IMPLEMENT_GLOBAL_SHADER(FComputeShaderExampleCS, "/FlockingComputeShaders/Private/ComputeShader.usf", "MainComputeShader", SF_Compute);

void FComputeShaderExample::RunComputeShader_RenderThread(FRHICommandListImmediate& RHICmdList, const FShaderUsageExampleParameters& DrawParameters, FUnorderedAccessViewRHIRef VelocityOutpuUAV, FTextureRHIRef VelocityInputRT , FUnorderedAccessViewRHIRef PositionOutputUAV, FTextureRHIRef PositionInputRT)
{

	UnbindRenderTargets(RHICmdList);
	RHICmdList.TransitionResource(EResourceTransitionAccess::ERWBarrier, EResourceTransitionPipeline::EGfxToCompute, VelocityOutpuUAV);
	RHICmdList.TransitionResource(EResourceTransitionAccess::ERWBarrier, EResourceTransitionPipeline::EGfxToCompute, PositionOutputUAV);

	FComputeShaderExampleCS::FParameters PassParameters;
	PassParameters.VelocityOutputTexture = VelocityOutpuUAV;
	PassParameters.VelocityInput = VelocityInputRT;
	PassParameters.PositionOutputTexture = PositionOutputUAV;
	PassParameters.PositionInput = PositionInputRT;
	PassParameters.TextureSize = FVector2D(DrawParameters.GetRenderTargetSize().X, DrawParameters.GetRenderTargetSize().Y);
	PassParameters.Range = DrawParameters.Range;
	PassParameters.AlignScaler = DrawParameters.AlignScaler;
	PassParameters.CohesionScaler = DrawParameters.CohesionScaler;
	PassParameters.SeparationScaler = DrawParameters.SeparationScaler;

	TShaderMapRef<FComputeShaderExampleCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	FComputeShaderUtils::Dispatch(RHICmdList, ComputeShader, PassParameters, 
								FIntVector(FMath::DivideAndRoundUp(DrawParameters.GetRenderTargetSize().X, NUM_THREADS_PER_GROUP_DIMENSION),
										   FMath::DivideAndRoundUp(DrawParameters.GetRenderTargetSize().Y, NUM_THREADS_PER_GROUP_DIMENSION), 1));
}
