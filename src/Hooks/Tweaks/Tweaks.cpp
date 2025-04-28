#include "Tweaks.h"

#include "RE/Offset.h"
#include "RE/Misc.h"
#include "Settings/INISettings.h"

#include "Hooks/Tweaks/Archetypes/BoundEffect.h"
#include "Hooks/Tweaks/Archetypes/CloakEffect.h"
#include "Hooks/Tweaks/Archetypes/LightEffect.h"
#include "Hooks/Tweaks/Archetypes/ScriptEffect.h"
#include "Hooks/Tweaks/Archetypes/SummonEffect.h"
#include "Hooks/Tweaks/Archetypes/ValueModifierEffect.h"

namespace Hooks::Tweaks
{
	bool Install() {
		logger::info("  >Installing Tweaks"sv);
		auto* extender = EffectExtender::GetSingleton();
		auto* silencer = CommentSilencer::GetSingleton();
		auto* dispeler = SpellDispeler::GetSingleton();
		if (!extender || !silencer || !dispeler) {
			logger::critical("    >Failed to get internal hook listeners."sv);
			return false;
		}
		return extender->Install() &&
			silencer->Install() &&
			dispeler->Install();
	}

	bool EffectExtender::Install() {
		auto* iniHolder = Settings::INI::Holder::GetSingleton();
		if (!iniHolder) {
			logger::info("    >Failed to fetch INI settings holder for Fixes."sv);
			return false;
		}

		auto extendInDialogueRaw = iniHolder->GetStoredSetting<bool>("Tweaks|bExtendEffectsInDialogue");
		extendInDialogue = extendInDialogueRaw.has_value() ? extendInDialogueRaw.value() : false;
		if (!extendInDialogueRaw.has_value()) {
			logger::warn("    >Extend In Dialogue tweak setting somehow not specified in the INI, treating as false."sv);
		}
		if (!extendInDialogue) {
			logger::info("    >Extend in Dialogue is disabled, VFunc hooks will not be installed"sv);
			return true;
		}

		return Archetypes::BoundEffect::Install() &&
			Archetypes::CloakEffect::Install() &&
			Archetypes::LightEffect::Install() &&
			Archetypes::ScriptEffect::Install() &&
			Archetypes::SummonEffect::Install() &&
			Archetypes::ValueEffect::Install();
	}

	void EffectExtender::ExtendEffectIfNecessary(RE::ActiveEffect* a_effect, float a_extension) {
		using ActiveEffectFlag = RE::EffectSetting::EffectSettingData::Flag;
		if (!extendInDialogue) {
			return;
		}

		auto* ui = RE::UI::GetSingleton();
		if (!ui || !a_effect) {
			return;
		}

		const auto* caster = a_effect->GetCasterActor().get();
		const auto* target = a_effect->GetTargetActor();
		const auto* base = a_effect->effect ? a_effect->effect->baseEffect : nullptr;
		if (!base || !caster || !caster->IsPlayerRef() || !target) {
			return;
		}

		if (base->data.flags.any(ActiveEffectFlag::kDetrimental,
			ActiveEffectFlag::kHostile,
			ActiveEffectFlag::kNoDuration)) {
			return;
		}

		if (ui->IsMenuOpen(RE::DialogueMenu::MENU_NAME)) {
			a_effect->elapsedSeconds -= a_extension;
		}
	}

	bool CommentSilencer::Install() {
		// While this works, it is not a good solution. The Modern Clap Bugfix is better.
		/*
		auto* iniHolder = Settings::INI::Holder::GetSingleton();
		if (!iniHolder) {
			logger::critical("Failed to fetch ini settings holder."sv);
			return false;
		}

		auto installRaw = iniHolder->GetStoredSetting<bool>("Tweaks|bSuppressMagicComments");
		bool install = installRaw.has_value() ? installRaw.value() : false;
		if (!installRaw.has_value()) {
			logger::warn("  >Setting {} not found in ini settings, treating as false.", "Tweaks|bSuppressMagicComments");
		}
		if (!install) {
			return true;
		}

		REL::Relocation<std::uintptr_t> target{ RE::Offset::DialogueTopicManager::SayTopic, 0xE2 };
		if (!(REL::make_pattern<"E8">().match(target.address()))) {
			logger::critical("Failed to validate pattern of the Comment Silencer."sv);
			return false;
		}

		auto& trampoline = SKSE::GetTrampoline();
		_func = trampoline.write_call<5>(target.address(), &Thunk);
		*/
		return true;
	}

