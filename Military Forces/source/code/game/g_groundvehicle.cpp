#include "g_groundvehicle.h"
#include "g_local.h"



void
Entity_GroundVehicle::Entity_GroundVehicle_Pain::execute( GameEntity* attacker, int damage )
{
	if( damage < 5 ) 
		return;
	if( damage > 50 ) 
		damage = 50;

	G_AddEvent( self_, EV_VEHICLE_HIT, damage, true );
}





