#include "pch.h"
#include <sstream>
#include <fstream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "Player.h"
#include "ZombieArena.h"
#include "TextureHolder.h"
#include "Bullet.h"
#include "Pickup.h"

using namespace sf;

int main()
{
	TextureHolder holder;

	enum class State { PAUSED, LEVELING_UP, GAME_OVER, PLAYING };

	State state = State::GAME_OVER;

	// get the creen res to create SFML window
	Vector2f resolution;
	resolution.x = VideoMode::getDesktopMode().width;
	resolution.y = VideoMode::getDesktopMode().height;

	RenderWindow window(VideoMode(resolution.x, resolution.y), "Zombie Arena", Style::Fullscreen);
	View mainView(sf::FloatRect(0, 0, resolution.x, resolution.y));

	Clock clock;
	Time gameTimeTotal;
	Vector2f mouseWorldPosition;
	Vector2i mouseScreenPosition;
	Player player;
	IntRect arena;

	// Create Background and load texture
	VertexArray background;
	Texture textureBackground = TextureHolder::GetTexture("graphics/background_sheet.png");

	int numZombies;
	int numZombiesAlive;
	Zombie* zombies = nullptr;

	const int maxSimultaneusBullets = 100;
	Bullet bullets[maxSimultaneusBullets];
	int currentBullet = 0;
	int bulletsSpare = 24;
	int bulletsInClip = 6;
	int clipSize = 6;
	float fireRate = 3;
	Time lastPressed;

	// Hide mouse pointer and show crosshair
	window.setMouseCursorVisible(false);
	Sprite spriteCrosshair;
	Texture textureCrosshair = TextureHolder::GetTexture("graphics/crosshair.png");
	spriteCrosshair.setTexture(textureCrosshair);
	spriteCrosshair.setOrigin(25, 25);

	// Create pickups
	Pickup healthPickup(1);
	Pickup ammoPickup(2);

	// About the game
	int score = 0;
	int hiScore = 0;

	// Game and home screen
	Sprite spriteGameOver;
	Texture textureGameOver = TextureHolder::GetTexture("graphics/background.png");
	spriteGameOver.setTexture(textureGameOver);
	spriteGameOver.setPosition(0, 0);

	// Creeate view for HUD
	View hudView(FloatRect(0, 0, resolution.x, resolution.y));
	// Sprite for ammo icon
	Sprite spriteAmmoIcon;
	Texture textureAmmoIcon = TextureHolder::GetTexture("graphics/ammo_icon.png");
	spriteAmmoIcon.setTexture(textureAmmoIcon);
	spriteAmmoIcon.setPosition(130, 980);

	// Font
	Font font;
	font.loadFromFile("fonts/zombiecontrol.ttf");

	// Paused
	Text pausedText;
	pausedText.setFont(font);
	pausedText.setCharacterSize(85);
	pausedText.setFillColor(Color::White);
	pausedText.setPosition(350, 200);
	pausedText.setString("Press Enter \nto continue");

	// Game over
	Text gameOverText;
	gameOverText.setFont(font);
	gameOverText.setCharacterSize(80);
	gameOverText.setFillColor(Color::White);
	gameOverText.setPosition(280, 540);
	gameOverText.setString("press Enter to play");

	// Levelling up
	Text levelUpText;;
	levelUpText.setFont(font);
	levelUpText.setCharacterSize(60);
	levelUpText.setFillColor(Color::White);
	levelUpText.setPosition(80, 150);
	std::stringstream levelUpStream;
	levelUpStream <<
		"1- Increased rate of fire" <<
		"\n2- Increased clip size(next reload)" <<
		"\n3- Increased max health" <<
		"\n4- Increased run speed" <<
		"\n5- More and better health pickups" <<
		"\n6- More and better ammo pickups";
	levelUpText.setString(levelUpStream.str());

	// Ammo
	Text ammoText;
	ammoText.setFont(font);
	ammoText.setCharacterSize(55);
	ammoText.setFillColor(Color::White);
	ammoText.setPosition(200, 980);

	// Score
	Text scoreText;
	scoreText.setFont(font);
	scoreText.setCharacterSize(55);
	scoreText.setFillColor(Color::White);
	scoreText.setPosition(20, 0);

	// Load score from file
	std::ifstream inputFile("gamedata/scores.txt");
	if (inputFile.is_open()) 
	{
		inputFile >> hiScore;
		inputFile.close();
	}

	// Hi Score
	Text hiScoreText;
	hiScoreText.setFont(font);
	hiScoreText.setCharacterSize(55);
	hiScoreText.setFillColor(Color::White);
	hiScoreText.setPosition(1400, 0);
	std::stringstream s;
	s << "Hi Score:" << hiScore;
	hiScoreText.setString(s.str());

	// Zombies remaining
	Text zombiesRemainingText;
	zombiesRemainingText.setFont(font);
	zombiesRemainingText.setCharacterSize(55);
	zombiesRemainingText.setFillColor(Color::White);
	zombiesRemainingText.setPosition(1500, 980);
	zombiesRemainingText.setString("Zombies: 100");

	// Wave number
	int wave = 0;
	Text waveNumberText;
	waveNumberText.setFont(font);
	waveNumberText.setCharacterSize(55);
	waveNumberText.setFillColor(Color::White);
	waveNumberText.setPosition(1250, 980);
	waveNumberText.setString("Wave: 0");

	// Health bar
	RectangleShape healthBar;
	healthBar.setFillColor(Color::Red);
	healthBar.setPosition(450, 980);

	// When did we last update the HUD?
	int framesSinceLastHUDUpdate = 0;

	// How often (in frames) should we update the HUD
	int fpsMeasurementFrameInterval = 1000;

	// Prepare the sounds
	SoundBuffer hitBuffer;
	hitBuffer.loadFromFile("sound/hit.wav");
	Sound hit;
	hit.setBuffer(hitBuffer);

	SoundBuffer splatBuffer;
	splatBuffer.loadFromFile("sound/splat.wav");
	Sound splat;
	splat.setBuffer(splatBuffer);

	SoundBuffer shootBuffer;
	shootBuffer.loadFromFile("sound/shoot.wav");
	Sound shoot;
	shoot.setBuffer(shootBuffer);

	SoundBuffer reloadBuffer;
	reloadBuffer.loadFromFile("sound/reload.wav");
	Sound reload;
	reload.setBuffer(reloadBuffer);

	SoundBuffer reloadFailerBuffer;
	reloadFailerBuffer.loadFromFile("sound/reload_failed.wav");
	Sound reloadFailed;
	reloadFailed.setBuffer(reloadFailerBuffer);

	SoundBuffer powerupBuffer;
	powerupBuffer.loadFromFile("sound/powerup.wav");
	Sound powerup;
	powerup.setBuffer(powerupBuffer);

	SoundBuffer pickupBuffer;
	pickupBuffer.loadFromFile("sound/pickup.wav");
	Sound pickup;
	pickup.setBuffer(pickupBuffer);

	while (window.isOpen())
	{
		// INPUT

		// EVENTS
		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == Event::KeyPressed)
			{
				// Pause while playing
				if (event.key.code == Keyboard::Return && state == State::PLAYING) state = State::PAUSED;
				// Restart while paused
				else if (event.key.code == Keyboard::Return && state == State::PAUSED)
				{
					state = State::PLAYING;
					clock.restart();
				}
				// Start new game in game over state
				else if (event.key.code == Keyboard::Return && state == State::GAME_OVER)
				{
					state = State::LEVELING_UP;
					wave = 0;
					score = 0;

					// Prepare gun and ammo for next game
					currentBullet = 0;
					bulletsSpare = 36;
					bulletsInClip = 6;
					clipSize = 6;
					fireRate = 3;

					// Reset players stats
					player.resetPlayerStats();
				}

				if (state == State::PLAYING)
				{
					// Reloading
					if (event.key.code == Keyboard::R)
					{
						if (bulletsSpare >= clipSize && bulletsInClip == 0)
						{
							bulletsInClip = clipSize;
							bulletsSpare -= clipSize;
							reload.play();
						}
						else if (bulletsSpare >= clipSize && 0 < bulletsInClip < clipSize)
						{
							int numberOfReloadBullets = clipSize - bulletsInClip;
							bulletsInClip += numberOfReloadBullets;
							bulletsSpare -= numberOfReloadBullets;
							reload.play();
						}
						else if (bulletsSpare > 0)
						{
							bulletsInClip = bulletsSpare;
							bulletsSpare = 0;
							reload.play();
						}
						else
						{
							reloadFailed.play();
						}
					}
				}				
			}
		}
		// Player quitting
		if (Keyboard::isKeyPressed(Keyboard::Escape)) { window.close(); }

		// Handle controls
		if (state == State::PLAYING)
		{
			if (Keyboard::isKeyPressed(Keyboard::W)) { player.moveUp(); }
			else { player.stopUp(); }

			if (Keyboard::isKeyPressed(Keyboard::S)) { player.moveDown(); }
			else { player.stopDown(); }

			if (Keyboard::isKeyPressed(Keyboard::A)) { player.moveLeft(); }
			else { player.stopLeft(); }

			if (Keyboard::isKeyPressed(Keyboard::D)) { player.moveRight(); }
			else { player.stopRight(); }

			// Fire a bullet
			if (Mouse::isButtonPressed(Mouse::Left))
			{
				if (gameTimeTotal.asMilliseconds() - lastPressed.asMilliseconds() > 1000 / fireRate && bulletsInClip > 0)
				{
					// Pass the centre of the player and the centre of the crosshair
					bullets[currentBullet].shoot(player.getCenter().x, player.getCenter().y, mouseWorldPosition.x, mouseWorldPosition.y);
					currentBullet++;
					
					if (currentBullet >= maxSimultaneusBullets) { currentBullet = 0; }

					lastPressed = gameTimeTotal;
					shoot.play();
					bulletsInClip--;
				}
			}
		}

		// Handle levelling up state
		if (state == State::LEVELING_UP)
		{
			// Handle the player levelling up
			if (event.key.code == Keyboard::Num1)
			{
				// Increase fire rate
				fireRate++;
				state = State::PLAYING;
			}

			if (event.key.code == Keyboard::Num2)
			{
				// Increase clip size
				clipSize += clipSize;
				state = State::PLAYING;
			}

			if (event.key.code == Keyboard::Num3)
			{
				// Increase health
				player.upgradeHealth();
				state = State::PLAYING;
			}

			if (event.key.code == Keyboard::Num4)
			{
				// Increase speed
				player.upgradeSpeed();
				state = State::PLAYING;
			}

			if (event.key.code == Keyboard::Num5)
			{
				// Upgrade pickup
				healthPickup.upgrade();
				state = State::PLAYING;
			}

			if (event.key.code == Keyboard::Num6)
			{
				// Upgrade ammo
				ammoPickup.upgrade();
				state = State::PLAYING;
			}

			if (state == State::PLAYING)
			{
				wave++;
				// Prepare the level
				arena.width = 500 + 100 * wave;
				arena.height = 500 + 100 * wave;
				arena.left = 0;
				arena.top = 0;

				// Pass the vertex array to the function
				int tileSize = createBackground(background, arena);

				player.spawn(arena, resolution, tileSize);

				// Configure the pickups
				healthPickup.setArena(arena);
				ammoPickup.setArena(arena);

				// create new batch of zombies
				numZombies = 5 + 5 * wave;
				delete[] zombies;
				zombies = createHorde(numZombies, arena);
				numZombiesAlive = numZombies;

				// Play powerup sound
				powerup.play();

				// restart the clock so there is no frame jump
				clock.restart();
			}
		}

		// Update the frame
		if (state == State::PLAYING)
		{
			Time dt = clock.restart();
			gameTimeTotal += dt;
			float dtAsSeconds = dt.asSeconds();

			mouseScreenPosition = Mouse::getPosition();
			mouseWorldPosition = window.mapPixelToCoords(Mouse::getPosition(), mainView);

			// Set the crosshair to the mouse world position
			spriteCrosshair.setPosition(mouseWorldPosition);

			player.update(dtAsSeconds, Mouse::getPosition());

			Vector2f playerPosition(player.getCenter());

			mainView.setCenter(player.getCenter());

			// Loop through each zombie and update it
			for (int i = 0; i < numZombies; i++)
			{
				if (zombies[i].isAlive()) { zombies[i].update(dt.asSeconds(), playerPosition); }
			}

			// Update in-flight bullets
			for (int i = 0; i < maxSimultaneusBullets; i++)
			{
				if (bullets[i].isInFlight()) { bullets[i].update(dtAsSeconds); }
			}

			// Update the pickups
			healthPickup.update(dtAsSeconds);
			ammoPickup.update(dtAsSeconds);

			// Collision detection
			for (int i = 0; i < maxSimultaneusBullets; i++)
			{
				for (int j = 0; j < numZombies; j++)
				{
					if (bullets[i].isInFlight() && zombies[j].isAlive())
					{
						if (bullets[i].getPosition().intersects(zombies[j].getPosition()))
						{
							// stop the bullet
							bullets[i].stop();

							// Register the hit and check for kill
							if (zombies[j].hit())
							{
								score += 10;
								if (score >= hiScore) { hiScore = score; }

								numZombiesAlive -= 1;

								if (numZombiesAlive == 0) { state = State::LEVELING_UP; }
							}
							splat.play();
						}
					}
				}
			}

			// Have zombies touched the player
			for (int i = 0; i < numZombies; i++)
			{
				if (player.getPosition().intersects(zombies[i].getPosition()) && zombies[i].isAlive())
				{
					if (player.hit(gameTimeTotal))
					{
						hit.play();
					}

					if (player.getHealth() <= 0)
					{ 
						state = State::GAME_OVER;
						std::ofstream outputFile("gamedata/scores.txt");
						outputFile << hiScore;
						outputFile.close();
					}
				}
			}

			// did player touch pickups
			if (player.getPosition().intersects(healthPickup.getPosition()) && healthPickup.isSpawned())
			{
				player.increaseHealthLevel(healthPickup.gotIt());
				pickup.play();
			}
			if (player.getPosition().intersects(ammoPickup.getPosition()) && ammoPickup.isSpawned())
			{
				bulletsSpare += ammoPickup.gotIt();
				reload.play();
			}

			// size up the health bar
			healthBar.setSize(Vector2f(player.getHealth() * 3, 70));

			// increment the number of frames since the last HUD calculation
			framesSinceLastHUDUpdate++;
			// Calculate FPS
			if (framesSinceLastHUDUpdate > fpsMeasurementFrameInterval)
			{
				// Update game HUD text
				std::stringstream ssAmmo;
				std::stringstream ssScore;
				std::stringstream ssHiScore;
				std::stringstream ssWave;
				std::stringstream ssZombiesAlive;

				// Update the ammo text
				ssAmmo << bulletsInClip << "/" << bulletsSpare;
				ammoText.setString(ssAmmo.str());

				// Update the score text
				ssScore << "Score:" << score;
				scoreText.setString(ssScore.str());

				// Update the high score text
				ssHiScore << "Hi Score:" << hiScore;
				hiScoreText.setString(ssHiScore.str());

				// Update the wave
				ssWave << "Wave:" << wave;
				waveNumberText.setString(ssWave.str());

				// Update the high score text
				ssZombiesAlive << "Zombies:" << numZombiesAlive;
				zombiesRemainingText.setString(ssZombiesAlive.str());

				framesSinceLastHUDUpdate = 0;
			}
		}

		// Draw the scene
		if (state == State::PLAYING)
		{
			window.clear();
			window.setView(mainView);
			window.draw(background, &textureBackground);
			for (int i = 0; i < numZombies; i++) { window.draw(zombies[i].getSprite()); }
			for (int i = 0; i < maxSimultaneusBullets; i++)
			{
				if (bullets[i].isInFlight()) { window.draw(bullets[i].getShape()); }
			}
			window.draw(player.getSprite());
			if (ammoPickup.isSpawned()) { window.draw(ammoPickup.getSprite()); }
			if (healthPickup.isSpawned()) { window.draw(healthPickup.getSprite()); }
			window.draw(spriteCrosshair);

			// Switch to the HUD view
			window.setView(hudView);

			// Draw all the HUD elements
			window.draw(spriteAmmoIcon);
			window.draw(ammoText);
			window.draw(scoreText);
			window.draw(hiScoreText);
			window.draw(healthBar);
			window.draw(waveNumberText);
			window.draw(zombiesRemainingText);
		}

		if (state == State::LEVELING_UP)
		{
			window.draw(spriteGameOver);
			window.draw(levelUpText);
		}

		if (state == State::PAUSED)
		{
			window.draw(pausedText);
		}

		if (state == State::GAME_OVER)
		{
			window.draw(spriteGameOver);
			window.draw(gameOverText);
			window.draw(scoreText);
			window.draw(hiScoreText);
		}

		window.display();
	}
	
	delete[] zombies;
	return 0;
}

