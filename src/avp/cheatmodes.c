#include "3dc.h"
#include "inline.h"
#include "module.h"
#include "stratdef.h"

#include "avp_userprofile.h"
#include "avp_menus.h"

int CheatMode_Active=0;
int CheatMode_Species=0;
int CheatMode_Environment=0;


extern void CheatMode_GetNextAllowedSpecies(int *speciesPtr, int searchForward);
extern void CheatMode_GetNextAllowedEnvironment(int *environmentPtr, int searchForward);
extern void CheatMode_CheckOptionsAreValid(void);

#define NUMBER_OF_ENVIRONMENTS (MAX_NO_OF_MARINE_EPISODES+MAX_NO_OF_PREDATOR_EPISODES+MAX_NO_OF_ALIEN_EPISODES)
#define NUMBER_OF_SPECIES 3
typedef struct
{
	unsigned char PlayableForSpecies[3];
	unsigned char PlayableForEnvironment[NUMBER_OF_ENVIRONMENTS];
	
} CHEATMODEDESC;

CHEATMODEDESC CheatModeDesc[] =
{
	// CHEATMODE_PIGSTICKING,
	{
		{
			0,	// Marine			
			1,	// Predator			
			0,	// Alien
		},
		{
			0,	//	AVP_ENVIRONMENT_DERELICT,
			0,	//	AVP_ENVIRONMENT_COLONY,
			0,	//	AVP_ENVIRONMENT_INVASION,
			0,	//	AVP_ENVIRONMENT_ORBITAL,
			0,	//	AVP_ENVIRONMENT_TYRARGO,
			0,	//	AVP_ENVIRONMENT_TYRARGOHANGAR,

			0,	//	AVP_ENVIRONMENT_WATERFALL_M,
			0,	//	AVP_ENVIRONMENT_VAULTS_M,
			0,	//	AVP_ENVIRONMENT_FERARCO_M,
			0,	//	AVP_ENVIRONMENT_TEMPLE_M,
			0,	//	AVP_ENVIRONMENT_GATEWAY_M,

			1,	//	AVP_ENVIRONMENT_WATERFALL,
			1,	//	AVP_ENVIRONMENT_AREA52,
			1,	//	AVP_ENVIRONMENT_VAULTS,
			1,	//	AVP_ENVIRONMENT_FURY161,
			1,	//	AVP_ENVIRONMENT_CAVERNS,
			0,	//	AVP_ENVIRONMENT_CAVERNSEND,

			1,	//	AVP_ENVIRONMENT_INVASION_P,
			1,	//	AVP_ENVIRONMENT_TYRARGO_P,
			1,	//	AVP_ENVIRONMENT_TEMPLE_P,
			1,	//	AVP_ENVIRONMENT_ESCAPE_P,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND_P,

			0,	//	AVP_ENVIRONMENT_TEMPLE,
			0,	//	AVP_ENVIRONMENT_ESCAPE,
			0,	//	AVP_ENVIRONMENT_FERARCO,
			0,	//	AVP_ENVIRONMENT_GATEWAY,
			0,	//	AVP_ENVIRONMENT_EARTHBOUND,

			0,	//	AVP_ENVIRONMENT_INVASION_A,
			0,	//	AVP_ENVIRONMENT_DERELICT_A,
			0,	//	AVP_ENVIRONMENT_TYRARGO_A,
			0,	//	AVP_ENVIRONMENT_FURY161_A,
			0,	//	AVP_ENVIRONMENT_CAVERNS_A,
		}
	},
	// CHEATMODE_SLUGTRAIL,
	{
		{
			1,	// Marine			
			1,	// Predator			
			0,	// Alien
		},
		{
			1,	//	AVP_ENVIRONMENT_DERELICT,
			1,	//	AVP_ENVIRONMENT_COLONY,
			1,	//	AVP_ENVIRONMENT_INVASION,
			1,	//	AVP_ENVIRONMENT_ORBITAL,
			1,	//	AVP_ENVIRONMENT_TYRARGO,
			0,	//	AVP_ENVIRONMENT_TYRARGOHANGAR,

			1,	//	AVP_ENVIRONMENT_WATERFALL_M,
			1,	//	AVP_ENVIRONMENT_VAULTS_M,
			1,	//	AVP_ENVIRONMENT_FERARCO_M,
			1,	//	AVP_ENVIRONMENT_TEMPLE_M,
			1,	//	AVP_ENVIRONMENT_GATEWAY_M,

			0,	//	AVP_ENVIRONMENT_WATERFALL,
			0,	//	AVP_ENVIRONMENT_AREA52,
			1,	//	AVP_ENVIRONMENT_VAULTS,
			1,	//	AVP_ENVIRONMENT_FURY161,
			1,	//	AVP_ENVIRONMENT_CAVERNS,
			0,	//	AVP_ENVIRONMENT_CAVERNSEND,

			1,	//	AVP_ENVIRONMENT_INVASION_P,
			1,	//	AVP_ENVIRONMENT_TYRARGO_P,
			1,	//	AVP_ENVIRONMENT_TEMPLE_P,
			1,	//	AVP_ENVIRONMENT_ESCAPE_P,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND_P,

			1,	//	AVP_ENVIRONMENT_TEMPLE,
			1,	//	AVP_ENVIRONMENT_ESCAPE,
			1,	//	AVP_ENVIRONMENT_FERARCO,
			1,	//	AVP_ENVIRONMENT_GATEWAY,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND,

			1,	//	AVP_ENVIRONMENT_INVASION_A,
			1,	//	AVP_ENVIRONMENT_DERELICT_A,
			1,	//	AVP_ENVIRONMENT_TYRARGO_A,
			1,	//	AVP_ENVIRONMENT_FURY161_A,
			1,	//	AVP_ENVIRONMENT_CAVERNS_A,
		}
	},
	// CHEATMODE_SNIPERMUNCH,
	{
		{
			0,	// Marine			
			0,	// Predator			
			1,	// Alien
		},
		{
			1,	//	AVP_ENVIRONMENT_DERELICT,
			1,	//	AVP_ENVIRONMENT_COLONY,
			1,	//	AVP_ENVIRONMENT_INVASION,
			1,	//	AVP_ENVIRONMENT_ORBITAL,
			1,	//	AVP_ENVIRONMENT_TYRARGO,
			1,	//	AVP_ENVIRONMENT_TYRARGOHANGAR,

			1,	//	AVP_ENVIRONMENT_WATERFALL_M,
			1,	//	AVP_ENVIRONMENT_VAULTS_M,
			1,	//	AVP_ENVIRONMENT_FERARCO_M,
			1,	//	AVP_ENVIRONMENT_TEMPLE_M,
			1,	//	AVP_ENVIRONMENT_GATEWAY_M,

			1,	//	AVP_ENVIRONMENT_WATERFALL,
			1,	//	AVP_ENVIRONMENT_AREA52,
			1,	//	AVP_ENVIRONMENT_VAULTS,
			1,	//	AVP_ENVIRONMENT_FURY161,
			1,	//	AVP_ENVIRONMENT_CAVERNS,
			1,	//	AVP_ENVIRONMENT_CAVERNSEND,

			1,	//	AVP_ENVIRONMENT_INVASION_P,
			1,	//	AVP_ENVIRONMENT_TYRARGO_P,
			1,	//	AVP_ENVIRONMENT_TEMPLE_P,
			1,	//	AVP_ENVIRONMENT_ESCAPE_P,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND_P,

			1,	//	AVP_ENVIRONMENT_TEMPLE,
			1,	//	AVP_ENVIRONMENT_ESCAPE,
			1,	//	AVP_ENVIRONMENT_FERARCO,
			1,	//	AVP_ENVIRONMENT_GATEWAY,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND,

			1,	//	AVP_ENVIRONMENT_INVASION_A,
			1,	//	AVP_ENVIRONMENT_DERELICT_A,
			1,	//	AVP_ENVIRONMENT_TYRARGO_A,
			1,	//	AVP_ENVIRONMENT_FURY161_A,
			1,	//	AVP_ENVIRONMENT_CAVERNS_A,
		}
	},
	// CHEATMODE_TERROR,
	{
		{
			0,	// Marine			
			1,	// Predator			
			1,	// Alien
		},
		{
			1,	//	AVP_ENVIRONMENT_DERELICT,
			1,	//	AVP_ENVIRONMENT_COLONY,
			1,	//	AVP_ENVIRONMENT_INVASION,
			1,	//	AVP_ENVIRONMENT_ORBITAL,
			1,	//	AVP_ENVIRONMENT_TYRARGO,
			1,	//	AVP_ENVIRONMENT_TYRARGOHANGAR,

			1,	//	AVP_ENVIRONMENT_WATERFALL_M,
			1,	//	AVP_ENVIRONMENT_VAULTS_M,
			1,	//	AVP_ENVIRONMENT_FERARCO_M,
			1,	//	AVP_ENVIRONMENT_TEMPLE_M,
			1,	//	AVP_ENVIRONMENT_GATEWAY_M,

			1,	//	AVP_ENVIRONMENT_WATERFALL,
			1,	//	AVP_ENVIRONMENT_AREA52,
			1,	//	AVP_ENVIRONMENT_VAULTS,
			1,	//	AVP_ENVIRONMENT_FURY161,
			1,	//	AVP_ENVIRONMENT_CAVERNS,
			1,	//	AVP_ENVIRONMENT_CAVERNSEND,

			1,	//	AVP_ENVIRONMENT_INVASION_P,
			1,	//	AVP_ENVIRONMENT_TYRARGO_P,
			1,	//	AVP_ENVIRONMENT_TEMPLE_P,
			1,	//	AVP_ENVIRONMENT_ESCAPE_P,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND_P,

			1,	//	AVP_ENVIRONMENT_TEMPLE,
			1,	//	AVP_ENVIRONMENT_ESCAPE,
			1,	//	AVP_ENVIRONMENT_FERARCO,
			1,	//	AVP_ENVIRONMENT_GATEWAY,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND,

			1,	//	AVP_ENVIRONMENT_INVASION_A,
			1,	//	AVP_ENVIRONMENT_DERELICT_A,
			1,	//	AVP_ENVIRONMENT_TYRARGO_A,
			1,	//	AVP_ENVIRONMENT_FURY161_A,
			1,	//	AVP_ENVIRONMENT_CAVERNS_A,
		}
	},
	// CHEATMODE_SUPERGORE,
	{
		{
			1,	// Marine			
			1,	// Predator			
			1,	// Alien
		},
		{
			1,	//	AVP_ENVIRONMENT_DERELICT,
			1,	//	AVP_ENVIRONMENT_COLONY,
			1,	//	AVP_ENVIRONMENT_INVASION,
			1,	//	AVP_ENVIRONMENT_ORBITAL,
			1,	//	AVP_ENVIRONMENT_TYRARGO,
			0,	//	AVP_ENVIRONMENT_TYRARGOHANGAR,

			1,	//	AVP_ENVIRONMENT_WATERFALL_M,
			1,	//	AVP_ENVIRONMENT_VAULTS_M,
			1,	//	AVP_ENVIRONMENT_FERARCO_M,
			1,	//	AVP_ENVIRONMENT_TEMPLE_M,
			1,	//	AVP_ENVIRONMENT_GATEWAY_M,

			1,	//	AVP_ENVIRONMENT_WATERFALL,
			1,	//	AVP_ENVIRONMENT_AREA52,
			1,	//	AVP_ENVIRONMENT_VAULTS,
			1,	//	AVP_ENVIRONMENT_FURY161,
			1,	//	AVP_ENVIRONMENT_CAVERNS,
			0,	//	AVP_ENVIRONMENT_CAVERNSEND,

			1,	//	AVP_ENVIRONMENT_INVASION_P,
			1,	//	AVP_ENVIRONMENT_TYRARGO_P,
			1,	//	AVP_ENVIRONMENT_TEMPLE_P,
			1,	//	AVP_ENVIRONMENT_ESCAPE_P,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND_P,

			1,	//	AVP_ENVIRONMENT_TEMPLE,
			1,	//	AVP_ENVIRONMENT_ESCAPE,
			1,	//	AVP_ENVIRONMENT_FERARCO,
			1,	//	AVP_ENVIRONMENT_GATEWAY,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND,

			1,	//	AVP_ENVIRONMENT_INVASION_A,
			1,	//	AVP_ENVIRONMENT_DERELICT_A,
			1,	//	AVP_ENVIRONMENT_TYRARGO_A,
			1,	//	AVP_ENVIRONMENT_FURY161_A,
			1,	//	AVP_ENVIRONMENT_CAVERNS_A,
		}
	},
	// CHEATMODE_GRENADE,
	{
		{
			1,	// Marine			
			0,	// Predator			
			0,	// Alien
		},
		{
			1,	//	AVP_ENVIRONMENT_DERELICT,
			1,	//	AVP_ENVIRONMENT_COLONY,
			1,	//	AVP_ENVIRONMENT_INVASION,
			1,	//	AVP_ENVIRONMENT_ORBITAL,
			1,	//	AVP_ENVIRONMENT_TYRARGO,
			0,	//	AVP_ENVIRONMENT_TYRARGOHANGAR,

			1,	//	AVP_ENVIRONMENT_WATERFALL_M,
			1,	//	AVP_ENVIRONMENT_VAULTS_M,
			1,	//	AVP_ENVIRONMENT_FERARCO_M,
			1,	//	AVP_ENVIRONMENT_TEMPLE_M,
			1,	//	AVP_ENVIRONMENT_GATEWAY_M,

			1,	//	AVP_ENVIRONMENT_WATERFALL,
			1,	//	AVP_ENVIRONMENT_AREA52,
			1,	//	AVP_ENVIRONMENT_VAULTS,
			1,	//	AVP_ENVIRONMENT_FURY161,
			1,	//	AVP_ENVIRONMENT_CAVERNS,
			1,	//	AVP_ENVIRONMENT_CAVERNSEND,

			1,	//	AVP_ENVIRONMENT_INVASION_P,
			1,	//	AVP_ENVIRONMENT_TYRARGO_P,
			1,	//	AVP_ENVIRONMENT_TEMPLE_P,
			1,	//	AVP_ENVIRONMENT_ESCAPE_P,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND_P,

			1,	//	AVP_ENVIRONMENT_TEMPLE,
			1,	//	AVP_ENVIRONMENT_ESCAPE,
			1,	//	AVP_ENVIRONMENT_FERARCO,
			1,	//	AVP_ENVIRONMENT_GATEWAY,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND,

			1,	//	AVP_ENVIRONMENT_INVASION_A,
			1,	//	AVP_ENVIRONMENT_DERELICT_A,
			1,	//	AVP_ENVIRONMENT_TYRARGO_A,
			1,	//	AVP_ENVIRONMENT_FURY161_A,
			1,	//	AVP_ENVIRONMENT_CAVERNS_A,
		}
	},
	// CHEATMODE_MIRROR,
	{
		{
			1,	// Marine			
			1,	// Predator			
			1,	// Alien
		},
		{
			1,	//	AVP_ENVIRONMENT_DERELICT,
			1,	//	AVP_ENVIRONMENT_COLONY,
			1,	//	AVP_ENVIRONMENT_INVASION,
			1,	//	AVP_ENVIRONMENT_ORBITAL,
			1,	//	AVP_ENVIRONMENT_TYRARGO,
			0,	//	AVP_ENVIRONMENT_TYRARGOHANGAR,

			1,	//	AVP_ENVIRONMENT_WATERFALL_M,
			1,	//	AVP_ENVIRONMENT_VAULTS_M,
			1,	//	AVP_ENVIRONMENT_FERARCO_M,
			1,	//	AVP_ENVIRONMENT_TEMPLE_M,
			1,	//	AVP_ENVIRONMENT_GATEWAY_M,

			1,	//	AVP_ENVIRONMENT_WATERFALL,
			1,	//	AVP_ENVIRONMENT_AREA52,
			1,	//	AVP_ENVIRONMENT_VAULTS,
			1,	//	AVP_ENVIRONMENT_FURY161,
			1,	//	AVP_ENVIRONMENT_CAVERNS,
			0,	//	AVP_ENVIRONMENT_CAVERNSEND,

			1,	//	AVP_ENVIRONMENT_INVASION_P,
			1,	//	AVP_ENVIRONMENT_TYRARGO_P,
			1,	//	AVP_ENVIRONMENT_TEMPLE_P,
			1,	//	AVP_ENVIRONMENT_ESCAPE_P,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND_P,

			1,	//	AVP_ENVIRONMENT_TEMPLE,
			1,	//	AVP_ENVIRONMENT_ESCAPE,
			1,	//	AVP_ENVIRONMENT_FERARCO,
			1,	//	AVP_ENVIRONMENT_GATEWAY,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND,

			1,	//	AVP_ENVIRONMENT_INVASION_A,
			1,	//	AVP_ENVIRONMENT_DERELICT_A,
			1,	//	AVP_ENVIRONMENT_TYRARGO_A,
			1,	//	AVP_ENVIRONMENT_FURY161_A,
			1,	//	AVP_ENVIRONMENT_CAVERNS_A,
		}
	},
	// CHEATMODE_PIPECLEANER,
	{
		{
			1,	// Marine			
			1,	// Predator			
			1,	// Alien
		},
		{
			1,	//	AVP_ENVIRONMENT_DERELICT,
			1,	//	AVP_ENVIRONMENT_COLONY,
			1,	//	AVP_ENVIRONMENT_INVASION,
			1,	//	AVP_ENVIRONMENT_ORBITAL,
			1,	//	AVP_ENVIRONMENT_TYRARGO,
			0,	//	AVP_ENVIRONMENT_TYRARGOHANGAR,

			1,	//	AVP_ENVIRONMENT_WATERFALL_M,
			1,	//	AVP_ENVIRONMENT_VAULTS_M,
			1,	//	AVP_ENVIRONMENT_FERARCO_M,
			1,	//	AVP_ENVIRONMENT_TEMPLE_M,
			1,	//	AVP_ENVIRONMENT_GATEWAY_M,

			1,	//	AVP_ENVIRONMENT_WATERFALL,
			1,	//	AVP_ENVIRONMENT_AREA52,
			1,	//	AVP_ENVIRONMENT_VAULTS,
			1,	//	AVP_ENVIRONMENT_FURY161,
			1,	//	AVP_ENVIRONMENT_CAVERNS,
			0,	//	AVP_ENVIRONMENT_CAVERNSEND,

			1,	//	AVP_ENVIRONMENT_INVASION_P,
			1,	//	AVP_ENVIRONMENT_TYRARGO_P,
			1,	//	AVP_ENVIRONMENT_TEMPLE_P,
			1,	//	AVP_ENVIRONMENT_ESCAPE_P,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND_P,

			1,	//	AVP_ENVIRONMENT_TEMPLE,
			1,	//	AVP_ENVIRONMENT_ESCAPE,
			1,	//	AVP_ENVIRONMENT_FERARCO,
			1,	//	AVP_ENVIRONMENT_GATEWAY,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND,

			1,	//	AVP_ENVIRONMENT_INVASION_A,
			1,	//	AVP_ENVIRONMENT_DERELICT_A,
			1,	//	AVP_ENVIRONMENT_TYRARGO_A,
			1,	//	AVP_ENVIRONMENT_FURY161_A,
			1,	//	AVP_ENVIRONMENT_CAVERNS_A,
		}
	},
	// CHEATMODE_DISCOINFERNO
	{
		{
			1,	// Marine			
			1,	// Predator			
			1,	// Alien
		},
		{
			1,	//	AVP_ENVIRONMENT_DERELICT,
			1,	//	AVP_ENVIRONMENT_COLONY,
			1,	//	AVP_ENVIRONMENT_INVASION,
			1,	//	AVP_ENVIRONMENT_ORBITAL,
			1,	//	AVP_ENVIRONMENT_TYRARGO,
			0,	//	AVP_ENVIRONMENT_TYRARGOHANGAR,

			1,	//	AVP_ENVIRONMENT_WATERFALL_M,
			1,	//	AVP_ENVIRONMENT_VAULTS_M,
			1,	//	AVP_ENVIRONMENT_FERARCO_M,
			1,	//	AVP_ENVIRONMENT_TEMPLE_M,
			1,	//	AVP_ENVIRONMENT_GATEWAY_M,

			1,	//	AVP_ENVIRONMENT_WATERFALL,
			1,	//	AVP_ENVIRONMENT_AREA52,
			1,	//	AVP_ENVIRONMENT_VAULTS,
			1,	//	AVP_ENVIRONMENT_FURY161,
			1,	//	AVP_ENVIRONMENT_CAVERNS,
			0,	//	AVP_ENVIRONMENT_CAVERNSEND,

			1,	//	AVP_ENVIRONMENT_INVASION_P,
			1,	//	AVP_ENVIRONMENT_TYRARGO_P,
			1,	//	AVP_ENVIRONMENT_TEMPLE_P,
			1,	//	AVP_ENVIRONMENT_ESCAPE_P,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND_P,

			1,	//	AVP_ENVIRONMENT_TEMPLE,
			1,	//	AVP_ENVIRONMENT_ESCAPE,
			1,	//	AVP_ENVIRONMENT_FERARCO,
			1,	//	AVP_ENVIRONMENT_GATEWAY,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND,

			1,	//	AVP_ENVIRONMENT_INVASION_A,
			1,	//	AVP_ENVIRONMENT_DERELICT_A,
			1,	//	AVP_ENVIRONMENT_TYRARGO_A,
			1,	//	AVP_ENVIRONMENT_FURY161_A,
			1,	//	AVP_ENVIRONMENT_CAVERNS_A,
		}
	},
	// CHEATMODE_TRIPTASTIC
	{
		{
			1,	// Marine			
			1,	// Predator			
			1,	// Alien
		},
		{
			1,	//	AVP_ENVIRONMENT_DERELICT,
			1,	//	AVP_ENVIRONMENT_COLONY,
			1,	//	AVP_ENVIRONMENT_INVASION,
			1,	//	AVP_ENVIRONMENT_ORBITAL,
			1,	//	AVP_ENVIRONMENT_TYRARGO,
			0,	//	AVP_ENVIRONMENT_TYRARGOHANGAR,

			1,	//	AVP_ENVIRONMENT_WATERFALL_M,
			1,	//	AVP_ENVIRONMENT_VAULTS_M,
			1,	//	AVP_ENVIRONMENT_FERARCO_M,
			1,	//	AVP_ENVIRONMENT_TEMPLE_M,
			1,	//	AVP_ENVIRONMENT_GATEWAY_M,

			1,	//	AVP_ENVIRONMENT_WATERFALL,
			1,	//	AVP_ENVIRONMENT_AREA52,
			1,	//	AVP_ENVIRONMENT_VAULTS,
			1,	//	AVP_ENVIRONMENT_FURY161,
			1,	//	AVP_ENVIRONMENT_CAVERNS,
			0,	//	AVP_ENVIRONMENT_CAVERNSEND,

			1,	//	AVP_ENVIRONMENT_INVASION_P,
			1,	//	AVP_ENVIRONMENT_TYRARGO_P,
			1,	//	AVP_ENVIRONMENT_TEMPLE_P,
			1,	//	AVP_ENVIRONMENT_ESCAPE_P,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND_P,

			1,	//	AVP_ENVIRONMENT_TEMPLE,
			1,	//	AVP_ENVIRONMENT_ESCAPE,
			1,	//	AVP_ENVIRONMENT_FERARCO,
			1,	//	AVP_ENVIRONMENT_GATEWAY,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND,

			1,	//	AVP_ENVIRONMENT_INVASION_A,
			1,	//	AVP_ENVIRONMENT_DERELICT_A,
			1,	//	AVP_ENVIRONMENT_TYRARGO_A,
			1,	//	AVP_ENVIRONMENT_FURY161_A,
			1,	//	AVP_ENVIRONMENT_CAVERNS_A,
		}
	},
	// CHEATMODE_MOTIONBLUR
	{
		{
			1,	// Marine			
			1,	// Predator			
			1,	// Alien
		},
		{
			1,	//	AVP_ENVIRONMENT_DERELICT,
			1,	//	AVP_ENVIRONMENT_COLONY,
			1,	//	AVP_ENVIRONMENT_INVASION,
			1,	//	AVP_ENVIRONMENT_ORBITAL,
			1,	//	AVP_ENVIRONMENT_TYRARGO,
			0,	//	AVP_ENVIRONMENT_TYRARGOHANGAR,

			1,	//	AVP_ENVIRONMENT_WATERFALL_M,
			1,	//	AVP_ENVIRONMENT_VAULTS_M,
			1,	//	AVP_ENVIRONMENT_FERARCO_M,
			1,	//	AVP_ENVIRONMENT_TEMPLE_M,
			1,	//	AVP_ENVIRONMENT_GATEWAY_M,

			1,	//	AVP_ENVIRONMENT_WATERFALL,
			1,	//	AVP_ENVIRONMENT_AREA52,
			1,	//	AVP_ENVIRONMENT_VAULTS,
			1,	//	AVP_ENVIRONMENT_FURY161,
			1,	//	AVP_ENVIRONMENT_CAVERNS,
			0,	//	AVP_ENVIRONMENT_CAVERNSEND,

			1,	//	AVP_ENVIRONMENT_INVASION_P,
			1,	//	AVP_ENVIRONMENT_TYRARGO_P,
			1,	//	AVP_ENVIRONMENT_TEMPLE_P,
			1,	//	AVP_ENVIRONMENT_ESCAPE_P,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND_P,

			1,	//	AVP_ENVIRONMENT_TEMPLE,
			1,	//	AVP_ENVIRONMENT_ESCAPE,
			1,	//	AVP_ENVIRONMENT_FERARCO,
			1,	//	AVP_ENVIRONMENT_GATEWAY,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND,

			1,	//	AVP_ENVIRONMENT_INVASION_A,
			1,	//	AVP_ENVIRONMENT_DERELICT_A,
			1,	//	AVP_ENVIRONMENT_TYRARGO_A,
			1,	//	AVP_ENVIRONMENT_FURY161_A,
			1,	//	AVP_ENVIRONMENT_CAVERNS_A,
		}
	},
	// CHEATMODE_UNDERWATER
	{
		{
			1,	// Marine			
			1,	// Predator			
			1,	// Alien
		},
		{
			1,	//	AVP_ENVIRONMENT_DERELICT,
			1,	//	AVP_ENVIRONMENT_COLONY,
			1,	//	AVP_ENVIRONMENT_INVASION,
			1,	//	AVP_ENVIRONMENT_ORBITAL,
			1,	//	AVP_ENVIRONMENT_TYRARGO,
			0,	//	AVP_ENVIRONMENT_TYRARGOHANGAR,

			1,	//	AVP_ENVIRONMENT_WATERFALL_M,
			1,	//	AVP_ENVIRONMENT_VAULTS_M,
			1,	//	AVP_ENVIRONMENT_FERARCO_M,
			1,	//	AVP_ENVIRONMENT_TEMPLE_M,
			1,	//	AVP_ENVIRONMENT_GATEWAY_M,

			1,	//	AVP_ENVIRONMENT_WATERFALL,
			1,	//	AVP_ENVIRONMENT_AREA52,
			1,	//	AVP_ENVIRONMENT_VAULTS,
			1,	//	AVP_ENVIRONMENT_FURY161,
			1,	//	AVP_ENVIRONMENT_CAVERNS,
			0,	//	AVP_ENVIRONMENT_CAVERNSEND,

			1,	//	AVP_ENVIRONMENT_INVASION_P,
			1,	//	AVP_ENVIRONMENT_TYRARGO_P,
			1,	//	AVP_ENVIRONMENT_TEMPLE_P,
			1,	//	AVP_ENVIRONMENT_ESCAPE_P,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND_P,

			1,	//	AVP_ENVIRONMENT_TEMPLE,
			1,	//	AVP_ENVIRONMENT_ESCAPE,
			1,	//	AVP_ENVIRONMENT_FERARCO,
			1,	//	AVP_ENVIRONMENT_GATEWAY,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND,

			1,	//	AVP_ENVIRONMENT_INVASION_A,
			1,	//	AVP_ENVIRONMENT_DERELICT_A,
			1,	//	AVP_ENVIRONMENT_TYRARGO_A,
			1,	//	AVP_ENVIRONMENT_FURY161_A,
			1,	//	AVP_ENVIRONMENT_CAVERNS_A,
		}
	},
	// CHEATMODE_JOHNWOOMODE
	{
		{
			1,	// Marine			
			1,	// Predator			
			1,	// Alien
		},
		{
			1,	//	AVP_ENVIRONMENT_DERELICT,
			1,	//	AVP_ENVIRONMENT_COLONY,
			1,	//	AVP_ENVIRONMENT_INVASION,
			1,	//	AVP_ENVIRONMENT_ORBITAL,
			1,	//	AVP_ENVIRONMENT_TYRARGO,
			0,	//	AVP_ENVIRONMENT_TYRARGOHANGAR,

			1,	//	AVP_ENVIRONMENT_WATERFALL_M,
			1,	//	AVP_ENVIRONMENT_VAULTS_M,
			1,	//	AVP_ENVIRONMENT_FERARCO_M,
			1,	//	AVP_ENVIRONMENT_TEMPLE_M,
			1,	//	AVP_ENVIRONMENT_GATEWAY_M,

			1,	//	AVP_ENVIRONMENT_WATERFALL,
			1,	//	AVP_ENVIRONMENT_AREA52,
			1,	//	AVP_ENVIRONMENT_VAULTS,
			1,	//	AVP_ENVIRONMENT_FURY161,
			1,	//	AVP_ENVIRONMENT_CAVERNS,
			0,	//	AVP_ENVIRONMENT_CAVERNSEND,

			1,	//	AVP_ENVIRONMENT_INVASION_P,
			1,	//	AVP_ENVIRONMENT_TYRARGO_P,
			1,	//	AVP_ENVIRONMENT_TEMPLE_P,
			1,	//	AVP_ENVIRONMENT_ESCAPE_P,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND_P,

			1,	//	AVP_ENVIRONMENT_TEMPLE,
			1,	//	AVP_ENVIRONMENT_ESCAPE,
			1,	//	AVP_ENVIRONMENT_FERARCO,
			1,	//	AVP_ENVIRONMENT_GATEWAY,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND,

			1,	//	AVP_ENVIRONMENT_INVASION_A,
			1,	//	AVP_ENVIRONMENT_DERELICT_A,
			1,	//	AVP_ENVIRONMENT_TYRARGO_A,
			1,	//	AVP_ENVIRONMENT_FURY161_A,
			1,	//	AVP_ENVIRONMENT_CAVERNS_A,
		}
	},
	// CHEATMODE_WARPSPEED
	{
		{
			1,	// Marine			
			1,	// Predator			
			1,	// Alien
		},
		{
			1,	//	AVP_ENVIRONMENT_DERELICT,
			1,	//	AVP_ENVIRONMENT_COLONY,
			1,	//	AVP_ENVIRONMENT_INVASION,
			1,	//	AVP_ENVIRONMENT_ORBITAL,
			1,	//	AVP_ENVIRONMENT_TYRARGO,
			0,	//	AVP_ENVIRONMENT_TYRARGOHANGAR,

			1,	//	AVP_ENVIRONMENT_WATERFALL_M,
			1,	//	AVP_ENVIRONMENT_VAULTS_M,
			1,	//	AVP_ENVIRONMENT_FERARCO_M,
			1,	//	AVP_ENVIRONMENT_TEMPLE_M,
			1,	//	AVP_ENVIRONMENT_GATEWAY_M,

			1,	//	AVP_ENVIRONMENT_WATERFALL,
			1,	//	AVP_ENVIRONMENT_AREA52,
			1,	//	AVP_ENVIRONMENT_VAULTS,
			1,	//	AVP_ENVIRONMENT_FURY161,
			1,	//	AVP_ENVIRONMENT_CAVERNS,
			0,	//	AVP_ENVIRONMENT_CAVERNSEND,

			1,	//	AVP_ENVIRONMENT_INVASION_P,
			1,	//	AVP_ENVIRONMENT_TYRARGO_P,
			1,	//	AVP_ENVIRONMENT_TEMPLE_P,
			1,	//	AVP_ENVIRONMENT_ESCAPE_P,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND_P,

			1,	//	AVP_ENVIRONMENT_TEMPLE,
			1,	//	AVP_ENVIRONMENT_ESCAPE,
			1,	//	AVP_ENVIRONMENT_FERARCO,
			1,	//	AVP_ENVIRONMENT_GATEWAY,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND,

			1,	//	AVP_ENVIRONMENT_INVASION_A,
			1,	//	AVP_ENVIRONMENT_DERELICT_A,
			1,	//	AVP_ENVIRONMENT_TYRARGO_A,
			1,	//	AVP_ENVIRONMENT_FURY161_A,
			1,	//	AVP_ENVIRONMENT_CAVERNS_A,
		}
	},
	// CHEATMODE_LANDOFTHEGIANTS,
	{
		{
			1,	// Marine			
			1,	// Predator			
			0,	// Alien
		},
		{
			1,	//	AVP_ENVIRONMENT_DERELICT,
			1,	//	AVP_ENVIRONMENT_COLONY,
			1,	//	AVP_ENVIRONMENT_INVASION,
			1,	//	AVP_ENVIRONMENT_ORBITAL,
			1,	//	AVP_ENVIRONMENT_TYRARGO,
			0,	//	AVP_ENVIRONMENT_TYRARGOHANGAR,

			1,	//	AVP_ENVIRONMENT_WATERFALL_M,
			1,	//	AVP_ENVIRONMENT_VAULTS_M,
			1,	//	AVP_ENVIRONMENT_FERARCO_M,
			1,	//	AVP_ENVIRONMENT_TEMPLE_M,
			1,	//	AVP_ENVIRONMENT_GATEWAY_M,

			1,	//	AVP_ENVIRONMENT_WATERFALL,
			1,	//	AVP_ENVIRONMENT_AREA52,
			1,	//	AVP_ENVIRONMENT_VAULTS,
			1,	//	AVP_ENVIRONMENT_FURY161,
			1,	//	AVP_ENVIRONMENT_CAVERNS,
			0,	//	AVP_ENVIRONMENT_CAVERNSEND,

			1,	//	AVP_ENVIRONMENT_INVASION_P,
			1,	//	AVP_ENVIRONMENT_TYRARGO_P,
			1,	//	AVP_ENVIRONMENT_TEMPLE_P,
			1,	//	AVP_ENVIRONMENT_ESCAPE_P,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND_P,

			1,	//	AVP_ENVIRONMENT_TEMPLE,
			1,	//	AVP_ENVIRONMENT_ESCAPE,
			1,	//	AVP_ENVIRONMENT_FERARCO,
			1,	//	AVP_ENVIRONMENT_GATEWAY,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND,

			1,	//	AVP_ENVIRONMENT_INVASION_A,
			1,	//	AVP_ENVIRONMENT_DERELICT_A,
			1,	//	AVP_ENVIRONMENT_TYRARGO_A,
			1,	//	AVP_ENVIRONMENT_FURY161_A,
			1,	//	AVP_ENVIRONMENT_CAVERNS_A,
		}
	},
	// CHEATMODE_IMPOSSIBLEMISSION,
	{
		{
			1,	// Marine			
			0,	// Predator			
			1,	// Alien
		},
		{
			1,	//	AVP_ENVIRONMENT_DERELICT,
			1,	//	AVP_ENVIRONMENT_COLONY,
			1,	//	AVP_ENVIRONMENT_INVASION,
			1,	//	AVP_ENVIRONMENT_ORBITAL,
			1,	//	AVP_ENVIRONMENT_TYRARGO,
			0,	//	AVP_ENVIRONMENT_TYRARGOHANGAR,

			1,	//	AVP_ENVIRONMENT_WATERFALL_M,
			1,	//	AVP_ENVIRONMENT_VAULTS_M,
			1,	//	AVP_ENVIRONMENT_FERARCO_M,
			1,	//	AVP_ENVIRONMENT_TEMPLE_M,
			1,	//	AVP_ENVIRONMENT_GATEWAY_M,

			1,	//	AVP_ENVIRONMENT_WATERFALL,
			1,	//	AVP_ENVIRONMENT_AREA52,
			1,	//	AVP_ENVIRONMENT_VAULTS,
			1,	//	AVP_ENVIRONMENT_FURY161,
			1,	//	AVP_ENVIRONMENT_CAVERNS,
			0,	//	AVP_ENVIRONMENT_CAVERNSEND,

			1,	//	AVP_ENVIRONMENT_INVASION_P,
			1,	//	AVP_ENVIRONMENT_TYRARGO_P,
			1,	//	AVP_ENVIRONMENT_TEMPLE_P,
			1,	//	AVP_ENVIRONMENT_ESCAPE_P,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND_P,

			1,	//	AVP_ENVIRONMENT_TEMPLE,
			1,	//	AVP_ENVIRONMENT_ESCAPE,
			1,	//	AVP_ENVIRONMENT_FERARCO,
			1,	//	AVP_ENVIRONMENT_GATEWAY,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND,

			1,	//	AVP_ENVIRONMENT_INVASION_A,
			1,	//	AVP_ENVIRONMENT_DERELICT_A,
			1,	//	AVP_ENVIRONMENT_TYRARGO_A,
			1,	//	AVP_ENVIRONMENT_FURY161_A,
			1,	//	AVP_ENVIRONMENT_CAVERNS_A,
		}
	},
	// CHEATMODE_RAINBOWBLOOD,
	{
		{
			1,	// Marine			
			1,	// Predator			
			1,	// Alien
		},
		{
			1,	//	AVP_ENVIRONMENT_DERELICT,
			1,	//	AVP_ENVIRONMENT_COLONY,
			1,	//	AVP_ENVIRONMENT_INVASION,
			1,	//	AVP_ENVIRONMENT_ORBITAL,
			1,	//	AVP_ENVIRONMENT_TYRARGO,
			0,	//	AVP_ENVIRONMENT_TYRARGOHANGAR,

			1,	//	AVP_ENVIRONMENT_WATERFALL_M,
			1,	//	AVP_ENVIRONMENT_VAULTS_M,
			1,	//	AVP_ENVIRONMENT_FERARCO_M,
			1,	//	AVP_ENVIRONMENT_TEMPLE_M,
			1,	//	AVP_ENVIRONMENT_GATEWAY_M,

			1,	//	AVP_ENVIRONMENT_WATERFALL,
			1,	//	AVP_ENVIRONMENT_AREA52,
			1,	//	AVP_ENVIRONMENT_VAULTS,
			1,	//	AVP_ENVIRONMENT_FURY161,
			1,	//	AVP_ENVIRONMENT_CAVERNS,
			0,	//	AVP_ENVIRONMENT_CAVERNSEND,

			1,	//	AVP_ENVIRONMENT_INVASION_P,
			1,	//	AVP_ENVIRONMENT_TYRARGO_P,
			1,	//	AVP_ENVIRONMENT_TEMPLE_P,
			1,	//	AVP_ENVIRONMENT_ESCAPE_P,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND_P,

			1,	//	AVP_ENVIRONMENT_TEMPLE,
			1,	//	AVP_ENVIRONMENT_ESCAPE,
			1,	//	AVP_ENVIRONMENT_FERARCO,
			1,	//	AVP_ENVIRONMENT_GATEWAY,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND,

			1,	//	AVP_ENVIRONMENT_INVASION_A,
			1,	//	AVP_ENVIRONMENT_DERELICT_A,
			1,	//	AVP_ENVIRONMENT_TYRARGO_A,
			1,	//	AVP_ENVIRONMENT_FURY161_A,
			1,	//	AVP_ENVIRONMENT_CAVERNS_A,
		}
	},
	// CHEATMODE_TICKERTAPE,
	{
		{
			1,	// Marine			
			1,	// Predator			
			0,	// Alien
		},
		{
			1,	//	AVP_ENVIRONMENT_DERELICT,
			1,	//	AVP_ENVIRONMENT_COLONY,
			1,	//	AVP_ENVIRONMENT_INVASION,
			1,	//	AVP_ENVIRONMENT_ORBITAL,
			1,	//	AVP_ENVIRONMENT_TYRARGO,
			0,	//	AVP_ENVIRONMENT_TYRARGOHANGAR,

			1,	//	AVP_ENVIRONMENT_WATERFALL_M,
			1,	//	AVP_ENVIRONMENT_VAULTS_M,
			1,	//	AVP_ENVIRONMENT_FERARCO_M,
			1,	//	AVP_ENVIRONMENT_TEMPLE_M,
			1,	//	AVP_ENVIRONMENT_GATEWAY_M,

			0,	//	AVP_ENVIRONMENT_WATERFALL,
			0,	//	AVP_ENVIRONMENT_AREA52,
			1,	//	AVP_ENVIRONMENT_VAULTS,
			1,	//	AVP_ENVIRONMENT_FURY161,
			1,	//	AVP_ENVIRONMENT_CAVERNS,
			0,	//	AVP_ENVIRONMENT_CAVERNSEND,

			1,	//	AVP_ENVIRONMENT_INVASION_P,
			1,	//	AVP_ENVIRONMENT_TYRARGO_P,
			1,	//	AVP_ENVIRONMENT_TEMPLE_P,
			1,	//	AVP_ENVIRONMENT_ESCAPE_P,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND_P,

			1,	//	AVP_ENVIRONMENT_TEMPLE,
			1,	//	AVP_ENVIRONMENT_ESCAPE,
			1,	//	AVP_ENVIRONMENT_FERARCO,
			1,	//	AVP_ENVIRONMENT_GATEWAY,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND,

			1,	//	AVP_ENVIRONMENT_INVASION_A,
			1,	//	AVP_ENVIRONMENT_DERELICT_A,
			1,	//	AVP_ENVIRONMENT_TYRARGO_A,
			1,	//	AVP_ENVIRONMENT_FURY161_A,
			1,	//	AVP_ENVIRONMENT_CAVERNS_A,
		}
	},
	// CHEATMODE_NAUSEA,
	{
		{
			1,	// Marine			
			1,	// Predator			
			1,	// Alien
		},
		{
			1,	//	AVP_ENVIRONMENT_DERELICT,
			1,	//	AVP_ENVIRONMENT_COLONY,
			1,	//	AVP_ENVIRONMENT_INVASION,
			1,	//	AVP_ENVIRONMENT_ORBITAL,
			1,	//	AVP_ENVIRONMENT_TYRARGO,
			0,	//	AVP_ENVIRONMENT_TYRARGOHANGAR,

			1,	//	AVP_ENVIRONMENT_WATERFALL_M,
			1,	//	AVP_ENVIRONMENT_VAULTS_M,
			1,	//	AVP_ENVIRONMENT_FERARCO_M,
			1,	//	AVP_ENVIRONMENT_TEMPLE_M,
			1,	//	AVP_ENVIRONMENT_GATEWAY_M,

			1,	//	AVP_ENVIRONMENT_WATERFALL,
			1,	//	AVP_ENVIRONMENT_AREA52,
			1,	//	AVP_ENVIRONMENT_VAULTS,
			1,	//	AVP_ENVIRONMENT_FURY161,
			1,	//	AVP_ENVIRONMENT_CAVERNS,
			0,	//	AVP_ENVIRONMENT_CAVERNSEND,

			1,	//	AVP_ENVIRONMENT_INVASION_P,
			1,	//	AVP_ENVIRONMENT_TYRARGO_P,
			1,	//	AVP_ENVIRONMENT_TEMPLE_P,
			1,	//	AVP_ENVIRONMENT_ESCAPE_P,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND_P,

			1,	//	AVP_ENVIRONMENT_TEMPLE,
			1,	//	AVP_ENVIRONMENT_ESCAPE,
			1,	//	AVP_ENVIRONMENT_FERARCO,
			1,	//	AVP_ENVIRONMENT_GATEWAY,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND,

			1,	//	AVP_ENVIRONMENT_INVASION_A,
			1,	//	AVP_ENVIRONMENT_DERELICT_A,
			1,	//	AVP_ENVIRONMENT_TYRARGO_A,
			1,	//	AVP_ENVIRONMENT_FURY161_A,
			1,	//	AVP_ENVIRONMENT_CAVERNS_A,
		}
	},
	// CHEATMODE_FREEFALL,
	{
		{
			1,	// Marine			
			1,	// Predator			
			1,	// Alien
		},
		{
			0,	//	AVP_ENVIRONMENT_DERELICT,
			0,	//	AVP_ENVIRONMENT_COLONY,
			0,	//	AVP_ENVIRONMENT_INVASION,
			0,	//	AVP_ENVIRONMENT_ORBITAL,
			0,	//	AVP_ENVIRONMENT_TYRARGO,
			0,	//	AVP_ENVIRONMENT_TYRARGOHANGAR,

			0,	//	AVP_ENVIRONMENT_WATERFALL_M,
			0,	//	AVP_ENVIRONMENT_VAULTS_M,
			0,	//	AVP_ENVIRONMENT_FERARCO_M,
			1,	//	AVP_ENVIRONMENT_TEMPLE_M,
			0,	//	AVP_ENVIRONMENT_GATEWAY_M,

			0,	//	AVP_ENVIRONMENT_WATERFALL,
			0,	//	AVP_ENVIRONMENT_AREA52,
			0,	//	AVP_ENVIRONMENT_VAULTS,
			0,	//	AVP_ENVIRONMENT_FURY161,
			0,	//	AVP_ENVIRONMENT_CAVERNS,
			0,	//	AVP_ENVIRONMENT_CAVERNSEND,

			0,	//	AVP_ENVIRONMENT_INVASION_P,
			0,	//	AVP_ENVIRONMENT_TYRARGO_P,
			1,	//	AVP_ENVIRONMENT_TEMPLE_P,
			0,	//	AVP_ENVIRONMENT_ESCAPE_P,
			0,	//	AVP_ENVIRONMENT_EARTHBOUND_P,

			1,	//	AVP_ENVIRONMENT_TEMPLE,
			1,	//	AVP_ENVIRONMENT_ESCAPE,
			1,	//	AVP_ENVIRONMENT_FERARCO,
			1,	//	AVP_ENVIRONMENT_GATEWAY,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND,

			1,	//	AVP_ENVIRONMENT_INVASION_A,
			1,	//	AVP_ENVIRONMENT_DERELICT_A,
			1,	//	AVP_ENVIRONMENT_TYRARGO_A,
			1,	//	AVP_ENVIRONMENT_FURY161_A,
			1,	//	AVP_ENVIRONMENT_CAVERNS_A,
		}
	},
	// CHEATMODE_BALLSOFFIRE,
	{
		{
			1,	// Marine			
			1,	// Predator			
			1,	// Alien
		},
		{
			1,	//	AVP_ENVIRONMENT_DERELICT,
			1,	//	AVP_ENVIRONMENT_COLONY,
			1,	//	AVP_ENVIRONMENT_INVASION,
			1,	//	AVP_ENVIRONMENT_ORBITAL,
			1,	//	AVP_ENVIRONMENT_TYRARGO,
			0,	//	AVP_ENVIRONMENT_TYRARGOHANGAR,

			1,	//	AVP_ENVIRONMENT_WATERFALL_M,
			1,	//	AVP_ENVIRONMENT_VAULTS_M,
			1,	//	AVP_ENVIRONMENT_FERARCO_M,
			1,	//	AVP_ENVIRONMENT_TEMPLE_M,
			1,	//	AVP_ENVIRONMENT_GATEWAY_M,

			1,	//	AVP_ENVIRONMENT_WATERFALL,
			1,	//	AVP_ENVIRONMENT_AREA52,
			1,	//	AVP_ENVIRONMENT_VAULTS,
			1,	//	AVP_ENVIRONMENT_FURY161,
			1,	//	AVP_ENVIRONMENT_CAVERNS,
			0,	//	AVP_ENVIRONMENT_CAVERNSEND,

			1,	//	AVP_ENVIRONMENT_INVASION_P,
			1,	//	AVP_ENVIRONMENT_TYRARGO_P,
			1,	//	AVP_ENVIRONMENT_TEMPLE_P,
			1,	//	AVP_ENVIRONMENT_ESCAPE_P,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND_P,

			1,	//	AVP_ENVIRONMENT_TEMPLE,
			1,	//	AVP_ENVIRONMENT_ESCAPE,
			1,	//	AVP_ENVIRONMENT_FERARCO,
			1,	//	AVP_ENVIRONMENT_GATEWAY,
			1,	//	AVP_ENVIRONMENT_EARTHBOUND,

			1,	//	AVP_ENVIRONMENT_INVASION_A,
			1,	//	AVP_ENVIRONMENT_DERELICT_A,
			1,	//	AVP_ENVIRONMENT_TYRARGO_A,
			1,	//	AVP_ENVIRONMENT_FURY161_A,
			1,	//	AVP_ENVIRONMENT_CAVERNS_A,
		}
	},


};

