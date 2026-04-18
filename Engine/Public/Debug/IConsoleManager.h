#pragma once
#include "Core/Delegate.h"
#include "Core/DataStructures/StringLookUp.h"
#include "Layers/Event.h"

namespace Turbo
{
    class FConsoleManager;
    using IConsoleManager = FConsoleManager;
    using FArgsVector = std::span<const std::string_view>;

    DECLARE_DELEGATE(FConsoleCommandDelegate, FConsoleManager& /* consoleManager */, const FArgsVector /* args */);

    struct FConsoleBufferChangedEvent : FEventBase
    {
        EVENT_BODY(FConsoleBufferChangedEvent)

        std::string_view mMessage;
    };

    struct FConsoleCommand
    {
        std::string mName;
        std::string mDescription;
        FConsoleCommandDelegate mDelegate;
    };

    struct FAutoConsoleCommand
    {
        FAutoConsoleCommand(std::string_view name, std::string_view description, FConsoleCommandDelegate delegate);
    };

    enum class EConsoleVariableType : uint8
    {
        Bool,
        Int32,
        Float,
    };

    struct FConsoleVariable
    {
        std::string mName;
        std::string mDescription;

        EConsoleVariableType mType;
        void* mDataPtr;

    public:
        [[nodiscard]] bool Parse(const std::string_view arg) const;

        void Set(bool bValue) const;
        void Set(int32 value) const;
        void Set(float value) const;

        [[nodiscard]] bool GetBool() const;
        [[nodiscard]] int32 GetInt() const;
        [[nodiscard]] float GetFloat() const;

        [[nodiscard]] std::string ValueToString() const;
    };

    template<typename T>
    struct TAutoConsoleVariable
    {
        T mVariableData;

        TAutoConsoleVariable() = delete;
        TAutoConsoleVariable(const std::string_view name, T defaultValue, const std::string_view description);

        void Set(T value) { mVariableData = value; }
        T Get() const { return mVariableData; }
    };

    template<typename T>
    static EConsoleVariableType GetConsoleVariableType() = delete;

    template<>
    inline EConsoleVariableType GetConsoleVariableType<bool>() { return EConsoleVariableType::Bool; }

    template<>
    inline EConsoleVariableType GetConsoleVariableType<int32>() { return EConsoleVariableType::Int32; }

    template<>
    inline EConsoleVariableType GetConsoleVariableType<float>() { return EConsoleVariableType::Float; }

    class FConsoleManager
    {
    public:
        [[nodiscard]] static FConsoleManager& GetSafe();
        [[nodiscard]] static FConsoleManager& Get();

        bool RegisterCommand(const FConsoleCommand& consoleCommand);
        bool UnregisterCommand(const FConsoleCommand& consoleCommand);
        bool UnregisterCommand(std::string_view consoleCommandName);

        bool RegisterConsoleVariable(const FConsoleVariable& consoleVariable);
        bool UnregisterConsoleVariable(const FConsoleVariable& consoleVariable);
        bool UnregisterConsoleVariable(std::string_view consoleVariableName);

        [[nodiscard]] FConsoleCommand* FindConsoleCommand(std::string_view name);
        [[nodiscard]] FConsoleVariable* FindConsoleVariable(std::string_view consoleVariableName);

        template <typename... Args>
        void Printf(fmt::format_string<Args...> fmt, Args&&... args)
        {
            Print(fmt::format(fmt, std::forward<Args>(args)...));
        }
        void Print(std::string_view message);

        void Parse(std::string_view input);
        std::vector<std::string_view> FindAutoCompleteCandidates(std::string_view command);

    private:
        TStringLookUp<FConsoleCommand> mCommands;
        TStringLookUp<FConsoleVariable> mVariables;
    };


    template <typename T>
    TAutoConsoleVariable<T>::TAutoConsoleVariable(const std::string_view name, T defaultValue, const std::string_view description): mVariableData(defaultValue)
    {
        const bool bResult = IConsoleManager::GetSafe().RegisterConsoleVariable({
            .mName = std::string(name),
            .mDescription = std::string(description),
            .mType = GetConsoleVariableType<T>(),
            .mDataPtr = &mVariableData
        });

        TURBO_CHECK_MSG(bResult, "There could be only one console variable with {} name", name)
    }
}
