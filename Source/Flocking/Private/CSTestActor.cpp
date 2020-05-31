// Fill out your copyright notice in the Description page of Project Settings.


#include "CSTestActor.h"
#include "FlockingComputeShader.h"

// Sets default values
ACSTestActor::ACSTestActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACSTestActor::BeginPlay()
{
	Super::BeginPlay();
	FFlockingComputeShader::Get().BeginRendering();
}

void ACSTestActor::BeginDestroy()
{
	FFlockingComputeShader::Get().EndRendering();
	Super::BeginDestroy();
}

// Called every frame
void ACSTestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FShaderUsageExampleParameters DrawParameters(VelocityRenderTarget, PositionRenderTarget);
	{
		DrawParameters.Range = Range;
	}
	FFlockingComputeShader::Get().UpdateParameters(DrawParameters);
}

