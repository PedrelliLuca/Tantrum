// Fill out your copyright notice in the Description page of Project Settings.


#include "ThrowableSystem.h"

void UThrowableSystem::AddThrowable(TWeakObjectPtr<AThrowable> throwable) {
	check(throwable.IsValid());
	_throwables.Emplace(throwable);
	throwable->OnDestroyed.AddDynamic(this, &UThrowableSystem::_throwableDestroyed);
}

TWeakObjectPtr<AThrowable> UThrowableSystem::GetThrowableClosestTo(const FVector& location) {
	TWeakObjectPtr<AThrowable> closest = nullptr;
	float smallestDistance = TNumericLimits<float>::Max();

	for (const auto& throwable : _throwables) {
		check(throwable.IsValid());

		if (!closest.IsValid()) { // 1st iteration
			closest = throwable;
			smallestDistance = (closest->GetActorLocation() - location).Size();
		}

		const float throwableDistance = (throwable->GetActorLocation() - location).Size();
		if (throwableDistance < smallestDistance) {
			smallestDistance = throwableDistance;
			closest = throwable;
		}
	}

	return closest;
}

void UThrowableSystem::_throwableDestroyed(AActor* const destroyedThrowableAsActor) {
	const auto destroyedThrowable = Cast<AThrowable>(destroyedThrowableAsActor);
	destroyedThrowable->OnDestroyed.RemoveAll(this);

	_throwables.Remove(destroyedThrowable);
}