	void CommentSilencer::AppendQuest(const RE::TESQuest* a_candidate) {
		if (blacklistedQuests.contains(a_candidate)) {
			return;
		}
		blacklistedQuests.emplace(a_candidate);
	}

	RE::DialogueItem* CommentSilencer::Thunk(RE::DialogueItem* a_dialogueItem,
		RE::TESQuest* a_quest,
		RE::TESTopic* a_topic,
		RE::TESTopicInfo* a_topicInfo,
		RE::TESObjectREFR* a_speaker) {
		auto* silencer = CommentSilencer::GetSingleton();
		auto* response = _func(a_dialogueItem, a_quest, a_topic, a_topicInfo, a_speaker);
		if (silencer && silencer->blacklistedQuests.contains(a_quest)) {
			delete response;
			return nullptr;
		}

		return RE::ConstructDialogueItem(a_dialogueItem, a_quest, a_topic, a_topicInfo, a_speaker);
	}

	bool SpellDispeler::Install() {
		return PlayerDrawMonitor::Install();
	}

	void SpellDispeler::ApendEffect(const RE::EffectSetting* a_candidate) {
		if (dispelEffects.contains(a_candidate)) {
			return;
		}
		const std::string proposed = Utilities::EDID::GetEditorID(a_candidate);
		if (!proposed.empty()) {
			logger::info("    >{}"sv, proposed);
		}
		else {
			const auto effectName = a_candidate->GetName();
			if (strcmp(effectName, "") != 0) {
				logger::info("    >{}"sv, effectName);
			}
			else {
				logger::info("    >[Unnamed Effect]"sv);
			}
		}
		dispelEffects.emplace(a_candidate);
	}

	void SpellDispeler::ClearDispelableSpells(RE::PlayerCharacter* a_player) {
		auto* magicTarget = a_player ? a_player->GetMagicTarget() : nullptr;
		if (!magicTarget || dispelEffects.empty()) {
			return;
		}

		auto effectList = magicTarget->GetActiveEffectList();
		if (effectList->empty()) {
			return;
		}

		for (auto activeEffect : *effectList) {
			const auto* base = activeEffect && activeEffect->effect ?
				activeEffect->effect->baseEffect : nullptr;
			if (!base || !dispelEffects.contains(base)) {
				continue;
			}

			const float effectDuration = activeEffect->duration;
			const float setElapsedTime = std::max(effectDuration - 0.1f, 0.1f);
			activeEffect->elapsedSeconds = setElapsedTime;
		}
	}

	bool SpellDispeler::PlayerDrawMonitor::Install() {
		auto* iniHolder = Settings::INI::Holder::GetSingleton();
		if (!iniHolder) {
			logger::critical("    >Failed to fetch ini settings holder for the Seathe monitor."sv);
			return false;
		}

		auto installRaw = iniHolder->GetStoredSetting<bool>(setting);
		bool install = installRaw.has_value() ? installRaw.value() : false;
		if (!installRaw.has_value()) {
			logger::warn("    >Setting {} not found in ini settings, treating as false.", setting);
		}
		if (!install) {
			return true;
		}

		REL::Relocation<std::uintptr_t> VTABLE{ RE::PlayerCharacter::VTABLE[0] };
		_func = VTABLE.write_vfunc(offset, Thunk);
		logger::info("    >Installed Player VFunc hook."sv);
		return true;
	}

	void SpellDispeler::PlayerDrawMonitor::Thunk(RE::PlayerCharacter* a_this, 
		bool a_draw) 
	{
		_func(a_this, a_draw);
		if (!a_draw) {
			if (auto* dispeler = SpellDispeler::GetSingleton(); dispeler) {
				dispeler->ClearDispelableSpells(a_this);
			}
		}
	}
}