#include "stdafx.h"
#include "MyGame.h"

CMyGame::CMyGame(void) : 
	m_player(400, 540, 0, 0, 0), startScreen(400, 300, "startScreen.bmp", 0), lossScreen(400, 300, "loss.bmp", 0),winScreen(400,300, "winScreen.bmp",0)
	// to initialise more sprites here use a comma-separated list
{
	// TODO: add initialisation here
	level = 1;
}

CMyGame::~CMyGame(void)
{
	// TODO: add destruction code here
}

/////////////////////////////////////////////////////
// Per-Frame Callback Funtions (must be implemented!)

void CMyGame::OnUpdate()
{
	if (IsMenuMode()) return;
	Uint32 t = GetTime();

	PlayerControl();
	// Pre-Update Position
	CVector v0 = m_player.GetPos();

	// Updates
	for (CSprite* pSprite : m_sprites)
		pSprite->Update(t);

	ShotControl();
	for (CSprite* pShot : shotList) pShot->Update(t);
	m_player.Update(t);
	

	int h = m_player.GetHeight() / 2 - 1;

	bool bTouchingPlatform = false;
	bool bTouchingRope = false;

	for (CSprite* pSprite : m_sprites) 
	{
		if (m_player.HitTest(pSprite, 0))
		{
			// platforms and walls
			if (((string)pSprite->GetProperty("tag") == "platform" && m_state != CLIMBING) || ((string)pSprite->GetProperty("tag") == "crate" && m_state != CLIMBING && !pSprite->IsDying()))
			{
				if (v0.m_y >= pSprite->GetTop())
				{
					bTouchingPlatform = true;
					if (m_player.GetYVelocity() < -950)
					{
						GameOver();
					}
					m_player.SetVelocity(0, 0);
					m_player.SetY(pSprite->GetTop() + h);
				}

				else if (v0.m_y <= pSprite->GetBottom() - h)
				{
					bTouchingPlatform = true;
					m_player.SetVelocity(0, 0);
					m_player.SetY(m_player.GetY()-5);
					//m_player.SetY(pSprite->GetBottom() - h);
				}

				if (v0.m_x >= pSprite->GetRight() + h)
				{
					bTouchingPlatform = true;
					m_player.SetVelocity(0, 0);
					m_player.SetX(m_player.GetX() + h);
					
				}

				else if (v0.m_x <= pSprite->GetLeft() - h)
				{
					bTouchingPlatform = true;
					m_player.SetVelocity(0, 0);
					m_player.SetX(m_player.GetX() - h);
				}


			}
			//Ropes
			if ((string)pSprite->GetProperty("tag") == "rope") 
			{
				bTouchingRope = true;
				m_player.SetVelocity(0, 0);
			}
			//Hazards
			if ((string)pSprite->GetProperty("tag") == "hazards")
			{
				GameOver();
			}
			//Keys
			if ((string)pSprite->GetProperty("tag") == "key") 
			{
				bGotKey = true;
				pSprite->Delete();
			}
			//Gate
			if ((string)pSprite->GetProperty("tag") == "gate" && bGotKey == true)
			{
				NewLevel();
			}
			//Enemy
			if ((string)pSprite->GetProperty("tag") == "enemy") 
			{
				GameOver();
			}
		}


	}
	m_sprites.delete_if(deleted);

	//Processing of airborne condition
	if (m_state == AIRBORNE && bTouchingPlatform) 
	{
		//Just Landed
		m_state = STANDING;
		m_player.SetImage(m_side == LEFT ? "stand_left" : "stand_right");
	}


	if (m_state != AIRBORNE && !bTouchingPlatform) 
	{
		//Just Taken Off
		m_state = AIRBORNE;
		m_player.SetImage(m_side == LEFT ? "jump_left" : "jump_right");
	}

	if (m_state == AIRBORNE && bTouchingRope) 
	{
		//Grabbing onto Rope
		m_state = CLIMBING;
		m_player.SetImage(m_side == LEFT ? "hang_left" : "hang_right");
	}


	if (m_state == CLIMBING) 
	{
		if ((IsKeyDown(SDLK_w) || IsKeyDown(SDLK_UP)) && m_state == CLIMBING)
		{
			m_player.Accelerate(0, 300);
			m_player.SetImage(m_side == LEFT ? "hang_left" : "hang_right");
			//m_player.SetAnimation("climb");
		}
		else if ((IsKeyDown(SDLK_s) || IsKeyDown(SDLK_DOWN)) && m_state == CLIMBING)
		{
			m_player.Accelerate(0, -300);
			m_player.SetImage(m_side == LEFT ? "hang_left" : "hang_right");
			//m_player.SetAnimation("climb");
		}
	}


	EnemyControl();
	CrateControl();

	if (IsGameOverMode()) 
	{
		jumpSound.Stop();
		bounceSound.Stop();
	}

}

