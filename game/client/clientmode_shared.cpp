//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Normal HUD mode
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//


#include "cbase.h"
#include "clientmode_shared.h"
#include "iinput.h"
#include "view_shared.h"
#include "iviewrender.h"
#include "hud_basechat.h"
#include "weapon_selection.h"
#include <vgui/IVGui.h>
#include <vgui/Cursor.h>
#include <vgui/IPanel.h>
#include <vgui/IInput.h>
#include "engine/IEngineSound.h"
#include <KeyValues.h>
#include <vgui_controls/AnimationController.h>
#include "vgui_int.h"
#include "hud_macros.h"
#include "hltvcamera.h"
#include "particlemgr.h"
#include "c_vguiscreen.h"
#include "c_team.h"
#include "c_rumble.h"
#include "fmtstr.h"
#include "cam_thirdperson.h"
#include <vgui/ILocalize.h>
#include "ienginevgui.h"
#include "c_achievement_manager.h"
#if defined( _X360 )
#include "xbox/xbox_console.h"
#endif

#if defined USES_ECON_ITEMS
#include "econ_item_view.h"
#endif

#if defined( TF_CLIENT_DLL )
#include "c_tf_player.h"
#include "econ_item_description.h"
#endif

// TFO
#include "clienteffectprecachesystem.h"
#include "fmod_manager.h"
#include "hl2_gamerules.h"
#include "GameBase_Client.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define ACHIEVEMENT_ANNOUNCEMENT_MIN_TIME 10

CLIENTEFFECT_REGISTER_BEGIN(PrecachePostProcessingEffectsGlow)
CLIENTEFFECT_MATERIAL("dev/glow_color")
CLIENTEFFECT_MATERIAL("dev/halo_add_to_screen")
CLIENTEFFECT_REGISTER_END_CONDITIONAL(engine->GetDXSupportLevel() >= 90)

class CHudWeaponSelection;
class CHudChat;
class CHudVote;

static vgui::HContext s_hVGuiContext = DEFAULT_VGUI_CONTEXT;

