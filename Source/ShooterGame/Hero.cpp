// Fill out your copyright notice in the Description page of Project Settings.


#include "Hero.h"

#include "DrawDebugHelpers.h"
#include "Enemy.h"
#include "SAWidget.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AHero::AHero()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>("Arm");
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = 0;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>("Camera");
	CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);


	HP = 100;
	Coins = 0;
	Caps = 0;
	RightClickment = false;
}

// Called when the game starts or when spawned
void AHero::BeginPlay()
{
	Super::BeginPlay();

	GameHUD = Cast<AGameHUD>(UGameplayStatics::GetPlayerController(this->GetOwner(), 0)->GetHUD());

	// Kontrola kolize CapsuleComponent
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AHero::OnCollide);
	GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &AHero::OnStopCollision);
}

// Called every frame
void AHero::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Yellow, FString::Printf(TEXT("Coins: %i"), Coins), true);
	//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(TEXT("Caps: %i"), Caps), true);

	if (RightClickment)
	{
		RightClick();
	}
	
	if (IsValid(GameHUD))
	{
		//!!!!!! NEVIM JINAK :DDDD !!!!!!!
		GameHUD->InGameWidget->SetHP(HP);
	}
	
	IsDead();

	FVector Location = GetActorLocation() + (CameraComponent->GetForwardVector() * 20);
	FVector ForwardVector = GetActorLocation() + (CameraComponent->GetForwardVector() * 10000);


	FHitResult HitResult;
	if (GetWorld()->LineTraceSingleByChannel(
		HitResult, Location,
		ForwardVector, ECC_LookAt
	))
	{
		if (HitResult.Actor.IsValid())
		{
			FString FullName = HitResult.Actor->GetName().ToLower();
			//UE_LOG(LogTemp, Warning, TEXT("Hit: %s"), *FullName)
		}
	}

	if (HitResult.Location.IsZero())
	{
		//DrawDebugLine(GetWorld(), Location, ForwardVector, FColor::Yellow, false, 0, 0, 1);
	}
	else
	{
		//DrawDebugLine(GetWorld(), Location, HitResult.Location, FColor::Yellow, false, 0, 0, 1);
	}
}

// Called to bind functionality to input
void AHero::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("Forward", this, &AHero::MoveForward);
	PlayerInputComponent->BindAxis("Sideways", this, &AHero::MoveSideways);
	PlayerInputComponent->BindAxis("MouseMovementX", this, &AHero::AddControllerYawInput);
	PlayerInputComponent->BindAxis("MouseMovementY", this, &AHero::AddControllerPitchInput);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("LeftClick", IE_Pressed, this, &AHero::LeftClick);
	PlayerInputComponent->BindAction("RightClick", IE_Pressed, this, &AHero::RightClick);
	PlayerInputComponent->BindAction("RightClick", IE_Released, this, &AHero::StopRightClick);


	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AHero::Interact);
}

void AHero::MoveForward(float Scale)
{
	AddMovementInput(GetActorForwardVector(), Scale);
}

void AHero::MoveSideways(float Scale)
{
	AddMovementInput(GetActorRightVector(), Scale);
}

void AHero::OnCollide(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                      int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Colliding with: %s"), *OtherActor->GetName());

	FString Actor = OtherActor->GetName().ToLower();

	if (Actor.Contains("cap"))
	{
		Caps += 1;
		OtherActor->Destroy();
	}

	if (Actor.Contains("coin"))
	{
		Coins += 1;
		OtherActor->Destroy();
	}

	if (Actor.Contains("kill"))
	{
		FVector Location = GetActorLocation();
		Location.Set(0, 0, 0);
		SetActorLocation(Location);
	}
}

void AHero::OnStopCollision(UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
                            class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// UE_LOG(LogTemp, Warning, TEXT("Collision stopped with: %s"), *OtherActor->GetFullName());
}

