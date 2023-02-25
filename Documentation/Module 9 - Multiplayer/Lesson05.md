# Authority
For Player-Controlled characters, the true ones belong to the machine where the Player Controller controlling them is. For example, Character #3 belongs to PC #3 and is replicated on the other machines, Server included.

For characters/pawns/actors that are not Player-Controlled, they all belong to the Server and are replicated on every single client. In our case, the throwable spheres belong to the server.  
This implies that the replication occurs only if the throwable moves on the server. If a client moves their replica of the throwable, the server doesn't replicate that movement. This is why, when we pull the throwable, we have to call the AThrowable::Pull() function on the server and not on the client.

Pointers are memory addresses. You won't have the same memory addresses on different machines, because the memory layouts will differ. Ptrs replication is managed by Unreal under the hood.