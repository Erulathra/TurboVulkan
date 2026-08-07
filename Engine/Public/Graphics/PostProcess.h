#pragma once

namespace Turbo
{
   struct FPostProcessSettings
   {
      float mEV100 = 14.f;

      float mAgXSaturation = 1.4f;
      glm::float3 mAgXOffset = glm::float3(0.f);
      glm::float3 mAgXSlope = glm::float3(1.f);
      glm::float3 mAgXPower = glm::float3(1.35f);
   };

   struct FWorldSettings
   {
      float mAmbientLight = 2000.f;
   };
}