void CMyGame::PlayerControl() 
{
	if (m_state == AIRBORNE && m_state != CLIMBING)m_player.Accelerate(0, -50);



	if (m_state == STANDING || m_state == RUNNING)
	{
		//this is where all the code should go -that resets the player's velocity,
		m_player.SetVelocity(0, 0);
		// and then checks for the A and D keys being pressed,
			// Run and Stand
		if (IsKeyDown(SDLK_a) || IsKeyDown(SDLK_LEFT))
		{
			m_player.Accelerate(-300, 0);
			if (m_state != RUNNING || m_side != LEFT)
				m_player.SetAnimation("run_left", 10);
			m_state = RUNNING;
			m_side = LEFT;
		}
		else if (IsKeyDown(SDLK_d) || IsKeyDown(SDLK_RIGHT))
		{
			m_player.Accelerate(300, 0);
			if (m_state != RUNNING || m_side != RIGHT)
				m_player.SetAnimation("run_right", 10);
			m_state = RUNNING;
			m_side = RIGHT;
		}
		// and finally sets the STANDING state is no key is pressed.
		else
		{
			if (m_state == RUNNING)
				m_player.SetImage(m_side == LEFT ? "stand_left" : "stand_right");
			m_state = STANDING;
		}
	}



		

}

void CMyGame::ShotControl() 
{
	if (IsKeyDown(SDLK_f) && m_side == LEFT)
	{
		//Shoots One Mud Ball at a time
		shotList.clear();
		CSprite* pShot = new CSpriteOval(m_player.GetX(), m_player.GetY(), 10, CColor::Red(), GetTime());
		pShot->SetMotion(-500, 0);
		shotList.push_back(pShot);

	}
	else if (IsKeyDown(SDLK_f) && m_side == RIGHT)
	{
		shotList.clear();
		CSprite* pShot = new CSpriteOval(m_player.GetX(), m_player.GetY(), 10, CColor::Red(), GetTime());
		pShot->SetMotion(500, 0);
		shotList.push_back(pShot);
	}
	
	for (CSprite* pShot : shotList) 
	{
		//Shot Gravity
		pShot->Accelerate(0, -10);
		Uint32 dt = GetDeltaTime();
		CVector v = pShot->GetPos();
		CVector vel = pShot->GetVelocity() * dt / 1000;
		for (CSprite* pSprite : m_sprites)
		{
			if ((string)pSprite->GetProperty("tag") == "platform")
			{
				CVector t = pSprite->GetPos() - pShot->GetPos();
				const float r = 10;
				Y = pSprite->GetHeight() / 2;
				X = pSprite->GetWidth() / 2;

				//GetTop() Ball is coming from the top but moving down to hit the top
				if (vel.m_y < 0)
				{
					f1 = (t.m_y + Y + r) / vel.m_y;
					f2 = (t.m_x - vel.m_x * f1) / (X + r);


					if (f1 >= 0 && f1 <= 1 && f2 >= -1 && f2 <= 1)
					{
						pShot->SetVelocity(Reflect(pShot->GetVelocity(), CVector(0, 1)));
						bounceSound.Play("bounce.wav");

					}

				}

				// GetBottom() Ball comes from below but moving up to hit bottom
				if (vel.m_y > 0)
				{
					f1 = (t.m_y - Y - r) / vel.m_y;
					f2 = (t.m_x - vel.m_x * f1) / (X + r);


					if (f1 >= 0 && f1 <= 1 && f2 >= -1 && f2 <= 1)
					{
						//The Reflection
						pShot->SetVelocity(Reflect(pShot->GetVelocity(), CVector(0, 1)));
						bounceSound.Play("bounce.wav");
					}

				}

				//GetLeft() Ball goes from right and hits left
				if (vel.m_x > 0)
				{
					f1 = (t.m_x - X - r) / vel.m_x;
					f2 = (t.m_y - vel.m_y * f1) / (Y + r);


					if (f1 >= 0 && f1 <= 1 && f2 >= -1 && f2 <= 1)
					{
						pShot->SetVelocity(Reflect(pShot->GetVelocity(), CVector(1, 0)));
						bounceSound.Play("bounce.wav");
					}
				}

				//GetRight() Ball goes from left and hits right
				if (vel.m_x < 0)
				{
					f1 = (t.m_x + X + r) / vel.m_x;
					f2 = (t.m_y - vel.m_y * f1) / (Y + r);


					if (f1 >= 0 && f1 <= 1 && f2 >= -1 && f2 <= 1)
					{
						pShot->SetVelocity(Reflect(pShot->GetVelocity(), CVector(1, 0)));
						bounceSound.Play("bounce.wav");
					}
				}
			}
			if (pShot->HitTest(pSprite))
			{
				if ((string)pSprite->GetProperty("tag") == "key")
				{
					bGotKey = true;
					pSprite->Delete();
					collectSound.Play("collect.wav");
					pShot->Die(2000);
				}
			}
		}



	}
	shotList.delete_if(deleted);
	m_sprites.delete_if(deleted);
}

