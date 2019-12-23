# ShooterGame

The purpose of this project is to replace the plasma launcher with a sticky grenade 
similar to Halo Type-1 plasma grenade.

'Q' is the key to switch the current weapon.
[default : ShooterWeapon_Instant, sub : ShooterWeapon_StickyGrenade]

If the current weapon is a StickyGrenade, it can be spawned when user clicks mouse-left button.
There is one replicated variable whose data type is Enum, and the value is determined in only server.
if the value is changed, it will be replicated to clients.

Bounce counts decide when the movement of the grenade stops. 
Default value is 2, so if the grenade bounces twice, 
it will stop immediately and the state of the grenade will be transitioned to the next state.

I prepended the Client, Server keyword to the beginning of the functions 
to make sure what machines this function will be called on during a multiplayer session.
