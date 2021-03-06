#include "bg_datamanager.h"
#include "bg_vehicleinfo.h"
#include "bg_md3utils.h"
#include "../game/q_shared.h"
#include "../qcommon/qfiles.h"
#include "../game/bg_public.h"

// decls
int FS_FOpenFileByMode( const char *qpath, fileHandle_t *f, fsMode_t mode );
int FS_Read2( void *buffer, int len, fileHandle_t f );
void FS_FCloseFile( fileHandle_t f );



GameObjectInfo_Aircraft::GameObjectInfo_Aircraft() :
	engines_(0),
	bayAnim_(0),
	gearAnim_(0),
	tailAngle_(0.0f),
	gearHeight_(0),
	abEffectModel_(0)
{

}

GameObjectInfo_Aircraft::~GameObjectInfo_Aircraft()
{
	delete bayAnim_;
	bayAnim_ = 0;
	delete gearAnim_;
	gearAnim_ = 0;
}

bool
GameObjectInfo_Aircraft::setupBoundingBox()
{
	std::string const& modelBaseName = getModelPath(false);

	fileHandle_t fileVehicle;

	if( FS_FOpenFileByMode((modelBaseName + ".md3").c_str(), &fileVehicle, FS_READ) < 0 ) 
	{
		Com_Error(ERR_FATAL, "Unable to open model file %s\n", modelBaseName.c_str() );
		return false;
	}

	bool success = Md3Utils::getModelDimensions( fileVehicle, mins_, maxs_ );

	// gear
	if( success && gearAnim_ )
	{
		// gear tag
		md3Tag_t tagVehicle;
		if( Md3Utils::getTagInfo( fileVehicle, "tag_gear", tagVehicle ) )
		{
			// gear
			fileHandle_t fileGear;
			if( FS_FOpenFileByMode((modelBaseName + "_gear.md3").c_str(), &fileGear, FS_READ) >= 0 ) 
			{
				// gear animations
				int frames = Md3Utils::getNumberOfFrames( fileGear );
				if( !frames )
					success = false;
				else
					gearAnim_->maxFrame_ = frames - 1;

				// gear bbox
				if( success )
				{
					md3Tag_t tagGear;
					if( Md3Utils::getTagInfo( fileGear, "tag_gear", tagGear ) )
					{
						vec3_t mins, maxs;
						if( Md3Utils::getModelDimensions( fileGear, mins, maxs ) )
						{
							float diffGear = tagGear.origin[2] - mins[2];
							mins_[2] = tagVehicle.origin[2] - diffGear;
						}
						else
							success = false;
					}
					else
						success = false;
				}

				FS_FCloseFile(fileGear);
			}
			else
			{
				success = false;
			}

		}
		else
			success = false;
	}
	FS_FCloseFile(fileVehicle);

	if( success && bayAnim_ )
	{
		// gear
		fileHandle_t fileBay;
		if( FS_FOpenFileByMode((modelBaseName + "_bay.md3").c_str(), &fileBay, FS_READ) >= 0 ) 
		{
			// gear animations
			int frames = Md3Utils::getNumberOfFrames( fileBay );
			if( !frames )
				success = false;
			else
				bayAnim_->maxFrame_ = frames - 1;

			FS_FCloseFile(fileBay);
		}
		else
			success = false;
	}

	return success;
}






GameObjectInfo_Plane::GameObjectInfo_Plane() :
	stallSpeed_(0),
	swingAngle_(0.0f)
{
}

GameObjectInfo_Plane::~GameObjectInfo_Plane()
{
}

bool
GameObjectInfo_Plane::setupBoundingBox()
{
	return GameObjectInfo_Aircraft::setupBoundingBox();
}