extern void CheatMode_GetNextAllowedSpecies(int *speciesPtr, int searchForward)
{
	while(!(CheatModeDesc[CheatMode_Active].PlayableForSpecies[*speciesPtr]
			&&ThereIsAnAllowedEnvironment()) )
	{
		if (searchForward)
		{
			(*speciesPtr)++;
			if (*speciesPtr>= NUMBER_OF_SPECIES)
			{
				*speciesPtr = 0;
			}
		}
		else
		{
			if (*speciesPtr==0)
			{
				*speciesPtr = NUMBER_OF_SPECIES;
			}
			(*speciesPtr)--;
		}
	}
}

static int ThereIsAnAllowedEnvironment(void)
{
	int i;
	for (i=0; i<NUMBER_OF_ENVIRONMENTS; i++)
	{
		if (EnvironmentPlayableBySpecies(i)) return 1;
	}
	return 0;
}

static int EnvironmentPlayableBySpecies(int environment)
{
	switch (CheatMode_Species)
	{
		default:
		case 0:
		{
			if (environment<=10)
			{
				if(UserProfilePtr->LevelCompleted[0][environment]) return 1;
			}
			return 0;
		}
		case 1:
		{
			if (environment>=11 && environment<=21)
			{
				if(UserProfilePtr->LevelCompleted[1][environment-11]) return 1;
			}
			return 0;
		}
		case 2:
		{
			if (environment>=22)
			{
				if(UserProfilePtr->LevelCompleted[2][environment-22]) return 1;
			}
			return 0;
		}
	}
}
extern void CheatMode_GetNextAllowedEnvironment(int *environmentPtr, int searchForward)
{
	while(! (CheatModeDesc[CheatMode_Active].PlayableForEnvironment[*environmentPtr]
			 && EnvironmentPlayableBySpecies(*environmentPtr) ) )
	{
		if (searchForward)
		{
			(*environmentPtr)++;
			if (*environmentPtr>=NUMBER_OF_ENVIRONMENTS)
			{
				*environmentPtr = 0;
			}
		}
		else
		{
			if (*environmentPtr==0)
			{
				*environmentPtr = NUMBER_OF_ENVIRONMENTS;
			}
			(*environmentPtr)--;
		}
	}
}

extern void CheatMode_GetNextAllowedMode(int *cheatModePtr, int searchForward)
{														  
	while(! (UserProfilePtr->CheatMode[*cheatModePtr]))
	{
		if (searchForward)
		{
			(*cheatModePtr)++;
			if (*cheatModePtr >= MAX_NUMBER_OF_CHEATMODES)
			{
				*cheatModePtr = 0;
			}
		}
		else
		{
			if (*cheatModePtr==0)
			{
				*cheatModePtr = MAX_NUMBER_OF_CHEATMODES;
			}
			(*cheatModePtr)--;
		}
	}
}

extern int AnyCheatModesAllowed(void)
{
	int i;
	for (i=0; i<MAX_NUMBER_OF_CHEATMODES; i++)
	{
		if (UserProfilePtr->CheatMode[i])
		{
			return 1;
		}
	}
	return 0;
}