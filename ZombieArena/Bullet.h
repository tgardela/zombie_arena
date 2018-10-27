#pragma once
#include <SFML/Graphics.hpp>

using namespace sf;

class Bullet
{
private:
	Vector2f m_Position;
	RectangleShape m_BulletShape;
	bool m_InFlight = false;
	float m_BulletSpeed = 1000;

	// What fraction of 1px does the bullet travel
	float m_BulletDistanceX;
	float m_BulletDistanceY;

	// Boundaries
	float m_MaxX;
	float m_MinX;
	float m_MaxY;
	float m_MinY;

public:
	Bullet();
	void stop();
	bool isInFlight();
	void shoot(float startX, float startY, float xTarget, float yTarget);
	FloatRect getPosition();
	RectangleShape getShape();
	void update(float elapsedTime);
};

