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
		template <typename EventType, typename FunctionType, typename ... Args>
		static void Dispatch(FEventBase& event, FunctionType callback, Args&& ... args)
		{
			if (event.mEventTypeId == EventType::GetStaticTypeId())
			{
				std::invoke(
					std::forward<FunctionType>(callback),
					static_cast<EventType&>(event),
					std::forward<Args>(args)...
				);
			}
		}

		template <typename EventType, typename LayerType, typename FunctionType, typename ... Args>
		static void DispatchLayer(FEventBase& event, LayerType* layer, FunctionType callback, Args&& ... args)
		{
			if (event.mEventTypeId == EventType::GetStaticTypeId())
			{
				std::invoke(
					std::forward<FunctionType>(callback),
					layer,
					static_cast<EventType&>(event),
					std::forward<Args>(args)...
				);
			}
		}
	};

} // Turbo