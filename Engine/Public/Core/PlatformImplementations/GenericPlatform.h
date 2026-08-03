#pragma once

namespace Turbo
{
   struct FGenericPlatform
   {
      static void YieldCPU();
      static std::optional<std::string> GetEnviromentalVariable(std::string_view variableName);
   };
}
