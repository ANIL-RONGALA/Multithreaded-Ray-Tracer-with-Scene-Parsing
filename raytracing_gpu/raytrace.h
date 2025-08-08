#pragma once

#include <glm/glm.hpp>
#include <tira/graphics/camera.h>

// Ray structure
struct Ray {
    glm::vec3 origin;       // Ray origin
    glm::vec3 direction;    // Ray direction
    unsigned int bounce = 0; // Bounce order for reflection/refraction
};

// Forward declaration
class Sphere;

// Hit structure
struct Hit {
    Sphere* object = nullptr;   // Pointer to the hit object
    glm::vec3 position;         // Position of intersection
    Ray ray;                    // Ray causing the hit
    float distance = 0.0f;      // Distance to the intersection
    glm::vec3 normal;           // Surface normal at the hit point
    glm::vec3 color;            // Color at the hit point
};

// Base class for all geometric objects
class Object {
public:
    virtual ~Object() = default;

    CUDA_CALLABLE virtual bool intersect(const Ray& ray, Hit& hitData) const { return false; }
    CUDA_CALLABLE virtual bool intersect(const Ray& ray, float maxDistance) const { return false; }
};

// Sphere class derived from Object
class Sphere : public Object {
public:
    float radius;              // Radius of the sphere
    glm::vec3 center;          // Center position of the sphere
    glm::vec3 surfaceColor;    // Color of the sphere surface

    // Intersection test with detailed hit information
    CUDA_CALLABLE bool intersect(const Ray& ray, Hit& hitData) const override {
        glm::vec3 oc = ray.origin - center;
        float b = glm::dot(oc, ray.direction);
        glm::vec3 closestPoint = oc - b * ray.direction;
        float discriminant = radius * radius - glm::dot(closestPoint, closestPoint);

        if (discriminant < 0) return false;

        float sqrtDisc = std::sqrt(discriminant);
        float t0 = -b - sqrtDisc;
        float t1 = -b + sqrtDisc;

        if (t0 < 0 || t1 < 0) return false;

        hitData.position = ray.origin + t0 * ray.direction;
        hitData.color = surfaceColor;
        hitData.object = const_cast<Sphere*>(this);
        hitData.normal = glm::normalize(hitData.position - center);
        hitData.distance = t0;
        return true;
    }

    // Intersection test for visibility checks
    CUDA_CALLABLE bool intersect(const Ray& ray, float maxDistance) const override {
        glm::vec3 oc = ray.origin - center;
        float b = glm::dot(oc, ray.direction);
        glm::vec3 closestPoint = oc - b * ray.direction;
        float discriminant = radius * radius - glm::dot(closestPoint, closestPoint);

        if (discriminant < 0) return false;

        float sqrtDisc = std::sqrt(discriminant);
        float t0 = -b - sqrtDisc;
        float t1 = -b + sqrtDisc;

        return (t0 >= 0 && t0 <= maxDistance) || (t1 >= 0 && t1 <= maxDistance);
    }

    // Intersection test using an alternative Pythagorean method
    bool intersectPythagorean(const Ray& ray, Hit& hitData) const {
        glm::vec3 oc = ray.origin - center;
        float a = glm::dot(ray.direction, ray.direction);
        float b = 2.0f * glm::dot(ray.direction, oc);
        float c = glm::dot(oc, oc) - radius * radius;

        float discriminant = b * b - 4 * a * c;
        if (discriminant < 0) return false;

        float sqrtDisc = std::sqrt(discriminant);
        float t0 = (-b - sqrtDisc) / (2.0f * a);
        float t1 = (-b + sqrtDisc) / (2.0f * a);

        if (t0 <= 0 && t1 <= 0) return false;

        float t = t0 > 0 ? t0 : t1;
        hitData.position = ray.origin + t * ray.direction;
        hitData.color = surfaceColor;
        hitData.object = const_cast<Sphere*>(this);
        hitData.normal = glm::normalize(hitData.position - center);
        hitData.distance = t;
        return true;
    }
};

// Light structure
struct Light {
    glm::vec3 position;   // Position of the light source
    glm::vec3 intensity;  // Color intensity of the light
};