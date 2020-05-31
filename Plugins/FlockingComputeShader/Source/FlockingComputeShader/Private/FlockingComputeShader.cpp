#include "FlockingComputeShader.h"

#include "ComputeShaderExample.h"
#include "PixelShaderExample.h"

#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "RHI.h"
#include "GlobalShader.h"
#include "RHICommandList.h"
#include "RenderGraphBuilder.h"
#include "RenderTargetPool.h"
#include "Runtime/Core/Public/Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"

IMPLEMENT_MODULE(FFlockingComputeShader, FlockingComputeShader)


void FFlockingComputeShader::StartupModule()
{
	OnPostResolvedSceneColorHandle.Reset();
	bCachedParametersValid = false;

	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("FlockingComputeShader"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/FlockingComputeShaders"), PluginShaderDir);
}

void FFlockingComputeShader::ShutdownModule()
{
	EndRendering();
	ResetAllShaderSourceDirectoryMappings();
}

void FFlockingComputeShader::BeginRendering()
{
	if (OnPostResolvedSceneColorHandle.IsValid())
	{
		return;
	}

	bCachedParametersValid = false;

	const FName RendererModuleName("Renderer");
	IRendererModule* RendererModule = FModuleManager::GetModulePtr<IRendererModule>(RendererModuleName);
	if (RendererModule)
	{
		OnPostResolvedSceneColorHandle = RendererModule->GetResolvedSceneColorCallbacks().AddRaw(this, &FFlockingComputeShader::PostResolveSceneColor_RenderThread);
	}
}

void FFlockingComputeShader::EndRendering()
{
	if (!OnPostResolvedSceneColorHandle.IsValid())
	{
		return;
	}

	const FName RendererModuleName("Renderer");
	IRendererModule* RendererModule = FModuleManager::GetModulePtr<IRendererModule>(RendererModuleName);
	if (RendererModule)
	{
		RendererModule->GetResolvedSceneColorCallbacks().Remove(OnPostResolvedSceneColorHandle);
	}

	OnPostResolvedSceneColorHandle.Reset();
}

void FFlockingComputeShader::UpdateParameters(FShaderUsageExampleParameters& DrawParameters)
{
	RenderEveryFrameLock.Lock();
	CachedShaderUsageExampleParameters = DrawParameters;
	bCachedParametersValid = true;
	RenderEveryFrameLock.Unlock();
}

void FFlockingComputeShader::PostResolveSceneColor_RenderThread(FRHICommandListImmediate& RHICmdList, class FSceneRenderTargets& SceneContext)
{
	if (!bCachedParametersValid)
	{
		return;
	}

	// Depending on your data, you might not have to lock here, just added this code to show how you can do it if you have to.
	RenderEveryFrameLock.Lock();
	FShaderUsageExampleParameters Copy = CachedShaderUsageExampleParameters;
	RenderEveryFrameLock.Unlock();

	Draw_RenderThread(Copy);
}

void FFlockingComputeShader::Draw_RenderThread(const FShaderUsageExampleParameters& DrawParameters)
{
	check(IsInRenderingThread());
	if (!DrawParameters.VelocityRenderTarget)
	{
		return;
	}

	if (!DrawParameters.PositionRenderTarget)
	{
		return;
	}

	FRHICommandListImmediate& RHICmdList = GRHICommandList.GetImmediateCommandList();


	if (!VelocityOutput.IsValid())
	{
		FPooledRenderTargetDesc ComputeShaderOutputDesc(FPooledRenderTargetDesc::Create2DDesc(DrawParameters.GetRenderTargetSize(), PF_A32B32G32R32F, FClearValueBinding::None, TexCreate_None, TexCreate_ShaderResource | TexCreate_UAV, false));

		ComputeShaderOutputDesc.DebugName = TEXT("ShaderPlugin_ComputeShaderOutput");
		GRenderTargetPool.FindFreeElement(RHICmdList, ComputeShaderOutputDesc, VelocityOutput, TEXT("ShaderPlugin_ComputeShaderOutput"));
	}

	if (!PositionOutput.IsValid())
	{
		FPooledRenderTargetDesc ComputeShaderOutputDesc(FPooledRenderTargetDesc::Create2DDesc(DrawParameters.GetRenderTargetSize(), PF_A32B32G32R32F, FClearValueBinding::None, TexCreate_None, TexCreate_ShaderResource | TexCreate_UAV, false));

		ComputeShaderOutputDesc.DebugName = TEXT("ShaderPlugin_ComputeShaderOutput2");
		GRenderTargetPool.FindFreeElement(RHICmdList, ComputeShaderOutputDesc, PositionOutput, TEXT("ShaderPlugin_ComputeShaderOutput2"));
	}

	FComputeShaderExample::RunComputeShader_RenderThread(RHICmdList, DrawParameters, VelocityOutput->GetRenderTargetItem().UAV, DrawParameters.VelocityRenderTarget->GetRenderTargetResource()->TextureRHI, PositionOutput->GetRenderTargetItem().UAV, DrawParameters.PositionRenderTarget->GetRenderTargetResource()->TextureRHI);
	FPixelShaderExample::DrawToRenderTarget_RenderThread(RHICmdList, DrawParameters, VelocityOutput->GetRenderTargetItem().TargetableTexture, PositionOutput->GetRenderTargetItem().TargetableTexture);
}
