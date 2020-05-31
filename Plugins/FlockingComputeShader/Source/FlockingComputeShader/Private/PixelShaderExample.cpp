#include "PixelShaderExample.h"
#include "ShaderParameterUtils.h"
#include "RHIStaticStates.h"
#include "Shader.h"
#include "GlobalShader.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterMacros.h"
#include "ShaderParameterStruct.h"
#include "UniformBuffer.h"
#include "RHICommandList.h"
#include "Containers/DynamicRHIResourceArray.h"
#include "Runtime/RenderCore/Public/PixelShaderUtils.h"


class FSimpleScreenVertexBuffer : public FVertexBuffer
{
public:
	/** Initialize the RHI for this rendering resource */
	void InitRHI()
	{
		TResourceArray<FFilterVertex, VERTEXBUFFER_ALIGNMENT> Vertices;
		Vertices.SetNumUninitialized(6);

		Vertices[0].Position = FVector4(-1, 1, 0, 1);
		Vertices[0].UV = FVector2D(0, 0);

		Vertices[1].Position = FVector4(1, 1, 0, 1);
		Vertices[1].UV = FVector2D(1, 0);

		Vertices[2].Position = FVector4(-1, -1, 0, 1);
		Vertices[2].UV = FVector2D(0, 1);

		Vertices[3].Position = FVector4(1, -1, 0, 1);
		Vertices[3].UV = FVector2D(1, 1);

		// Create vertex buffer. Fill buffer with initial data upon creation
		FRHIResourceCreateInfo CreateInfo(&Vertices);
		VertexBufferRHI = RHICreateVertexBuffer(Vertices.GetResourceDataSize(), BUF_Static, CreateInfo);
	}
};
TGlobalResource<FSimpleScreenVertexBuffer> GSimpleScreenVertexBuffer;


class FSimplePassThroughVS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FSimplePassThroughVS);

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return true;
	}

	FSimplePassThroughVS() { }
	FSimplePassThroughVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer) : FGlobalShader(Initializer)	{ }
};


class FPixelShaderExamplePS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FPixelShaderExamplePS);
	SHADER_USE_PARAMETER_STRUCT(FPixelShaderExamplePS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_TEXTURE(Texture2D<float4>, ComputeShaderOutput)
		SHADER_PARAMETER(FVector2D, TextureSize) // Metal doesn't support GetDimensions(), so we send in this data via our parameters.
	END_SHADER_PARAMETER_STRUCT()

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}
};

// This will tell the engine to create the shader and where the shader entry point is.
//                           ShaderType                            ShaderPath                Shader function name    Type
IMPLEMENT_GLOBAL_SHADER(FSimplePassThroughVS, "/FlockingComputeShaders/Private/PixelShader.usf", "MainVertexShader", SF_Vertex);
IMPLEMENT_GLOBAL_SHADER(FPixelShaderExamplePS, "/FlockingComputeShaders/Private/PixelShader.usf", "MainPixelShader", SF_Pixel);

void FPixelShaderExample::DrawToRenderTarget_RenderThread(FRHICommandListImmediate& RHICmdList, const FShaderUsageExampleParameters& DrawParameters, FTextureRHIRef ComputeShaderVelocityOutput, FTextureRHIRef ComputeShaderPositionOutput)
{

	FRHIRenderPassInfo RenderPassInfo(DrawParameters.VelocityRenderTarget->GetRenderTargetResource()->GetRenderTargetTexture(), ERenderTargetActions::Clear_Store);
	RHICmdList.BeginRenderPass(RenderPassInfo, TEXT("ShaderPlugin_OutputToRenderTarget"));

	auto ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
	//TShaderMapRef<FSimplePassThroughVS> VertexShader(ShaderMap);
	//TShaderMapRef<FPixelShaderExamplePS> PixelShader(ShaderMap);

	TShaderMapRef<FSimplePassThroughVS> VertexShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	TShaderMapRef<FPixelShaderExamplePS> PixelShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

		
	// Set the graphic pipeline state.
	FGraphicsPipelineStateInitializer GraphicsPSOInit;
	RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
	GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
	GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
	GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
	GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GFilterVertexDeclaration.VertexDeclarationRHI;
	//GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VertexShader);
	//GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PixelShader);
	GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
	GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
	GraphicsPSOInit.PrimitiveType = PT_TriangleStrip;
	SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
	
	// Setup the pixel shader
	FPixelShaderExamplePS::FParameters PassParameters; 
	PassParameters.ComputeShaderOutput = ComputeShaderVelocityOutput;
	PassParameters.TextureSize = FVector2D(DrawParameters.GetRenderTargetSize().X, DrawParameters.GetRenderTargetSize().Y);
	
	SetShaderParameters(RHICmdList, PixelShader, PixelShader.GetPixelShader(), PassParameters);
	
	// Draw
	RHICmdList.SetStreamSource(0, GSimpleScreenVertexBuffer.VertexBufferRHI, 0);
	RHICmdList.DrawPrimitive(0, 2, 1);
	
	// Resolve render target
	RHICmdList.CopyToResolveTarget(DrawParameters.VelocityRenderTarget->GetRenderTargetResource()->GetRenderTargetTexture(), DrawParameters.VelocityRenderTarget->GetRenderTargetResource()->TextureRHI, FResolveParams());

	RHICmdList.EndRenderPass();



	//second pass
	FRHIRenderPassInfo RenderPassInfo2(DrawParameters.PositionRenderTarget->GetRenderTargetResource()->GetRenderTargetTexture(), ERenderTargetActions::Clear_Store);
	RHICmdList.BeginRenderPass(RenderPassInfo2, TEXT("ShaderPlugin_OutputToRenderTarget2"));

	SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

	// Setup the pixel shader
	FPixelShaderExamplePS::FParameters PassParameters2;
	PassParameters2.ComputeShaderOutput = ComputeShaderPositionOutput;
	PassParameters2.TextureSize = FVector2D(DrawParameters.GetRenderTargetSize().X, DrawParameters.GetRenderTargetSize().Y);
	SetShaderParameters(RHICmdList, PixelShader, PixelShader.GetPixelShader(), PassParameters2);

	// Draw
	RHICmdList.SetStreamSource(0, GSimpleScreenVertexBuffer.VertexBufferRHI, 0);
	RHICmdList.DrawPrimitive(0, 2, 1);

	// Resolve render target
	RHICmdList.CopyToResolveTarget(DrawParameters.PositionRenderTarget->GetRenderTargetResource()->GetRenderTargetTexture(), DrawParameters.PositionRenderTarget->GetRenderTargetResource()->TextureRHI, FResolveParams());

	RHICmdList.EndRenderPass();

}
