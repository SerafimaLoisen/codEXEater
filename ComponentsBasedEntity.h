#pragma once

#include "Entity.h"
#include "IClonable.h"
#include <string>
#include <vector>
#include <memory>

class GameEngine;
class Player;
class EntityComponent;

class ComponentsBasedEntity : public Entity, public IClonableEntity
{
private:
	std::vector<EntityComponent*> components = {};
	std::string spriteName;
	std::vector<std::string> altSprite;
	bool useAltSprite = false;

#pragma region GRAVITY
	bool isGravityEnabled = false;
	bool onGround = false;
	float verticalVelocity = 0;
	float gravityForce = 0.5f;

public:
	bool isCollidingWithPlatform(const Platform& platform);
#pragma endregion

	GameEngine* gameEngineInstance = nullptr;
	Player* player = nullptr;

private:
	void onDeath();

public:

	ComponentsBasedEntity(
		int x, int y, int w, int h, int health, bool _isGravityEnabled = false, 
		bool _useAltSprite = false, 
		std::vector<std::string> _altSprite = {"?"},
		int color = 4,
		std::string _spriteName = "enemy_default");

	~ComponentsBasedEntity();

	void render() override;
	void update() override;
	void takeDamage(int damage) override;

	ComponentsBasedEntity* clone(int _x, int _y) override;

	#pragma region SETTERS
	void SetSpriteName(std::string _spriteName);
	void SetWidth(int _width);
	void SetHeight(int _height);
	void SetAltSprite(std::vector<std::string> _altSprite);
	void SetUseAltSprite(bool _useAltSprite);
	void SetVerticalVelocity(float _verticalVelocity);
	void AddComponent(EntityComponent& component);
	#pragma endregion

	#pragma region GETTERS
	bool GetIsGravityEnabled();
	#pragma endregion
};

