//========= Copyright Bernt Andreas Eide, All rights reserved. ============//
//
// Purpose: Client Mode for TFO. Special accessors. 
//
//=============================================================================//
#include "cbase.h"
#include "GameBase_Client.h"
#include "ienginevgui.h"
#include "c_baseplayer.h"
#include "c_baseentity.h"
#include "fmod_manager.h"
#include "viewrender.h"

// ADD INCLUDES FOR OTHER MENUS: (NON-BASEVIEWPORT/INTERFACE)
#include "NotePanel.h"
#include "DeathPanel.h"
#include "SavingPanel.h"
#include "DialoguePanel.h"
#include "InventoryPanel.h"
#include "CreditsPanel.h"
#include "LoadingPanel.h"
#include "MainMenu.h"
#include "steam/steam_api.h"
#include "c_achievement_manager.h"
#include "gameconsoledialog.h"
#include "hud_crosshairs.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static void EnableFilmGrain(IConVar *pConVar, char const *pOldString, float flOldValue)
{
	ConVarRef var(pConVar);
	if (var.GetBool())
	{
		IMaterial *pMaterial = materials->FindMaterial("effects/filmgrain", TEXTURE_GROUP_OTHER, true);
		if (pMaterial && view)
			view->SetScreenOverlayMaterial(pMaterial);
	}
	else
	{
		if (view)
			view->SetScreenOverlayMaterial(NULL);
	}
}

