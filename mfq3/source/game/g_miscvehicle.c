/*
 * $Id: g_miscvehicle.c,v 1.1 2001-11-15 21:35:14 thebjoern Exp $
*/


#include "g_local.h"


//------------------------------------------------------------------
//
// MISC_VEHICLE
//
//------------------------------------------------------------------

void misc_vehicle_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath )
{
	if( self->score && attacker && attacker->client ) {
		vec3_t	pos;
		int score;
		VectorAdd( self->r.absmin, self->r.absmax, pos );
		VectorScale( pos, 0.5f, pos );
		if( g_gametype.integer >= GT_TEAM ) {
			if( self->s.generic1 == attacker->client->ps.persistant[PERS_TEAM] ) score = -self->score;
			else if( self->s.generic1 != attacker->client->ps.persistant[PERS_TEAM] ) score = self->score;
		} else {
			score = self->score;
		}

		AddScore( attacker, pos, score );
	}

	ExplodeVehicle(self);
	self->freeAfterEvent = qtrue;

	G_RadiusDamage( self->r.currentOrigin, self, 150, 150, self, MOD_VEHICLEEXPLOSION );

	trap_LinkEntity( self );
	
}

static void faceFirstWaypoint( gentity_t* ent ) {
	
	vec3_t	dir;

	if( !ent->nextWaypoint ) return;

	VectorSubtract( ent->nextWaypoint->pos, ent->s.origin, dir );
	VectorNormalize( dir );
	vectoangles( dir, ent->s.angles );
	VectorCopy( ent->s.angles, ent->s.apos.trBase );
}

static void SP_misc_plane( gentity_t *ent )
{
	unsigned int vehIndex = ent->s.modelindex;
	qboolean landed = qfalse;
	trace_t	trace;
	vec3_t	startpos, endpos, startorg, forward;
	gentity_t *test;

	VectorCopy( ent->s.origin, startorg );
	VectorCopy( ent->s.origin, startpos );
	startpos[2] += 64;
	VectorCopy( ent->s.origin, endpos );
	endpos[2] -= 256;
	trap_Trace (&trace, startpos, NULL, NULL, endpos, ENTITYNUM_NONE, MASK_SOLID );
	if( trace.entityNum != ENTITYNUM_NONE ) {
		test = &g_entities[trace.entityNum];
		if( canLandOnIt(test) ) {
			landed = qtrue;
			ent->s.origin[2] = test->r.maxs[2] - availableVehicles[vehIndex].mins[2] + 
				availableVehicles[vehIndex].gearheight;
		}			
	}
	// set flags
	ent->s.ONOFF = OO_NOTHING_ON;

	// gearheight
	if( availableVehicles[vehIndex].caps & HC_GEAR ) {
		ent->gearheight = (float)availableVehicles[vehIndex].gearheight;
	} else {
		ent->s.ONOFF |= OO_GEAR;
	}

	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );

	// check if it is a drone
	if( ent->targetname ) {
		char filename[MAX_FILEPATH];
		char buffer[33];
		trap_Cvar_VariableStringBuffer("mapname", buffer, 32);
		Com_sprintf( filename, MAX_FILEPATH, "dronefiles/%s/%s.drone", buffer, ent->targetname );
		if( LoadVehicleScripts( ent, filename ) ) {
			if( !(ent->s.ONOFF & OO_LANDED) && landed ) landed = qfalse;
			faceFirstWaypoint(ent);
			ent->s.eFlags |= EF_PILOT_ONBOARD;
			ent->s.pos.trType = TR_LINEAR;//TR_INTERPOLATE;
			ent->s.apos.trType = TR_LINEAR;//TR_INTERPOLATE;
			ent->think = Drone_Plane_Think;
			ent->nextthink = level.time + 100;
		} else {
			ent->idxScriptBegin = ent->idxScriptEnd = -1;
		}
	} 

	// speed, throttle depend on landed/airborne 
	if( landed ) {
		ent->speed = 0;
		ent->s.frame = 0;
		ent->s.ONOFF |= OO_LANDED|OO_STALLED|OO_GEAR|OO_COCKPIT;
		ent->s.angles[0] = ent->s.apos.trBase[0] = 0;
	}
	else if( ent->s.eFlags & EF_PILOT_ONBOARD ) {			// only drones can fly
		ent->speed = availableVehicles[vehIndex].maxspeed*0.7f;// 70% of topspeed
		ent->s.frame = MF_THROTTLE_MILITARY;				// thrust
		ent->s.origin[2] = ent->s.pos.trBase[2] = ent->r.currentOrigin[2] = startorg[2];
		AngleVectors( ent->s.angles, forward, 0, 0 );
		VectorScale( forward, ent->speed, ent->s.pos.trDelta );
	}

	ent->s.pos.trTime = level.time;
	trap_LinkEntity (ent);

}

static void SP_misc_gv( gentity_t *ent )
{
	unsigned int vehIndex = ent->s.modelindex;
	trace_t	trace;
	vec3_t	startpos, endpos;
	gentity_t *test;

	VectorCopy( ent->s.origin, startpos );
	startpos[2] += 64;
	VectorCopy( ent->s.origin, endpos );
	endpos[2] -= 512;
	trap_Trace (&trace, startpos, NULL, NULL, endpos, ENTITYNUM_NONE, MASK_SOLID );
	if( trace.entityNum != ENTITYNUM_NONE ) {
		test = &g_entities[trace.entityNum];
		ent->s.origin[2] = trace.endpos[2] - availableVehicles[vehIndex].mins[2] + 1;
	}

	// set flags
	ent->s.ONOFF = OO_LANDED;

	ent->speed = 0;
	ent->s.frame = 0;// throttle
	
	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );

	// check if it is a drone
	if( ent->targetname ) {
		char filename[MAX_FILEPATH];
		char buffer[33];
		trap_Cvar_VariableStringBuffer("mapname", buffer, 32);
		Com_sprintf( filename, MAX_FILEPATH, "dronefiles/%s/%s.drone", buffer, ent->targetname );
		if( LoadVehicleScripts( ent, filename ) ) {
			faceFirstWaypoint(ent);
			ent->s.eFlags |= EF_PILOT_ONBOARD;
			ent->s.pos.trType = TR_LINEAR;//TR_INTERPOLATE;
			ent->s.apos.trType = TR_LINEAR;//TR_INTERPOLATE;
			ent->think = Drone_Plane_Think;
			ent->nextthink = level.time + 100;
		} else {
			ent->idxScriptBegin = ent->idxScriptEnd = -1;
		}

	} 

	trap_LinkEntity (ent);

	// set functions
