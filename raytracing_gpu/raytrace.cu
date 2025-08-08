#include <tira/cuda/error.h>
#include "raytrace.h"

// CUDA device function for light calculation
__device__ glm::vec3 calculateIllumination(const light* lights, size_t lightCount, const sphere* spheres, size_t sphereCount, const hit& intersection) {
    glm::vec3 accumulatedLight(0.0f);

    for (size_t lightIndex = 0; lightIndex < lightCount; ++lightIndex) {
        ray lightRay;
        lightRay.origin = intersection.pos;
        glm::vec3 toLight = lights[lightIndex].position - intersection.pos;
        float lightDistance = glm::length(toLight);
        lightRay.direction = glm::normalize(toLight);

        // Check for occlusion
        bool isOccluded = false;
        for (size_t sphereIndex = 0; sphereIndex < sphereCount; ++sphereIndex) {
            if (&spheres[sphereIndex] == static_cast<const sphere*>(intersection.obj)) continue;
            if (spheres[sphereIndex].intersect(lightRay, lightDistance)) {
                isOccluded = true;
                break;
            }
        }

        // Accumulate light if not occluded
        if (!isOccluded) {
            float intensity = glm::dot(intersection.norm, lightRay.direction);
            if (intensity > 0) {
                accumulatedLight += intensity * lights[lightIndex].color;
            }
        }
    }

    return intersection.color * glm::clamp(accumulatedLight, 0.0f, 1.0f);
}

// CUDA kernel for tracing pixels
__global__ void traceKernel(unsigned char* outputImage, int imageWidth, int imageHeight, tira::camera camera, const sphere* spheres, size_t sphereCount, const light* lights, size_t lightCount) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= imageWidth || y >= imageHeight) return;

    float normalizedX = static_cast<float>(x) / imageWidth - 0.5f;
    float normalizedY = -static_cast<float>(y) / imageHeight + 0.5f;

    ray pixelRay;
    pixelRay.origin = camera.position();
    pixelRay.direction = camera.ray(normalizedX, normalizedY);

    hit closestIntersection;
    closestIntersection.t = 1e9f;
    bool hasHit = false;

    for (size_t sphereIndex = 0; sphereIndex < sphereCount; ++sphereIndex) {
        hit currentHit;
        if (spheres[sphereIndex].intersect(pixelRay, currentHit)) {
            hasHit = true;
            if (currentHit.t < closestIntersection.t) {
                closestIntersection = currentHit;
            }
        }
    }

    unsigned int pixelIndex = (y * imageWidth + x) * 3;
    if (hasHit) {
        glm::vec3 color = calculateIllumination(lights, lightCount, spheres, sphereCount, closestIntersection);
        outputImage[pixelIndex + 0] = static_cast<unsigned char>(color.r * 255.0f);
        outputImage[pixelIndex + 1] = static_cast<unsigned char>(color.g * 255.0f);
        outputImage[pixelIndex + 2] = static_cast<unsigned char>(color.b * 255.0f);
    } else {
        outputImage[pixelIndex + 0] = 0;
        outputImage[pixelIndex + 1] = 0;
        outputImage[pixelIndex + 2] = 0;
    }
}

// Function to manage GPU ray tracing
void gpuRayTracing(unsigned char* outputImage, int imageWidth, int imageHeight, const tira::camera& camera, const std::vector<sphere>& spheres, const std::vector<light>& lights, int cudaDeviceId) {
    size_t sphereCount = spheres.size();
    size_t lightCount = lights.size();

    cudaDeviceProp deviceProperties;
    HANDLE_ERROR(cudaGetDeviceProperties(&deviceProperties, cudaDeviceId));

    // Allocate GPU memory for image, spheres, and lights
    unsigned char* gpuImage;
    HANDLE_ERROR(cudaMalloc(&gpuImage, imageWidth * imageHeight * 3));

    sphere* gpuSpheres;
    HANDLE_ERROR(cudaMalloc(&gpuSpheres, sphereCount * sizeof(sphere)));
    HANDLE_ERROR(cudaMemcpy(gpuSpheres, spheres.data(), sphereCount * sizeof(sphere), cudaMemcpyHostToDevice));

    light* gpuLights;
    HANDLE_ERROR(cudaMalloc(&gpuLights, lightCount * sizeof(light)));
    HANDLE_ERROR(cudaMemcpy(gpuLights, lights.data(), lightCount * sizeof(light), cudaMemcpyHostToDevice));

    // Configure CUDA kernel launch
    int threadsPerBlock = deviceProperties.maxThreadsPerBlock;
    int blockSize = static_cast<int>(sqrt(threadsPerBlock));
    dim3 blockDimensions(blockSize, blockSize);
    dim3 gridDimensions((imageWidth + blockDimensions.x - 1) / blockDimensions.x, (imageHeight + blockDimensions.y - 1) / blockDimensions.y);

    // Launch kernel
    traceKernel<<<gridDimensions, blockDimensions>>>(gpuImage, imageWidth, imageHeight, camera, gpuSpheres, sphereCount, gpuLights, lightCount);
    HANDLE_ERROR(cudaDeviceSynchronize());

    // Copy results back to CPU
    HANDLE_ERROR(cudaMemcpy(outputImage, gpuImage, imageWidth * imageHeight * 3, cudaMemcpyDeviceToHost));

    // Free GPU memory
    HANDLE_ERROR(cudaFree(gpuSpheres));
    HANDLE_ERROR(cudaFree(gpuLights));
    HANDLE_ERROR(cudaFree(gpuImage));
}