static ConVar tfo_fx_filmgrain("tfo_fx_filmgrain", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Enable or Disable film grain.", true, 0, true, 1, EnableFilmGrain);
static ConVar tfo_fx_filmgrain_strength("tfo_fx_filmgrain_strength", "2", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Set film grain strength.", true, 0.75f, true, 2.0f);

// GameUI
static CDllDemandLoader g_GameUIDLL("GameUI");

class CGameBase_Client : public IGameBase_Client
{
private:

	CGameConsoleDialog *m_pGameConsole;
	CAchievementManager *AchievementManager;
	CMainMenu *MainMenu;
	CLoadingPanel *LoadingPanel;
	CNotePanel *NotePanel;
	CDeathPanel *DeathPanel;
	CSavingPanel *SavePanel;
	CDialogueMenu *DialoguePanel;
	CInventoryPanel *InventoryPanel;
	CCreditsPanel *CreditsPanel;

public:

	CGameBase_Client(void)
	{
		m_pGameConsole = NULL;
		AchievementManager = NULL;
		MainMenu = NULL;
		LoadingPanel = NULL;
		NotePanel = NULL;
		DeathPanel = NULL;
		SavePanel = NULL;
		DialoguePanel = NULL;
		InventoryPanel = NULL;
		CreditsPanel = NULL;
	}

	// System
	void Initialize(bool bInGame = false);
	bool CanLoadMainMenu(void);
	void MapLoad(const char *map, bool bLoad = false, bool bReload = false);
	void SaveGame(int iSlot = 0, bool bSaveStation = false);
	void MoveConsoleToFront(void);
	void SetLoadingScreen(bool state);
	void ActivateShaderEffects(void);
	void DeactivateShaderEffects(void);

	// VGUI Inits
	void CreateGameUIPanels(vgui::VPANEL parent);
	void CreateInGamePanels(vgui::VPANEL parent);

	// Cleanup on game exit
	void DestroyPanels(void);

	// Misc
	void ShowConsole(bool bToggle, bool bClose, bool bClear);
	const char *GetAchievementForGUI(int iID);
	void SetAchievement(const char *szAchievement);
	bool HasAllAchievements(void);

	// VGUI
	void ResetAll(void);
	void OpenPanel(int iPanel);
	void ClosePanels(int iExcluded);

	// Inventory, Note & Dialogoue System
	void AddToInventory(const char *szFile);
	void RemoveItemFromInventory(const char *szFile = NULL, int iID = -1, bool bDrop = false);
	void SaveInventory(const char *szFile);
	void DeleteAutoSaveFile(void);
	bool PlayerHasItem(const char *szItem);
	KeyValues *GetInventoryItemDetails(const char *szFile);
	void ShowNote(const char *szFile);
	void StartDialogueScene(const char *szFile, const char *szEntity, bool bOption1, bool bOption2, bool bOption3);
};

// We create the global game ui panels here then we call Initialize when we've fully instanciated the global panels. 
void CGameBase_Client::CreateGameUIPanels(vgui::VPANEL parent)
{
	// Init Game UI
	CreateInterfaceFn gameUIFactory = g_GameUIDLL.GetFactory();
	if (gameUIFactory)
	{
		GameUI = (IGameUI *)gameUIFactory(GAMEUI_INTERFACE_VERSION, NULL);
		if (!GameUI)
			Error("Couldn't load GameUI!\n");
	}
	else
		Error("Couldn't load GameUI!\n");

	// Create and parent us to the main menu. @ game ui.
	MainMenu = new CMainMenu(parent);
	GameUI->SetMainMenuOverride(MainMenu->GetVPanel());

	LoadingPanel = new CLoadingPanel(NULL);
	LoadingPanel->SetVisible(false);
	GameUI->SetLoadingBackgroundDialog(LoadingPanel->GetVPanel()); // Parent our panel to the actual loading screen game ui from the engine.
	LoadingPanel->SetIsLoadingMainMenu(true); // Hide loading gui until we know that the main menu is fully loaded.

	m_pGameConsole = new CGameConsoleDialog();
	AchievementManager = new CAchievementManager();

	// Finally we try to load the main menu background map:
	if (!CanLoadMainMenu())
		Initialize(); // Initialize is called from baseviewport (event handler) once the bg map has loaded. In this case there's no map to load.
}

// Create in-game panels such as the inventory panel, note panel, etc...
void CGameBase_Client::CreateInGamePanels(vgui::VPANEL parent)
{
	NotePanel = new CNotePanel(parent);
	DeathPanel = new CDeathPanel(parent);
	SavePanel = new CSavingPanel(parent);
	DialoguePanel = new CDialogueMenu(parent);
	InventoryPanel = new CInventoryPanel(parent);
	CreditsPanel = new CCreditsPanel(parent);
}

// Destroy all instances of vgui panels/frames.
void CGameBase_Client::DestroyPanels(void)
{
	pszInventoryList.Purge();

	g_GameUIDLL.Unload();
	GameUI = NULL;

	if (AchievementManager)
		delete AchievementManager;

	if (m_pGameConsole)
	{
		m_pGameConsole->SetParent((vgui::Panel *)NULL);
		delete m_pGameConsole;
	}

	if (NotePanel)
	{
		NotePanel->SetParent((vgui::Panel *)NULL);
		delete NotePanel;
	}

	if (DeathPanel)
	{
		DeathPanel->SetParent((vgui::Panel *)NULL);
		delete DeathPanel;
	}

	if (SavePanel)
	{
		SavePanel->SetParent((vgui::Panel *)NULL);
		delete SavePanel;
	}

	if (LoadingPanel)
	{
		LoadingPanel->SetParent((vgui::Panel *)NULL);
		delete LoadingPanel;
	}

	if (DialoguePanel)
	{
		DialoguePanel->SetParent((vgui::Panel *)NULL);
		delete DialoguePanel;
	}

	if (InventoryPanel)
	{
		InventoryPanel->SetParent((vgui::Panel *)NULL);
		delete InventoryPanel;
	}

	if (CreditsPanel)
	{
		CreditsPanel->SetParent((vgui::Panel *)NULL);
		delete CreditsPanel;
	}
}

// We check if we can load the main menu, if not we load a 2d menu.
bool CGameBase_Client::CanLoadMainMenu(void)
{
	// If we're in-game, reset all stuff:
	C_BasePlayer *pClient = C_BasePlayer::GetLocalPlayer();
	if (pClient)
	{
		// If we're in-game we might also be in a background map = main menu map = no need to proceed... Unless the user fucked up and wrote map_background blabla tho...
		if (engine->IsLevelMainMenuBackground())
		{
			Warning("You're already in the main menu area!\n");
			return false;
		}

		ResetAll();
	}

	// Play Load/Wait Sound:
	FMODManager()->PlayLoadingSound("musics/titles_loop.wav");

	// Disconnect us, close console, enable loading interface progress then disable loading layout:
	ShowConsole(false, true, true);
	engine->ClientCmd_Unrestricted("progress_enable\n");
	engine->ClientCmd_Unrestricted("hideconsole\n");

	// Reset Loading Image and MainMenu Screenie:
	if (LoadingPanel)
		LoadingPanel->SetIsLoadingMainMenu(true);

	// In HL2_CLIENT we re-enable this once we know that the menu has been successfully / finished loading... :) Notice that hl2_client is server only, we'll be running an event to change stuff on the client...

	// Force skill execution just in case...
	engine->ClientCmd("exec Skill\n");

	// We load a random background map.(between chapt. 1-4)
	int iMap = random->RandomInt(1, 4);
	const char *szMap = VarArgs("maps/background0%i.bsp", iMap);

	// No map?
	if (!filesystem->FileExists(szMap, "MOD"))
	{
		engine->ClientCmd_Unrestricted("disconnect\n"); // Make sure we disconnect when we run this command
		FMODManager()->TransitionAmbientSound("musics/titles_loop.wav");
		return false;
	}

	// Run the BG map...
	engine->ClientCmd(VarArgs("map_background background0%i\n", iMap));
	return true;
}

// Called on map load, load, map creation, changelevel & reload.
void CGameBase_Client::MapLoad(const char *map, bool bLoad, bool bReload)
{
	if (!map)
		return;

	if (!bLoad && !bReload)
	{
		if (!filesystem->FileExists(VarArgs("maps/%s.bsp", map), "MOD"))
		{
			Warning("No map with the name %s was found!\n", map);
			return;
		}
	}

	if (bLoad)
	{
		if (!filesystem->FileExists(VarArgs("save/%s.sav", map), "MOD"))
		{
			Warning("No save with the name %s was found!\n", map);
			return;
		}
	}

	C_BasePlayer *pClient = C_BasePlayer::GetLocalPlayer();
	if (pClient)
	{
		if (engine->IsLevelMainMenuBackground())
		{
			if (bReload)
			{
				Warning("The reload command can not be used when in a background map/main menu area!\n");
				return;
			}

			if (!bLoad && !bReload)
				FMODManager()->PlayLoadingSound("musics/dance_knights.wav");

			if (bLoad)
				FMODManager()->PlayLoadingSound("musics/tfo_recovery_dead.wav");
		}
		else
		{
			if (!bLoad && !bReload)
				FMODManager()->PlayLoadingSound("horror/ambient/ambientshort_loop.wav");

			if (bLoad)
				FMODManager()->PlayLoadingSound("musics/tfo_the_void.wav");
		}

		if (bReload)
			FMODManager()->PlayLoadingSound("musics/piano_openbiblioteque.wav");
	}
	else
	{
		if (!bLoad && !bReload)
			FMODManager()->PlayLoadingSound("musics/dance_knights.wav");

		if (bLoad)
			FMODManager()->PlayLoadingSound("musics/tfo_recovery_dead.wav");

		if (bReload)
		{
			Warning("This command can only be used when you're in-game!\n");
			return;
		}
	}

	// Enable Stuff:
	ConVar *hud_draw = cvar->FindVar("cl_drawhud");
	if (hud_draw)
		hud_draw->SetValue(1);

	// Make sure we draw the HUD:
	ConVar *pTFOHUD = cvar->FindVar("tfo_drawhud");
	if (pTFOHUD)
		pTFOHUD->SetValue(1);

	bool bShouldLoadRecentSave = (strlen(HL2GameRules()->GetCurrentLoadedSave()) > 0) ? true : false;

	ResetAll();
	engine->ClientCmd("progress_enable\n"); // Enables the progress bar feature in the loading panel.

	// Force skill execution just in case...
	engine->ClientCmd("exec Skill\n");

	if (LoadingPanel)
		LoadingPanel->SetIsLoadingMainMenu(false);

	if (bReload)
	{
		if (bShouldLoadRecentSave)
		{
			MapLoad(HL2GameRules()->GetCurrentLoadedSave(), true, false);
			return;
		}
		else
			engine->ClientCmd_Unrestricted("reload\n");
	}

	if (!bLoad && !bReload)
	{
		// Get our global loading image and set it to the map name so our loading image can "find" it...
		ConVar *loadIMG = cvar->FindVar("tfo_loading_image");
		if (loadIMG)
			loadIMG->SetValue(map);

		engine->ClientCmd(VarArgs("map %s\n", map));
	}

	if (bLoad)
	{
		HL2GameRules()->SetCurrentLoadedSave(map);

		// Quick update loading image:
		KeyValues *pkvSaveData = new KeyValues("SaveData");
		if (pkvSaveData->LoadFromFile(filesystem, VarArgs("resource/data/saves/%s.txt", map), "MOD"))
		{
			// Get our global loading image and set it to the map name so our loading image can "find" it...
			ConVar *loadIMG = cvar->FindVar("tfo_loading_image");
			if (loadIMG)
				loadIMG->SetValue(pkvSaveData->GetString("SnapShot"));
		}
		pkvSaveData->deleteThis();

		engine->ClientCmd(VarArgs("load %s\n", map));
	}
}

// Called when you save through the console.
// Called when you save through save stations too, if bSaveStation = true you'll bypass sv_cheats.
void CGameBase_Client::SaveGame(int iSlot, bool bSaveStation)
{
	CBasePlayer *pClient = CBasePlayer::GetLocalPlayer();
	if (!pClient || engine->IsLevelMainMenuBackground())
	{
		Warning("Can't save before the game has started!\n");
		return;
	}

	// If we're saving through the console then sv_cheats must be on.
	if (!bSaveStation)
	{
		ConVar *cheat_var = cvar->FindVar("sv_cheats");
		if (cheat_var)
		{
			if (!cheat_var->GetBool())
			{
				Warning("You can't save through the console unless sv_cheats has been turned on!\n");
				return;
			}
		}
		else
		{
			Warning("You can't save through the console unless sv_cheats has been turned on!\n");
			return;
		}
	}

	if (iSlot <= 0 || iSlot > 4)
	{
		Warning("Argument must be 1 or higher and less or equal to 4!\nMax 4 slots!\n");
		return;
	}

	switch (iSlot)
	{
	case 1:
		SaveInventory("Save1");
		engine->ClientCmd("save Save1\n");
		break;
	case 2:
		SaveInventory("Save2");
		engine->ClientCmd("save Save2\n");
		break;
	case 3:
		SaveInventory("Save3");
		engine->ClientCmd("save Save3\n");
		break;
	case 4:
		SaveInventory("Save4");
		engine->ClientCmd("save Save4\n");
		break;
	}
}

// Always force the console to be in front of everything else.
void CGameBase_Client::MoveConsoleToFront(void)
{
	if (m_pGameConsole && m_pGameConsole->IsVisible())
		m_pGameConsole->MoveToFront();
}

// Update the loading screen control state.
void CGameBase_Client::SetLoadingScreen(bool state)
{
	if (LoadingPanel)
		LoadingPanel->SetIsLoadingMainMenu(!state);
}

void CGameBase_Client::ActivateShaderEffects(void)
{
	if (tfo_fx_filmgrain.GetBool())
	{
		engine->ClientCmd_Unrestricted("tfo_fx_filmgrain 1\n");
		return;
	}

	if (view)
		view->SetScreenOverlayMaterial(NULL);
}

void CGameBase_Client::DeactivateShaderEffects(void)
{
	if (view)
		view->SetScreenOverlayMaterial(NULL);
}

// Initialize the main menu (background map load if possible)
// Called on game startup after bg load or if bg doesn't exist. Will be called again on bg loading manually.
// bool bInGame = true will auto fix key input issues related to the main menu and keyboard option menu.
void CGameBase_Client::Initialize(bool bInGame)
{
	if (bInGame)
	{
		MainMenu->OnShowPanel(true);
		return;
	}

	if (!MainMenu || !LoadingPanel)
	{
		Error("Couldn't create GameUI!\n");
		return;
	}

	// Enable the loading screen and main menu:
	MainMenu->PlayMenuSound();

	// Did we want to launch with the console?
	if (ShouldOpenConsole())
		ShowConsole(true, false, false);

	m_bWantsConsole = false;

	// Set the language to english
	ConVarRef cc_lang("cc_lang");
	cc_lang.SetValue("english");

	// Disable Stuff:
	ConVar *hud_draw = cvar->FindVar("cl_drawhud");
	if (hud_draw)
		hud_draw->SetValue(0);

	// Make sure we hide the HUD:
	ConVar *pTFOHUD = cvar->FindVar("tfo_drawhud");
	if (pTFOHUD)
		pTFOHUD->SetValue(0);

	engine->ClientCmd("progress_enable\n");
}

// Add stuff to inventory, called by our event listener.
void CGameBase_Client::AddToInventory(const char *szFile)
{
	if (!szFile)
	{
		Warning("Tried to add an inventory item with a faulty name!\n");
		return;
	}

	if (strlen(szFile) <= 0)
	{
		Warning("Tried to add an inventory item with a faulty name!\n");
		return;
	}

	if (pszInventoryList.Count() >= 12)
	{
		Warning("Your inventory is full!\n");
		return;
	}

	InventoryItem_t pItem;
	KeyValues *pkvNewItem = GetInventoryItemDetails(szFile);
	if (pkvNewItem)
	{
		KeyValues *pkvInvData = pkvNewItem->FindKey("InventoryData");
		if (pkvInvData)
		{
			const char *szOverridenFile = ReadAndAllocStringValue(pkvInvData, "FileNameOverride");
			if (szOverridenFile && strlen(szOverridenFile) > 0) // We want to override the original szFile's name to this.
				Q_strncpy(pItem.inventoryItem, szOverridenFile, 32);
			else // Use the input filename.
				Q_strncpy(pItem.inventoryItem, szFile, 32);
		}

		pkvNewItem->deleteThis();
	}
	else
	{
		Warning("Unable to load resource/data/inventory/items/%s.txt!\n", szFile);
		return;
	}

	pszInventoryList.AddToTail(pItem);
}

// Remove the item at the X index or by comparing the name of the item.
void CGameBase_Client::RemoveItemFromInventory(const char *szFile, int iID, bool bDrop)
{
	if (!szFile && (iID < 0))
		return;

	char pszItem[32];
	if (szFile)
		Q_strncpy(pszItem, szFile, 32);
	else if (iID < 0)
		return;

	// If bDrop is true we don't delete the item, we tell the server to create the item.
	if (bDrop && szFile)
		engine->ClientCmd_Unrestricted(VarArgs("tfo_inventory_call_drop %s\n", pszItem));

	for (int i = (pszInventoryList.Count() - 1); i >= 0; i--)
	{
		if (szFile)
		{
			if (!strcmp(pszItem, pszInventoryList[i].inventoryItem))
			{
				pszInventoryList.Remove(i);
				break;
			}
		}
		else if (iID >= 0)
		{
			if (i == iID)
			{
				pszInventoryList.Remove(i);
				break;
			}
		}
	}
}

// Save the inventory. Output : fileName
void CGameBase_Client::SaveInventory(const char *szFile)
{
	char fileName[80];
	Q_snprintf(fileName, 80, "resource/data/saves/%s.txt", szFile);

	FileHandle_t SaveDataFile = g_pFullFileSystem->Open(fileName, "w");
	if (SaveDataFile != FILESYSTEM_INVALID_HANDLE)
	{
		char szfileLayout1[128];
		Q_snprintf(szfileLayout1, sizeof(szfileLayout1),
			"\"Save\"\n"
			"{\n"
			"\"SnapShot\" \"%s\"\n"
			"\n"
			"\"Items\"\n"
			"{\n"
			, HL2GameRules()->GetCurrentLoadingImage()
			);

		g_pFullFileSystem->Write(&szfileLayout1, strlen(szfileLayout1), SaveDataFile);

		// Write Items:
		for (int i = 0; i < pszInventoryList.Count(); i++)
		{
			char szItems[128];
			Q_snprintf(szItems, sizeof(szItems),
				"\"Slot%i\" \"%s\"\n"
				, (i + 1), pszInventoryList[i].inventoryItem
				);

			g_pFullFileSystem->Write(&szItems, strlen(szItems), SaveDataFile);
		}

		char szfileEnd[128];
		Q_snprintf(szfileEnd, sizeof(szfileEnd),
			"}\n"
			"\n"
			"}\n"
			);

		g_pFullFileSystem->Write(&szfileEnd, strlen(szfileEnd), SaveDataFile);

		g_pFullFileSystem->Close(SaveDataFile);

		if (!strcmp(szFile, "AutoSave"))
		{
			pszInventoryList.Purge();
			// Close VGUI Panels now!
			ResetAll();
		}
		else
			HL2GameRules()->SetCurrentLoadedSave(szFile);
	}
}

// Delete AutoSave file....
void CGameBase_Client::DeleteAutoSaveFile(void)
{
	if (filesystem->FileExists("resource/data/saves/AutoSave.txt", "MOD"))
		filesystem->RemoveFile("resource/data/saves/AutoSave.txt", "MOD");
}

// Check if the player has a certain inv. item
bool CGameBase_Client::PlayerHasItem(const char *szItem)
{
	bool bFound = false;

	for (int i = 0; i < pszInventoryList.Count(); i++)
	{
		if (!strcmp(szItem, pszInventoryList[i].inventoryItem))
		{
			bFound = true;
			break;
		}
	}

	return bFound;
}

KeyValues *CGameBase_Client::GetInventoryItemDetails(const char *szFile)
{
	if (!szFile)
	{
		Warning("Inventory item has faulty filename!\n");
		return NULL;
	}

	if (strlen(szFile) <= 0)
	{
		Warning("Inventory item has faulty filename!\n");
		return NULL;
	}

	char fullPathToFile[80];
	Q_snprintf(fullPathToFile, 80, "resource/data/inventory/items/%s.txt", szFile);

	KeyValues *pkvData = new KeyValues("InventoryData");
	if (pkvData->LoadFromFile(filesystem, fullPathToFile, "MOD"))
		return pkvData;

	pkvData->deleteThis();
	return NULL;
}

// Show Notes... Called by event...
void CGameBase_Client::ShowNote(const char *szFile)
{
	if (!szFile)
	{
		Warning("Note has faulty filename!\n");
		return;
	}

	if (strlen(szFile) <= 0)
	{
		Warning("Note has faulty filename!\n");
		return;
	}

	// Parse it in our note view then show it!
	if (NotePanel)
		NotePanel->ParseScriptFile(szFile);
}

// Start a dialogue scene.
void CGameBase_Client::StartDialogueScene(const char *szFile, const char *szEntity, bool bOption1, bool bOption2, bool bOption3)
{
	if (DialoguePanel != NULL)
	{
		if (!DialoguePanel->IsVisible())
			OpenPanel(4);

		DialoguePanel->SetupDialogueScene(szFile, szEntity, bOption1, bOption2, bOption3);
	}
}

// Reset vgui layouts @ all panels.
void CGameBase_Client::ResetAll()
{
	// Make sure fullbright never gets forced on.
	ConVar *fullBright = cvar->FindVar("mat_fullbright");
	if (fullBright)
		fullBright->SetValue(0);

	if (InventoryPanel != NULL)
		pszInventoryList.Purge();

	if (NotePanel != NULL)
		NotePanel->PerformDefaultLayout();

	if (DeathPanel != NULL)
		DeathPanel->PerformDefaultLayout();

	if (SavePanel != NULL)
		SavePanel->PerformDefaultLayout();

	if (DialoguePanel != NULL)
	{
		if (DialoguePanel->IsVisible())
			OpenPanel(4);

		DialoguePanel->PerformDefaultLayout();
	}

	if (MainMenu != NULL)
		MainMenu->PerformDefaultLayout();

	if (m_pGameConsole)
		ShowConsole(false, true, false);

	if (CreditsPanel != NULL)
		CreditsPanel->PerformDefaultLayout();

	ClosePanels(0);
}

// Show the console, clear it or close it.
void CGameBase_Client::ShowConsole(bool bToggle, bool bClose, bool bClear)
{
	if (m_pGameConsole)
	{
		if (bToggle)
			m_pGameConsole->ToggleConsole(!m_pGameConsole->IsVisible());

		if (bClose && m_pGameConsole->IsVisible())
			m_pGameConsole->ToggleConsole(false, true);

		if (bClear)
			m_pGameConsole->Clear();
	}
}

// Returns the game folder path to the desired achievement. (texture path)
const char *CGameBase_Client::GetAchievementForGUI(int iID)
{
	const char *szResult = NULL;

	if (AchievementManager && AchievementManager->HasAchievement(NULL, iID))
		szResult = VarArgs("achievements/achievement_%i_y", (iID + 1));
	else
		szResult = VarArgs("achievements/achievement_%i", (iID + 1));

	return szResult;
}

// Sets the state of the input string achievement, if exist.
void CGameBase_Client::SetAchievement(const char *szAchievement)
{
	if (!szAchievement || !AchievementManager)
		return;

	AchievementManager->WriteToAchievement(szAchievement);
}

// Returns true or false if you have all the achievements.
bool CGameBase_Client::HasAllAchievements(void)
{
	if (!AchievementManager)
		return false;

	return AchievementManager->HasAllAchievements();
}

// Close all open in-game panels.
void CGameBase_Client::ClosePanels(int iExcluded)
{
	// Disable HUD Crosshairs:
	CHudCrosshairs *pHudHR = GET_HUDELEMENT(CHudCrosshairs);
	if (pHudHR)
		pHudHR->Hide();

	if (NotePanel && NotePanel->IsVisible() && (iExcluded != 1))
	{
		NotePanel->SetVisible(false);
		NotePanel->OnShowPanel(false);
	}

	if (DeathPanel && DeathPanel->IsVisible() && (iExcluded != 2))
		DeathPanel->OnShowPanel(false);

	if (SavePanel && SavePanel->IsVisible() && (iExcluded != 3))
	{
		SavePanel->SetVisible(false);
		SavePanel->OnShowPanel(false);
	}

	if (DialoguePanel && DialoguePanel->IsVisible() && (iExcluded != 4))
	{
		ConVar* dialogue_menu = cvar->FindVar("cl_dialoguepanel");

		DialoguePanel->SetVisible(false);
		DialoguePanel->OnShowPanel(false);

		if (dialogue_menu)
			dialogue_menu->SetValue(0);
	}

	if (InventoryPanel && InventoryPanel->IsVisible() && (iExcluded != 5))
	{
		InventoryPanel->SetVisible(false);
		InventoryPanel->OnShowPanel(false);
	}

	if (CreditsPanel && CreditsPanel->IsVisible() && (iExcluded != 6))
		CreditsPanel->OnShowPanel(false);
}

// Open a GameUI panel for TFO.
void CGameBase_Client::OpenPanel(int iPanel)
{
	ShowConsole(false, true, false);
	ClosePanels(iPanel);

	switch (iPanel)
	{
	case 1:
	{
		if (NotePanel != NULL)
		{
			if (NotePanel->IsVisible())
			{
				NotePanel->Close();
				NotePanel->OnShowPanel(false);
			}
			else
			{
				NotePanel->Activate();
				NotePanel->OnShowPanel(true);
			}
		}
		break;
	}
	case 2:
	{
		if (DeathPanel != NULL)
		{
			if (DeathPanel->IsVisible())
				DeathPanel->OnShowPanel(false);
			else
			{
				DeathPanel->Activate();
				DeathPanel->OnShowPanel(true);
			}
		}
		break;
	}
	case 3:
	{
		if (SavePanel != NULL)
		{
			if (SavePanel->IsVisible())
			{
				SavePanel->SetVisible(false);
				SavePanel->OnShowPanel(false);
			}
			else
			{
				SavePanel->Activate();
				SavePanel->OnShowPanel(true);
			}
		}
		break;
	}
	case 4:
	{
		if (DialoguePanel != NULL)
		{
			// Use this convar to check on the server for enabling zoom fx. ( if bugs then change this to a clientCMD )...
			ConVar* dialogue_menu = cvar->FindVar("cl_dialoguepanel");

			if (DialoguePanel->IsVisible())
			{
				DialoguePanel->SetVisible(false);
				DialoguePanel->OnShowPanel(false);

				if (dialogue_menu)
					dialogue_menu->SetValue(0);
			}
			else
			{
				DialoguePanel->SetVisible(true);
				DialoguePanel->Activate();
				DialoguePanel->OnShowPanel(true);

				if (dialogue_menu)
					dialogue_menu->SetValue(1);
			}
		}
		break;
	}
	case 5:
	{
		if (InventoryPanel != NULL)
		{
			if (InventoryPanel->IsVisible())
			{
				InventoryPanel->Close();
				InventoryPanel->OnShowPanel(false);
			}
			else
			{
				InventoryPanel->Activate();
				InventoryPanel->OnShowPanel(true);
			}
		}
		break;
	}
	case 6:
	{
		if (CreditsPanel != NULL)
		{
			if (CreditsPanel->IsVisible())
			{
				CreditsPanel->Close();
				CreditsPanel->OnShowPanel(false);
			}
			else
			{
				CreditsPanel->Activate();
				CreditsPanel->OnShowPanel(true);
			}
		}
		break;
	}
	default:
		Warning("Invalid GameUI command!\n");
		break;
	}
}

static CGameBase_Client g_GBSClient;
IGameBase_Client *GameBaseClient = (IGameBase_Client *)&g_GBSClient;

// Console Commands
CON_COMMAND(OpenGameConsole, "Toggle the Console ON or OFF...")
{
	GameBaseClient->ShowConsole(true, false, false);
};

CON_COMMAND(CloseGameConsole, "Force the Console to close!")
{
	GameBaseClient->ShowConsole(false, true, false);
};

CON_COMMAND(ClearGameConsole, "Reset Console/Clear all history text.")
{
	GameBaseClient->ShowConsole(false, false, true);
};

CON_COMMAND(tfo_gameui_command, "Open or Close gameui panels for tfo.")
{
	if (args.ArgC() != 2)
	{
		Warning("This command requires 2 arguments!\n");
		return;
	}

	const char *szPanel = args[1];

	// Which panel do we want to open or close?
	if (!strcmp(szPanel, "OpenNotePanel"))
		GameBaseClient->OpenPanel(1);
	else if (!strcmp(szPanel, "OpenDeathPanel"))
		GameBaseClient->OpenPanel(2);
	else if (!strcmp(szPanel, "OpenSavePanel"))
		GameBaseClient->OpenPanel(3);
	else if (!strcmp(szPanel, "OpenDialogue"))
		GameBaseClient->OpenPanel(4);
	else if (!strcmp(szPanel, "OpenInventoryPanel"))
		GameBaseClient->OpenPanel(5);
	else if (!strcmp(szPanel, "OpenCreditsPanel"))
		GameBaseClient->OpenPanel(6);
	else
		Warning("Invalid panel!\n");
};

// Takes us back to the main menu 3D screen!
CON_COMMAND(tfo_mainmenu, "Return to the main menu screen!")
{
	if (!GameBaseClient->CanLoadMainMenu())
		GameBaseClient->Initialize();
};