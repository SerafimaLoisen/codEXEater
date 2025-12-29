#include "EmitProjectilesComponent.h"
#include "GameObject.h"
#include "Projectile.h"
#include "Bullet.h"
#include "ParryBullet.h"
#include "ComponentsBasedEntity.h"

EmitProjectilesComponent::EmitProjectilesComponent(ComponentsBasedEntity* _target,
    EmitProjectilesComponentConfig _config,
    std::vector<std::shared_ptr<Projectile>>& _projectilesArray,
    GameObject* _projectilesTarget) :
    EntityComponent(_target),
    config(_config), projectilesArray(_projectilesArray),
    projectilesTarget(_projectilesTarget)
{
    direction = _config.direction;
    projectileSpeed = _config.projectileSpeed;
    projectileMaxTravelDistance = _config.projectileMaxTravelDistance;
    numberOfEmissionsInOneSequence = _config.numberOfEmissionsInOneSequence;
    timeBetweenEmissionsInOneSequence = _config.timeBetweenEmissionsInOneSequence;
    timeBetweenSequences = _config.timeBetweenSequences;
    sequencesTimer = _config.startDelay;
    emitTowardsTarget = _config.emitTowardsTarget;
    limitDirectionToOneAxis = _config.limitDirectionToOneAxis;
    projectileTypeFactor = _config.projectileTypeFactor;
}

void EmitProjectilesComponent::Process()
{
    std::vector<float> velocity;

    if (sequencesTimer <= 0) {
        if (emissionsTimer <= 0)
        {
            float factor = 1;

            if (emitTowardsTarget && projectilesTarget)
            {
                std::vector<float> dir = { static_cast<float>((projectilesTarget->getX() + projectilesTarget->getWidth() / 2) - target->getX()), static_cast<float>((projectilesTarget->getY() + projectilesTarget->getHeight() / 2) - target->getY()) };

                float length = sqrt(dir[0] * dir[0] + dir[1] * dir[1]);

                dir[0] /= length;
                dir[1] /= length;

                velocity = { dir[0] * projectileSpeed, dir[1] * projectileSpeed };

                if (limitDirectionToOneAxis) {
                    if (abs(dir[0]) > abs(dir[1])) {
                        velocity = { (dir[0] > 0 ? static_cast<float>(projectileSpeed) : -projectileSpeed), 0 };
                    }
                    else velocity = { 0, (dir[1] > 0 ? static_cast<float>(projectileSpeed) : -projectileSpeed) };
                }
            }
            else velocity = { static_cast<float>(direction[0]) * projectileSpeed, static_cast<float>(direction[1]) * projectileSpeed };

            Emit(velocity);

            emissionsTimer = timeBetweenEmissionsInOneSequence;
            emissionsCounter++;

            if (emissionsCounter >= numberOfEmissionsInOneSequence) {
                sequencesTimer = timeBetweenSequences;
                emissionsCounter = 0;
            }

        }
        else if (emissionsTimer > 0) emissionsTimer--;
    }
    else if (sequencesTimer > 0) sequencesTimer--;
}

void EmitProjectilesComponent::Emit(std::vector<float>& velocity) {
    float rnd = (std::rand() % 100) / 100.0;
    bool regularType = rnd >= projectileTypeFactor;

    std::shared_ptr<Projectile> projectile;

    std::vector<int> coord = { target->getX() + target->getWidth() / 2, target->getY() + target->getHeight() / 2 };

    if (regularType) projectile = std::make_shared<Bullet>(coord[0], coord[1], -1);
    else projectile = std::make_shared<ParryBullet>(coord[0], coord[1], -1);

    projectile->setUseFloatCoord(true);
    projectile->setVelocity(velocity);
    projectile->setMaxTravelDistance(projectileMaxTravelDistance);
    projectile->setColor(target->getColor());

    projectilesArray.push_back(projectile);
}

EmitProjectilesComponent* EmitProjectilesComponent::clone(ComponentsBasedEntity& _target)
{
    EmitProjectilesComponent* clone = new EmitProjectilesComponent(&_target, config, projectilesArray, projectilesTarget);
    return clone;
}

EmitProjectilesComponentConfig::EmitProjectilesComponentConfig(
    std::vector<int> _direction,
    float _projectileSpeed,
    float _projectileMaxTravelDistance,
    int _numberOfEmissionsInOneSequence,
    int _timeBetweenEmissionsInOneSequence,
    int _timeBetweenSequences,
    int _startDelay,
    bool _emitTowardsTarget, bool _limitDirectionToOneAxis, float _projectileTypeFactor) :
    direction(_direction),
    projectileSpeed(_projectileSpeed),
    projectileMaxTravelDistance(_projectileMaxTravelDistance),
    numberOfEmissionsInOneSequence(_numberOfEmissionsInOneSequence),
    timeBetweenEmissionsInOneSequence(_timeBetweenEmissionsInOneSequence),
    timeBetweenSequences(_timeBetweenSequences),
    startDelay(_startDelay),
    emitTowardsTarget(_emitTowardsTarget),
    limitDirectionToOneAxis(_limitDirectionToOneAxis),
    projectileTypeFactor(_projectileTypeFactor)
{
}