## GameInstance
- The `GameInstance` is a per-machine object that's always available through the `UWorld`. This latter is always accessible via `AController`/`AActor`. This implies that, in local multiplayer (split screen or not), you're going to have a single game instance.
- In general you put in the `GameInstance` things that you want to be global, like the `GameState`, the `HUD` (it's ok since the )
- `GameInstance` is not meant for replication, it inherits from `UObject`, not from `AActor`!!

## GameMode
- Only the server has a `GameMode`, it's not retriveable from the clients! This is why most of the Tantrum logic we previously had on the `GameMode` must be transfered to the `GameInstance`. 

## GameState
- Inherits from `AActor`, so it can be replicated.
- Holds values that relate to the actual state of the game (game is paused, game is over, global timer, ...) 

## PlayerState
- Not available for AI (NPCs and Enemies alike).
- It is what allows us to have the race keep going for one player while it's over for the other player. 
- In general, it holds specific player values related to the game (score, win condition, ...)