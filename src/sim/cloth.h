#include <vector>

#include <glm/glm.hpp>

#ifndef SIM_CLOTH
#define SIM_CLOTH

namespace Sim
{
    struct ClothPoint
    {
        bool frozen;
        float mass;
        glm::vec3 v;
        glm::vec3 x;
        glm::vec3 xPrev;
        glm::vec3 x0;
    };

    // Only spheres are supported 
    struct ClothCollider {
        glm::vec3 x;
        float radius;
        float radiusEpsilon;
    };

    class Cloth
    {
    public:
        float collisionCorrectionTerm = 0.9;
        std::vector<ClothCollider*> colliders;
        float damping = 0.9;
        float springHardness = 15;
    private:
        int width = 0;
        int height = 0;

        float edgeLength = 1;
        glm::vec3 gravity = glm::vec3(0, 0, 1);
        std::vector<ClothPoint> points;

        inline void setPoint(int x, int y, ClothPoint p)
        {
            points[x * height + y] = p;
        }

        glm::vec3 computeSpringForce(
            ClothPoint p1,
            ClothPoint p2
        );

    public:
        Cloth();
        ~Cloth();

        inline int getWidth()
        {
            return width;
        }
        
        inline int getHeight()
        {
            return height;
        }

        inline ClothPoint getPoint(int x, int y)
        {
            return points[x * height + y];
        }

        inline ClothPoint* getPointPtr(int x, int y)
        {
            return &points[x * height + y];
        }

        inline void setGravity(glm::vec3 gravity)
        {
            this->gravity = gravity;
        }

        inline glm::vec3 getGravity() {
            return this->gravity;
        }

        void init(int width, int height, float edgeLength);
        void step(float dt);
    };
}

#endif