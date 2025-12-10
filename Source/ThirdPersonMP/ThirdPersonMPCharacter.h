// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
//#include "ThirdPersonMPProjectile.h"
#include "ThirdPersonMPCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class AThirdPersonMPProjectile; //前向声明
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class AThirdPersonMPCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
protected:

		 
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> FireAction;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;
	
	/** 玩家的最大生命值。这是玩家的最高生命值。此值为玩家出生时的生命值。*/
	UPROPERTY(EditDefaultsOnly, Category = "Health")
	float MaxHealth;
 
	/** 玩家的当前生命值。降到0就表示死亡。*/
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
	float CurrentHealth;
 
	/** RepNotify，用于同步对当前生命值所做的更改。*/
	UFUNCTION()
	void OnRep_CurrentHealth();

	UPROPERTY(EditAnywhere, Category="Combat")
	TSubclassOf<AThirdPersonMPProjectile> Bullet;

public:

	/** Constructor */
	AThirdPersonMPCharacter();

	/** 属性复制 */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
	
	/** 响应要更新的生命值。修改后，立即在服务器上调用，并在客户端上调用以响应RepNotify*/
	void OnHealthUpdate();

	/** 投射物类。*/
	UPROPERTY(EditDefaultsOnly, Category="Gameplay|Projectile")
	TSubclassOf<class AThirdPersonMPProjectile> ProjectileClass;
 
	/** 射击之间的延迟，单位为秒。用于控制测试发射物的射击速度，还可防止服务器函数的溢出导致将SpawnProjectile直接绑定至输入。*/
	UPROPERTY(EditDefaultsOnly, Category="Gameplay")
	float FireRate;
 
	/** 若为true，则正在发射投射物。*/
	bool bIsFiringWeapon;
 
	/** 用于启动武器射击的函数。*/
	UFUNCTION(BlueprintCallable, Category="Gameplay")
	void StartFire();
 
	/** 用于结束武器射击的函数。一旦调用这段代码，玩家可再次使用StartFire。*/
	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void StopFire();
 
	/** 用于生成投射物的服务器函数。*/
	UFUNCTION(Server, Reliable)
	void HandleFire();
 
	/** 定时器句柄，用于提供生成间隔时间内的射速延迟。*/
	FTimerHandle FiringTimer;

public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	/** 最大生命值的取值函数。*/
	UFUNCTION(BlueprintPure, Category="Health")
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
 
	/** 当前生命值的取值函数。*/
	UFUNCTION(BlueprintPure, Category="Health")
	FORCEINLINE float GetCurrentHealth() const { return CurrentHealth; }
 
	/** 当前生命值的存值函数。将此值的范围限定在0到MaxHealth之间，并调用OnHealthUpdate。仅在服务器上调用。*/
	UFUNCTION(BlueprintCallable, Category="Health")
	void SetCurrentHealth(float healthValue);
 
	/** 承受伤害的事件。从APawn覆盖。*/
	UFUNCTION(BlueprintCallable, Category = "Health")
	float TakeDamage( float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser ) override;
 

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

