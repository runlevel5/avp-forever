#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fixer.h"

#include "3dc.h"
#include "platform.h"
#include "psndplat.h"
#include "module.h"
#include "stratdef.h"
#include "avp_userprofile.h"
#include "projfont.h"
#include "savegame.h"
#include "pldnet.h"
#include "krender.h"
#include "kshape.h"
#include "d3d_hud.h"


/* winmain.c */
BOOL KeepMainRifFile = FALSE;
int HWAccel = 1;


/* krender.c -- thought this file was unused */
void KDraw_Item_2dTexturePolygon(int *itemptr)
{
	fprintf(stderr, "KDraw_Item_2dTexturePolygon(%p)\n", itemptr);
}


/* item.c -- thought this file was unused */
void Draw_Item_2dTexturePolygon(int *itemptr)
{
	extern int ScanDrawMode;
	
	if (ScanDrawMode == ScanDrawDirectDraw) {
		KDraw_Item_2dTexturePolygon(itemptr);
	}
}


/* indexfnt.cpp */
void INDEXFNT_PFLoadHook(FontIndex I_Font_New, PFFONT *pffont_New)
{
	fprintf(stderr, "INDEXFNT_PFLoadHook(%d, %p)\n", I_Font_New, pffont_New);
}	


/* avp_mp_config.cpp */
char* GetCustomMultiplayerLevelName(int index, int gameType)
{
	fprintf(stderr, "GetCustomMultiplayerLevelName(%d, %d)\n", index, gameType);
	
	return NULL;
}

void BuildMultiplayerLevelNameArray()
{
	fprintf(stderr, "BuildMultiplayerLevelNameArray()\n");
}

BOOL BuildLoadIPAddressMenu()
{
	fprintf(stderr, "BuildLoadIPAddressMenu()\n");
	
	return FALSE;
}


/* avp_intro.cpp */
void DrawMainMenusBackdrop()
{
	fprintf(stderr, "DrawMainMenusBackdrop()\n");
}


/* cd_player.cpp */
int CDPlayerVolume;

void CheckCDVolume()
{
	fprintf(stderr, "CheckCDVolume()\n");
}


/* smacker.c */
int FmvColourRed;
int FmvColourGreen;
int FmvColourBlue;
int IntroOutroMoviesAreActive = 1;
int MoviesAreActive;

void GetFMVInformation(int *messageNumberPtr, int *frameNumberPtr)
{
	fprintf(stderr, "GetFMVInformation(%p, %p)\n", messageNumberPtr, frameNumberPtr);
}

void InitialiseTriggeredFMVs()
{
	fprintf(stderr, "InitialiseTriggeredFMVs()\n");
}

void StartFMVAtFrame(int number, int frame)
{
	fprintf(stderr, "StartFMVAtFrame(%d, %d)\n", number, frame);
}

void StartTriggerPlotFMV(int number)
{
	fprintf(stderr, "StartTriggerPlotFMV(%d)\n", number);
}

void UpdateAllFMVTextures()
{
	fprintf(stderr, "UpdateAllFMVTextures()\n");
}


/* alt_tab.cpp */
void ATIncludeSurface(void * pSurface, void * hBackup)
{
	fprintf(stderr, "ATIncludeSurface(%p, %p)\n", pSurface, hBackup);
}

void ATRemoveSurface(void * pSurface)
{
	fprintf(stderr, "ATRemoveSurface(%p)\n", pSurface);
}

void ATRemoveTexture(void * pTexture)
{
	fprintf(stderr, "ATRemoveTexture(%p)\n", pTexture);
}


/* avp_menugfx.cpp */
char AAFontWidths[256];
AVPMENUGFX AvPMenuGfxStorage[MAX_NO_OF_AVPMENUGFXS]; /* TODO: this is initialized in avp_menugfx.cpp */

void FadedScreen(int alpha)
{
	fprintf(stderr, "FadedScreen(%d)\n", alpha);
}

void ClearScreenToBlack()
{
	fprintf(stderr, "ClearScreenToBlack()\n");
}

void DrawAvPMenuGfx(enum AVPMENUGFX_ID menuGfxID, int topleftX, int topleftY, int alpha,enum AVPMENUFORMAT_ID format)
{
	fprintf(stderr, "DrawAvPMenuGfx(%d, %d, %d, %d, %d)\n", menuGfxID, topleftX, topleftY, alpha, format);
}