void CMyGame::EnemyControl() 
{
	for (CSprite* pEnemy : m_sprites)
	{
		if ((string)pEnemy->GetProperty("tag") == "enemy")
		{
			for (CSprite* pCollider : m_sprites)
			{
				//Left Collider
				if ((string)pCollider->GetProperty("tag") == "lcollider" && pEnemy->HitTest(pCollider, 0))
				{
					pEnemy->SetVelocity(100, 0);
					pEnemy->SetOmega(3.82 * 100);
				}
				//Right Collider
				else if ((string)pCollider->GetProperty("tag") == "rcollider" && pEnemy->HitTest(pCollider, 0))
				{
					pEnemy->SetVelocity(-100, 0);
					pEnemy->SetOmega(-3.82 * 100);
				}
			}

		}
	}
}

void CMyGame::CrateControl() 
{
	for (CSprite* pSprite : m_sprites) 
	{
		for (CSprite* pShot : shotList) 
		{
			if ((string)pSprite->GetProperty("tag") == "crate")
			{
				if (pSprite->HitTest(pShot) && !pSprite->IsDying()) 
				{
					pSprite->AddImage("crate.png", "destroyed", 5, 2, 0, 1, 4, 0, CColor::Blue());
					pSprite->SetAnimation("destroyed", 10);
					pSprite->Die(1000);
					cratebreakSound.Play("cratebreak.wav");
				}

			}
		}

		

	}
	m_sprites.delete_if(deleted);
}

