// Copyright Epic Games, Inc. All Rights Reserved.

#include "TantrumGameInstance.h"

// void UTantrumGameInstance::DisplayCountdown(const float gameCountdownDuration, ATantrumPlayerController* const tantrumPlayerController) {
//     if (!GetWorld()) {
//         return;
//     }
//
//     UTantrumGameWidget* gameWidget = nullptr;
//     const auto maybeGameWidget = _gameWidgets.Find(tantrumPlayerController);
//     if (!maybeGameWidget) {
//         gameWidget = CreateWidget<UTantrumGameWidget>(tantrumPlayerController, _gameWidgetClass);
//         if (IsValid(gameWidget)) {
//             gameWidget->AddToPlayerScreen();
//             _gameWidgets.Add(tantrumPlayerController, gameWidget);
//         }
//     } else {
//         gameWidget = *maybeGameWidget;
//     }
//
//     if (IsValid(gameWidget)) {
//         gameWidget->StartCountdown(gameCountdownDuration, tantrumPlayerController);
//         _tantrumGameStateBase = GetWorld()->GetGameState<ATantrumGameStateBase>();
//         if (!IsValid(_tantrumGameStateBase)) {
//             GetWorld()->GameStateSetEvent.AddUObject(this, &UTantrumGameInstance::_onGameStateSet);
//         }
//     }
// }
//
// void UTantrumGameInstance::DisplayLevelComplete(ATantrumPlayerController* tantrumPlayerController) {
//     const auto maybeGameWidget = _gameWidgets.Find(tantrumPlayerController);
//     if (maybeGameWidget) {
//         (*maybeGameWidget)->DisplayResults();
//     }
// }
//
//// We get here by clicking on the "Retry" button of the widget
// void UTantrumGameInstance::OnRetrySelected(ATantrumPlayerController* tantrumPlayerController) {
//     const auto maybeGameWidget = _gameWidgets.Find(tantrumPlayerController);
//     if (maybeGameWidget) {
//         // Simple cleanup of the UI
//         RestartGame(tantrumPlayerController);
//         // This is the real restart, performed via GameMode (see function implementation). The GameMode can be accessed only via server,
//         // that's the machine that has authority on it, and GameMode is not replicated.
//         tantrumPlayerController->ServerRestartLevel();
//     }
// }
//
// void UTantrumGameInstance::RestartGame(ATantrumPlayerController* tantrumPlayerController) {
//     const auto gameWidget = _gameWidgets.Find(tantrumPlayerController);
//     if (gameWidget) {
//         (*gameWidget)->RemoveResults();
//         // restore game input
//         FInputModeGameOnly inputMode;
//         tantrumPlayerController->SetInputMode(inputMode);
//         tantrumPlayerController->SetShowMouseCursor(false);
//     }
// }
//
// void UTantrumGameInstance::_onGameStateSet(AGameStateBase* gameStateBase) {
//     _tantrumGameStateBase = Cast<ATantrumGameStateBase>(gameStateBase);
// }
