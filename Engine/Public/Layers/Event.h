#pragma once

/** Must be at begin of the class */
#define EVENT_BODY(EventType)																	\
public:																						\
static entt::id_type GetStaticTypeId() { return entt::type_id<EventType>().index(); }	\
EventType() : FEventBase(EventType::GetStaticTypeId()) {}								\

namespace Turbo
{
	enum class EEventReply
	{
		Unhandled = 0,
		Handled,
	};


	struct FEventBase
	{
		const entt::id_type mEventTypeId;
		EEventReply mEventReply = EEventReply::Unhandled;

	public:
		void Handle() { mEventReply = EEventReply::Handled; }

	protected:
		explicit FEventBase(entt::id_type eventType)
			: mEventTypeId(eventType)
		{ }
	};

	struct FEventDispatcher
	{
		template <typename EventType, typename FunctionType>
		static void Dispatch(FEventBase& event, FunctionType callback)
		{
			if (event.mEventTypeId == EventType::GetStaticTypeId())
			{
				std::invoke(callback, static_cast<EventType&>(event));
			}
		}

		template <typename EventType, typename LayerType, typename FunctionType>
		static void Dispatch(FEventBase& event, LayerType* layer, FunctionType callback)
		{
			if (event.mEventTypeId == EventType::GetStaticTypeId())
			{
				std::invoke(callback, layer, static_cast<EventType&>(event));
			}
		}
	};

} // Turbo