void CMyGame::SetupLevel1()
{
	CSprite* pSprite;
	int type, x, y;

	level = 1;
	// spawn the player
	m_player.SetPos(740, 500);
	m_player.SetImage("stand_left");

	// declaring new file for reading in data
	fstream myfile;
	// opening file for reading
	myfile.open("level1.txt", ios_base::in);

	// reading while the end of file has not been reached
	bool neof; // not end of file
	do
	{
		// read in data from file line by line
		myfile >> type >> x >> y;

		neof = myfile.good();
		//cout << type << " " << x << " " << y << endl;
		if (neof)
		{
			if (type == 1)
			{
				//Normal Platform
				pSprite = new CSprite(x, y, "platform1.png", CColor::Black(), GetTime());
				pSprite->SetProperty("tag", "platform");
				m_sprites.push_back(pSprite);
			}

			if (type == 2)
			{
				//Short Platform
				pSprite = new CSprite(x, y, "shortplatform.png", CColor::Black(), GetTime());
				pSprite->SetProperty("tag", "platform");
				m_sprites.push_back(pSprite);
			}

			if (type == 3)
			{
				//Vertical Short Platform
				pSprite = new CSprite(x, y, "vshortplatform.png", CColor::Black(), GetTime());
				pSprite->SetProperty("tag", "platform");
				m_sprites.push_back(pSprite);
			}

			if (type == 4)
			{
				// Rope
				pSprite = new CSprite(x, y, "vine.png", CColor::Black(), GetTime());
				pSprite->SetProperty("tag", "rope");
				m_sprites.push_back(pSprite);
			}

			if (type == 5)
			{
				//Spikes
				pSprite = new CSprite(x, y, "spikes.png", CColor::White(), GetTime());
				pSprite->SetProperty("tag", "hazards");
				pSprite->LoadImage("spikes.png", "normal", 7, 1, 3, 0, CColor::Blue());
				pSprite->SetImage("normal");
				m_sprites.push_back(pSprite);

			}

			if (type == 6)
			{
				//Keys
				pSprite = new CSprite(621, 36, "bannana.png", CColor::White(), GetTime());
				pSprite->SetProperty("tag", "key");
				m_sprites.push_back(pSprite);
			}
			if (type == 7) 
			{
				//Gate
				pSprite = new CSprite(x, y, "treegate.png", CColor::Black(), GetTime());
				pSprite->SetProperty("tag", "gate");
				m_sprites.push_back(pSprite);
			}
		}
	} while (neof);
	myfile.close();

	// Enemies
	pSprite = new CSprite(222, 30, "skull.png", CColor::Blue(), GetTime());
	pSprite->SetProperty("tag", "enemy");
	pSprite->SetVelocity(100, 0);
	pSprite->SetOmega(3.82 * 100);
	m_sprites.push_back(pSprite);

	/*pSprite = new CSprite(740, 30, "spikes.png", CColor::White(), GetTime());
	pSprite->SetProperty("tag", "hazards");
	m_sprites.push_back(pSprite);*/



	//Colliders 
	pSprite = new CSpriteRect(-10, 400, 20, 900, CColor::Black(), CColor::White(), GetTime());
	pSprite->SetProperty("tag", "lcollider");
	m_sprites.push_back(pSprite);

	pSprite = new CSpriteRect(441, 50, 20, 900, CColor::Black(), CColor::White(), GetTime());
	pSprite->SetProperty("tag", "rcollider");
	m_sprites.push_back(pSprite);







}