void DrawAvPMenuGfx_CrossFade(enum AVPMENUGFX_ID menuGfxID,enum AVPMENUGFX_ID menuGfxID2,int alpha)
{
	fprintf(stderr, "DrawAvPMenuGfx_CrossFade(%d, %d, %d)\n", menuGfxID, menuGfxID2, alpha);
}

void DrawAvPMenuGfx_Faded(enum AVPMENUGFX_ID menuGfxID, int topleftX, int topleftY, int alpha,enum AVPMENUFORMAT_ID format)
{
	fprintf(stderr, "DrawAvPMenuGfx_Faded(%d, %d, %d, %d, %d)\n", menuGfxID, topleftX, topleftY, alpha, format);
}

int RenderMenuText(char *textPtr, int x, int y, int alpha, enum AVPMENUFORMAT_ID format)
{
	fprintf(stderr, "RenderMenuText(%s, %d, %d, %d, %d)\n", textPtr, x, y, alpha, format);
	
	return 0;
}

void LoadAllSplashScreenGfx()
{
	fprintf(stderr, "LoadAllSplashScreenGfx()\n");
}

void LoadAvPMenuGfx(enum AVPMENUGFX_ID menuGfxID)
{
	fprintf(stderr, "LoadAvPMenuGfx(%d)\n", menuGfxID);
}


/* avp_menus.cpp */
AVP_USER_PROFILE *UserProfilePtr;
CONTROL_METHODS PlayerControlMethods;
int NumberOfSessionsFound;
JOYSTICK_CONTROL_METHODS PlayerJoystickControlMethods;
SESSION_DESC SessionData[MAX_NO_OF_SESSIONS];
SAVE_SLOT_HEADER SaveGameSlot[NUMBER_OF_SAVE_SLOTS];

void GetFilenameForSaveSlot(int i, unsigned char *filenamePtr)
{
	fprintf(stderr, "GetFilenameForSaveSlot(%d, %p)\n", i, filenamePtr);
}

void RenderBriefingText(int centreY, int brightness)
{
	fprintf(stderr, "RenderBriefingText(%d, %d)\n", centreY, brightness);
}

void LoadLevelHeader(SAVE_BLOCK_HEADER* header)
{
	fprintf(stderr, "LoadLevelHeader(%p)\n", header);
}

void AvP_TriggerInGameMenus()
{
	fprintf(stderr, "AvP_TriggerInGameMenus()\n");
}

void ScanSaveSlots()
{
	fprintf(stderr, "ScanSaveSlots()\n");
}

void SaveLevelHeader()
{
	fprintf(stderr, "SaveLevelHeader()\n");
}

int InGameMenusAreRunning()
{
	fprintf(stderr, "InGameMenusAreRunning()\n");
	
	return 0;
}


/* avpreg.cpp */
char *AvpCDPath = 0;

void GetPathFromRegistry()
{
extern char * SecondTex_Directory;
extern char * SecondSoundDir;

	fprintf(stderr, "GetPathFromRegistry()\n");
	
	SecondTex_Directory = "./graphics";
	SecondSoundDir = "./sounds";
}


/* avp_userprofile.c */
int SmackerSoundVolume;

int NumberOfUserProfiles()
{
	fprintf(stderr, "NumberOfUserProfiles()\n");
	
	return 0;
}

/* d3_func.cpp */
int GetTextureHandle(IMAGEHEADER *imageHeaderPtr)
{
	fprintf(stderr, "GetTextureHandle(%p)\n", imageHeaderPtr);
	
	return 1;
}

/* d3d_render.cpp -- some of these got mixed in with d3_func.cpp! */
int NumberOfLandscapePolygons;
int FMVParticleColour;
int WireFrameMode;
int WaterFallBase;

void InitDrawTest()
{
	fprintf(stderr, "InitDrawTest()\n");
}

void InitForceField()
{
	fprintf(stderr, "InitForceField()\n");
}

void DrawScanlinesOverlay(float level)
{
	fprintf(stderr, "DrawScanlinesOverlay(%f)\n", level);
}

void RenderString(char *stringPtr, int x, int y, int colour)
{
	fprintf(stderr, "RenderString(%s, %d, %d, %d)\n", stringPtr, x, y, colour);
}

void RenderStringCentred(char *stringPtr, int centreX, int y, int colour)
{
	fprintf(stderr, "RenderStringCentred(%s, %d, %d, %d)\n", stringPtr, centreX, y, colour);
}

