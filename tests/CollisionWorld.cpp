// (C) king.com Ltd 2021
#include "CollisionWorld.h"

#include "glm/glm.hpp"
namespace King {
namespace {

bool RaycastCircle(glm::vec2 origin, glm::vec2 direction, glm::vec2 center, const float radius, glm::vec2& point, glm::vec2& normal) {
    assert(radius > 0);
    if (glm::pow(origin.x - center.x, 2.0f) + glm::pow(origin.y - center.y, 2.0f) < radius * radius) {
        return false;
    }

    glm::vec2 CO = origin - center;
    float a = direction.x * direction.x + direction.y * direction.y;
    float b = 2.0f * (direction.x * CO.x + direction.y * CO.y);
    float c = (CO.x * CO.x + CO.y * CO.y) - (radius * radius);

    float squareRootTerm = b * b - 4.0f * a * c;
    if (squareRootTerm < 0) {
        return false;
    }

    float lambda1 = (-b + glm::sqrt(squareRootTerm)) / (2.0f * a);
    float lambda2 = (-b - glm::sqrt(squareRootTerm)) / (2.0f * a);

    if (lambda1 > 0 && lambda2 > 0) {
        float minLambda = glm::min(lambda1, lambda2);
        point = origin + direction * minLambda;
        normal = glm::normalize(point - center);
        assert(glm::dot(normal, direction) < 0);
        return true;
    }

    if (lambda1 > 0) {
        point = origin + direction * lambda1;
        normal = glm::normalize(point - center);
        assert(glm::dot(normal, direction) < 0);
        return true;
    }

    if (lambda2 > 0) {
        point = origin + direction * lambda2;
        normal = glm::normalize(point - center);
        assert(glm::dot(normal, direction) < 0);
        return true;
    }

    return false;
}

bool RaycastRectWithRadius(glm::vec2 origin, glm::vec2 direction, BoxCollider box, const float radius, glm::vec2& point, glm::vec2& normal) {
    if (origin.x > box.mTopLeft.x - radius && origin.x < box.mBottomRight.x + radius && origin.y < box.mBottomRight.y + radius) {
        // we're inside the bounding box.
        // if we're inside one of the rounded corners check collision with the corner circle
        if ((origin.x < box.mTopLeft.x || origin.x > box.mBottomRight.x) && (origin.y < box.mTopLeft.y || origin.y > box.mBottomRight.y)) {
            glm::vec2 closestCorner = { glm::clamp(origin.x, box.mTopLeft.x, box.mBottomRight.x), glm::clamp(origin.y, box.mTopLeft.y, box.mBottomRight.y) };

            return RaycastCircle(origin, direction, closestCorner, radius, point, normal);
        }
    }

    float edgeDistances[4] = {-1, -1, -1, -1}; // left, right, top, bottom
    glm::vec2 normals[4] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };

    if (direction.x != 0) {
        edgeDistances[0] = ((box.mTopLeft.x - radius) - origin.x) / direction.x;
        edgeDistances[1] = ((box.mBottomRight.x + radius) - origin.x) / direction.x;
    }
    if (direction.y != 0) {
        edgeDistances[2] = ((box.mTopLeft.y - radius) - origin.y) / direction.y;
        edgeDistances[3] = ((box.mBottomRight.y + radius) - origin.y) / direction.y;
    }

    bool hasCollision = false;
    int collisionSide = 0;
    float nearestD = -1;
    for (int i = 0; i < 4; ++i) {
        if (edgeDistances[i] > 0 && (edgeDistances[i] < nearestD || !hasCollision)) {
            hasCollision = true;
            nearestD = edgeDistances[i];
            collisionSide = i;
        }
    }

    if (!hasCollision) {
        return false;
    }

    glm::vec2 tmpPoint = origin + direction * nearestD;

    if ((tmpPoint.x > box.mTopLeft.x && tmpPoint.x < box.mBottomRight.x) || (tmpPoint.y > box.mTopLeft.y && tmpPoint.y < box.mBottomRight.y)) {
        point = tmpPoint;
        normal = normals[collisionSide];
        return true;
    }

    // We have to check against the rounded corner
    glm::vec2 closestCorner = { glm::clamp(tmpPoint.x, box.mTopLeft.x, box.mBottomRight.x), glm::clamp(tmpPoint.y, box.mTopLeft.y, box.mBottomRight.y) };

    if (RaycastCircle(origin, direction, closestCorner, radius, point, normal)) {
        return true;
    }

    return false;
}

glm::vec2 ReflectVector(const glm::vec2& in, const glm::vec2& normal) {
    glm::vec2 unitNormal = glm::normalize(normal);
    glm::vec2 reflection = in - 2.0f * glm::dot(in, unitNormal) * unitNormal;

    return reflection;
}

}