void CMyGame::SetupLevel2() 
{
	CSprite* pSprite;

	int type, x, y;
	level = 2;
	// spawn the player
	m_player.SetPos(20, 50);
	m_player.SetImage("stand_right");

	// declaring new file for reading in data
	fstream myfile;
	// opening file for reading
	myfile.open("level2.txt", ios_base::in);

	// reading while the end of file has not been reached
	bool neof; // not end of file
	do
	{
		// read in data from file line by line
		myfile >> type >> x >> y;

		neof = myfile.good();
		//cout << type << " " << x << " " << y << endl;
		if (neof)
		{
			if (type == 1)
			{
				//Normal Platform
				pSprite = new CSprite(x, y, "platform1.png", CColor::Black(), GetTime());
				pSprite->SetProperty("tag", "platform");
				m_sprites.push_back(pSprite);
			}

			if (type == 2)
			{
				//Short Platform
				pSprite = new CSprite(x, y, "shortplatform.png", CColor::Black(), GetTime());
				pSprite->SetProperty("tag", "platform");
				m_sprites.push_back(pSprite);
			}

			if (type == 3)
			{
				//Vertical Short Platform
				pSprite = new CSprite(x, y, "vshortplatform.png", CColor::Black(), GetTime());
				pSprite->SetProperty("tag", "platform");
				m_sprites.push_back(pSprite);
			}

			if (type == 4)
			{
				// Vine
				pSprite = new CSprite(x, y, "vine.png", CColor::Black(), GetTime());
				pSprite->SetProperty("tag", "rope");
				m_sprites.push_back(pSprite);
			}

			if (type == 5)
			{
				//Spikes
				pSprite = new CSprite(x, y, "spikes.png", CColor::White(), GetTime());
				pSprite->SetProperty("tag", "hazards");
				pSprite->LoadImage("spikes.png", "normal", 7, 1, 3, 0, CColor::Blue());
				pSprite->SetImage("normal");
				m_sprites.push_back(pSprite);
			}

			if (type == 6)
			{
				//Keys
				pSprite = new CSprite(x, y, "bannana.png", CColor::White(), GetTime());
				pSprite->SetProperty("tag", "key");
				m_sprites.push_back(pSprite);
			}

			if (type == 7)
			{
				//Gate
				pSprite = new CSprite(x, y, "treegate.png", CColor::Black(), GetTime());
				pSprite->SetProperty("tag", "gate");
				m_sprites.push_back(pSprite);
			}

			if (type == 8) 
			{
				// Vine Short (y = 75)
				pSprite = new CSprite(x, y, "vine2.png", CColor::White(), GetTime());
				pSprite->SetProperty("tag", "rope");
				m_sprites.push_back(pSprite);
			}
			if (type == 9) 
			{
				//Very Short Platform
				pSprite = new CSprite(x, y, "reallyshortplatform.png", CColor::Black(), GetTime());
				pSprite->SetProperty("tag", "platform");
				m_sprites.push_back(pSprite);
			}

		}
	} while (neof);
	myfile.close();
	// spawn all other sprites here ...

	// Enemies
	pSprite = new CSprite(240, 250, "skull.png", CColor::Blue(), GetTime());
	pSprite->SetProperty("tag", "enemy");
	pSprite->SetVelocity(100, 0);
	pSprite->SetOmega(3.82 * 100);
	m_sprites.push_back(pSprite);


	//Colliders 
	pSprite = new CSpriteRect(230, 400, 20, 900, CColor::Black(), CColor::White(), GetTime());
	pSprite->SetProperty("tag", "lcollider");
	m_sprites.push_back(pSprite);

	pSprite = new CSpriteRect(490, 400, 20, 900, CColor::Black(), CColor::White(), GetTime());
	pSprite->SetProperty("tag", "rcollider");
	m_sprites.push_back(pSprite);



	




}