void PostLandscapeRendering()
{
	fprintf(stderr, "PostLandscapeRendering()\n");
}

void SetFogDistance(int fogDistance)
{
	fprintf(stderr, "SetFontDistance(%d)\n", fogDistance);
}

void CheckWireFrameMode(int shouldBeOn)
{
//	fprintf(stderr, "CheckWireFrameMode(%d)\n", shouldBeOn);
	if (shouldBeOn)
		fprintf(stderr, "CheckWireFrameMode(%d)\n", shouldBeOn);
}

void WriteEndCodeToExecuteBuffer()
{
	fprintf(stderr, "WriteEndCodeToExecuteBuffer()\n");
}

BOOL UnlockExecuteBufferAndPrepareForUse()
{
	fprintf(stderr, "UnlockExecuteBufferAndPrepareForUse()\n");
	
	return FALSE;
}

void ReloadImageIntoD3DImmediateSurface(IMAGEHEADER* iheader)
{
	fprintf(stderr, "ReloadImageIntoD3DImmediateSurface(%p)\n", iheader);
}

void ReleaseDirect3DNotDDOrImages()
{
	fprintf(stderr, "ReleaseDirect3DNotDDOrImages()\n");
}

void ReleaseDirect3DNotDD()
{
	fprintf(stderr, "ReleaseDirect3DNotDD()\n");
}

void ReleaseDirect3D()
{
	fprintf(stderr, "ReleaseDirect3D()\n");
}

void ReleaseD3DTexture(void* D3DTexture)
{
	fprintf(stderr, "ReleaseD3DTexture(%p)\n", D3DTexture);
}

BOOL ExecuteBuffer()
{
//	fprintf(stderr, "ExecuteBuffer()\n");
	
	return FALSE;
}

BOOL EndD3DScene()
{
	fprintf(stderr, "EndD3DScene()\n");
	
	return FALSE;
}

void D3D_ZBufferedGouraudPolygon_Output(POLYHEADER *inputPolyPtr,RENDERVERTEX *renderVerticesPtr)
{
	fprintf(stderr, "D3D_ZBufferedGouraudPolygon_Output(%p, %p)\n", inputPolyPtr, renderVerticesPtr);
}

void D3D_ZBufferedCloakedPolygon_Output(POLYHEADER *inputPolyPtr,RENDERVERTEX *renderVerticesPtr)
{
	fprintf(stderr, "D3D_ZBufferedCloakedPolygon_Output(%p, %p)\n", inputPolyPtr, renderVerticesPtr);
}

void D3D_SkyPolygon_Output(POLYHEADER *inputPolyPtr,RENDERVERTEX *renderVerticesPtr)
{
	fprintf(stderr, "D3D_SkyPolygon_Output(%p, %p)\n", inputPolyPtr, renderVerticesPtr);
}

void D3D_PlayerOnFireOverlay()
{
	fprintf(stderr, "D3D_PlayerOnFireOverlay()\n");
}

void D3D_PlayerDamagedOverlay(int intensity)
{
	fprintf(stderr, "D3D_PlayerDamagedOverlay(%d)\n", intensity);
}

void D3D_FadeDownScreen(int brightness, int colour)
{
	fprintf(stderr, "D3D_FadeDownScreen(%d, %d)\n", brightness, colour);
}

void D3D_DrawWaterTest(MODULE *testModulePtr)
{
	fprintf(stderr, "D3D_DrawWaterTest(%p)\n", testModulePtr);
}

void D3D_DrawParticle_Rain(PARTICLE *particlePtr,VECTORCH *prevPositionPtr)
{
	fprintf(stderr, "D3D_DrawParticle_Rain(%p, %p)\n", particlePtr, prevPositionPtr);
}

void D3D_DrawCable(VECTORCH *centrePtr, MATRIXCH *orientationPtr)
{
	fprintf(stderr, "D3D_DrawCable(%p, %p)\n", centrePtr, orientationPtr);
}

void D3D_DrawBackdrop()
{
	fprintf(stderr, "D3D_DrawBackdrop()\n");
}

void D3D_BackdropPolygon_Output(POLYHEADER *inputPolyPtr,RENDERVERTEX *renderVerticesPtr)
{
	fprintf(stderr, "D3D_BackdropPolygon_Output(%p, %p)\n", inputPolyPtr, renderVerticesPtr);
}

