#pragma once
#include "FenixProjectilesAPI.h"
#include "NewProjectiles.h"

class PaddingsProjectileHook
{
public:
	static void Hook()
	{
		auto& trmpl = SKSE::GetTrampoline();

		_TESForm__SetInitedFormFlag_140194B90 =
			trmpl.write_call<5>(REL::ID(42920).address() + 0x392, Ctor);  // SkyrimSE.exe+74ACE2
		_TESObjectREFR__ReadFromSaveGame_140286FD0 =
			trmpl.write_call<5>(REL::ID(42953).address() + 0x4b, LoadGame);  // SkyrimSE.exe+74D28B
	}

private:
	static void Ctor(RE::Projectile* proj, char a2)
	{
		_TESForm__SetInitedFormFlag_140194B90(proj, a2);
		set_NormalType(proj);
	}

	static void __fastcall LoadGame(RE::Projectile* proj, RE::BGSLoadGameBuffer* buf)
	{
		_TESObjectREFR__ReadFromSaveGame_140286FD0(proj, buf);
		set_NormalType(proj);
	}

	static inline REL::Relocation<decltype(Ctor)> _TESForm__SetInitedFormFlag_140194B90;
	static inline REL::Relocation<decltype(LoadGame)> _TESObjectREFR__ReadFromSaveGame_140286FD0;
};

class InitStartPosHook
{
public:
	static void Hook()
	{
		auto& trmpl = SKSE::GetTrampoline();

		_TESObjectREFR__SetPosition_140296910 =
			trmpl.write_call<5>(REL::ID(43030).address() + 0x215, InitStartPos);          // SkyrimSE.exe+754A35
		_mb_autoaim = trmpl.write_call<5>(REL::ID(43009).address() + 0x201, mb_autoaim);  // SkyrimSE.exe+7518E1
	}

private:
	static void InitStartPos(RE::Projectile* proj, RE::NiPoint3* pos)
	{
		if (!is_CustomPosType(proj)) {
			_TESObjectREFR__SetPosition_140296910(proj, pos);
		}
	}

	static void mb_autoaim(RE::PlayerCharacter* player, RE::Projectile* proj, RE::NiAVObject* player_bone, float* dir1,
		float* dir2, RE::NiPoint3* a7, float a8, float a9)
	{
		if (!is_CustomPosType(proj)) {
			_mb_autoaim(player, proj, player_bone, dir1, dir2, a7, a8, a9);
		}
	}

	static inline REL::Relocation<decltype(InitStartPos)> _TESObjectREFR__SetPosition_140296910;
	static inline REL::Relocation<decltype(mb_autoaim)> _mb_autoaim;
};

class MultipleBeamsHook
{
public:
	static void Hook()
	{
		auto& trmpl = SKSE::GetTrampoline();

		_RefHandle__get = trmpl.write_call<5>(REL::ID(42928).address() + 0x117, NewBeam);  // SkyrimSE.exe+74B287

		{
			// SkyrimSE.exe+733F93
			uintptr_t ret_addr = REL::ID(42586).address() + 0x2d3;

			struct Code : Xbyak::CodeGenerator
			{
				Code(uintptr_t func_addr, uintptr_t ret_addr)
				{
					Xbyak::Label nocancel;

					// rsi  = proj
					// xmm0 -- xmm2 = node pos
					mov(r9, rsi);
					mov(rax, func_addr);
					call(rax);
					mov(rax, ret_addr);
					jmp(rax);
				}
			} xbyakCode{ uintptr_t(update_node_pos), ret_addr };

			FenixUtils::add_trampoline<5, 42586, 0x2c1>(&xbyakCode);  // SkyrimSE.exe+621285
		}

		_TESObjectREFR__SetPosition_140296910 =
			trmpl.write_call<5>(REL::ID(42586).address() + 0x2db, UpdatePos);  // SkyrimSE.exe+733F9B
		_j_TESObjectREFR__SetRotationZ_14074FEE0 =
			trmpl.write_call<5>(REL::ID(42586).address() + 0x1ba, UpdateRotZ);                          // SkyrimSE.exe+733E7A
		_Projectile__SetRotationX = trmpl.write_call<5>(REL::ID(42586).address() + 0x1cd, UpdateRotX);  // SkyrimSE.exe+733E8D
	}

private:
	static bool NewBeam(uint32_t* handle, RE::Projectile** proj)
	{
		auto found = _RefHandle__get(handle, proj);
		if (!found || !*proj)
			return found;

		return !is_CustomPosType(*proj);
	}

