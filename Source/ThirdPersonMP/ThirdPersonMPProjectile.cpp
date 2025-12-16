// Fill out your copyright notice in the Description page of Project Settings.


#include "ThirdPersonMPProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
AThirdPersonMPProjectile::AThirdPersonMPProjectile()
{
	bReplicates = true;
	
	//定义将作为投射物及其碰撞的根组件的SphereComponent。
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("RootComponent"));
	SphereComponent->InitSphereRadius(37.5f);
	SphereComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	RootComponent = SphereComponent;
	//在击中事件上注册此投射物撞击函数。
	if (GetLocalRole() == ROLE_Authority)
	{
		SphereComponent->OnComponentHit.AddDynamic(this, &AThirdPersonMPProjectile::OnProjectileImpact);
	}

	//定义将作为视觉呈现的网格体。
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	StaticMesh->SetupAttachment(RootComponent);

	//定义投射物移动组件。
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovementComponent->SetUpdatedComponent(SphereComponent);
	ProjectileMovementComponent->InitialSpeed = 1500.0f;
	ProjectileMovementComponent->MaxSpeed = 1500.0f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->ProjectileGravityScale = 0.6f;

	//定义伤害类型和伤害值。
	DamageType = UDamageType::StaticClass();
	Damage = 10.0f;
	
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AThirdPersonMPProjectile::Destroyed()
{
	FVector spawnLocation = GetActorLocation();
	UGameplayStatics::SpawnEmitterAtLocation(this, ExplosionEffect, spawnLocation, FRotator::ZeroRotator, true, EPSCPoolMethod::AutoRelease);
}

void AThirdPersonMPProjectile::OnProjectileImpact(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if ( OtherActor )
	{
		UGameplayStatics::ApplyPointDamage(OtherActor, Damage, NormalImpulse, Hit, GetInstigator()->Controller, this, DamageType);
	}
 
	Destroy();
}

// Called when the game starts or when spawned
void AThirdPersonMPProjectile::BeginPlay()
{
	Super::BeginPlay();

	// [delta 251215 to do: visualize the fvector settings in the editor instead of hard coding]
	if (MeshAsset)
	{
		StaticMesh->SetStaticMesh(MeshAsset);
		StaticMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -37.5f));
		StaticMesh->SetRelativeScale3D(FVector(0.75f, 0.75f, 0.75f));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MeshAsset not set on %s."), *GetName());
	}
}

// Called every frame
void AThirdPersonMPProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