void D3D_RenderHUDString_Centred(char *stringPtr, int centreX, int y, int colour)
{
	fprintf(stderr, "D3D_RenderHUDString_Centred(%s, %d, %d, %d)\n", stringPtr, centreX, y, colour);
}
        
/* dd_func.cpp */
long BackBufferPitch;
int VideoModeColourDepth;

int ChangePalette (unsigned char* NewPalette)
{
	fprintf(stderr, "ChangePalette(%p)\n", NewPalette);
	
	return 0;
}

void BlitWin95Char(int x, int y, unsigned char toprint)
{
	fprintf(stderr, "BlitWin95Char(%d, %d, %d)\n", x, y, toprint);
}

void ColourFillBackBuffer(int FillColour)
{
	fprintf(stderr, "ColourFillBackBuffer(%d)\n", FillColour);
}

void LockSurfaceAndGetBufferPointer()
{
	fprintf(stderr, "LockSurfaceAndGetBufferPointer()\n");
}

void finiObjectsExceptDD()
{
	fprintf(stderr, "finiObjectsExceptDD()\n");
}

void finiObjects()
{
	fprintf(stderr, "finiObjects()\n");
}

void UnlockSurface()
{
	fprintf(stderr, "UnlockSurface()\n");
}

void ReleaseDDSurface(void* DDSurface)
{
	fprintf(stderr, "ReleaseDDSurface(%p)\n", DDSurface);
}

BOOL ChangeDirectDrawObject()
{
	fprintf(stderr, "ChangeDirectDrawObject()\n");
	
	return FALSE;
}


/* dx_proj.cpp */
int use_mmx_math = 0;


/* cd_player.c */
void CDDA_Start()
{
	fprintf(stderr, "CDDA_Start()\n");
}

void CDDA_End()
{
	fprintf(stderr, "CDDA_End()\n");
}

void CDDA_ChangeVolume(int volume)
{
	fprintf(stderr, "CDDA_ChangeVolume(%d)\n", volume);
}

int CDDA_CheckNumberOfTracks()
{
	fprintf(stderr, "CDDA_CheckNumberOfTracks()\n");
	
	return 0;
}

int CDDA_IsOn()
{
	fprintf(stderr, "CDDA_IsOn()\n");
	
	return 0;
}

int CDDA_IsPlaying()
{
	fprintf(stderr, "CDDA_IsPlaying()\n");
	
	return 0;
}

void CDDA_Play(int CDDATrack)
{
	fprintf(stderr, "CDDA_Play(%d)\n", CDDATrack);
}

void CDDA_PlayLoop(int CDDATrack)
{
	fprintf(stderr, "CDDA_PlayLoop(%d)\n", CDDATrack);
}

void CDDA_Stop()
{
	fprintf(stderr, "CDDA_Stop()\n");
}

void CDDA_SwitchOn()
{
	fprintf(stderr, "CDDA_SwitchOn()\n");
}


/* dxlog.c */
void dx_str_log(char const * str, int line, char const * file)
{
	fprintf(stderr, "dx_str_log: %s/%d: %s\n", file, line, str);
}

void dx_strf_log(char const * fmt, ... )
{
	va_list ap;
        va_start(ap, fmt);
        fprintf(stderr, "dx_strf_log: ");
	vfprintf(stderr,fmt,ap);
	fprintf(stderr, "\n");
        va_end(ap);
}

void dx_line_log(int line, char const * file)
{
	fprintf(stderr, "dx_line_log: %s/%d\n", file, line);
}


/* pldnet.c */
MULTIPLAYER_START* marineStartPositions=0;
MULTIPLAYER_START* predatorStartPositions=0;
MULTIPLAYER_START* alienStartPositions=0;
int numMarineStartPos=0;
int numPredatorStartPos=0;
int numAlienStartPos=0;
NETGAME_GAMEDATA netGameData; /* TODO: this constant is initialized in pldnet.c */
DPID myIgniterId = 0;
DPID myNetworkKillerId = 0;
int ShowMultiplayerScoreTimer=0;
int MyHitBodyPartId=-1;
DPID MultiplayerObservedPlayer=0;
DPID AVPDPNetID;

BOOL AreThereAnyLivesLeft()
{
	fprintf(stderr, "AreThereAnyLivesLeft()\n");
	
	return FALSE;
}

void TurnOffMultiplayerObserveMode()
{
	fprintf(stderr, "TurnOffMultiplayerObserveMode()\n");
}