ConVar cl_drawhud( "cl_drawhud", "1", FCVAR_CHEAT, "Enable the rendering of the hud" );
ConVar hud_freezecamhide( "hud_freezecamhide", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Hide the HUD during freeze-cam" );
ConVar cl_show_num_particle_systems( "cl_show_num_particle_systems", "0", FCVAR_CLIENTDLL, "Display the number of active particle systems." );

extern ConVar v_viewmodel_fov;
extern ConVar voice_modenable;

#ifdef VOICE_VOX_ENABLE
void VoxCallback( IConVar *var, const char *oldString, float oldFloat )
{
	if ( engine && engine->IsConnected() )
	{
		ConVarRef voice_vox( var->GetName() );
		if ( voice_vox.GetBool() && voice_modenable.GetBool() )
		{
			engine->ClientCmd_Unrestricted( "voicerecord_toggle on\n" );
		}
		else
		{
			engine->ClientCmd_Unrestricted( "voicerecord_toggle off\n" );
		}
	}
}
ConVar voice_vox( "voice_vox", "0", FCVAR_ARCHIVE, "Voice chat uses a vox-style always on", true, 0, true, 1, VoxCallback );

// --------------------------------------------------------------------------------- //
// CVoxManager.
// --------------------------------------------------------------------------------- //
class CVoxManager : public CAutoGameSystem
{
public:
	CVoxManager() : CAutoGameSystem( "VoxManager" )
	{
	}

	virtual void LevelInitPostEntity( void )
	{
		if ( voice_vox.GetBool() && voice_modenable.GetBool() )
		{
			engine->ClientCmd_Unrestricted( "voicerecord_toggle on\n" );
		}
	}

	virtual void LevelShutdownPreEntity( void )
	{
		if ( voice_vox.GetBool() )
		{
			engine->ClientCmd_Unrestricted( "voicerecord_toggle off\n" );
		}
	}
};

static CVoxManager s_VoxManager;
// --------------------------------------------------------------------------------- //
#endif // VOICE_VOX_ENABLE

CON_COMMAND( hud_reloadscheme, "Reloads hud layout and animation scripts." )
{
	ClientModeShared *mode = ( ClientModeShared * )GetClientModeNormal();
	if ( !mode )
		return;

	mode->ReloadScheme();
}

#ifdef _DEBUG
CON_COMMAND_F( crash, "Crash the client. Optional parameter -- type of crash:\n 0: read from NULL\n 1: write to NULL\n 2: DmCrashDump() (xbox360 only)", FCVAR_CHEAT )
{
	int crashtype = 0;
	int dummy;
	if ( args.ArgC() > 1 )
	{
		 crashtype = Q_atoi( args[1] );
	}
	switch (crashtype)
	{
		case 0:
			dummy = *((int *) NULL);
			Msg("Crashed! %d\n", dummy); // keeps dummy from optimizing out
			break;
		case 1:
			*((int *)NULL) = 42;
			break;
#if defined( _X360 )
		case 2:
			XBX_CrashDump(false);
			break;
#endif
		default:
			Msg("Unknown variety of crash. You have now failed to crash. I hope you're happy.\n");
			break;
	}
}
#endif // _DEBUG

static void __MsgFunc_Rumble( bf_read &msg )
{
	unsigned char waveformIndex;
	unsigned char rumbleData;
	unsigned char rumbleFlags;

	waveformIndex = msg.ReadByte();
	rumbleData = msg.ReadByte();
	rumbleFlags = msg.ReadByte();

	RumbleEffect( waveformIndex, rumbleData, rumbleFlags );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ClientModeShared::ClientModeShared()
{
	m_pViewport = NULL;
	m_pChatElement = NULL;
	m_pWeaponSelection = NULL;
	m_nRootSize[ 0 ] = m_nRootSize[ 1 ] = -1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ClientModeShared::~ClientModeShared()
{
	delete m_pViewport; 
}

void ClientModeShared::ReloadScheme( void )
{
	m_pViewport->ReloadScheme( "resource/ClientScheme.res" );
	ClearKeyValuesCache();
}

//----------------------------------------------------------------------------
// Purpose: Let the client mode set some vgui conditions
//-----------------------------------------------------------------------------
void ClientModeShared::ComputeVguiResConditions( KeyValues *pkvConditions ) 
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ClientModeShared::Init()
{
	m_pChatElement = ( CBaseHudChat * )GET_HUDELEMENT( CHudChat );
	Assert( m_pChatElement );

	m_pWeaponSelection = ( CBaseHudWeaponSelection * )GET_HUDELEMENT( CHudWeaponSelection );
	Assert( m_pWeaponSelection );

	KeyValuesAD pConditions( "conditions" );
	ComputeVguiResConditions( pConditions );

	// Derived ClientMode class must make sure m_Viewport is instantiated
	Assert( m_pViewport );
	m_pViewport->LoadControlSettings( "scripts/HudLayout.res", NULL, NULL, pConditions );

	ListenForGameEvent( "player_connect" );
	ListenForGameEvent( "player_disconnect" );
	ListenForGameEvent( "player_team" );
	ListenForGameEvent( "server_cvar" );
	ListenForGameEvent( "player_changename" );
	ListenForGameEvent( "teamplay_broadcast_audio" );
	ListenForGameEvent( "achievement_earned" );

#if defined( TF_CLIENT_DLL )
	ListenForGameEvent( "item_found" );
#endif 

#ifndef _XBOX
	HLTVCamera()->Init();
#endif

	m_CursorNone = vgui::dc_none;

	HOOK_MESSAGE(Rumble);
	
	AchievementManager->Load();
}


void ClientModeShared::InitViewport()
{
}


void ClientModeShared::VGui_Shutdown()
{
	delete m_pViewport;
	m_pViewport = NULL;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ClientModeShared::Shutdown()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : frametime - 
//			*cmd - 
//-----------------------------------------------------------------------------
bool ClientModeShared::CreateMove( float flInputSampleTime, CUserCmd *cmd )
{
	// Let the player override the view.
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if(!pPlayer)
		return true;

	// Let the player at it
	return pPlayer->CreateMove( flInputSampleTime, cmd );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pSetup - 
//-----------------------------------------------------------------------------
void ClientModeShared::OverrideView( CViewSetup *pSetup )
{
	QAngle camAngles;

	// Let the player override the view.
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if(!pPlayer)
		return;

	pPlayer->OverrideView( pSetup );

	if( ::input->CAM_IsThirdPerson() )
	{
		Vector cam_ofs = g_ThirdPersonManager.GetCameraOffsetAngles();
		Vector cam_ofs_distance = g_ThirdPersonManager.GetFinalCameraOffset();

		cam_ofs_distance *= g_ThirdPersonManager.GetDistanceFraction();

		camAngles[ PITCH ] = cam_ofs[ PITCH ];
		camAngles[ YAW ] = cam_ofs[ YAW ];
		camAngles[ ROLL ] = 0;

		Vector camForward, camRight, camUp;
		

		if ( g_ThirdPersonManager.IsOverridingThirdPerson() == false )
		{
			engine->GetViewAngles( camAngles );
		}
			
		// get the forward vector
		AngleVectors( camAngles, &camForward, &camRight, &camUp );
	
		VectorMA( pSetup->origin, -cam_ofs_distance[0], camForward, pSetup->origin );
		VectorMA( pSetup->origin, cam_ofs_distance[1], camRight, pSetup->origin );
		VectorMA( pSetup->origin, cam_ofs_distance[2], camUp, pSetup->origin );

		// Override angles from third person camera
		VectorCopy( camAngles, pSetup->angles );
	}
	else if (::input->CAM_IsOrthographic())
	{
		pSetup->m_bOrtho = true;
		float w, h;
		::input->CAM_OrthographicSize( w, h );
		w *= 0.5f;
		h *= 0.5f;
		pSetup->m_OrthoLeft   = -w;
		pSetup->m_OrthoTop    = -h;
		pSetup->m_OrthoRight  = w;
		pSetup->m_OrthoBottom = h;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool ClientModeShared::ShouldDrawEntity(C_BaseEntity *pEnt)
{
	return true;
}

bool ClientModeShared::ShouldDrawParticles( )
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Allow weapons to override mouse input (for binoculars)
//-----------------------------------------------------------------------------
void ClientModeShared::OverrideMouseInput( float *x, float *y )
{
	C_BaseCombatWeapon *pWeapon = GetActiveWeapon();
	if ( pWeapon )
	{
		pWeapon->OverrideMouseInput( x, y );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool ClientModeShared::ShouldDrawViewModel()
{
	return true;
}

bool ClientModeShared::ShouldDrawDetailObjects( )
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if VR mode should black out everything outside the HUD.
//			This is used for things like sniper scopes and full screen UI
//-----------------------------------------------------------------------------
bool ClientModeShared::ShouldBlackoutAroundHUD()
{
	return enginevgui->IsGameUIVisible();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool ClientModeShared::ShouldDrawCrosshair( void )
{
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Don't draw the current view entity if we are using the fake viewmodel instead
//-----------------------------------------------------------------------------
bool ClientModeShared::ShouldDrawLocalPlayer( C_BasePlayer *pPlayer )
{
	if ( ( pPlayer->index == render->GetViewEntity() ) && !C_BasePlayer::ShouldDrawLocalPlayer() )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: The mode can choose to not draw fog
//-----------------------------------------------------------------------------
bool ClientModeShared::ShouldDrawFog( void )
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ClientModeShared::AdjustEngineViewport( int& x, int& y, int& width, int& height )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ClientModeShared::PreRender( CViewSetup *pSetup )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ClientModeShared::PostRender()
{
	// Let the particle manager simulate things that haven't been simulated.
	ParticleMgr()->PostRender();
}

void ClientModeShared::PostRenderVGui()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ClientModeShared::Update()
{
	if ( m_pViewport->IsVisible() != cl_drawhud.GetBool() )
	{
		m_pViewport->SetVisible( cl_drawhud.GetBool() );
	}

	UpdateRumbleEffects();

	if ( cl_show_num_particle_systems.GetBool() )
	{
		int nCount = 0;

		for ( int i = 0; i < g_pParticleSystemMgr->GetParticleSystemCount(); i++ )
		{
			const char *pParticleSystemName = g_pParticleSystemMgr->GetParticleSystemNameFromIndex(i);
			CParticleSystemDefinition *pParticleSystem = g_pParticleSystemMgr->FindParticleSystem( pParticleSystemName );
			if ( !pParticleSystem )
				continue;

			for ( CParticleCollection *pCurCollection = pParticleSystem->FirstCollection();
				  pCurCollection != NULL;
				  pCurCollection = pCurCollection->GetNextCollectionUsingSameDef() )
			{
				++nCount;
			}
		}

		engine->Con_NPrintf( 0, "# Active particle systems: %i", nCount );
	}
}

//-----------------------------------------------------------------------------
// This processes all input before SV Move messages are sent
//-----------------------------------------------------------------------------

void ClientModeShared::ProcessInput(bool bActive)
{
	gHUD.ProcessInput( bActive );
}

//-----------------------------------------------------------------------------
// Purpose: We've received a keypress from the engine. Return 1 if the engine is allowed to handle it.
//-----------------------------------------------------------------------------
int	ClientModeShared::KeyInput( int down, ButtonCode_t keynum, const char *pszCurrentBinding )
{
	if ( engine->Con_IsVisible() )
		return 1;
	
	// Should we start typing a message?
	if ( pszCurrentBinding &&
		( Q_strcmp( pszCurrentBinding, "messagemode" ) == 0 ||
		  Q_strcmp( pszCurrentBinding, "say" ) == 0 ) )
	{
		if ( down )
		{
			StartMessageMode( MM_SAY );
		}
		return 0;
	}
	else if ( pszCurrentBinding &&
				( Q_strcmp( pszCurrentBinding, "messagemode2" ) == 0 ||
				  Q_strcmp( pszCurrentBinding, "say_team" ) == 0 ) )
	{
		if ( down )
		{
			StartMessageMode( MM_SAY_TEAM );
		}
		return 0;
	}

	// Let game-specific hud elements get a crack at the key input
	if ( !HudElementKeyInput( down, keynum, pszCurrentBinding ) )
	{
		return 0;
	}

	C_BaseCombatWeapon *pWeapon = GetActiveWeapon();
	if ( pWeapon )
	{
		return pWeapon->KeyInput( down, keynum, pszCurrentBinding );
	}

	return 1;
}

//-----------------------------------------------------------------------------
// Purpose: See if hud elements want key input. Return 0 if the key is swallowed
//-----------------------------------------------------------------------------
int ClientModeShared::HudElementKeyInput( int down, ButtonCode_t keynum, const char *pszCurrentBinding )
{
	if ( m_pWeaponSelection )
	{
		if ( !m_pWeaponSelection->KeyInput( down, keynum, pszCurrentBinding ) )
		{
			return 0;
		}
	}
	return 1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool ClientModeShared::DoPostScreenSpaceEffects( const CViewSetup *pSetup )
{
	g_GlowObjectManager.RenderGlowEffects(pSetup);
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : vgui::Panel
//-----------------------------------------------------------------------------
vgui::Panel *ClientModeShared::GetMessagePanel()
{
	if ( m_pChatElement && m_pChatElement->GetInputPanel() && m_pChatElement->GetInputPanel()->IsVisible() )
		return m_pChatElement->GetInputPanel();

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: The player has started to type a message
//-----------------------------------------------------------------------------
void ClientModeShared::StartMessageMode( int iMessageModeType )
{
	// Can only show chat UI in multiplayer!!!
	if ( gpGlobals->maxClients == 1 )
	{
		return;
	}
	if ( m_pChatElement )
	{
		m_pChatElement->StartMessageMode( iMessageModeType );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *newmap - 
//-----------------------------------------------------------------------------
void ClientModeShared::LevelInit( const char *newmap )
{
	// Make sure we always use the proper loading screen.
	engine->ClientCmd("progress_enable\n");
	
	// Reset ALL In-game VGUI:
	GameBaseClient->ResetAll();

	m_pViewport->GetAnimationController()->StartAnimationSequence("LevelInit");

	// Tell the Chat Interface
	if ( m_pChatElement )
	{
		m_pChatElement->LevelInit( newmap );
	}

	// we have to fake this event clientside, because clients connect after that
	IGameEvent *event = gameeventmanager->CreateEvent( "game_newmap" );
	if ( event )
	{
		event->SetString("mapname", newmap );
		gameeventmanager->FireEventClientSide( event );
	}

	// Create a vgui context for all of the in-game vgui panels...
	if ( s_hVGuiContext == DEFAULT_VGUI_CONTEXT )
	{
		s_hVGuiContext = vgui::ivgui()->CreateContext();
	}

	// Reset any player explosion/shock effects
	CLocalPlayerFilter filter;
	enginesound->SetPlayerDSP( filter, 0, true );

	SetScreenBlurState(false);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ClientModeShared::LevelShutdown( void )
{
	// Make sure we always use the proper loading screen.
	engine->ClientCmd("progress_enable\n");

	// Reset All VGUI (in-game stuff).
	GameBaseClient->ResetAll();

	// Reset the third person camera so we don't crash
	g_ThirdPersonManager.Init();

	if ( m_pChatElement )
	{
		m_pChatElement->LevelShutdown();
	}
	if ( s_hVGuiContext != DEFAULT_VGUI_CONTEXT )
	{
		vgui::ivgui()->DestroyContext( s_hVGuiContext );
 		s_hVGuiContext = DEFAULT_VGUI_CONTEXT;
	}

	// Reset any player explosion/shock effects
	CLocalPlayerFilter filter;
	enginesound->SetPlayerDSP( filter, 0, true );

	SetScreenBlurState(false);
}


void ClientModeShared::Enable()
{
	vgui::VPANEL pRoot = VGui_GetClientDLLRootPanel();;

	// Add our viewport to the root panel.
	if( pRoot != 0 )
	{
		m_pViewport->SetParent( pRoot );
	}

	// All hud elements should be proportional
	// This sets that flag on the viewport and all child panels
	m_pViewport->SetProportional( true );

	m_pViewport->SetCursor( m_CursorNone );
	vgui::surface()->SetCursor( m_CursorNone );

	m_pViewport->SetVisible( true );
	if ( m_pViewport->IsKeyBoardInputEnabled() )
	{
		m_pViewport->RequestFocus();
	}

	Layout();
}


void ClientModeShared::Disable()
{
	vgui::VPANEL pRoot = VGui_GetClientDLLRootPanel();;

	// Remove our viewport from the root panel.
	if( pRoot != 0 )
	{
		m_pViewport->SetParent( (vgui::VPANEL)NULL );
	}

	m_pViewport->SetVisible( false );
}


void ClientModeShared::Layout()
{
	vgui::VPANEL pRoot = VGui_GetClientDLLRootPanel();
	int wide, tall;

	// Make the viewport fill the root panel.
	if( pRoot != 0 )
	{
		vgui::ipanel()->GetSize(pRoot, wide, tall);

		bool changed = wide != m_nRootSize[ 0 ] || tall != m_nRootSize[ 1 ];
		m_nRootSize[ 0 ] = wide;
		m_nRootSize[ 1 ] = tall;

		m_pViewport->SetBounds(0, 0, wide, tall);
		if ( changed )
		{
			ReloadScheme();
		}
	}
}

float ClientModeShared::GetViewModelFOV( void )
{
	return v_viewmodel_fov.GetFloat();
}

class CHudChat;

bool PlayerNameNotSetYet( const char *pszName )
{
	if ( pszName && pszName[0] )
	{
		// Don't show "unconnected" if we haven't got the players name yet
		if ( Q_strnicmp(pszName,"unconnected",11) == 0 )
			return true;
		if ( Q_strnicmp(pszName,"NULLNAME",11) == 0 )
			return true;
	}

	return false;
}

void ClientModeShared::FireGameEvent( IGameEvent *event )
{
	CBaseHudChat *hudChat = (CBaseHudChat *)GET_HUDELEMENT( CHudChat );

	const char *eventname = event->GetName();

	if ( Q_strcmp( "player_connect", eventname ) == 0 )
	{
		if (!hudChat || PlayerNameNotSetYet(event->GetString("name")))
			return;

		wchar_t wszLocalized[100];
		wchar_t wszPlayerName[MAX_PLAYER_NAME_LENGTH];
		g_pVGuiLocalize->ConvertANSIToUnicode(event->GetString("name"), wszPlayerName, sizeof(wszPlayerName));
		g_pVGuiLocalize->ConstructString(wszLocalized, sizeof(wszLocalized), g_pVGuiLocalize->Find("#game_player_joined_game"), 1, wszPlayerName);

		char szLocalized[100];
		g_pVGuiLocalize->ConvertUnicodeToANSI(wszLocalized, szLocalized, sizeof(szLocalized));

		hudChat->Printf(CHAT_FILTER_JOINLEAVE, "%s", szLocalized);
	}
	else if ( Q_strcmp( "player_disconnect", eventname ) == 0 )
	{
		C_BasePlayer *pPlayer = USERID2PLAYER( event->GetInt("userid") );

		if (!hudChat || !pPlayer || PlayerNameNotSetYet(event->GetString("name")))
			return;

		wchar_t wszPlayerName[MAX_PLAYER_NAME_LENGTH];
		g_pVGuiLocalize->ConvertANSIToUnicode(pPlayer->GetPlayerName(), wszPlayerName, sizeof(wszPlayerName));

		wchar_t wszReason[64];
		const char* pszReason = event->GetString("reason");
		if (pszReason && (pszReason[0] == '#') && g_pVGuiLocalize->Find(pszReason))
		{
			V_wcsncpy(wszReason, g_pVGuiLocalize->Find(pszReason), sizeof(wszReason));
		}
		else
		{
			g_pVGuiLocalize->ConvertANSIToUnicode(pszReason, wszReason, sizeof(wszReason));
		}

		wchar_t wszLocalized[100];
		if (IsPC())
		{
			g_pVGuiLocalize->ConstructString(wszLocalized, sizeof(wszLocalized), g_pVGuiLocalize->Find("#game_player_left_game"), 2, wszPlayerName, wszReason);
		}
		else
		{
			g_pVGuiLocalize->ConstructString(wszLocalized, sizeof(wszLocalized), g_pVGuiLocalize->Find("#game_player_left_game"), 1, wszPlayerName);
		}

		char szLocalized[100];
		g_pVGuiLocalize->ConvertUnicodeToANSI(wszLocalized, szLocalized, sizeof(szLocalized));

		hudChat->Printf(CHAT_FILTER_JOINLEAVE, "%s", szLocalized);
	}
	else if ( Q_strcmp( "player_team", eventname ) == 0 )
	{
		C_BasePlayer *pPlayer = USERID2PLAYER( event->GetInt("userid") );
		if ( !hudChat )
			return;

		bool bDisconnected = event->GetBool("disconnect");

		if ( bDisconnected )
			return;

		int team = event->GetInt( "team" );
		bool bAutoTeamed = event->GetInt( "autoteam", false );
		bool bSilent = event->GetInt( "silent", false );

		const char *pszName = event->GetString( "name" );
		if ( PlayerNameNotSetYet( pszName ) )
			return;

		if ( !bSilent )
		{
			wchar_t wszPlayerName[MAX_PLAYER_NAME_LENGTH];
			g_pVGuiLocalize->ConvertANSIToUnicode( pszName, wszPlayerName, sizeof(wszPlayerName) );

			wchar_t wszTeam[64];
			C_Team *pTeam = GetGlobalTeam( team );
			if ( pTeam )
			{
				g_pVGuiLocalize->ConvertANSIToUnicode( pTeam->Get_Name(), wszTeam, sizeof(wszTeam) );
			}
			else
			{
				_snwprintf ( wszTeam, sizeof( wszTeam ) / sizeof( wchar_t ), L"%d", team );
			}

			wchar_t wszLocalized[100];
			if (bAutoTeamed)
			{
				g_pVGuiLocalize->ConstructString(wszLocalized, sizeof(wszLocalized), g_pVGuiLocalize->Find("#game_player_joined_autoteam"), 2, wszPlayerName, wszTeam);
			}
			else
			{
				g_pVGuiLocalize->ConstructString(wszLocalized, sizeof(wszLocalized), g_pVGuiLocalize->Find("#game_player_joined_team"), 2, wszPlayerName, wszTeam);
			}

			char szLocalized[100];
			g_pVGuiLocalize->ConvertUnicodeToANSI(wszLocalized, szLocalized, sizeof(szLocalized));

			hudChat->Printf(CHAT_FILTER_TEAMCHANGE, "%s", szLocalized);
		}

		if ( pPlayer && pPlayer->IsLocalPlayer() )
		{
			// that's me
			pPlayer->TeamChange( team );
		}
	}
	else if ( Q_strcmp( "player_changename", eventname ) == 0 )
	{
		if ( !hudChat )
			return;

		const char *pszOldName = event->GetString("oldname");
		if ( PlayerNameNotSetYet(pszOldName) )
			return;

		wchar_t wszOldName[MAX_PLAYER_NAME_LENGTH];
		g_pVGuiLocalize->ConvertANSIToUnicode( pszOldName, wszOldName, sizeof(wszOldName) );

		wchar_t wszNewName[MAX_PLAYER_NAME_LENGTH];
		g_pVGuiLocalize->ConvertANSIToUnicode( event->GetString( "newname" ), wszNewName, sizeof(wszNewName) );

		wchar_t wszLocalized[100];
		g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#game_player_changed_name" ), 2, wszOldName, wszNewName );

		char szLocalized[100];
		g_pVGuiLocalize->ConvertUnicodeToANSI( wszLocalized, szLocalized, sizeof(szLocalized) );

		hudChat->Printf( CHAT_FILTER_NAMECHANGE, "%s", szLocalized );
	}
	else if (Q_strcmp( "teamplay_broadcast_audio", eventname ) == 0 )
	{
		int team = event->GetInt( "team" );

		bool bValidTeam = false;

		if ( (GetLocalTeam() && GetLocalTeam()->GetTeamNumber() == team) )
		{
			bValidTeam = true;
		}

		//If we're in the spectator team then we should be getting whatever messages the person I'm spectating gets.
		if ( bValidTeam == false )
		{
			CBasePlayer *pSpectatorTarget = UTIL_PlayerByIndex( GetSpectatorTarget() );

			if ( pSpectatorTarget && (GetSpectatorMode() == OBS_MODE_IN_EYE || GetSpectatorMode() == OBS_MODE_CHASE) )
			{
				if ( pSpectatorTarget->GetTeamNumber() == team )
				{
					bValidTeam = true;
				}
			}
		}

		if ( team == 0 && GetLocalTeam() > 0 )
		{
			bValidTeam = false;
		}

		if ( team == 255 )
		{
			bValidTeam = true;
		}

		if ( bValidTeam == true )
		{
			EmitSound_t et;
			et.m_pSoundName = event->GetString("sound");
			et.m_nFlags = event->GetInt("additional_flags");

			CLocalPlayerFilter filter;
			C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, et );
		}
	}
	else if ( Q_strcmp( "server_cvar", eventname ) == 0 )
	{
		wchar_t wszCvarName[64];
		g_pVGuiLocalize->ConvertANSIToUnicode(event->GetString("cvarname"), wszCvarName, sizeof(wszCvarName));

		wchar_t wszCvarValue[64];
		g_pVGuiLocalize->ConvertANSIToUnicode(event->GetString("cvarvalue"), wszCvarValue, sizeof(wszCvarValue));

		wchar_t wszLocalized[256];
		g_pVGuiLocalize->ConstructString(wszLocalized, sizeof(wszLocalized), g_pVGuiLocalize->Find("#game_server_cvar_changed"), 2, wszCvarName, wszCvarValue);

		char szLocalized[256];
		g_pVGuiLocalize->ConvertUnicodeToANSI(wszLocalized, szLocalized, sizeof(szLocalized));

		hudChat->Printf(CHAT_FILTER_SERVERMSG, "%s", szLocalized);
	}
	else if ( Q_strcmp( "achievement_earned", eventname ) == 0 )
	{
		// unused
	}
	else
	{
		DevMsg( 2, "Unhandled GameEvent in ClientModeShared::FireGameEvent - %s\n", event->GetName()  );
	}
}

void ClientModeShared::UpdateReplayMessages()
{
}

void ClientModeShared::ClearReplayMessageList()
{
}

void ClientModeShared::DisplayReplayMessage( const char *pLocalizeName, float flDuration, bool bUrgent,
											 const char *pSound, bool bDlg )
{
}

void ClientModeShared::DisplayReplayReminder()
{
}


//-----------------------------------------------------------------------------
// In-game VGUI context 
//-----------------------------------------------------------------------------
void ClientModeShared::ActivateInGameVGuiContext( vgui::Panel *pPanel )
{
	vgui::ivgui()->AssociatePanelWithContext( s_hVGuiContext, pPanel->GetVPanel() );
	vgui::ivgui()->ActivateContext( s_hVGuiContext );
}

void ClientModeShared::DeactivateInGameVGuiContext()
{
	vgui::ivgui()->ActivateContext( DEFAULT_VGUI_CONTEXT );
}