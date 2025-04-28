#pragma once

namespace RE
{
	namespace Offset
	{
		// Credit for VTABLE Offsets: PO3, Thiaggo
		namespace BoundItemEffect
		{
			constexpr auto VTABLE_BoundItemEffect = REL::ID(257627); //1416377C8
		}

		namespace Character
		{
			constexpr auto VTABLE_CharacterMagicTarget = REL::ID(261401); //14165E468
		}

		namespace CloakEffect
		{
			constexpr auto VTABLE_CloakEffect = REL::ID(257635); //141637990
		}

		namespace DialogueItem
		{
			constexpr auto Ctor = REL::ID(34413);
		}

		namespace DialogueTopicManager
		{
			constexpr auto SayTopic = REL::ID(25014); //14038F9A0
		}

		namespace PlayerCharacter
		{
			constexpr auto VTABLE_PlayerCharacterMagicTarget = REL::ID(261920); //141664030
		}

		namespace LightEffect
		{
			constexpr auto VTABLE_LightEffect = REL::ID(257725); //1416392D0
		}

		namespace ScriptEffect
		{
			constexpr auto VTABLE_ScriptEffect = REL::ID(257919); //14163BC48
		}

		namespace SummonCreatureEffect
		{
			constexpr auto VTABLE_SummonCreatureEffect = REL::ID(258015); //14163CFF8
		}

		namespace ValueModifierEffect
		{
			constexpr auto VTABLE_ValueModifierEffect = REL::ID(258043); //14163DF38
		}
	}
}