//	ent->touch = Touch_Plane;
//	ent->pain = Plane_Pain;
}

void DroneInit()
{
	unsigned int i;
	unsigned long cat;

	// finish drone init
	for( i = 0; i < MAX_GENTITIES; ++i ) {
		if( Q_stricmp( (&g_entities[i])->classname, "misc_vehicle" ) == 0 ) {
			cat = availableVehicles[(&g_entities[i])->s.modelindex].id & CAT_ANY;
			if( cat & CAT_PLANE ) {
				SP_misc_plane( &g_entities[i] );
			} else {
				SP_misc_gv( &g_entities[i] );
			}
		}	
	}
}

/*QUAKED misc_vehicle (0 .5 .8) (-32 -32 0) (32 32 32)
*/
void SP_misc_vehicle( gentity_t *sp_ent ) 
{
	char modelname[128];
	int i, k;
	unsigned long gameset = G_GetGameset();
	qboolean found = qfalse;
	unsigned long cat;
	gentity_t* ent = G_Spawn();

	ent->model = sp_ent->model;
	ent->targetname = sp_ent->targetname;
	VectorCopy( sp_ent->s.origin, ent->s.origin );
	VectorCopy( sp_ent->s.angles, ent->s.angles );
	ent->health = sp_ent->health;

	// is it a random one ?
	if( strcmp( ent->model, "randomplane" ) == 0 ||
		strcmp( ent->model, "randomground" ) == 0 ) {
		int j = 0;
		if( strcmp( ent->model, "randomplane" ) == 0 ) cat = CAT_PLANE;
		else /*if( strcmp( ent->model, "randomground" ) == 0 )*/ cat = CAT_GROUND;
		j = 0;
		for( i = 0; i < bg_numberOfVehicles; ++i ) {
			if( (availableVehicles[i].id&CAT_ANY & cat) &&
				(availableVehicles[i].id&MF_GAMESET_ANY & gameset ) ) ++j;
		}
		k = rand()%j+1;
		j = 0;
		for( i = 0; i < bg_numberOfVehicles; ++i ) {
			if( (availableVehicles[i].id&CAT_ANY & cat) &&
				(availableVehicles[i].id&MF_GAMESET_ANY & gameset ) ) {
				++j;
				// found it
				if( j == k ) {
					found = qtrue;
					break;
				}
			}
		}

	} else {
		// is it in the list ?
		for( i = 0; i < bg_numberOfVehicles; ++i ) {
			// vehicle is in list
			if( strcmp( availableVehicles[i].modelName, ent->model ) == 0 ) {
				if( gameset & (availableVehicles[i].id&MF_GAMESET_ANY) ) {
					found = qtrue;
				}
				break;
			}
		}
	}
	if( !found ) {
		G_Printf ("%s (%s) cannot be spawned\n", ent->classname, ent->model);
		G_FreeEntity(ent);
		return;
	}
	cat = (availableVehicles[i].id&CAT_ANY);
	if( cat & CAT_PLANE ) {
		Com_sprintf(modelname, 127, "models/vehicles/planes/%s/%s.md3", availableVehicles[i].modelName,
			availableVehicles[i].modelName );
	} else {
		Com_sprintf(modelname, 127, "models/vehicles/ground/%s/%s.md3", availableVehicles[i].modelName,
			availableVehicles[i].modelName );
	}
	G_Printf( "Spawning a %s\n", modelname );

	ent->s.eType = ET_MISC_VEHICLE;
	ent->s.modelindex = i;

	if( ent->team && g_gametype.integer >= GT_TEAM ) {
		if ( Q_stricmp( ent->team, "red" ) == 0 ) {
			ent->s.generic1 = TEAM_RED;
		} else if ( Q_stricmp( ent->team, "blue" ) == 0 ) {
			ent->s.generic1 = TEAM_BLUE;	
		}
	}

	VectorCopy(availableVehicles[i].mins, ent->r.mins);
	VectorCopy(availableVehicles[i].maxs, ent->r.maxs);
	G_SetOrigin( ent, ent->s.origin );
	VectorCopy( ent->s.angles, ent->s.apos.trBase );
	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );

	ent->s.groundEntityNum = ENTITYNUM_NONE;
	ent->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	ent->s.pos.trType = TR_STATIONARY;
	ent->s.apos.trType = TR_STATIONARY;
	if( ent->health <= 0 ) ent->health = availableVehicles[i].maxhealth;
	ent->takedamage = qtrue;
	ent->inuse = qtrue;
	ent->die = misc_vehicle_die;
	if( !ent->score ) ent->score = 1;
	ent->classname = "misc_vehicle";
	ent->r.contents = CONTENTS_SOLID;//CONTENTS_BODY;
	ent->clipmask = MASK_PLAYERSOLID;

	ent->idxScriptBegin = ent->idxScriptEnd = -1;

	trap_LinkEntity (ent);

	G_FreeEntity(sp_ent);
}
