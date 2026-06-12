// Copyright Epic Games, Inc. All Rights Reserved.

#include "IpvMultiCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Net/UnrealNetwork.h"    
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "ThirdPersonMPProjectile.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "IpvMulti.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameEngine.h"
#include "Engine/GameViewportClient.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Styling/CoreStyle.h"

AIpvMultiCharacter::AIpvMultiCharacter()
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
	
	//Initialize the player's Health  
	MaxHealth = 100.0f;    
	CurrentHealth = MaxHealth;
	
	//Initialize projectile class
	ProjectileClass = AThirdPersonMPProjectile::StaticClass();
	//Initialize fire rate
	FireRate = 0.25f;
	bIsFiringWeapon = false;

	HealthBarHiddenLevelNames.Add(TEXT("L_MainMenu"));
}

void AIpvMultiCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (bIsDead)
	{
		HandleDeathVisual();
	}

	if (IsLocallyControlled())
	{
		EnsureHealthBarWidget();
		UpdateHealthHUD();
	}
}

void AIpvMultiCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RemoveHealthBarWidget();
	Super::EndPlay(EndPlayReason);
}

void AIpvMultiCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (IsLocallyControlled())
	{
		EnsureHealthBarWidget();
		UpdateHealthHUD();
	}
}

void AIpvMultiCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

	if (IsLocallyControlled())
	{
		EnsureHealthBarWidget();
		UpdateHealthHUD();
	}
}

bool AIpvMultiCharacter::ShouldDisplayHealthBar() const
{
	if (!GetWorld())
	{
		return false;
	}

	const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);
	for (const FString& HiddenLevelName : HealthBarHiddenLevelNames)
	{
		if (CurrentLevelName.Equals(HiddenLevelName, ESearchCase::IgnoreCase))
		{
			return false;
		}
	}

	return true;
}

void AIpvMultiCharacter::EnsureHealthBarWidget()
{
	if (!IsLocallyControlled() || !ShouldDisplayHealthBar() || HealthBarSlate.IsValid())
	{
		return;
	}

	HealthBarSlate = SNew(SProgressBar)
		.Percent(1.f)
		.BarFillType(EProgressBarFillType::LeftToRight);

	HealthBarContainer = SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(FMargin(24.f, 24.f, 0.f, 0.f))
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
			.Padding(FMargin(4.f))
			[
				SNew(SBox)
				.WidthOverride(320.f)
				.HeightOverride(28.f)
				[
					HealthBarSlate.ToSharedRef()
				]
			]
		];

	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->AddViewportWidgetContent(HealthBarContainer.ToSharedRef(), 10);
	}
}

void AIpvMultiCharacter::RemoveHealthBarWidget()
{
	if (HealthBarContainer.IsValid() && GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(HealthBarContainer.ToSharedRef());
	}

	HealthBarContainer.Reset();
	HealthBarSlate.Reset();
}

void AIpvMultiCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AIpvMultiCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AIpvMultiCharacter::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AIpvMultiCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AIpvMultiCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AIpvMultiCharacter::Look);
	}
	else
	{
		UE_LOG(LogIpvMulti, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
	
	// Handle firing projectiles
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AIpvMultiCharacter::StartFire);
}

void AIpvMultiCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AIpvMultiCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AIpvMultiCharacter::DoMove(float Right, float Forward)
{
	if (bIsDead)
	{
		return;
	}

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

void AIpvMultiCharacter::DoLook(float Yaw, float Pitch)
{
	if (bIsDead)
	{
		return;
	}

	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AIpvMultiCharacter::DoJumpStart()
{
	if (bIsDead)
	{
		return;
	}

	// signal the character to jump
	Jump();
}

void AIpvMultiCharacter::DoJumpEnd()
{
	if (bIsDead)
	{
		return;
	}

	// signal the character to stop jumping
	StopJumping();
}

void AIpvMultiCharacter::OpenLobby()
{
	UWorld* World = GetWorld();
	if (World != nullptr)
	{
		World->ServerTravel("/Game/Levels/Lobby?listen");
	}
}

void AIpvMultiCharacter::CallOpenLevel(const FString& Address)
{
	UGameplayStatics::OpenLevel(this, *Address);
}

void AIpvMultiCharacter::CallClientTravel(const FString& Address)
{
	APlayerController* PlayerController = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
	if (PlayerController != nullptr)
	{
		PlayerController->ClientTravel(Address, TRAVEL_Absolute);
	}
}

// Replicated Properties    
void AIpvMultiCharacter::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const    
{       
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);   
	DOREPLIFETIME(AIpvMultiCharacter, CurrentHealth);
	DOREPLIFETIME(AIpvMultiCharacter, bIsDead);
}

void AIpvMultiCharacter::UpdateHealthHUD()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	if (!ShouldDisplayHealthBar())
	{
		RemoveHealthBarWidget();
		return;
	}

	EnsureHealthBarWidget();

	if (HealthBarSlate.IsValid())
	{
		const float Percent = MaxHealth > 0.f ? FMath::Clamp(CurrentHealth / MaxHealth, 0.f, 1.f) : 0.f;
		HealthBarSlate->SetPercent(Percent);
	}
}

void AIpvMultiCharacter::OnHealthUpdate()
{
	UpdateHealthHUD();
}

void AIpvMultiCharacter::OnRep_CurrentHealth()
{
	OnHealthUpdate();
}

void AIpvMultiCharacter::OnRep_bIsDead()
{
	if (bIsDead)
	{
		HandleDeathVisual();
	}
}

void AIpvMultiCharacter::SetCurrentHealth(float healthValue)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (bIsDead)
		{
			return;
		}

		CurrentHealth = FMath::Clamp(healthValue, 0.f, MaxHealth);

		if (CurrentHealth <= 0.f)
		{
			HandleDeath();
		}

		OnHealthUpdate();
	}
}

void AIpvMultiCharacter::HandleDeath()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;
	HandleDeathVisual();
}

void AIpvMultiCharacter::HandleDeathVisual()
{
	if (bDeathVisualApplied)
	{
		return;
	}

	bDeathVisualApplied = true;

	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (USkeletalMeshComponent* MeshComponent = GetMesh())
	{
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		MeshComponent->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
		MeshComponent->SetSimulatePhysics(true);
	}

	if (IsLocallyControlled())
	{
		DisableInput(Cast<APlayerController>(GetController()));
	}

	UpdateHealthHUD();
}

float AIpvMultiCharacter::TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (GetLocalRole() != ROLE_Authority || bIsDead)
	{
		return 0.f;
	}

	const float DamageApplied = FMath::Min(DamageTaken, CurrentHealth);
	SetCurrentHealth(CurrentHealth - DamageTaken);
	return DamageApplied;
}

void AIpvMultiCharacter::StartFire()
{
	if (bIsDead || bIsFiringWeapon)
	{
		return;
	}

	bIsFiringWeapon = true;
	UWorld* World = GetWorld();
	World->GetTimerManager().SetTimer(FiringTimer, this, &AIpvMultiCharacter::StopFire, FireRate, false);
	HandleFire();
}
 
void AIpvMultiCharacter::StopFire()
{
	bIsFiringWeapon = false;
}
 
void AIpvMultiCharacter::HandleFire_Implementation()
{
	if (bIsDead)
	{
		return;
	}

	FVector spawnLocation = GetActorLocation() + ( GetActorRotation().Vector()  * 100.0f ) + (GetActorUpVector() * 50.0f);
	FRotator spawnRotation = GetActorRotation();
 
	FActorSpawnParameters spawnParameters;
	spawnParameters.Instigator = GetInstigator();
	spawnParameters.Owner = this;
 

	if (ProjectileClass)
	{
		GetWorld()->SpawnActor<AThirdPersonMPProjectile>(ProjectileClass, spawnLocation, spawnRotation, spawnParameters);
	}
}
