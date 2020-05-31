// Fill out your copyright notice in the Description page of Project Settings.


#include "InstancingRootActor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AInstancingRootActor::AInstancingRootActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//this staticmeshComponent act as the bound of the UInstancedStaticMeshComponent ot avoid blink
	rootStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("rootStaticMesh"));
	RootComponent = rootStaticMesh;

	InstancedComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("UInstancedStaticMeshComponent"));
	InstancedComponent->SetupAttachment(rootStaticMesh);

	//set the number of custom data needed
	InstancedComponent->NumCustomDataFloats = 1;
	InstancedComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AInstancingRootActor::InitInstances(int instatnceNum)
{
	if (instatnceNum <= 1)
	{
		instatnceNum = 1;
	}
	else if (instatnceNum > 16384)
	{
		instatnceNum = 16384;
	}
	InstancedComponent->ClearInstances();
		//create instances
		for (int i = 0; i < instatnceNum; i++)
		{
			FTransform t = FTransform();
			//t.SetLocation(FVector(FMath::RandRange(-100, 100) * i, FMath::RandRange(-1024, 1024), FMath::RandRange(-1024, 1024)));
			InstancedComponent->AddInstance(t);
			InstancedComponent->SetCustomDataValue(i, 0, i);
			UE_LOG(LogTemp, Warning, TEXT("AddInstance %d"), i);
		}
}


int AInstancingRootActor::GetInstanceCount()
{
	if (InstancedComponent != nullptr)
	{
		return InstancedComponent->GetInstanceCount();
	}
	return 0;
}

// Called when the game starts or when spawned
void AInstancingRootActor::BeginPlay()
{
	Super::BeginPlay();
	//create the material used to draw instance
	if (DrawMat != nullptr)
	{
		DrawMatInstance = InstancedComponent->CreateDynamicMaterialInstance(0, DrawMat);
		InitInstances(16384);
		UE_LOG(LogTemp, Warning, TEXT("InitInstances!"));
	}
}

// Called every frame
void AInstancingRootActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}



