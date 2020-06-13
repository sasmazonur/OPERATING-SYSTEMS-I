# Adventure
## Description
This program implements a simple game akin to old text adventure games like Adventure using C and Unix:
http://en.wikipedia.org/wiki/Colossal_Cave_Adventure

This program split up into two C programs. The first program will be contained in `buildrooms.c`, which when compiled with the same name (minus the extension) and run creates a series of files that hold descriptions of the in-game rooms and how the rooms are connected.

The second program will be called `adventure.c` and when compiled with the same name and run provides an interface for playing the game using the most recently generated rooms.

In the game, the player will begin in the `"starting room"` and will win the game automatically upon entering the `"ending room"`, which causes the game to exit, displaying the path taken by the player.

During the game, the player can also enter a command that returns the current time - this functionality utilizes mutexes and multithreading.

#### Room Name
- A room name cannot be assigned to more than one room.
- Each name can be at max 8 characters long, with only uppercase and lowercase letters allowed (thus, no numbers, special characters, or spaces). This restriction is not extended to the room file's filename.
- You must hard code a list of ten different Room Names into your rooms program and have your rooms program randomly assign one of these to each room generated. Thus, for a given run of your rooms program, 7 of the 10 hard-coded room names will be used.
#### Room Type
- The possible room type entries are: START_ROOM, END_ROOM, and MID_ROOM.
- The assignment of which room gets which type should be randomly generated each time the rooms program is run.
- Naturally, only one room should be assigned the START_ROOM type, and only one room should be assigned the END_ROOM type. The rest of the rooms will receive the MID_ROOM type.

#### Outbound connections to other rooms
- There must be at least 3 outbound connections and at most 6 outbound connections from this room to other rooms.
- The oubound connections from one room to other rooms should be assigned randomly each time the rooms program is run.
- Outbound connections must have matching connections coming back: if room A connects to room B, then room B must have a connection back to room A. Because of all of these specs, there will always be at least one path through.
- A room cannot have an outbound connection that points to itself.
- A room cannot have more than one outbound connection to the same room.
