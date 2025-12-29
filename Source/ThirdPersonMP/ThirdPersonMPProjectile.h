// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThirdPersonMPProjectile.generated.h"

UCLASS()
class THIRDPERSONMP_API AThirdPersonMPProjectile : public AActor
{
	GENERATED_BODY()
public:
	/**
	 * 这些声明中的各个类型前面都冠以 class 关键字。
	 * 这样，这些声明除了是变量声明之外，还是各自类的前向声明，从而确保各自的类会在头文件中被识别。
	 */
	
	// 用于测试碰撞的球体组件。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* SphereComponent;

	// 用于提供对象视觉呈现效果的静态网格体。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* StaticMesh;

	// 用于提供对象视觉呈现效果的静态网格体资产。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Components")
	UStaticMesh* MeshAsset;

	// 用于处理投射物移动的移动组件。
	/**
	 * 像角色移动组件一样，投射物移动组件在移动其所归属的Actor时，
	 * 若该Actor的 bReplicates 设为 True，则投射物移动组件自动处理复制。
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class UTPSProjectileMovementComponent* ProjectileMovementComponent;

	// 在投射物撞击其他对象并爆炸时使用的粒子。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Components")
	class UParticleSystem* ExplosionEffect;

	//此投射物将造成的伤害类型和伤害。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	TSubclassOf<class UDamageType> DamageType;

	//此投射物造成的伤害。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage")
	float Damage;

public:
	// Sets default values for this actor's properties
	AThirdPersonMPProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Destroyed() override;

	UFUNCTION(Category="Projectile")
	void OnProjectileImpact(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
 

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