void TransmitPlayerLeavingNetMsg()
{
	fprintf(stderr, "TransmitPlayerLeavingNetMsg()\n");
}

void TransmitEndOfGameNetMsg()
{
	fprintf(stderr, "TransmitEndOfGameNetMsg()\n");
}

void TeleportNetPlayerToAStartingPosition(STRATEGYBLOCK *playerSbPtr, int startOfGame)
{
	fprintf(stderr, "TeleportNetPlayerToAStartingPosition(%p, %d)\n", playerSbPtr, startOfGame);
}

void SpeciesTag_DetermineMyNextCharacterType()
{
	fprintf(stderr, "SpeciesTag_DetermineMyNextCharacterType()\n");
}

void ShowNearestPlayersName()
{
	fprintf(stderr, "ShowNearestPlayersName()\n");
}

void RestartNetworkGame(int seed)
{
	fprintf(stderr, "RestartNetworkGame(%d)\n", seed);
}

int PlayerIdInPlayerList(DPID Id)
{
	fprintf(stderr, "PlayerIdInPlayerList(%d)\n", Id);
	
	return 0;
}

void NetSendMessages()
{
	fprintf(stderr, "NetSendMessages()\n");
}

void NetCollectMessages()
{
	fprintf(stderr, "NetCollectMessages()\n");
}

void GetNextMultiplayerObservedPlayer()
{
	fprintf(stderr, "GetNextMultiplayerObservedPlayer()\n");
}

void DoMultiplayerSpecificHud()
{
	fprintf(stderr, "DoMultiplayerSpecificHud()\n");
}

void AddNetMsg_AlienAIKilled(STRATEGYBLOCK *sbPtr,int death_code,int death_time, int GibbFactor,DAMAGE_PROFILE* damage)
{
	fprintf(stderr, "AddNetMsg_AlienAIKilled(%p, %d, %d, %d, %p)\n", sbPtr, death_code, death_time, GibbFactor, damage);
}

void AddNetMsg_AlienAISeqChange(STRATEGYBLOCK *sbPtr,int sequence_type,int sub_sequence,int sequence_length,int tweening_time)
{
	fprintf(stderr, "AddNetMsg_AlienAISeqChange(%p, %d, %d, %d, %d)\n", sbPtr, sequence_type, sub_sequence, sequence_length, tweening_time);
}

void AddNetMsg_ChatBroadcast(char *string,BOOL same_species_only)
{
	fprintf(stderr, "AddNetMsg_ChatBroadcast(%s, %d)\n", string, same_species_only);
}

void AddNetMsg_CreateWeapon(char* objectName,int type,VECTORCH* location)
{
	fprintf(stderr, "AddNetMsg_CreateWeapon(%s, %d, %p)\n", objectName, type, location);
}

void AddNetMsg_FarAlienPosition(STRATEGYBLOCK* sbPtr,int targetModuleIndex,int index,BOOL indexIsModuleIndex)
{
	fprintf(stderr, "AddNetMsg_FarAlienPosition(%p, %d, %d, %d)\n", sbPtr, targetModuleIndex, index, indexIsModuleIndex);
}

void AddNetMsg_GhostHierarchyDamaged(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple, int sectionID,VECTORCH* incoming)
{
	fprintf(stderr, "AddNetMsg_GhostHierarchyDamaged(%p, %p, %d, %d, %p)\n", sbPtr, damage, multiple, sectionID, incoming);
}

void AddNetMsg_Gibbing(STRATEGYBLOCK *sbPtr,int gibbFactor,int seed)
{
	fprintf(stderr, "AddNetMsg_Gibbing(%p, %d, %d)\n", sbPtr, gibbFactor, seed);
}

void AddNetMsg_InanimateObjectDamaged(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple)
{
	fprintf(stderr, "AddNetMsg_InanimateObjectDamaged(%p, %p, %d)\n", sbPtr, damage, multiple);
}

void AddNetMsg_InanimateObjectDestroyed(STRATEGYBLOCK *sbPtr)
{
	fprintf(stderr, "AddNetMsg_InanimateObjectDestroyed(%p)\n", sbPtr);
}

void AddNetMsg_LOSRequestBinarySwitch(STRATEGYBLOCK *sbPtr)
{
	fprintf(stderr, "AddNetMsg_LOSRequestBinarySwitch(%p)\n", sbPtr);
}

