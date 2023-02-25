// Fill out your copyright notice in the Description page of Project Settings.


#include "TantrumLocalMPCamera.h"

static TAutoConsoleVariable<bool> CVarDrawMidPoint(
	TEXT("Tantrum.Camera.Debug.DrawMidPoint"),
	true,
	TEXT("Draw MidPoint between Players"),
	ECVF_Default
);

ATantrumLocalMPCamera::ATantrumLocalMPCamera() {
	PrimaryActorTick.bCanEverTick = true;

	_cameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	RootComponent = _cameraBoom; 
	_cameraBoom->TargetArmLength = 400.0f;

	// Camera creation
	_followCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	_followCamera->FieldOfView = 90.0f;
	_followCamera->bConstrainAspectRatio = true;
	_followCamera->AspectRatio = 1.777778f;
	_followCamera->PostProcessBlendWeight = 1.0f;

	_followCamera->SetupAttachment(_cameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	_followCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
}

void ATantrumLocalMPCamera::Tick(float deltaSeconds) {
	Super::Tick(deltaSeconds);

	auto maxDistSq = 0.0f;
	auto midPoint = FVector::ZeroVector;
	auto lastPosition = FVector::ZeroVector;
	int32 numPlayers = 0;

	// Getting:
	// 1. maxDistSq, i.e. the largest gap between any two players in the world. The greater the distance, the more the camera should zoom out.
	// 2. midPoint, i.e. the sum of all the positions of the player controlled pawns divided by the num of players.
	for (auto iterator = GetWorld()->GetPlayerControllerIterator(); iterator; ++iterator) {
		const auto playerC = iterator->Get();
		if (playerC && playerC->PlayerState) {
			const auto pawnPosition = playerC->GetPawn()->GetActorLocation();
			if (!lastPosition.IsNearlyZero()) {
				const auto distSq = (pawnPosition - lastPosition).SizeSquared();
				if (distSq > maxDistSq) {
					maxDistSq = distSq;
				}
			}

			midPoint += pawnPosition;
			lastPosition = pawnPosition;
			++numPlayers;
		}
	}

	midPoint /= numPlayers > 0 ? static_cast<float>(numPlayers) : 1.0f;

	if (CVarDrawMidPoint->GetBool()) {
		DrawDebugSphere(GetWorld(), midPoint, 25.0f, 10, FColor::Blue);
	}

	const auto maxDist = maxDistSq > KINDA_SMALL_NUMBER ? FMath::Min(FMath::Sqrt(maxDistSq), _maxPlayerDistance) : 0.0f;

	// Translating the relation between the player distance and the [min, max] player distance interval into...
	const auto distanceRatio = maxDist > _minPlayerDistance ? (maxDist - _minPlayerDistance) / (_maxPlayerDistance - _minPlayerDistance) : 0.0f;
	// ... a relation between the arm length and the [min, max] arm length interval.
	_cameraBoom->TargetArmLength = FMath::Lerp(_minArmLength, _maxArmLength, distanceRatio);
}

// Called when the game starts or when spawned
void ATantrumLocalMPCamera::BeginPlay() {
	Super::BeginPlay();
	
	_tantrumGameMode = Cast<ATantrumGameModeBase>(GetWorld()->GetAuthGameMode());
}