void CMyGame::SetupLevel3()
{
	CSprite* pSprite;

	int type, x, y;
	level = 3;
	// spawn the player
	m_player.SetPos(20, 50);
	m_player.SetImage("stand_right");

	// declaring new file for reading in data
	fstream myfile;
	// opening file for reading
	myfile.open("level3.txt", ios_base::in);

	// reading while the end of file has not been reached
	bool neof; // not end of file
	do
	{
		// read in data from file line by line
		myfile >> type >> x >> y;

		neof = myfile.good();
		//cout << type << " " << x << " " << y << endl;
		if (neof)
		{	
			if (type == 1)
			{
				//Normal Platform
				pSprite = new CSprite(x, y, "platform1.png", CColor::Black(), GetTime());
				pSprite->SetProperty("tag", "platform");
				m_sprites.push_back(pSprite);
			}

			if (type == 2)
			{
				//Short Platform
				pSprite = new CSprite(x, y, "shortplatform.png", CColor::Black(), GetTime());
				pSprite->SetProperty("tag", "platform");
				m_sprites.push_back(pSprite);
			}

			if (type == 3)
			{
				//Vertical Short Platform
				pSprite = new CSprite(x, y, "vshortplatform.png", CColor::Black(), GetTime());
				pSprite->SetProperty("tag", "platform");
				m_sprites.push_back(pSprite);
			}

			if (type == 4)
			{
				// Vine
				pSprite = new CSprite(x, y, "vine.png", CColor::Black(), GetTime());
				pSprite->SetProperty("tag", "rope");
				m_sprites.push_back(pSprite);
			}

			if (type == 5)
			{
				//Spikes
				pSprite = new CSprite(x, y, "spikes.png", CColor::White(), GetTime());
				pSprite->SetProperty("tag", "hazards");
				pSprite->LoadImage("spikes.png", "normal", 7, 1, 3, 0, CColor::Blue());
				pSprite->SetImage("normal");
				m_sprites.push_back(pSprite);

			}

			if (type == 6)
			{
				//Keys
				pSprite = new CSprite(x, y, "bannana.png", CColor::White(), GetTime());
				pSprite->SetProperty("tag", "key");
				m_sprites.push_back(pSprite);
			}

			if (type == 7)
			{
				//Gate
				pSprite = new CSprite(x, y, "treegate.png", CColor::Black(), GetTime());
				pSprite->SetProperty("tag", "gate");
				m_sprites.push_back(pSprite);
			}

			if (type == 8)
			{
				// Vine Short (y = 75)
				pSprite = new CSprite(x, y, "vine2.png", CColor::White(), GetTime());
				pSprite->SetProperty("tag", "rope");
				m_sprites.push_back(pSprite);
			}
			if (type == 9)
			{
				//Very Short Platform
				pSprite = new CSprite(x, y, "reallyshortplatform.png", CColor::Black(), GetTime());
				pSprite->SetProperty("tag", "platform");
				m_sprites.push_back(pSprite);
			}
			if (type == 10) 
			{
				//Crate
				pSprite = new CSprite(x, y, "crate.png", CColor::Blue(), GetTime());
				pSprite->SetProperty("tag", "crate");
				pSprite->LoadImage("crate.png", "normal", 5, 2, 0, 1, CColor::Blue());
				pSprite->SetImage("normal");
				m_sprites.push_back(pSprite);
			}
			if (type == 11) 
			{
				// Enemies
				pSprite = new CSprite(x, y, "skull.png", CColor::Blue(), GetTime());
				pSprite->SetProperty("tag", "enemy");
				pSprite->SetVelocity(100, 0);
				pSprite->SetOmega(3.82 * 100);
				m_sprites.push_back(pSprite);

			}

		}
	} while (neof);
	myfile.close();
	// spawn all other sprites here ...


	//Colliders 
	pSprite = new CSpriteRect(430, 300, 20, 150, CColor::Black(), CColor::White(), GetTime());
	pSprite->SetProperty("tag", "lcollider");
	m_sprites.push_back(pSprite);

	pSprite = new CSpriteRect(671, 300, 20, 150, CColor::Black(), CColor::White(), GetTime());
	pSprite->SetProperty("tag", "rcollider");
	m_sprites.push_back(pSprite);

	pSprite = new CSpriteRect(300, 500, 20, 150, CColor::Black(), CColor::White(), GetTime());
	pSprite->SetProperty("tag", "lcollider");
	m_sprites.push_back(pSprite);

	pSprite = new CSpriteRect(541, 500, 20, 150, CColor::Black(), CColor::White(), GetTime());
	pSprite->SetProperty("tag", "rcollider");
	m_sprites.push_back(pSprite);






}