void AHero::LeftClick()
{
	FVector Location = GetActorLocation() + (CameraComponent->GetForwardVector() * 20);
	FVector ForwardVector = GetActorLocation() + (CameraComponent->GetForwardVector() * 10000);

	FHitResult HitResult;
	if (GetWorld()->LineTraceSingleByChannel(HitResult, Location, ForwardVector, ECC_LookAt))
	{
		if (HitResult.Actor.IsValid())
		{
			FString FullName = HitResult.Actor->GetName().ToLower();
			UE_LOG(LogTemp, Warning, TEXT("SHOT: %s"), *FullName)
			//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, FString::Printf(TEXT("SHOT: %s"), *FullName), true);
		}
	}


	if (!HitResult.Location.IsZero() && HitResult.Actor.IsValid())
	{
		DrawDebugBox(GetWorld(), HitResult.ImpactPoint, FVector(4, 4, 4), FColor::Red, false, 1, 0, 1);
		DrawDebugLine(GetWorld(), Location, HitResult.Location, FColor::Red, false, 1, 0, 1);

		if (HitResult.Actor->GetName().ToLower().Contains("friend"))
		{
			AEnemy* Enemy = Cast<AEnemy>(HitResult.GetActor());
			FVector Direction = (HitResult.GetActor()->GetActorLocation()) - HitResult.ImpactPoint;
			Enemy->StaticMeshComponent->AddImpulse(Direction * 10, NAME_None, true);
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Cyan,
			                                 FString::Printf(TEXT("Why would you shoot friend :(")), false);
			HP -= 5;
			LaunchCharacter(ForwardVector, false, false);
		}

		if (HitResult.Actor->GetName().ToLower().Contains("hurtboard"))
		{
			HP -= 5;
			LaunchCharacter(ForwardVector*-0.05, true, true);
		}
		
		if (HitResult.Actor->GetName().ToLower().Contains("enemy"))
		{
			AEnemy* Enemy = Cast<AEnemy>(HitResult.GetActor());
			FVector Direction = (HitResult.GetActor()->GetActorLocation()) - HitResult.ImpactPoint;
			Enemy->StaticMeshComponent->AddImpulse(Direction * 10, NAME_None, true);
			Enemy->HP -= 25;
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, FString::Printf(TEXT("Target HP: %i"), Enemy->HP),
			                                 false);
			Enemy->IsDead();
		}
	}
}

void AHero::RightClick()
{
	FVector Location = GetActorLocation() + (CameraComponent->GetForwardVector() * 20);
	FVector ForwardVector = GetActorLocation() + (CameraComponent->GetForwardVector() * 10000);

	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(HitResult, Location, ForwardVector, ECC_LookAt);

	UE_LOG(LogTemp, Warning, TEXT("Hooked!"));

	if (!HitResult.Location.IsZero() && HitResult.Actor.IsValid())
	{
		if (HitResult.Actor->GetName().ToLower().Contains("hook"))
		{
			if (RightClickment == false)
			{
				HookStart = Location;
				HookEnd = HitResult.ImpactPoint;
				//DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 8, 10,FColor::Blue, false, 1, 0, 1);
				//DrawDebugLine(GetWorld(), Location, HitResult.Location, FColor::Blue, false, 1, 0, 1);
			}
			RightClickment = true;
			DrawDebugSphere(GetWorld(), HookEnd, 8, 10,FColor::Blue, false, 0.1, 0, 1);
			DrawDebugLine(GetWorld(), Location, HookEnd, FColor::Blue, false, 0.1, 1, 1);
			LaunchCharacter(ForwardVector * 0.08, true, true);
			
		}
		if (!HitResult.Actor->GetName().ToLower().Contains("hook"))
		{
		RightClickment = false;
		}
	}
}

void AHero::StopRightClick()
{
	RightClickment = false;
}


void AHero::Interact()
{
	FString InteractedWith = "WIP";
	UE_LOG(LogTemp, Warning, TEXT("SHOT: %s"), *InteractedWith);
}

void AHero::IsDead()
{
	if (HP <= 0)
	{
		Destroy();
	}
}