void
GameObjectInfo_Plane::createAllPlanes( GameObjectList& gameObjects )
{
	GameObjectInfo_Plane* veh = 0;
	Loadout loadout;

	// --- MODERN ---


	// F-16
	veh = dynamic_cast<GameObjectInfo_Plane*>(createVehicle(GameObjectInfo::GO_CAT_PLANE));
	veh->descriptiveName_ =	"F-16C Falcon";
	veh->tinyName_ = "F-16C";
	veh->modelName_ = "f-16";
	veh->gameSet_ = MF_GAMESET_MODERN;
	veh->category_ = GameObjectInfo::GO_CAT_PLANE;
	veh->class_ = CLASS_PLANE_FIGHTER;
	veh->flags_ = 0;
	veh->caps_ = HC_GEAR|HC_SPEEDBRAKE|HC_VAPOR;
	veh->renderFlags_ = 0;	
    VectorSet( veh->turnSpeed_, 85, 60, 280 );		
	VectorSet( veh->camDist_, 0, 100, 70 );
	VectorSet( veh->camHeight_ , 0, 100, 20);		
    veh->maxHealth_ = 120;		
	veh->shadowShader_ = SHADOW_DEFAULT;	
	Vector4Set( veh->shadowCoords_, 4, 0, 34, 24 );	
	Vector4Set( veh->shadowAdjusts_, 0, 0, 0, 0 );	
	VectorSet( veh->gunoffset_, 16.65f, -3.391f, 0.378f );		
	VectorSet( veh->cockpitview_, 19, 0, 5 );	
	veh->airRadar_ = new VehicleRadarInfo(0, 12000, CAT_PLANE|CAT_HELO);		
	veh->groundRadar_ = new VehicleRadarInfo(-1, 9000, CAT_GROUND|CAT_BOAT);			
	veh->minThrottle_ = 0;
	veh->maxThrottle_ = 15;
	veh->acceleration_ = 260;
	veh->maxFuel_ = 60;
	veh->engines_ = 1;		
	veh->bayAnim_ = 0;		
	veh->gearAnim_ = new VehiclePartAnimInfo(1400);		
	veh->tailAngle_ = 0;		
	veh->abEffectModel_ = AB_BALL;	
    veh->stallSpeed_ = 200;	
	veh->swingAngle_ = 0;	
    veh->maxSpeed_ = 700;		
	loadout.clear();
	addWeaponToLoadout(loadout, "20mm MG", "MG", 450, Armament::ST_PRIMARY_WEAPON );
	addWeaponToLoadout(loadout, "AIM-9 Sidewinder", "AIM-9", 4, Armament::ST_SECONDARY_WEAPON );
	addWeaponToLoadout(loadout, "AIM-120 AMRAAM", "AIM-120", 4, Armament::ST_SECONDARY_WEAPON );
	addWeaponToLoadout(loadout, "Flares", "Flares", 30, Armament::ST_NOT_SELECTABLE );
	veh->defaultLoadouts_.insert(std::make_pair("Anti Aircraft", loadout));
	loadout.clear();
	addWeaponToLoadout(loadout, "20mm MG", "MG", 450, Armament::ST_PRIMARY_WEAPON );
	addWeaponToLoadout(loadout, "AIM-9 Sidewinder", "AIM-9", 2, Armament::ST_SECONDARY_WEAPON );
	addWeaponToLoadout(loadout, "AIM-120 AMRAAM", "AIM-120", 2, Armament::ST_SECONDARY_WEAPON );
	addWeaponToLoadout(loadout, "Mk-82", "Mk-82", 12, Armament::ST_SECONDARY_WEAPON );
	addWeaponToLoadout(loadout, "Flares", "Flares", 30, Armament::ST_NOT_SELECTABLE );
	veh->defaultLoadouts_.insert(std::make_pair("Iron Bombs", loadout));
	gameObjects.push_back(veh);

	// JAS-39
	veh = dynamic_cast<GameObjectInfo_Plane*>(createVehicle(GameObjectInfo::GO_CAT_PLANE));
	veh->descriptiveName_ =	"Saab JAS 39 Gripen";
	veh->tinyName_ = "Jas-39";
	veh->modelName_ = "jas-39";
	veh->gameSet_ = MF_GAMESET_MODERN;
	veh->category_ = GameObjectInfo::GO_CAT_PLANE;
	veh->class_ = CLASS_PLANE_FIGHTER;
	veh->flags_ = 0;
	veh->caps_ = HC_GEAR|HC_SPEEDBRAKE|HC_VAPOR;
	veh->renderFlags_ = 0;	
    VectorSet( veh->turnSpeed_, 90, 70, 320 );		
	VectorSet( veh->camDist_, 0, 100, 70 );
	VectorSet( veh->camHeight_ , 0, 100, 20);		
    veh->maxHealth_ = 110;		
	veh->shadowShader_ = SHADOW_DEFAULT;	
	Vector4Set( veh->shadowCoords_, 6, 0, 36, 20 );	
	Vector4Set( veh->shadowAdjusts_, 0, 0, 0, 0 );	
	VectorSet( veh->gunoffset_, 19.698f, -1.328f, -3.016f );		
	VectorSet( veh->cockpitview_, 19, 0, 5 );	
	veh->airRadar_ = new VehicleRadarInfo(0, 10000, CAT_PLANE|CAT_HELO);		
	veh->groundRadar_ = new VehicleRadarInfo(-1, 8000, CAT_GROUND|CAT_BOAT);			
	veh->minThrottle_ = 0;
	veh->maxThrottle_ = 15;
	veh->acceleration_ = 250;
	veh->maxFuel_ = 60;
	veh->engines_ = 1;		
	veh->bayAnim_ = 0;		
	veh->gearAnim_ = new VehiclePartAnimInfo(1400);		
	veh->tailAngle_ = 0;		
	veh->abEffectModel_ = AB_BALL;	
    veh->stallSpeed_ = 200;	
	veh->swingAngle_ = 0;	
    veh->maxSpeed_ = 720;		
	gameObjects.push_back(veh);

	// A-10
	veh = dynamic_cast<GameObjectInfo_Plane*>(createVehicle(GameObjectInfo::GO_CAT_PLANE));
	veh->descriptiveName_ =	"A-10 Thunderbolt II";
	veh->tinyName_ = "A-10";
	veh->modelName_ = "a10";
	veh->gameSet_ = MF_GAMESET_MODERN;
	veh->category_ = GameObjectInfo::GO_CAT_PLANE;
	veh->class_ = CLASS_PLANE_BOMBER;
	veh->flags_ = 0;
	veh->caps_ = HC_GEAR|HC_SPEEDBRAKE|HC_VAPOR;
	veh->renderFlags_ = 0;	
    VectorSet( veh->turnSpeed_, 80, 60, 200 );		
	VectorSet( veh->camDist_, 0, 100, 70 );
	VectorSet( veh->camHeight_ , 0, 100, 20);		
    veh->maxHealth_ = 280;		
	veh->shadowShader_ = SHADOW_DEFAULT;	
	Vector4Set( veh->shadowCoords_, 5, 0, 34, 26 );	
	Vector4Set( veh->shadowAdjusts_, 0, 0, 0, 0 );	
	VectorSet( veh->gunoffset_, 26.25f, -0.94f, 1.811f );		
	VectorSet( veh->cockpitview_, 11, 0, 5 );	
	veh->airRadar_ = new VehicleRadarInfo(0, 8000, CAT_PLANE|CAT_HELO);		
	veh->groundRadar_ = new VehicleRadarInfo(-1, 10000, CAT_GROUND|CAT_BOAT);			
	veh->minThrottle_ = 0;
	veh->maxThrottle_ = 10;
	veh->acceleration_ = 270;
	veh->maxFuel_ = 80;
	veh->engines_ = 2;		
	veh->bayAnim_ = 0;		
	veh->gearAnim_ = new VehiclePartAnimInfo(1400);		
	veh->tailAngle_ = 0;		
	veh->abEffectModel_ = AB_BALL;	
    veh->stallSpeed_ = 170;	
	veh->swingAngle_ = 0;	
    veh->maxSpeed_ = 620;		
	gameObjects.push_back(veh);

	// F-5
	veh = dynamic_cast<GameObjectInfo_Plane*>(createVehicle(GameObjectInfo::GO_CAT_PLANE));
	veh->descriptiveName_ =	"F-5 Tiger";
	veh->tinyName_ = "F-5";
	veh->modelName_ = "f-5";
	veh->gameSet_ = MF_GAMESET_MODERN;
	veh->category_ = GameObjectInfo::GO_CAT_PLANE;
	veh->class_ = CLASS_PLANE_FIGHTER;
	veh->flags_ = 0;
	veh->caps_ = HC_GEAR|HC_SPEEDBRAKE|HC_VAPOR|HC_DUALGUNS;
	veh->renderFlags_ = 0;	
    VectorSet( veh->turnSpeed_, 80, 80, 380 );		
	VectorSet( veh->camDist_, 0, 100, 70 );
	VectorSet( veh->camHeight_ , 0, 100, 20);		
    veh->maxHealth_ = 100;		
	veh->shadowShader_ = SHADOW_DEFAULT;	
	Vector4Set( veh->shadowCoords_, 5, 0, 34, 26 );	
	Vector4Set( veh->shadowAdjusts_, 0, 0, 0, 0 );	
	VectorSet( veh->gunoffset_, 26.25f, -0.94f, 1.811f );		
	VectorSet( veh->cockpitview_, 11, 0, 5 );	
	veh->airRadar_ = new VehicleRadarInfo(0, 6000, CAT_PLANE|CAT_HELO);		
	veh->groundRadar_ = new VehicleRadarInfo(-1, 4000, CAT_GROUND|CAT_BOAT);			
	veh->minThrottle_ = 0;
	veh->maxThrottle_ = 15;
	veh->acceleration_ = 270;
	veh->maxFuel_ = 80;
	veh->engines_ = 2;		
	veh->bayAnim_ = 0;		
	veh->gearAnim_ = new VehiclePartAnimInfo(1400);		
	veh->tailAngle_ = 0;		
	veh->abEffectModel_ = AB_RED_SMALL;	
    veh->stallSpeed_ = 170;	
	veh->swingAngle_ = 0;	
    veh->maxSpeed_ = 620;		
	gameObjects.push_back(veh);

	// F-15
	veh = dynamic_cast<GameObjectInfo_Plane*>(createVehicle(GameObjectInfo::GO_CAT_PLANE));
	veh->descriptiveName_ =	"F-15E Eagle";
	veh->tinyName_ = "F-15";
	veh->modelName_ = "f-15";
	veh->gameSet_ = MF_GAMESET_MODERN;
	veh->category_ = GameObjectInfo::GO_CAT_PLANE;
	veh->class_ = CLASS_PLANE_BOMBER;
	veh->flags_ = 0;
	veh->caps_ = HC_GEAR|HC_SPEEDBRAKE|HC_VAPOR;
	veh->renderFlags_ = MFR_DUALPILOT|MFR_BIGVAPOR;	
    VectorSet( veh->turnSpeed_, 65, 50, 240 );		
	VectorSet( veh->camDist_, 0, 100, 70 );
	VectorSet( veh->camHeight_ , 0, 100, 20);		
    veh->maxHealth_ = 150;		
	veh->shadowShader_ = SHADOW_DEFAULT;	
	Vector4Set( veh->shadowCoords_, 6, 0, 48, 34 );	
	Vector4Set( veh->shadowAdjusts_, 0, 0, 0, 0 );	
	VectorSet( veh->gunoffset_, 19.1f,7.87f,-0.112f );		
	VectorSet( veh->cockpitview_, 28, 0, 6 );	
	veh->airRadar_ = new VehicleRadarInfo(0, 14000, CAT_PLANE|CAT_HELO);		
	veh->groundRadar_ = new VehicleRadarInfo(-1, 7000, CAT_GROUND|CAT_BOAT);			
	veh->minThrottle_ = 0;
	veh->maxThrottle_ = 15;
	veh->acceleration_ = 260;
	veh->maxFuel_ = 80;
	veh->engines_ = 2;		
	veh->bayAnim_ = 0;		
	veh->gearAnim_ = new VehiclePartAnimInfo(1400);		
	veh->tailAngle_ = 0;		
	veh->abEffectModel_ = AB_RED;	
    veh->stallSpeed_ = 220;	
	veh->swingAngle_ = 0;	
    veh->maxSpeed_ = 880;		
	gameObjects.push_back(veh);

	// F-14
	veh = dynamic_cast<GameObjectInfo_Plane*>(createVehicle(GameObjectInfo::GO_CAT_PLANE));
	veh->descriptiveName_ =	"F-14 Tomcat";
	veh->tinyName_ = "F-14";
	veh->modelName_ = "f-14";
	veh->gameSet_ = MF_GAMESET_MODERN;
	veh->category_ = GameObjectInfo::GO_CAT_PLANE;
	veh->class_ = CLASS_PLANE_FIGHTER;
	veh->flags_ = 0;
	veh->caps_ = HC_GEAR|HC_SPEEDBRAKE|HC_VAPOR|HC_SWINGWING;
	veh->renderFlags_ = MFR_DUALPILOT|MFR_BIGVAPOR;	
    VectorSet( veh->turnSpeed_, 60, 55, 220 );		
	VectorSet( veh->camDist_, 0, 100, 70 );
	VectorSet( veh->camHeight_ , 0, 100, 20);		
    veh->maxHealth_ = 150;		
	veh->shadowShader_ = SHADOW_DEFAULT;	
	Vector4Set( veh->shadowCoords_, 6, 0, 48, 34 );	
	Vector4Set( veh->shadowAdjusts_, 0, 0, 0, 0 );	
	VectorSet( veh->gunoffset_, 37.8f,-2.4f,-1.3f );		
	VectorSet( veh->cockpitview_, 28, 0, 6 );	
	veh->airRadar_ = new VehicleRadarInfo(0, 20000, CAT_PLANE|CAT_HELO);		
	veh->groundRadar_ = new VehicleRadarInfo(-1, 5500, CAT_GROUND|CAT_BOAT);			
	veh->minThrottle_ = 0;
	veh->maxThrottle_ = 15;
	veh->acceleration_ = 260;
	veh->maxFuel_ = 70;
	veh->engines_ = 2;		
	veh->bayAnim_ = 0;		
	veh->gearAnim_ = new VehiclePartAnimInfo(1400);		
	veh->tailAngle_ = 0;		
	veh->abEffectModel_ = AB_RED;	
    veh->stallSpeed_ = 220;	
	veh->swingAngle_ = 50;	
    veh->maxSpeed_ = 840;		
	gameObjects.push_back(veh);

	// F-18
	veh = dynamic_cast<GameObjectInfo_Plane*>(createVehicle(GameObjectInfo::GO_CAT_PLANE));
	veh->descriptiveName_ =	"F-18 Hornet";
	veh->tinyName_ = "F-18";
	veh->modelName_ = "f-18";
	veh->gameSet_ = MF_GAMESET_MODERN;
	veh->category_ = GameObjectInfo::GO_CAT_PLANE;
	veh->class_ = CLASS_PLANE_FIGHTER;
	veh->flags_ = 0;
	veh->caps_ = HC_GEAR|HC_SPEEDBRAKE|HC_VAPOR;
	veh->renderFlags_ = 0;	
    VectorSet( veh->turnSpeed_, 70, 60, 240 );		
	VectorSet( veh->camDist_, 0, 100, 70 );
	VectorSet( veh->camHeight_ , 0, 100, 20);		
    veh->maxHealth_ = 130;		
	veh->shadowShader_ = SHADOW_DEFAULT;	
	Vector4Set( veh->shadowCoords_, 8,0,42,42 );	
	Vector4Set( veh->shadowAdjusts_, 0, 0, 0, 0 );	
	VectorSet( veh->gunoffset_, 40.3f, 0, 0.478f );		
	VectorSet( veh->cockpitview_, 19, 0, 5 );	
	veh->airRadar_ = new VehicleRadarInfo(0, 11000, CAT_PLANE|CAT_HELO);		
	veh->groundRadar_ = new VehicleRadarInfo(-1, 8500, CAT_GROUND|CAT_BOAT);			
	veh->minThrottle_ = 0;
	veh->maxThrottle_ = 15;
	veh->acceleration_ = 260;
	veh->maxFuel_ = 60;
	veh->engines_ = 2;		
	veh->bayAnim_ = 0;		
	veh->gearAnim_ = new VehiclePartAnimInfo(1400);		
	veh->tailAngle_ = 0;		
	veh->abEffectModel_ = AB_BALL;	
    veh->stallSpeed_ = 200;	
	veh->swingAngle_ = 0;	
    veh->maxSpeed_ = 720;		
	gameObjects.push_back(veh);

	// B-2
	veh = dynamic_cast<GameObjectInfo_Plane*>(createVehicle(GameObjectInfo::GO_CAT_PLANE));
	veh->descriptiveName_ =	"B-2 Spirit";
	veh->tinyName_ = "B-2";
	veh->modelName_ = "b-2";
	veh->gameSet_ = MF_GAMESET_MODERN;
	veh->category_ = GameObjectInfo::GO_CAT_PLANE;
	veh->class_ = CLASS_PLANE_BOMBER;
	veh->flags_ = 0;
	veh->caps_ = HC_GEAR|HC_SPEEDBRAKE|HC_VAPOR|HC_WEAPONBAY;
	veh->renderFlags_ = 0;	
    VectorSet( veh->turnSpeed_, 50, 30, 150 );		
	VectorSet( veh->camDist_, 0, 120, 100 );
	VectorSet( veh->camHeight_ , 0, 100, 35);		
    veh->maxHealth_ = 300;		
	veh->shadowShader_ = SHADOW_DEFAULT;	
	Vector4Set( veh->shadowCoords_, 3, 0, 76, 76 );	
	Vector4Set( veh->shadowAdjusts_, 0, 0, 0, 0 );	
	VectorSet( veh->gunoffset_, 16.65f, -3.391f, 0.378f );		
	VectorSet( veh->cockpitview_, 19, 0, 5 );	
	veh->airRadar_ = new VehicleRadarInfo(0, 5000, CAT_PLANE|CAT_HELO);		
	veh->groundRadar_ = new VehicleRadarInfo(-1, 9000, CAT_GROUND|CAT_BOAT);			
	veh->minThrottle_ = 0;
	veh->maxThrottle_ = 10;
	veh->acceleration_ = 260;
	veh->maxFuel_ = 60;
	veh->engines_ = 2;		
	veh->bayAnim_ = new VehiclePartAnimInfo(200);		
	veh->gearAnim_ = new VehiclePartAnimInfo(1400);		
	veh->tailAngle_ = 0;		
	veh->abEffectModel_ = AB_BALL;	
    veh->stallSpeed_ = 200;	
	veh->swingAngle_ = 0;	
    veh->maxSpeed_ = 700;		
	gameObjects.push_back(veh);



	// --- WW2 ---



	// P-51
	veh = dynamic_cast<GameObjectInfo_Plane*>(createVehicle(GameObjectInfo::GO_CAT_PLANE));
	veh->descriptiveName_ =	"P-51d Mustang";
	veh->tinyName_ = "P-51d";
	veh->modelName_ = "p-51d";
	veh->gameSet_ = MF_GAMESET_WW2;
	veh->category_ = GameObjectInfo::GO_CAT_PLANE;
	veh->class_ = CLASS_PLANE_FIGHTER;
	veh->flags_ = 0;
	veh->caps_ = HC_GEAR|HC_PROP|HC_TAILDRAGGER|HC_DUALGUNS;
	veh->renderFlags_ = 0;	
    VectorSet( veh->turnSpeed_, 60, 60, 200 );		
	VectorSet( veh->camDist_, 0, 100, 50 );
	VectorSet( veh->camHeight_ , 0, 100, 10);		
    veh->maxHealth_ = 100;		
	veh->shadowShader_ = SHADOW_DEFAULT;	
	Vector4Set( veh->shadowCoords_, 2, 0, 30, 26 );	
	Vector4Set( veh->shadowAdjusts_, 0, 0, 0, 0 );	
	VectorSet( veh->gunoffset_, 3.5f, -9.2f, 0.136f );		
	VectorSet( veh->cockpitview_, -4, 0, 8 );	
	veh->airRadar_ = 0;		
	veh->groundRadar_ = 0;			
	veh->minThrottle_ = 0;
	veh->maxThrottle_ = 10;
	veh->acceleration_ = 220;
	veh->maxFuel_ = 60;
	veh->engines_ = 1;		
	veh->bayAnim_ = 0;		
	veh->gearAnim_ = new VehiclePartAnimInfo(1400);		
	veh->tailAngle_ = -14.0f;		
	veh->abEffectModel_ = 0;	
    veh->stallSpeed_ = 90;	
	veh->swingAngle_ = 0;	
    veh->maxSpeed_ = 420;		
	gameObjects.push_back(veh);

	// Spitfire
	veh = dynamic_cast<GameObjectInfo_Plane*>(createVehicle(GameObjectInfo::GO_CAT_PLANE));
	veh->descriptiveName_ =	"Spitfire Mk 5b";
	veh->tinyName_ = "Spitfire";
	veh->modelName_ = "spitfire_mk5b";
	veh->gameSet_ = MF_GAMESET_WW2;
	veh->category_ = GameObjectInfo::GO_CAT_PLANE;
	veh->class_ = CLASS_PLANE_FIGHTER;
	veh->flags_ = 0;
	veh->caps_ = HC_GEAR|HC_PROP|HC_TAILDRAGGER|HC_DUALGUNS;
	veh->renderFlags_ = 0;	
    VectorSet( veh->turnSpeed_, 80, 80, 240 );		
	VectorSet( veh->camDist_, 0, 100, 50 );
	VectorSet( veh->camHeight_ , 0, 100, 10);		
    veh->maxHealth_ = 100;		
	veh->shadowShader_ = SHADOW_DEFAULT;	
	Vector4Set( veh->shadowCoords_, -4, 0, 22, 20 );	
	Vector4Set( veh->shadowAdjusts_, 0, 0, 0, 0 );	
	VectorSet( veh->gunoffset_, 2.518f, -7.371f, -0.155f );		
	VectorSet( veh->cockpitview_, -4, 0, 6 );	
	veh->airRadar_ = 0;		
	veh->groundRadar_ = 0;			
	veh->minThrottle_ = 0;
	veh->maxThrottle_ = 10;
	veh->acceleration_ = 220;
	veh->maxFuel_ = 60;
	veh->engines_ = 1;		
	veh->bayAnim_ = 0;		
	veh->gearAnim_ = new VehiclePartAnimInfo(1400);		
	veh->tailAngle_ = -14.0f;		
	veh->abEffectModel_ = 0;	
    veh->stallSpeed_ = 85;	
	veh->swingAngle_ = 0;	
    veh->maxSpeed_ = 400;		
	gameObjects.push_back(veh);

	// Bf109
	veh = dynamic_cast<GameObjectInfo_Plane*>(createVehicle(GameObjectInfo::GO_CAT_PLANE));
	veh->descriptiveName_ =	"Messerschmitt Bf-109g";
	veh->tinyName_ = "Bf-109";
	veh->modelName_ = "bf-109g";
	veh->gameSet_ = MF_GAMESET_WW2;
	veh->category_ = GameObjectInfo::GO_CAT_PLANE;
	veh->class_ = CLASS_PLANE_FIGHTER;
	veh->flags_ = 0;
	veh->caps_ = HC_GEAR|HC_PROP|HC_TAILDRAGGER|HC_DUALGUNS;
	veh->renderFlags_ = 0;	
    VectorSet( veh->turnSpeed_, 70, 65, 240 );		
	VectorSet( veh->camDist_, 0, 100, 50 );
	VectorSet( veh->camHeight_ , 0, 100, 10);		
    veh->maxHealth_ = 100;		
	veh->shadowShader_ = SHADOW_DEFAULT;	
	Vector4Set( veh->shadowCoords_, -3, 0, 18, 16 );	
	Vector4Set( veh->shadowAdjusts_, 0, 0, 0, 0 );	
	VectorSet( veh->gunoffset_, 2.518f, -7.371f, -0.155f );		
	VectorSet( veh->cockpitview_, -4, 0, 6 );	
	veh->airRadar_ = 0;		
	veh->groundRadar_ = 0;			
	veh->minThrottle_ = 0;
	veh->maxThrottle_ = 10;
	veh->acceleration_ = 220;
	veh->maxFuel_ = 60;
	veh->engines_ = 1;		
	veh->bayAnim_ = 0;		
	veh->gearAnim_ = new VehiclePartAnimInfo(1400);		
	veh->tailAngle_ = -14.0f;		
	veh->abEffectModel_ = 0;	
    veh->stallSpeed_ = 90;	
	veh->swingAngle_ = 0;	
    veh->maxSpeed_ = 410;		
	gameObjects.push_back(veh);

	// Fw190
	veh = dynamic_cast<GameObjectInfo_Plane*>(createVehicle(GameObjectInfo::GO_CAT_PLANE));
	veh->descriptiveName_ =	"Focke-Wulf Fw190 A8";
	veh->tinyName_ = "Fw-190";
	veh->modelName_ = "fw190a8";
	veh->gameSet_ = MF_GAMESET_WW2;
	veh->category_ = GameObjectInfo::GO_CAT_PLANE;
	veh->class_ = CLASS_PLANE_FIGHTER;
	veh->flags_ = 0;
	veh->caps_ = HC_GEAR|HC_PROP|HC_TAILDRAGGER|HC_DUALGUNS;
	veh->renderFlags_ = 0;	
    VectorSet( veh->turnSpeed_, 60, 60, 220 );		
	VectorSet( veh->camDist_, 0, 100, 50 );
	VectorSet( veh->camHeight_ , 0, 100, 10);		
    veh->maxHealth_ = 100;		
	veh->shadowShader_ = SHADOW_DEFAULT;	
	Vector4Set( veh->shadowCoords_, -2, 0, 18, 18 );	
	Vector4Set( veh->shadowAdjusts_, 0, 0, 0, 0 );	
	VectorSet( veh->gunoffset_, 2.518f, -7.371f, -0.155f );		
	VectorSet( veh->cockpitview_, -4, 0, 6 );	
	veh->airRadar_ = 0;		
	veh->groundRadar_ = 0;			
	veh->minThrottle_ = 0;
	veh->maxThrottle_ = 10;
	veh->acceleration_ = 220;
	veh->maxFuel_ = 60;
	veh->engines_ = 1;		
	veh->bayAnim_ = 0;		
	veh->gearAnim_ = new VehiclePartAnimInfo(1400);		
	veh->tailAngle_ = -14.0f;		
	veh->abEffectModel_ = 0;	
    veh->stallSpeed_ = 90;	
	veh->swingAngle_ = 0;	
    veh->maxSpeed_ = 420;		
	gameObjects.push_back(veh);

	// B-17
	veh = dynamic_cast<GameObjectInfo_Plane*>(createVehicle(GameObjectInfo::GO_CAT_PLANE));
	veh->descriptiveName_ =	"B-17g";
	veh->tinyName_ = "B-17";
	veh->modelName_ = "b17g";
	veh->gameSet_ = MF_GAMESET_WW2;
	veh->category_ = GameObjectInfo::GO_CAT_PLANE;
	veh->class_ = CLASS_PLANE_BOMBER;
	veh->flags_ = 0;
	veh->caps_ = HC_GEAR|HC_PROP|HC_TAILDRAGGER|HC_WEAPONBAY;
	veh->renderFlags_ = 0;	
    VectorSet( veh->turnSpeed_, 40, 30, 100 );		
	VectorSet( veh->camDist_, 0, 100, 80 );
	VectorSet( veh->camHeight_ , 0, 100, 10);		
    veh->maxHealth_ = 1500;		
	veh->shadowShader_ = SHADOW_DEFAULT;	
	Vector4Set( veh->shadowCoords_, -12, 0, 60, 60 );	
	Vector4Set( veh->shadowAdjusts_, 0, 0, 0, 0 );	
	VectorSet( veh->gunoffset_, 2.518f, -7.371f, -0.155f );		
	VectorSet( veh->cockpitview_, -4, 0, 6 );	
	veh->airRadar_ = 0;		
	veh->groundRadar_ = 0;			
	veh->minThrottle_ = 0;
	veh->maxThrottle_ = 10;
	veh->acceleration_ = 220;
	veh->maxFuel_ = 60;
	veh->engines_ = 4;		
	veh->bayAnim_ = new VehiclePartAnimInfo(500);		
	veh->gearAnim_ = new VehiclePartAnimInfo(1400);		
	veh->tailAngle_ = -8.0f;		
	veh->abEffectModel_ = 0;	
    veh->stallSpeed_ = 100;	
	veh->swingAngle_ = 0;	
    veh->maxSpeed_ = 320;		
	gameObjects.push_back(veh);



	// --- WW1 ---



	// Dr.1
	veh = dynamic_cast<GameObjectInfo_Plane*>(createVehicle(GameObjectInfo::GO_CAT_PLANE));
	veh->descriptiveName_ =	"Fokker Dr.1";
	veh->tinyName_ = "Dr.1";
	veh->modelName_ = "dr1";
	veh->gameSet_ = MF_GAMESET_WW1;
	veh->category_ = GameObjectInfo::GO_CAT_PLANE;
	veh->class_ = CLASS_PLANE_FIGHTER;
	veh->flags_ = 0;
	veh->caps_ = HC_PROP|HC_TAILDRAGGER|HC_DUALGUNS;
	veh->renderFlags_ = 0;	
    VectorSet( veh->turnSpeed_, 70, 80, 320 );		
	VectorSet( veh->camDist_, 0, 100, 30 );
	VectorSet( veh->camHeight_ , 0, 100, 10);		
    veh->maxHealth_ = 100;		
	veh->shadowShader_ = SHADOW_DEFAULT;	
	Vector4Set( veh->shadowCoords_, -3, 0, 13, 12 );	
	Vector4Set( veh->shadowAdjusts_, 0, 0, 0, 0 );	
	VectorSet( veh->gunoffset_, 2.518f, -1.371f, -0.155f );		
	VectorSet( veh->cockpitview_, -4, 0, 6 );	
	veh->airRadar_ = 0;		
	veh->groundRadar_ = 0;			
	veh->minThrottle_ = 0;
	veh->maxThrottle_ = 10;
	veh->acceleration_ = 200;
	veh->maxFuel_ = 60;
	veh->engines_ = 1;		
	veh->bayAnim_ = 0;		
	veh->gearAnim_ = 0;		
	veh->tailAngle_ = -14.0f;		
	veh->abEffectModel_ = 0;	
    veh->stallSpeed_ = 50;	
	veh->swingAngle_ = 0;	
    veh->maxSpeed_ = 150;		
	gameObjects.push_back(veh);

	// Camel
	veh = dynamic_cast<GameObjectInfo_Plane*>(createVehicle(GameObjectInfo::GO_CAT_PLANE));
	veh->descriptiveName_ =	"Sopwith camel";
	veh->tinyName_ = "Camel";
	veh->modelName_ = "camel";
	veh->gameSet_ = MF_GAMESET_WW1;
	veh->category_ = GameObjectInfo::GO_CAT_PLANE;
	veh->class_ = CLASS_PLANE_FIGHTER;
	veh->flags_ = 0;
	veh->caps_ = HC_PROP|HC_TAILDRAGGER|HC_DUALGUNS;
	veh->renderFlags_ = 0;	
    VectorSet( veh->turnSpeed_, 60, 70, 260 );		
	VectorSet( veh->camDist_, 0, 100, 30 );
	VectorSet( veh->camHeight_ , 0, 100, 10);		
    veh->maxHealth_ = 100;		
	veh->shadowShader_ = SHADOW_DEFAULT;	
	Vector4Set( veh->shadowCoords_, -1, 0, 15, 14 );	
	Vector4Set( veh->shadowAdjusts_, 0, 0, 0, 0 );	
	VectorSet( veh->gunoffset_, 2.518f, -1.371f, -0.155f );		
	VectorSet( veh->cockpitview_, -4, 0, 6 );	
	veh->airRadar_ = 0;		
	veh->groundRadar_ = 0;			
	veh->minThrottle_ = 0;
	veh->maxThrottle_ = 10;
	veh->acceleration_ = 200;
	veh->maxFuel_ = 60;
	veh->engines_ = 1;		
	veh->bayAnim_ = 0;		
	veh->gearAnim_ = 0;		
	veh->tailAngle_ = -14.0f;		
	veh->abEffectModel_ = 0;	
    veh->stallSpeed_ = 55;	
	veh->swingAngle_ = 0;	
    veh->maxSpeed_ = 170;		
	gameObjects.push_back(veh);
}










