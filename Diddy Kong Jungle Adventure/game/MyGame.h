#pragma once

class CMyGame : public CGame
{
	// Define sprites and other instance variables here
	CSprite m_player;		// Player sprite
	CSprite background;
	CSprite startScreen;
	CSprite lossScreen;
	CSprite winScreen;

	CSpriteList m_sprites;	// All other sprites
	CSpriteList shotList;



	CSoundPlayer jumpSound;
	CSoundPlayer bounceSound;
	CSoundPlayer collectSound;
	CSoundPlayer clickSound;
	CSoundPlayer cratebreakSound;

	// Player State
	enum { STANDING, RUNNING, AIRBORNE, CLIMBING } m_state;
	enum { LEFT, RIGHT } m_side;

	bool bGotKey;
	int level;
	float f1;
	float f2;
	float Y;
	float X;
	

	//Functions
	void PlayerControl();
	void EnemyControl();
	void ShotControl();
	void SetupLevel1();
	void SetupLevel2();
	void SetupLevel3();
	void CrateControl();


public:
	CMyGame(void);
	~CMyGame(void);

	// Per-Frame Callback Funtions (must be implemented!)
	virtual void OnUpdate();
	virtual void OnDraw(CGraphics* g);

	// Game Life Cycle
	virtual void OnInitialize();
	virtual void OnDisplayMenu();
	virtual void OnStartGame();
	virtual void OnStartLevel(Sint16 nLevel);
	virtual void OnGameOver();
	virtual void OnTerminate();

	// Keyboard Event Handlers
	virtual void OnKeyDown(SDLKey sym, SDLMod mod, Uint16 unicode);
	virtual void OnKeyUp(SDLKey sym, SDLMod mod, Uint16 unicode);

	// Mouse Events Handlers
	virtual void OnMouseMove(Uint16 x,Uint16 y,Sint16 relx,Sint16 rely,bool bLeft,bool bRight,bool bMiddle);
	virtual void OnLButtonDown(Uint16 x,Uint16 y);
	virtual void OnLButtonUp(Uint16 x,Uint16 y);
	virtual void OnRButtonDown(Uint16 x,Uint16 y);
	virtual void OnRButtonUp(Uint16 x,Uint16 y);
	virtual void OnMButtonDown(Uint16 x,Uint16 y);
	virtual void OnMButtonUp(Uint16 x,Uint16 y);
};