void CMyGame::OnDraw(CGraphics* g)
{
	if (IsMenuMode())
	{
		startScreen.Draw(g);
		return;
	}

	

	background.Draw(g);
	for (CSprite* pSprite : m_sprites) 
	{
		if ((string)pSprite->GetProperty("tag") != "rcollider" && (string)pSprite->GetProperty("tag") != "lcollider" && (string)pSprite->GetProperty("tag") != "gate")
		{
			pSprite->Draw(g);
		}

		if (bGotKey == true) 
		{
			if ((string)pSprite->GetProperty("tag") == "gate") 
			{
				pSprite->Draw(g);
			}
		}
		//pSprite->Draw(g);
	}
		
	for (CSprite* pShot : shotList) pShot->Draw(g);
	m_player.Draw(g);
	



	/*for (CSprite* pSprite : m_sprites)
	{
		*g << font(28) << color(CColor::Red()) << xy(10, 570) << "Speed: " << pSprite->GetLeft();
	}*/

	/*
	for (CSprite* pShot : shotList) 
	{
		*g << font(28) << color(CColor::Red()) << xy(10, 30) << "Speed: " << pShot->GetYVelocity();
	}*/

	//* g << font(28) << color(CColor::Red()) << xy(10, 30) << "State: " << m_state;
	
	if (bGotKey) 
	{
		*g << font(28) << color(CColor::Yellow()) << xy(10, 570) << "You Have The Bannana, Go To The Tree Gate!!";
	}

	if (IsGameOverMode())
	{
		lossScreen.Draw(g);
		return;
	}

	if (level == 4) 
	{
		winScreen.Draw(g);
		return;
	}

	if (IsPaused()) 
	{
		*g << font(60) << color(CColor::Red()) << xy(300, 300) << "Paused";
	}
	
}

/////////////////////////////////////////////////////
// Game Life Cycle

// one time initialisation
void CMyGame::OnInitialize()
{
	m_player.LoadImage("diddystandr.png", "stand_right", 7, 1, 0, 0, CColor::Blue());
	m_player.LoadImage("diddystandl.png", "stand_left", 7, 1, 6, 0, CColor::Blue());
	m_player.LoadAnimation("diddyrunr.png", "run_right", CSprite::Sheet(5, 1).Row(0).From(0).To(4), CColor::Blue());
	m_player.LoadAnimation("diddyrunl.png", "run_left", CSprite::Sheet(5, 1).Row(0).From(4).To(0), CColor::Blue());
	m_player.LoadImage("diddyrunr.png", "jump_right",   5, 1, 2, 0, CColor::Blue());
	m_player.LoadImage("diddyrunl.png", "jump_left",    5, 1, 2, 0, CColor::Blue());
	m_player.LoadImage("player.png", "crouch_right", 11, 6, 2, 4, CColor::White());
	m_player.LoadImage("player.png", "crouch_left",  11, 6, 2, 5, CColor::White());
	m_player.LoadImage("diddyclimbr.png", "hang_right", 3, 1, 0, 0, CColor::Blue());
	m_player.LoadImage("diddyclimbl.png", "hang_left", 3, 1, 2, 0, CColor::Blue());
	m_player.AddImage("player.png", "climb",		 11, 6, 9, 2, 10, 2, CColor::White());



	/*m_player.LoadImage("diddyjumpr.png", "jump_right", 8, 1, 2, 0, CColor::Blue());
	m_player.LoadImage("diddyjumpl.png", "jump_left", 8, 1, 5, 0, CColor::Blue());*/
}

// called when a new game is requested (e.g. when F2 pressed)
// use this function to prepare a menu or a welcome screen
void CMyGame::OnDisplayMenu()
{
	
}

// called when a new game is started
// as a second phase after a menu or a welcome screen
void CMyGame::OnStartGame()
{

	SetLevel(level);

	if (level == 4) 
	{
		SetLevel(1);
	}
}

