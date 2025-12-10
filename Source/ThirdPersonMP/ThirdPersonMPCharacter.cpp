// Copyright Epic Games, Inc. All Rights Reserved.

#include "ThirdPersonMPCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "ThirdPersonMP.h"
#include "Net/UnrealNetwork.h"     
#include "Engine/Engine.h"
#include "ThirdPersonMPProjectile.h"

AThirdPersonMPCharacter::AThirdPersonMPCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	//初始化玩家生命值
	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;
}

void AThirdPersonMPCharacter::StartFire()
{
	UE_LOG(LogThirdPersonMP, Warning, TEXT("'%s' Start fire!"), *GetNameSafe(this));
	if (!bIsFiringWeapon)
	{
		bIsFiringWeapon = true;
		UWorld* World = GetWorld();
		
		//时长为 FireRate 的定时器结束时，会调用 StopFire
		World->GetTimerManager().SetTimer(FiringTimer, this, &AThirdPersonMPCharacter::StopFire, FireRate, false);
		
		HandleFire();
	}
}
 
void AThirdPersonMPCharacter::StopFire()
{
	bIsFiringWeapon = false;
}
 
void AThirdPersonMPCharacter::HandleFire_Implementation()
{
	FVector spawnLocation = GetActorLocation() + ( GetActorRotation().Vector()  * 100.0f ) + (GetActorUpVector() * 50.0f);
	FRotator spawnRotation = GetActorRotation();
 
	FActorSpawnParameters spawnParameters;
	spawnParameters.Instigator = GetInstigator();
	spawnParameters.Owner = this;
 
	AThirdPersonMPProjectile* spawnedProjectile = GetWorld()->SpawnActor<AThirdPersonMPProjectile>(Bullet, spawnLocation, spawnRotation, spawnParameters);
}

void AThirdPersonMPCharacter::OnRep_CurrentHealth()
{
	OnHealthUpdate();
}

// 复制的属性
void AThirdPersonMPCharacter::GetLifetimeReplicatedProps(TArray <FLifetimeProperty> & OutLifetimeProps) const
{
	//必须调用 GetLifetimeReplicatedProps 的 Super 版本，否则从Actor父类继承的属性不会复制，即便该父类指定要复制。
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
 
	//复制当前生命值。
	DOREPLIFETIME(AThirdPersonMPCharacter, CurrentHealth);
}

void AThirdPersonMPCharacter::SetCurrentHealth(float healthValue)
{
	/*
	 *虽然并非所有变量都需要这样的"存值"函数，但对于游戏期间会频繁变化的敏感游戏进程变量，
	 *尤其是可被很多不同源修改的变量，这种函数很有用。对于类似的单机游戏和多人游戏，
	 *使用存值函数都是最佳做法，因为它使这些变量的实时更改更加一致，更便于调试，也更容易扩展新功能。
	 */
	if (GetLocalRole() == ROLE_Authority)
	{
		CurrentHealth = FMath::Clamp(healthValue, 0.f, MaxHealth);
		OnHealthUpdate();
	}
}

float AThirdPersonMPCharacter::TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float damageApplied = CurrentHealth - DamageTaken;
	SetCurrentHealth(damageApplied);
	return damageApplied;
}

void AThirdPersonMPCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AThirdPersonMPCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AThirdPersonMPCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AThirdPersonMPCharacter::Look);

		// Fire binding (Enhanced Input)
		if (FireAction)
		{
			UE_LOG(LogThirdPersonMP, Warning, TEXT("'%s' Bind fire action!"), *GetNameSafe(this));
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &AThirdPersonMPCharacter::StartFire);
			//EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AThirdPersonMPCharacter::StopFire);
		}
		else
		{
			UE_LOG(LogThirdPersonMP, Error, TEXT("'%s' Failed to find a FireAction!"), *GetNameSafe(this));
		}
	}
	else
	{
		UE_LOG(LogThirdPersonMP, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
 
}

void AThirdPersonMPCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AThirdPersonMPCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AThirdPersonMPCharacter::OnHealthUpdate()
{
	//客户端特定的功能
	if (IsLocallyControlled())
	{
		FString healthMessage = FString::Printf(TEXT("You now have %f health remaining."), CurrentHealth);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);
 
		if (CurrentHealth <= 0)
		{
			FString deathMessage = FString::Printf(TEXT("You have been killed."));
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, deathMessage);
		}
	}
 
	//服务器特定的功能
	if (GetLocalRole() == ROLE_Authority)
	{
		FString healthMessage = FString::Printf(TEXT("%s now has %f health remaining."), *GetFName().ToString(), CurrentHealth);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);
	}
 
	//在所有机器上都执行的函数。
	/*
		因任何因伤害或死亡而产生的特殊功能都应放在这里。
	*/
}

void AThirdPersonMPCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AThirdPersonMPCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AThirdPersonMPCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void AThirdPersonMPCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}
