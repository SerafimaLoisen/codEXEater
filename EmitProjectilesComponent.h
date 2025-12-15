#pragma once
#include "EntityComponent.h"
#include "ComponentConfig.h"
#include <vector>
#include <memory>

class GameObject;
class Projectile;

struct EmitProjectilesComponentConfig : ComponentConfig {
	EmitProjectilesComponentConfig(
		std::vector<int> _direction = { -1, 0 },
		float _projectileSpeed = 1,
		int _numberOfEmissionsInOneSequence = 4,
		int _timeBetweenEmissionsInOneSequence = 2,
		int _timeBetweenSequences = 4,
		bool _emitTowardsTarget = false,
		bool _limitDirectionToOneAxis = true,
		float _projectileTypeFactor = 0);

	std::vector<int> direction = { -1, 0 };
	float projectileSpeed;
	int numberOfEmissionsInOneSequence = 4;
	int timeBetweenEmissionsInOneSequence = 2;
	int timeBetweenSequences = 4;
	bool emitTowardsTarget = false;
	bool limitDirectionToOneAxis = true;
	float projectileTypeFactor = 0;
};

class EmitProjectilesComponent : public EntityComponent
{
public:
	EmitProjectilesComponent(Entity* _target, EmitProjectilesComponentConfig _config, std::vector<std::shared_ptr<Projectile>>& _projectilesArray,  GameObject* _projectilesTarget=nullptr);
	void Process() override;
	EmitProjectilesComponent* clone(Entity& _target) override;

private:
	std::vector<int> direction = { -1, 0 };
	float projectileSpeed;
	int numberOfEmissionsInOneSequence = 4;
	int timeBetweenEmissionsInOneSequence = 2;
	int timeBetweenSequences = 4;
	bool emitTowardsTarget = false;
	bool limitDirectionToOneAxis = true;
	GameObject* projectilesTarget = nullptr;
	std::vector<std::shared_ptr<Projectile>>& projectilesArray;
	float projectileTypeFactor = 0;

	int emissionsTimer = 0;
	int sequencesTimer = 0;
	int emissionsCounter = 0;

	EmitProjectilesComponentConfig config;

	void Emit(std::vector<float>& velocity);
};