// called when a new level started - first call for nLevel = 1
void CMyGame::OnStartLevel(Sint16 nLevel)
{
	// Clean up first
	for (CSprite *pSprite : m_sprites)
		delete pSprite;
	m_sprites.clear();
	shotList.clear();
	bGotKey = false;

	background.LoadImage("background.bmp", "map1", CColor::Black());
	background.SetImage("map1");
	background.SetPosition(400, 300);

	CSprite *pSprite;
	pSprite = new CSprite(400, 10, "floor.png", CColor::Black(), GetTime());
//	pSprite = new CSpriteRect(400, 10, 800, 20, CColor::Black(), CColor::White(), GetTime());
	pSprite->SetProperty("tag", "platform");
	m_sprites.push_back(pSprite);

	// Left Border
	pSprite = new CSpriteRect(-10, 400, 20, 1100, CColor::Black(), CColor::White(), GetTime());
	pSprite->SetProperty("tag", "platform");
	m_sprites.push_back(pSprite);

	//Right Border
	pSprite = new CSpriteRect(810, 400, 20, 1100, CColor::Black(), CColor::White(), GetTime());
	pSprite->SetProperty("tag", "platform");
	m_sprites.push_back(pSprite);

	switch (nLevel)
	{
	case 0:
		break;
	case 1: // build Level 1 sprites
		
		SetupLevel1();

		break;

	case 2:// Level 2 

		SetupLevel2();
		
		break;
	case 3:
		SetupLevel3();
		break;
	case 4:
		level = 4;
		for (CSprite* pSprite : m_sprites)
			delete pSprite;
		m_sprites.clear();
		shotList.clear();
		break;
	}

	// any initialisation common to all levels
	m_player.SetImage("stand_right");
	m_player.SetVelocity(0, 0);
	m_state = STANDING;
	m_side = RIGHT;


}

// called when the game is over
void CMyGame::OnGameOver()
{
}

// one time termination code
void CMyGame::OnTerminate()
{
}

/////////////////////////////////////////////////////
// Keyboard Event Handlers

void CMyGame::OnKeyDown(SDLKey sym, SDLMod mod, Uint16 unicode)
{
	if (sym == SDLK_F4 && (mod & (KMOD_LALT | KMOD_RALT)))
		StopGame();
	if (sym == SDLK_SPACE)
		PauseGame();
	if (sym == SDLK_F2)
		NewGame();


	if ((sym == SDLK_w || sym == SDLK_UP) && (m_state == STANDING || m_state == RUNNING))
	{
		m_player.Accelerate(0, 800);
		jumpSound.Play("jump.wav");
		if (IsKeyDown(SDLK_a) || IsKeyDown(SDLK_LEFT)) 
		{
			m_player.Accelerate(-300, 0);
		}	
		else if (IsKeyDown(SDLK_d) || IsKeyDown(SDLK_RIGHT)) 
		{
			m_player.Accelerate(300, 0);
		}
			
		m_state = AIRBORNE;
		m_player.SetImage(m_side == LEFT ? "jump_left" : "jump_right");
	}

}

void CMyGame::OnKeyUp(SDLKey sym, SDLMod mod, Uint16 unicode)
{
}


/////////////////////////////////////////////////////
// Mouse Events Handlers

void CMyGame::OnMouseMove(Uint16 x,Uint16 y,Sint16 relx,Sint16 rely,bool bLeft,bool bRight,bool bMiddle)
{
}

void CMyGame::OnLButtonDown(Uint16 x,Uint16 y)
{
	if (IsMenuMode()) StartGame(); clickSound.Play("click.wav");
}

void CMyGame::OnLButtonUp(Uint16 x,Uint16 y)
{
}

void CMyGame::OnRButtonDown(Uint16 x,Uint16 y)
{
}

void CMyGame::OnRButtonUp(Uint16 x,Uint16 y)
{
}

void CMyGame::OnMButtonDown(Uint16 x,Uint16 y)
{
}

void CMyGame::OnMButtonUp(Uint16 x,Uint16 y)
{
}
