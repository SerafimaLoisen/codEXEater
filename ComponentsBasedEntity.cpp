#include "GraphicsManager.h"
#include "GameEngine.h"
#include "ComponentsBasedEntity.h"
#include <iostream>
#include "EntityComponent.h"

ComponentsBasedEntity::ComponentsBasedEntity(
	int x, int y, int w, int h, int health, bool _isGravityEnabled,
	bool _useAltSprite, std::vector<std::string> _altSprite, int color,
	std::string _spriteName) :
	Entity(x, y, w, h, health, color),
	isGravityEnabled(_isGravityEnabled),
	useAltSprite(_useAltSprite),
	altSprite(_altSprite),
	spriteName(_spriteName)
{
	gameEngineInstance = GameEngine::getInstance();
	if (gameEngineInstance) {
		player = gameEngineInstance->getPlayerPtr();
	}
}

ComponentsBasedEntity::~ComponentsBasedEntity()
{
	for (int i = components.size()-1; i >= 0; i--)
		delete components[i];
}

void ComponentsBasedEntity::render()
{
	if (!isAlive()) return;
	const auto& sprite = useAltSprite ? altSprite : GraphicsManager::getGraphic(spriteName);
	GraphicsManager::renderAt(x, y, sprite, color);
}

void ComponentsBasedEntity::update() {
	
	if (!isAlive()) return;
	
	// GRAVITY
	if (isGravityEnabled) {
		verticalVelocity += gravityForce;
		y += verticalVelocity;
	}
	
	for (auto* component : components)
		component->Process();
}

void ComponentsBasedEntity::takeDamage(int damage)
{
	Entity::takeDamage(damage);
	if (health <= 0) {
		onDeath();
	}
}

void ComponentsBasedEntity::onDeath() {
	
}

ComponentsBasedEntity* ComponentsBasedEntity::clone(int _x, int _y) {
	ComponentsBasedEntity* clone = new ComponentsBasedEntity(_x, _y, width, height, health, isGravityEnabled, useAltSprite, altSprite);
	
	for (auto* component : components) {
		EntityComponent* clonedComp = component->clone(*clone);
		clone->AddComponent(*clonedComp);
	}
	
	return clone;
}

bool ComponentsBasedEntity::isCollidingWithPlatform(const Platform& platform) {
	
	// Проверяем пересечение по X
	bool xCollision = (x < platform.getX() + platform.getWidth()) &&
		(x + width > platform.getX());

	if (!xCollision) return false;

	// Проверяем столкновение сверху (падаем на платформу)
	if (verticalVelocity >= 0 &&
		y + height <= platform.getY() &&
		y + height + verticalVelocity >= platform.getY()) 
	{
		return true;
	}

	return false;
}

#pragma region SETTERS
void ComponentsBasedEntity::SetWidth(int _width) {
	width = _width;
}
void ComponentsBasedEntity::SetHeight(int _height) {
	height = _height;
}
void ComponentsBasedEntity::SetSpriteName(std::string _spriteName) {
	spriteName = _spriteName;
}

void ComponentsBasedEntity::SetVerticalVelocity(float _verticalVelocity) {
	verticalVelocity = _verticalVelocity;
}
void ComponentsBasedEntity::SetAltSprite(std::vector<std::string> _altSprite) {
	altSprite = _altSprite;
}
void ComponentsBasedEntity::SetUseAltSprite(bool _useAltSprite) {
	useAltSprite = _useAltSprite;
}

void ComponentsBasedEntity::AddComponent(EntityComponent& component) {
	components.push_back(&component);
}
#pragma endregion

#pragma region GETTERS
bool ComponentsBasedEntity::GetIsGravityEnabled() {
	return isGravityEnabled;
}
#pragma endregion