void AddNetMsg_LocalObjectDamaged(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage,int multiple, int sectionID,int delta_seq,int delta_sub_seq,VECTORCH* incoming)
{
	fprintf(stderr, "AddNetMsg_LocalObjectDamaged(%p, %p, %d, %d, %d, %d, %p)\n", sbPtr, damage, multiple, sectionID, delta_seq, delta_sub_seq, incoming);
}

void AddNetMsg_LocalObjectDestroyed(STRATEGYBLOCK *sbPtr)
{
	fprintf(stderr, "AddNetMsg_LocalObjectDestroyed(%p)\n", sbPtr);
}

void AddNetMsg_LocalObjectDestroyed_Request(STRATEGYBLOCK *sbPtr)
{
	fprintf(stderr, "AddNetMsg_LocalObjectDestroyed_Request(%p)\n", sbPtr);
}

void AddNetMsg_LocalObjectOnFire(STRATEGYBLOCK *sbPtr)
{
	fprintf(stderr, "AddNetMsg_LocalObjectOnFire(%p)\n", sbPtr);
}

void AddNetMsg_LocalObjectState(STRATEGYBLOCK *sbPtr)
{
	fprintf(stderr, "AddNetMsg_LocalObjectState(%p)\n", sbPtr);
}

void AddNetMsg_MakeDecal(enum DECAL_ID decalID, VECTORCH *normalPtr, VECTORCH *positionPtr, int moduleIndex)
{
	fprintf(stderr, "AddNetMsg_MakeDecal(%d, %p, %p, %d)\n", decalID, normalPtr, positionPtr, moduleIndex);
}

void AddNetMsg_MakeExplosion(VECTORCH *positionPtr, enum EXPLOSION_ID explosionID)
{
	fprintf(stderr, "AddNetMsg_MakeExplosion(%p, %d)\n", positionPtr, explosionID);
}

void AddNetMsg_MakeFlechetteExplosion(VECTORCH *positionPtr, int seed)
{
	fprintf(stderr, "AddNetMsg_MakeFlechetteExplosion(%p, %d)\n", positionPtr, seed);
}

void AddNetMsg_MakePlasmaExplosion(VECTORCH *positionPtr, VECTORCH *fromPositionPtr, enum EXPLOSION_ID explosionID)
{
	fprintf(stderr, "AddNetMsg_MakePlasmaExplosion(%p, %p, %d)\n", positionPtr, fromPositionPtr, explosionID);
}

void AddNetMsg_ObjectPickedUp(char* objectName)
{
	fprintf(stderr, "AddNetMsg_ObjectPickedUp(%s)\n", objectName);
}

void AddNetMsg_PlatformLiftState(STRATEGYBLOCK *sbPtr)
{
	fprintf(stderr, "AddNetMsg_PlatformLiftState(%p)\n", sbPtr);
}

void AddNetMsg_PlayerDeathAnim(int deathId,int objectId)
{
	fprintf(stderr, "AddNetMsg_PlayerDeathAnim(%d, %d)\n", deathId, objectId);
}

void AddNetMsg_PlayerKilled(int objectId,DAMAGE_PROFILE* damage)
{
	fprintf(stderr, "AddNetMsg_PlayerKilled(%d, %p)\n", objectId, damage);
}

void AddNetMsg_PredatorLaserSights(VECTORCH *positionPtr, VECTORCH *normalPtr, DISPLAYBLOCK *dispPtr)
{
	fprintf(stderr, "AddNetMsg_PredatorLaserSights(%p, %p, %p)\n", positionPtr, normalPtr, dispPtr);
}

void AddNetMsg_RequestPlatformLiftActivate(STRATEGYBLOCK *sbPtr)
{
	fprintf(stderr, "AddNetMsg_RequestPlatformLiftActivate(%p)\n", sbPtr);
}

void AddNetMsg_RestartNetworkGame(int seed)
{
	fprintf(stderr, "AddNetMsg_RestartNetworkGame(%d)\n", seed);
}

void AddNetMsg_SpotAlienSound(int soundCategory,int alienType,int pitch,VECTORCH *position)
{
	fprintf(stderr, "AddNetMsg_SpotAlienSound(%d, %d, %d, %p)\n", soundCategory, alienType, pitch, position);
}

void AddNetMsg_SpotOtherSound(enum soundindex SoundIndex,VECTORCH *position,int explosion)
{
	fprintf(stderr, "AddNetMsg_SpotOtherSound(%d, %p, %d)\n", SoundIndex, position, explosion);
}