CollisionWorld::CollisionWorld() {
}

int CollisionWorld::AddBoxCollider(glm::vec2 topLeft, glm::vec2 size, unsigned int layer, unsigned int mask) {
    mBoxColliders.emplace_back(std::make_pair(mNextColliderId, BoxCollider(topLeft, size)));
    mAllColliders.push_back(std::make_pair(mNextColliderId, ColliderInfo{ColliderType::Box, layer, mask}));
    return mNextColliderId++;
}

int CollisionWorld::AddCircleCollider(glm::vec2 position, float radius, unsigned int layer, unsigned int mask) {
    mCircleCollider.emplace_back(std::make_pair(mNextColliderId, CircleCollider(position, radius)));
    mAllColliders.push_back(std::make_pair(mNextColliderId, ColliderInfo{ColliderType::Circle, layer, mask}));
    return mNextColliderId++;
}

RigidBody* CollisionWorld::AddRigidBody(glm::vec2 movement, std::vector<int> colliders) {
    auto rigidBody = std::make_unique<RigidBody>(movement, std::move(colliders));
    auto rigidBodyPtr = rigidBody.get();
    mRigidBodies.emplace_back(std::move(rigidBody));
    return rigidBodyPtr;
}

bool CollisionWorld::CircleCast(const glm::vec2& origin, const glm::vec2& direction, float radius, float maxDistance, CircleCastResult& result, unsigned int mask) {
    bool hasCollision = false;
    CircleCastResult tmpResult;
    tmpResult.mDistance = -1;

    glm::vec2 point;
    glm::vec2 normal;

    for (const auto& collider : mAllColliders) {
        if(!(collider.second.mLayer & mask)) {
            continue;
        }

        if(collider.second.type == ColliderType::Box) {
            const auto boxCollider = GetBoxCollider(collider.first);
            assert(boxCollider);

            if (!RaycastRectWithRadius(origin, glm::normalize(direction), *boxCollider, radius, point, normal)) {
                continue;
            }

            float distance = glm::length(point - origin);
            if (distance <= maxDistance && (!hasCollision || distance < tmpResult.mDistance)) {
                tmpResult.mDistance = distance;
                tmpResult.mNormal = normal;
                tmpResult.mPoint = point;
                tmpResult.mId = collider.first;
                hasCollision = true;
            }
        }
    }

    if (hasCollision) {
        result = tmpResult;
    }

    return hasCollision;
}

BoxCollider* CollisionWorld::GetBoxCollider(int id) {
    for(auto& collider : mBoxColliders) {
        if(collider.first == id) {
            return &collider.second;
        }
    }

    return nullptr;
}

CircleCollider* CollisionWorld::GetCircleCollider(int id) {
    for(auto& collider : mCircleCollider) {
        if(collider.first == id) {
            return &collider.second;
        }
    }

    return nullptr;
}

