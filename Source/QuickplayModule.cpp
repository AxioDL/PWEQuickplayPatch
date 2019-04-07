#include <PrimeAPI.h>
#include <Runtime/CArchitectureQueue.hpp>
#include <Runtime/CMainFlow.hpp>
#include <Runtime/CGameState.hpp>
#include <Runtime/CPlayerState.hpp>
#include <Runtime/CStateManager.hpp>
#include <Runtime/CWorldState.hpp>
#include <Runtime/World/CPlayer.hpp>
#include <Math/CVector3f.hpp>
#include <Math/CTransform4f.hpp>
#include <dolphin/dvd.h>
#include <dolphin/os.h>

// IMPORTANT NOTE: Most of the values, enums & structs declared here
// are mirrored in Prime World Editor NDolphinIntegration.h.

// Debug config file magic
const uint32 gkDebugConfigMagic = 0x00BADB01;

// Current quickplay version
// This should match EQuickplayVersion::Current in Prime World Editor
const uint32 gkQuickplayVersion = 1;

// Feature mask enum
enum EQuickplayFeature
{
    /** On boot, automatically load the area specified by WorldID and AreaID */
    kQF_JumpToArea          = 0x00000001,
    /** Spawn the player in the location specified by SpawnTransform */
    kQF_SetSpawnPosition    = 0x00000002,
	/** Give the player all items on spawn */
	kQF_GiveAllItems		= 0x00000004,
};

// Contains debug parameters for quickplay from the editor
// This is a mix of user-selected options and context from the current editor state
struct SQuickplayParms
{
	uint32 Magic;
	uint32 Version;
	uint32 FeatureFlags;
	uint32 BootWorldAssetID;
	uint32 BootAreaAssetID;
	CTransform4f SpawnTransform;
};
SQuickplayParms gQuickplayParms;

#define QUICKPLAY_BUFFER_SIZE ((sizeof(SQuickplayParms) + 31) & ~31)

// Patches
PATCH_SYMBOL(CMainFlow::AdvanceGameState(CArchitectureQueue&), Hook_CMainFlow_AdvanceGameState(CMainFlow*, CArchitectureQueue&));
PATCH_SYMBOL(CStateManager::InitializeState(uint, TAreaId, uint), Hook_CStateManager_InitializeState(CStateManager&, uint, TAreaId, uint));

// Forward decls
void LoadDebugParamsFromDisc();

// Module init
void _prolog()
{
	MODULE_INIT;
	OSReport("Quickplay module loaded\n");
	LoadDebugParamsFromDisc();
}

// This callback workaround is needed because the game doesn't have any synchronous
// DVD reading functions linked into the DOL, so we have to use the async one
volatile s32 gDvdBytesRead = -1;

void DvdLoadFinishedCallback(s32 Result, DVDFileInfo* pFileInfo)
{
	gDvdBytesRead = Result;
}

void LoadDebugParamsFromDisc()
{
	DVDFileInfo File;
	const char* pkFailReason = NULL;
	
	// Debug config is stored in the "dbgconfig" file in the filesystem root
	if (DVDOpen("dbgconfig", &File))
	{
		int RawLength = DVDGetLength(&File);
		int Length = OSRoundUp32B(RawLength);
		
		if (RawLength >= sizeof(SQuickplayParms))
		{
			// The DVD read buffer must be aligned to 32 bytes.
			// Since we can't control the alignment of stack variables, supply
			// an extra 32 bytes to the array so we can ensure proper alignment.
			int Length = OSRoundUp32B(DVDGetLength(&File));
			int AllocSize = Length + 32;
			uint8 Buffer[QUICKPLAY_BUFFER_SIZE + 32];
			void* pAlignedBuffer = (void*) OSRoundUp32B(&Buffer[0]);

			if (DVDReadAsyncPrio(&File, pAlignedBuffer, Length, 0, DvdLoadFinishedCallback, 0))
			{
				while (gDvdBytesRead < 0) {}
				
				// DVD load complete - parse data
				SQuickplayParms* pParms = static_cast<SQuickplayParms*>(pAlignedBuffer);
				
				if (gDvdBytesRead < sizeof(SQuickplayParms))
				{
					pkFailReason = "Failed to read enough data from dbgconfig file.";
				}
				else if (pParms->Magic != gkDebugConfigMagic)
				{
					pkFailReason = "Invalid dbgconfig magic.";
				}
				else if (pParms->Version != gkQuickplayVersion)
				{
					pkFailReason = "Invalid quickplay version.";
				}
				else
				{
					OSReport("Quickplay parameters loaded successfully!\n");
					memcpy(&gQuickplayParms, pParms, sizeof(SQuickplayParms));
				}
			}
			else
			{
				pkFailReason = "Failed to read dbgconfig file.";
			}
		}
		else
		{
			pkFailReason = "dbgconfig file is too small.";
		}
		
		DVDClose(&File);
	}
	else
	{
		pkFailReason = "Failed to open dbgconfig file.";
	}
	
	if (pkFailReason != NULL)
	{
		OSReport("%s Quickplay debug features will not be enabled.\n", pkFailReason);
		gQuickplayParms.FeatureFlags = 0;
	}
}

