// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InstancingRootActor.generated.h"

UCLASS()
class FLOCKING_API AInstancingRootActor : public AActor
{
	GENERATED_BODY()
		
public:	
	// Sets default values for this actor's properties
	AInstancingRootActor();
	// Called every frame
	virtual void Tick(float DeltaTime) override;



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	class UMaterialInstanceDynamic* DrawMatInstance;
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
		class UInstancedStaticMeshComponent* InstancedComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
		class UStaticMeshComponent* rootStaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flocking")
		class UMaterial* DrawMat;

	UFUNCTION(BlueprintCallable)
		void InitInstances(int instatnceNum);

	UFUNCTION(BlueprintCallable)
		int GetInstanceCount();
};