void CollisionWorld::ProcessCollisions(float dt) {
    auto findColliderInfo = [&](int id) -> const ColliderInfo&{
        for(const auto& collider : mAllColliders) {
            if(collider.first == id) {
                return collider.second;
            }
        }
        assert(!"Collider integrity broken");
        static ColliderInfo ci;
        return ci;
    };

    // Rigid body updates
    for (const auto& rigidbody: mRigidBodies) {
        float distanceToMove = glm::length(rigidbody->mMovement) * dt;

        if(rigidbody->mIsStatic) {
            continue;
        }

        // Process sphere colliders movement
        King::CollisionWorld::CircleCastResult result;
        for (const auto& collider : rigidbody->mColliders) {
            const auto& colliderInfo = findColliderInfo(collider);

            if(colliderInfo.type == ColliderType::Circle) {
                const auto sphereCollider = GetCircleCollider(collider);
                if(!sphereCollider) {
                    continue;
                }

                auto direction = glm::normalize(rigidbody->mMovement);
                auto movement = glm::length(rigidbody->mMovement);

                while (distanceToMove > 0 && CircleCast(sphereCollider->mPosition,
                                                        direction,
                                                        sphereCollider->mRadius,
                                                        distanceToMove,
                                                        result,
                                                        colliderInfo.mMask)) {
                    distanceToMove -= result.mDistance;
                    sphereCollider->mPosition = result.mPoint;
                    direction = ReflectVector(direction, result.mNormal);
                    rigidbody->mMovement = direction * movement;
                    NotifyCollission(result.mId, collider);

                }

                if (distanceToMove > 0) {
                    sphereCollider->mPosition += direction * distanceToMove;
                }
            }
        }
    }

    // Box to box collisions
    auto b2b = [](const BoxCollider& a, const BoxCollider& b) -> bool {
        if(a.mTopLeft.x >= b.mBottomRight.x || a.mBottomRight.x <= b.mTopLeft.x) {
            return false;
        }

        if(a.mTopLeft.y >= b.mBottomRight.y || a.mBottomRight.y <= b.mTopLeft.y) {
            return false;
        }
        return true;
    };

    // Circle to box collisions
    auto c2b = [](const CircleCollider& a, const BoxCollider& b) -> bool {
        if(glm::length(b.mTopLeft - a.mPosition) < a.mRadius) {
            return true;
        }
        else if(glm::length(b.mBottomRight - a.mPosition) < a.mRadius) {
            return true;
        }
        else if(glm::length(glm::vec2{b.mBottomRight.x, b.mTopLeft.y} - a.mPosition) < a.mRadius) {
            return true;
        }
        else if(glm::length(glm::vec2{b.mTopLeft.x, b.mBottomRight.y} - a.mPosition) < a.mRadius) {
            return true;
        }

        return false;
    };

    // Check all colliders
    std::vector<std::pair<int, int>> collisions;
    for (auto c1 = mAllColliders.begin(); c1 != mAllColliders.end(); c1++) {
        for(auto c2 = mAllColliders.begin(); c2 != mAllColliders.end(); c2++) {
            if(!((*c1).second.mMask | (*c2).second.mLayer || (*c2).second.mMask | (*c1).second.mLayer)) {
                continue;
            }

            // Box to box
            if((*c1).second.type == ColliderType::Box && (*c2).second.type == ColliderType::Box) {
                const auto collider1 = GetBoxCollider((*c1).first);
                const auto collider2 = GetBoxCollider((*c2).first);
                assert(collider1);
                assert(collider2);

                if(b2b(*collider1, *collider2)) {
                    collisions.push_back(std::make_pair((*c1).first, (*c2).first));
                }
            }
            // Circle to box
            if((*c1).second.type == ColliderType::Circle && (*c2).second.type == ColliderType::Box) {
                const auto collider1 = GetCircleCollider((*c1).first);
                const auto collider2 = GetBoxCollider((*c2).first);
                assert(collider1);
                assert(collider2);

                if(c2b(*collider1, *collider2)) {
                    collisions.push_back(std::make_pair((*c1).first, (*c2).first));
                }
            }
            //Box to circle
            if((*c1).second.type == ColliderType::Box && (*c2).second.type == ColliderType::Circle) {
                const auto collider1 = GetBoxCollider((*c1).first);
                const auto collider2 = GetCircleCollider((*c2).first);
                assert(collider1);
                assert(collider2);

                if(c2b(*collider2, *collider1)) {
                    collisions.push_back(std::make_pair((*c2).first, (*c1).first));
                }
            }
        }
    }

    for (const auto& collision : collisions) {
        NotifyCollission(collision.first, collision.second);
    }
}

void CollisionWorld::RemoveBoxCollider(int boxId) {
    mBoxColliders.erase(std::remove_if(mBoxColliders.begin(), mBoxColliders.end(), [&](const auto& bc) {return bc.first == boxId;}), mBoxColliders.end());
    mAllColliders.erase(std::remove_if(mAllColliders.begin(), mAllColliders.end(), [&](const auto& c) {return c.first == boxId;}),
                        mAllColliders.end());
}

void CollisionWorld::RemoveCircleCollider(int colliderId) {
    mCircleCollider.erase(std::remove_if(mCircleCollider.begin(), mCircleCollider.end(), [&](const auto& cc) {return cc.first == colliderId;}), mCircleCollider.end()) ;
    mAllColliders.erase(std::remove_if(mAllColliders.begin(), mAllColliders.end(), [&](const auto& c) {return c.first == colliderId;}),
                        mAllColliders.end());
}

void CollisionWorld::RemoveRigidBody(RigidBody* rigidbody) {
    mRigidBodies.erase(std::remove_if(mRigidBodies.begin(), mRigidBodies.end(), [&](const auto& rb) {return rb.get() == rigidbody;}),
                       mRigidBodies.end());
}

void CollisionWorld::ClearAll() {
    mBoxColliders.clear();
    mAllColliders.clear();
}

void CollisionWorld::NotifyCollission(int l, int r) {
    for (auto listener : mListeners) {
        listener->OnCollision(l, r);
    }
}

void CollisionWorld::AddCollisionListener(CollisionListener& listener) {
    mListeners.push_back(&listener);
}

void CollisionWorld::RemoveCollisionListener(CollisionListener& listener) {
    mListeners.erase(std::remove_if(mListeners.begin(), mListeners.end(), [&listener](CollisionListener* l) {
                         return &listener == l;
                     }
                                    ), mListeners.end());
}

void CollisionWorld::ClearCollisionListeners() {
    mListeners.clear();
}
}