// Hooks
void Hook_CMainFlow_AdvanceGameState(CMainFlow* pMainFlow, CArchitectureQueue& Queue)
{
	// Hook into CMainFlow::AdvanceGameState(). When this function is called with
	// the game state set to PreFrontEnd, that indicates that engine initialization
	// is complete and the game is proceeding to the main menu. We hook in here to
	// bypass the main menu and boot directly into the game.
	static bool sHasDoneInitialBoot = false;
	
	// Make sure the patch does not run twice if the player quits out to main menu
	if (!sHasDoneInitialBoot && 
		(gQuickplayParms.FeatureFlags & kQF_JumpToArea) &&
		pMainFlow->GetGameState() == kCFS_PreFrontEnd)
	{
		sHasDoneInitialBoot = true;
		gpGameState->SetCurrentWorldId( gQuickplayParms.BootWorldAssetID );
		gpGameState->CurrentWorldState()->SetDesiredAreaAssetId( gQuickplayParms.BootAreaAssetID );
		pMainFlow->SetGameState(kCFS_Game, Queue);
		return;
	}
	else
	{
		pMainFlow->AdvanceGameState(Queue);
	}
}

void Hook_CStateManager_InitializeState(CStateManager& StateMgr, uint WorldAssetId, TAreaId AreaId, uint AreaAssetId)
{
	// This function runs when a world is being initialized for gameplay.
	static bool sDoneFirstInit = false;	

	// Allow the original function to run first before we execute custom logic
	StateMgr.InitializeState(WorldAssetId, AreaId, AreaAssetId);
	CStateManager::EInitPhase Phase = StateMgr.GetInitPhase();
	
	if (!sDoneFirstInit && Phase == CStateManager::kInit_Done)
	{
		sDoneFirstInit = true;
		
		// Spawn the player in the location specified by SpawnTransform.
		// This feature doesn't make much sense without JumpToArea, so we require that flag to be on too.
		if ( (gQuickplayParms.FeatureFlags & kQF_JumpToArea) &&
			 (gQuickplayParms.FeatureFlags & kQF_SetSpawnPosition) )
		{
			StateMgr.GetPlayer()->Teleport(gQuickplayParms.SpawnTransform, StateMgr, true);
		}
		
		// Fill out all inventory values to capacity.
		if (gQuickplayParms.FeatureFlags & kQF_GiveAllItems)
		{
			CPlayerState* pPlayerState = StateMgr.GetPlayerState();
			for (uint ItemIdx = 0; ItemIdx < CPlayerState::kItem_Max; ItemIdx++)
			{
				CPlayerState::EItemType Item = (CPlayerState::EItemType) ItemIdx;
				uint32 Max = gkPowerUpMaxValues[ItemIdx];
				pPlayerState->ResetPowerUp(Item, Max);
				pPlayerState->IncrPickUp(Item, Max);
			}
		}
	}
}