	static void update_node_pos(float x, float y, float z, RE::Projectile* proj)
	{
		if (auto node = proj->Get3D()) {
			if (!is_CustomPosType(proj)) {
				node->local.translate.x = x;
				node->local.translate.y = y;
				node->local.translate.z = z;
			}
		}
	}

	static void UpdatePos(RE::Projectile* proj, RE::NiPoint3* pos)
	{
		if (!is_CustomPosType(proj)) {
			_TESObjectREFR__SetPosition_140296910(proj, pos);
		}
	}

	static void UpdateRotZ(RE::Projectile* proj, float rot_Z)
	{
		if (!is_CustomPosType(proj)) {
			_j_TESObjectREFR__SetRotationZ_14074FEE0(proj, rot_Z);
		}
	}

	static void UpdateRotX(RE::Projectile* proj, float rot_X)
	{
		if (!is_CustomPosType(proj)) {
			_Projectile__SetRotationX(proj, rot_X);
		}
	}

	static inline REL::Relocation<decltype(NewBeam)> _RefHandle__get;
	static inline REL::Relocation<decltype(UpdatePos)> _TESObjectREFR__SetPosition_140296910;
	static inline REL::Relocation<decltype(UpdateRotZ)> _j_TESObjectREFR__SetRotationZ_14074FEE0;
	static inline REL::Relocation<decltype(UpdateRotX)> _Projectile__SetRotationX;
};

class NormLightingsHook
{
public:
	static void Hook()
	{
		_BeamProjectile__ctor =
			SKSE::GetTrampoline().write_call<5>(REL::ID(42928).address() + 0x185, Ctor);  // SkyrimSE.exe+74B2F5
	}

private:
	static RE::BeamProjectile* Ctor(RE::BeamProjectile* proj, void* LaunchData) {
		proj = _BeamProjectile__ctor(proj, LaunchData);
		if (auto spell = proj->spell; spell && spell->GetCastingType() == RE::MagicSystem::CastingType::kFireAndForget) {
			set_CustomPosType(proj);
		}
		return proj;
	}

	static inline REL::Relocation<decltype(Ctor)> _BeamProjectile__ctor;
};

class AutoAimHook
{
public:
	static void Hook()
	{
		_Projectile__apply_gravity =
			SKSE::GetTrampoline().write_call<5>(REL::ID(43006).address() + 0x69, change_direction);  // SkyrimSE.exe+751309
	}

private:
	static bool change_direction(RE::Projectile* proj, RE::NiPoint3* dV, float dtime)
	{
		bool ans = _Projectile__apply_gravity(proj, dV, dtime);
		if (is_AutoAimType(proj)) {
			AutoAim::change_direction(proj, dV, dtime);
		}
		return ans;
	}

	static inline REL::Relocation<decltype(change_direction)> _Projectile__apply_gravity;
};

class SetNewTypeHook
{
public:
	static void Hook()
	{
		_MissileProjectile__ctor =
			SKSE::GetTrampoline().write_call<5>(REL::ID(42928).address() + 0x219, Ctor);  // SkyrimSE.exe+74B389
	}

private:
	static RE::MissileProjectile* Ctor(RE::MissileProjectile* proj, void* LaunchData)
	{
		proj = _MissileProjectile__ctor(proj, LaunchData);

		using namespace AutoAim;
		if (auto type = AutoAim::is_homie(proj->GetBaseObject()); type.first != AutoAimTypes::Normal) {
			if (auto caster = proj->shooter.get().get()) {
				bool isPlayer = caster->IsPlayerRef();
				using Caster_t = FenixProjsAutoAimData::AutoAimCaster;
				if (type.second.caster == Caster_t::NPC && !isPlayer || type.second.caster == Caster_t::Player && isPlayer ||
					type.second.caster == Caster_t::Both) {
					set_AutoAimType(proj, type);
				}
			}
		}
		return proj;
	}

