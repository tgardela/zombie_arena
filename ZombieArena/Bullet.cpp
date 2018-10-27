#include "pch.h"
#include "Bullet.h"


Bullet::Bullet()
{
	m_BulletShape.setSize(Vector2f(2, 2));
}

void Bullet::shoot(float startX, float startY, float targetX, float targetY)
{
	m_InFlight = true;
	m_Position.x = startX;
	m_Position.y = startY;

	// Gradient of flight path
	float gradient = (startX - targetX) / (startY - targetY);

	if (gradient < 0) { gradient *= -1; }

	// Ratio between X and Y
	float ratioXY = m_BulletSpeed / (1 + gradient);

	// Set thevertical and horizontal speed
	m_BulletDistanceY = ratioXY;
	m_BulletDistanceX = ratioXY * gradient;

	// Point the bullet
	if (targetX < startX) { m_BulletDistanceX *= -1;  }
	if (targetY < startY) { m_BulletDistanceY *= -1;  }

	// Set range
	float range = 1000;
	m_MinX = startX - range;
	m_MaxX = startX + range;
	m_MinY = startY - range;
	m_MaxY = startY + range;

	m_BulletShape.setPosition(m_Position);
}

void Bullet::stop() { m_InFlight = false; }

bool Bullet::isInFlight() { return m_InFlight;  }

FloatRect Bullet::getPosition() { return m_BulletShape.getGlobalBounds(); }

RectangleShape Bullet::getShape() { return m_BulletShape; }

void Bullet::update(float elapsedTime)
{
	// Update position of the bullet
	m_Position.x += m_BulletDistanceX * elapsedTime;
	m_Position.y += m_BulletDistanceY * elapsedTime;

	// Move the bullet
	m_BulletShape.setPosition(m_Position);

	// Check the range
	if (m_Position.x < m_MinX || m_Position.x > m_MaxX || m_Position.y < m_MinY || m_Position.y > m_MaxY) { m_InFlight = false; }
}