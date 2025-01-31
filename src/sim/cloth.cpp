#include "cloth.h"
#include <GTE/Mathematics/OdeRungeKutta4.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

using namespace Sim;

Cloth::Cloth() {}
Cloth::~Cloth() {}

void Cloth::init(int width, int height, float edgeLength)
{
    ClothPoint p0;
    p0.v = glm::vec3(0);
    p0.x = glm::vec3(0);
    p0.x0 = glm::vec3(0);
    // This is not how Verlet integration x1 term should be initialized from x0, but it's ok
    // since we can pretend that at time t=0 no force is applied to cloth.
    //
    // That is, because cloth is at rest at t=0, meaning all forces and velocities can be 0 for any, but finite
    // amount of time.
    //
    // So: x_1 = x_0 + v_0 * dt + f(x_0) * dt*dt
    // But x_0 = some constant, v_0 = 0 and f(x_0) = 0 so x_1 = x_0 for any dt
    //
    // In this simulation there is force at t=0, that is gravity, but since things won't move we can pretend
    // like gravity starts working at t=0+dt
    p0.xPrev = glm::vec3(0);
    p0.mass = 1;
    p0.frozen = false;

    this->width = width;
    this->height = height;
    this->edgeLength = edgeLength;

    points.clear();
    points.resize(width * height);

    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            ClothPoint p = p0;

            p.x = glm::vec3(x * edgeLength, 0, y * edgeLength);
            p.x0 = p.x;
            setPoint(x, y, p);
        }
    }
}

glm::vec3 Cloth::computeSpringForce(
    ClothPoint p1,
    ClothPoint p2)
{
    auto zeroDistance = glm::length(p1.x0 - p2.x0);

    auto displacement = p1.x - p2.x;
    auto distance = glm::length(displacement);
    auto displacementNorm = glm::normalize(displacement);

    auto vRelativeProj = glm::proj(p1.v - p2.v, displacement);
    auto vRelativeScalar = glm::length(vRelativeProj);

    auto forceScalar = -springHardness * (distance - zeroDistance);
    auto dampingScalar = vRelativeScalar * damping;
    return displacementNorm * (forceScalar - dampingScalar) + gravity;
}

void Cloth::step(float dt)
{
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            if (getPoint(x, y).frozen)
                continue;

            auto totalForce = glm::vec3(0);

            for (int xo : {1, 0, -1})
            {
                for (int yo : {1, 0, -1})
                {
                    if (xo == 0 && yo == 0)
                        continue;

                    const int xx = x + xo;
                    const int yy = y + yo;

                    if (xx < 0 || xx >= width || yy < 0 || yy >= height)
                    {
                        continue;
                    }

                    auto otherPoint = getPoint(xx, yy);
                    totalForce += computeSpringForce(getPoint(x, y), getPoint(xx, yy));
                }
            }

            totalForce += gravity;

            // go from force to acceleration
            totalForce = totalForce / getPoint(x, y).mass;

            auto p = getPointPtr(x, y);

            for (auto colliderPtr : colliders)
            {
                const auto collider = *colliderPtr;
                const auto displacementVec = getPoint(x, y).x - collider.x;
                const auto distance = glm::length(displacementVec);

                if (distance < collider.radius + collider.radiusEpsilon)
                {
                    // For spheres collision normal is just this vector normalized
                    // For any other figure, the collision normal would be:
                    // 1. For point-face collision: normal of that face
                    // 2. For point-point collision: same algorithm as in sphere
                    //    - just use displacement vector of these two points
                    // Check for collision is completely different story though, and is not that easy to implement

                    const auto penetration = (collider.radius + collider.radiusEpsilon) - distance;
                    const auto displacementNormal = glm::normalize(displacementVec);

                    // See Baumgarte stabilization for controlling violation of constraints
                    const auto displacementCorrection = displacementNormal *
                                                        penetration *
                                                        1;

                    // Hack: Invalidate verlet as if this point was stationary
                    p->x += displacementCorrection;
                    p->xPrev = p->x;


                    totalForce += glm::normalize(totalForce) * glm::dot(
                        totalForce,
                        displacementNormal
                    );
                    // totalForce = glm::vec3(0);
                }
            }

            p->xPrev = p->x;
            p->x += p->x - p->xPrev + totalForce * dt * dt;

            // TODO(teawithsand): collision checking here
            // compute collision normal and apply correcting force if there is a collision
        }
    }
}