	static inline REL::Relocation<decltype(Ctor)> _MissileProjectile__ctor;
};

class ManyProjsHook
{
public:
	static void Hook()
	{
		_castProjectile =
			SKSE::GetTrampoline().write_call<5>(REL::ID(33670).address() + 0x575, castProjectile);  // SkyrimSE.exe+5504F5
	}

private:
	static bool castProjectile(RE::MagicCaster* a, RE::BGSProjectile* bproj, RE::Actor* a_char, RE::CombatController* a4,
		RE::NiPoint3* startPos, float rotationZ, float rotationX, uint32_t area, void* a9)
	{
		if (!ManyProjs::is_ManyCast(bproj))
			return _castProjectile(a, bproj, a_char, a4, startPos, rotationZ, rotationX, area, a9);
	
		ManyProjs::onManyCasted(a, a_char, startPos, { rotationX, rotationZ });

		return true;
	}

	static inline REL::Relocation<decltype(castProjectile)> _castProjectile;
};

class DebugAPIHook
{
public:
	static void Hook() { _Update = REL::Relocation<uintptr_t>(REL::ID(RE::VTABLE_PlayerCharacter[0])).write_vfunc(0xad, Update); }

private:

	static void Update(RE::PlayerCharacter* a, float delta)
	{
		_Update(a, delta);
		DebugAPI_IMPL::DebugAPI::Update();
	}

	static inline REL::Relocation<decltype(Update)> _Update;
};

class CursorDetectedHook
{
public:
	static void Hook() { _Update = REL::Relocation<uintptr_t>(REL::ID(RE::VTABLE_PlayerCharacter[0])).write_vfunc(0xad, Update); }

private:
	static void Update(RE::PlayerCharacter* a, float delta)
	{
		_Update(a, delta);

		if (auto obj = a->GetEquippedObject(false))
			if (auto spel = obj->As<RE::SpellItem>())
				if (auto mgef = FenixUtils::getAVEffectSetting(spel))
					if (auto target = AutoAim::find_cursor_target(a,
							static_cast<float>(AutoAim::is_homie(mgef->data.projectileBase).second.param2)))
						draw_line(a->GetPosition(), target->GetPosition(), 5, 0);
	}

	static inline REL::Relocation<decltype(Update)> _Update;
};

class CursorCircleHook
{
public:
	static void Hook()
	{
		_Update = REL::Relocation<uintptr_t>(REL::ID(RE::VTABLE_PlayerCharacter[0]))
		              .write_vfunc(0xad, Update);
	}

private:
	static void Update(RE::PlayerCharacter* a, float delta)
	{
		_Update(a, delta);

		if (auto obj = a->GetEquippedObject(false))
			if (auto spel = obj->As<RE::SpellItem>())
				if (auto mgef = FenixUtils::getAVEffectSetting(spel)) {
					float alpha_max = static_cast<float>(
						AutoAim::is_homie(mgef->data.projectileBase).second.param2);
					alpha_max = alpha_max / 180.0f * 3.1415926f;

					RE::NiPoint3 origin, caster_dir;
					FenixUtils::Actor__get_eye_pos(a, origin, 2);

					const float circle_dist = 2000;
					caster_dir = FenixUtils::rotate(1, a->data.angle);

					float circle_r = circle_dist * tan(alpha_max);
					RE::NiPoint3 right_dir = RE::NiPoint3(0, 0, -1).UnitCross(caster_dir);
					if (right_dir.SqrLength() == 0)
						right_dir = { 1, 0, 0 };
					right_dir *= circle_r;
					RE::NiPoint3 up_dir = right_dir.Cross(caster_dir);

					origin += caster_dir * circle_dist;

					RE::NiPoint3 old = origin + right_dir;
					const int N = 31;
					for (int i = 1; i <= N; i++) {
						float alpha = 2 * 3.1415926f / N * i;

						auto cur_p =
							origin + right_dir * cos(alpha) + up_dir * sin(alpha);

						draw_line(old, cur_p, 5, 0);
						old = cur_p;
					}
				}
	}

	static inline REL::Relocation<decltype(Update)> _Update;
};
