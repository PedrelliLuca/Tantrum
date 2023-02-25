# Online Multiplayer and Networking
Very useful [crash course](https://cedric-neukirchen.net/Downloads/Compendium/UE4_Network_Compendium_by_Cedric_eXi_Neukirchen.pdf).

Of the three kinds of multiplayer (Split Screen, Local, Online), online is by far the most complex one. Why? 

In local multiplayer scenarios, the world stays the same: in Split Screen you have multiple Viewports, in Local is one camera capturing all the action. There is a single Game Instance, on a single machine that all players are on.

In Online multiplayer, there are multiple Game Instances, one for every player's machine. The key here is to keep things in sync: you don't want different players to see different things, i.e. to have different Game States than others.

## The Client-Server Model
[Documentation](https://docs.unrealengine.com/4.27/en-US/InteractiveExperiences/Networking/Overview/). 1 authority (the server), all clients connect to it. When a client needs to send a message, it cannot communicate with other clients directly. The client sends the message to the server, which then relays it to anybody else.

Dedicated Server: machine that is always on and doesn't run any graphics, just keeps the world information. The clients do much more processing.
Listening Server: one of the players will act as authority and server. 

Generally speaking, there are more client heavy and more server heavy models. Server heavy: a lot of the replication exists on the server, the clients are just listening to commands. Client heavy: client runs a lot of commands, much more responsive then Server heavy but makes cheating really easy.

Connection process: client requests a connection. If the server accepts, it sends the map to the client, which will then load it.

## RPCs (Remote Procedure Calls)
[Documentation](https://docs.unrealengine.com/4.27/en-US/InteractiveExperiences/Networking/Actors/RPCs/).

An RPC is a function that can run on either a client, or a server, or a multicast (it's broadcasted). In this latter case, it can only be fired from the server, clients cannot communicate to other clients. By default RPCs are unreliable, you need to make the reliable by using a UPROPERTY() modifier.

Something like firing has to be **reliable**, because it happens once, when the player hits the trigger. If that packet is missed, the game goes out of sync, and there's nothing that could potentially correct it.  
This is not the case of movement for example, which can be **unrealiable**: the player's velocity is going to be updated every single frame, so if you miss 1 or 2 packets it's fine: a few frames later, you're gonna find out where the player is and what their speed is.

Having reliable RPCs inside a Tick() function is very bad, since these calls are really expansive. Movement is tick-related but, as we said, it can be unreliable, so it's fine, we can always correct it in the next frames.

## Property Replication
[Documentation](https://docs.unrealengine.com/4.27/en-US/InteractiveExperiences/Networking/Actors/Properties/).

RPCs are function calls, like FireWeapon() or MoveTo(). Health is an attribute, a property, not a function call. You need to use property replication for member variables.

Player **on the server** changes health => the server will tell all other clients of the new health. The client that changes health cannot say so to other clients directly.