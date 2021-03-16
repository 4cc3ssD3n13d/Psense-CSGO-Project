// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "animation_system.h"
#include "..\ragebot\aim.h"
#include "..\autowall\autowall.h"

void resolver::initialize(player_t* e, adjust_data* record, const float& pitch)
{
	player = e;
	player_record = record;

	original_pitch = math::normalize_pitch(pitch);
}

void resolver::reset()
{
	player = nullptr;
	player_record = nullptr;

	original_pitch = 0.0f;
}

//void resolver::DetectSide(player_t* e, int* side)
//{
//    Vector src3D, dst3D, forward, right, up, src, dst;
//    float back_position, right_position, left_position;
//    trace_t tr;
//    CTraceFilter filter;
//
//    Vector engineViewAngles;
//    m_engine()->GetViewAngles(engineViewAngles);
//    engineViewAngles.x = 0.0f;
//
//    math::angle_vectors(engineViewAngles, &forward, &right, &up);
//
//    filter.pSkip = player;
//    src3D = player->m_angEyeAngles();
//    dst3D = src3D + (forward * 384);
//
//    m_trace()->TraceRay(Ray_t(src3D, dst3D), MASK_SHOT, &filter, &tr);
//    back_position = (tr.endpos - tr.startpos).Length();
//
//    m_trace()->TraceRay(Ray_t(src3D + right * 35, dst3D + right * 35), MASK_SHOT, &filter, &tr);
//    right_position = (tr.endpos - tr.startpos).Length();
//
//    m_trace()->TraceRay(Ray_t(src3D - right * 35, dst3D - right * 35), MASK_SHOT, &filter, &tr);
//    left_position = (tr.endpos - tr.startpos).Length();
//
//    if (left_position > right_position) {
//        *side = -1;
//    }
//    else if (right_position > left_position) {
//        *side = 1;
//    }
//    else
//        *side = 0;
//}

void resolver::ResolveAngles(player_t* e)
{
	resolver* resolver_info;
	player_info_t player_info;
	adjust_data* record;

	if (!m_engine()->GetPlayerInfo(player->EntIndex(), &player_info))
		return;

	int missed = g_ctx.globals.missed_shots[player->EntIndex()];

	if (player_info.fakeplayer || !g_ctx.local()->is_alive() || player->m_iTeamNum() == g_ctx.local()->m_iTeamNum())
	{
		missed = 0;
		return;
	}

	auto simtime = player->m_flSimulationTime();
	auto oldsimtime = player->m_flOldSimulationTime();
	auto simtimediff = simtime - oldsimtime;
	auto choked_packets = TIME_TO_TICKS(max(0, simtimediff));

	auto animstate = player->get_animation_state();

	// if animstate.
	if (!animstate)
		return;

	// not using desync.
	if (choked_packets < 1)
		return;

	auto max_body_yaw = player->get_animation_state()->pad10[516];
	auto min_body_yaw = player->get_animation_state()->pad10[512];
	static float fake_desync[65] = { -1, 1 };
	float speed = player->m_vecVelocity().Length2D();
	float eye_angles = player->m_angEyeAngles().y;

	// get run and stop or full side way moving or break animation is lby  [ lby / sideways method ].
	float unk1 = (animstate->m_flStopToFullRunningFraction * -0.30000001) - 0.19999999 * animstate->m_flFeetSpeedForwardsOrSideWays;
	float unk2 = unk1 + 1.f;
	float unk3;

	if (animstate->m_fDuckAmount > 0)
	{
		unk2 += ((animstate->m_fDuckAmount * animstate->m_flFeetSpeedUnknownForwardOrSideways) * (0.5f - unk2));
	}

	unk3 = *(float*)(animstate + 0x334) * unk2;

	// get fake desync is goal feet yaw.
	fake_desync[player->EntIndex()] = animstate->m_flGoalFeetYaw;

	// fake pitch correction fix ( ghetto  / available crash).
	if (player->m_angEyeAngles().x == -540.f || player->m_angEyeAngles().x == -1620.f)
		player->m_angEyeAngles().x = 88.8;

	// shot  real angle cant use angle is eye.
	if (speed > 0.1f && speed <= 1.0f /*|| fabs(animstate->flUpVelocity) > 100.0f*/)
	{
		switch (g_ctx.globals.missed_shots[player->EntIndex()] % 3)
		{
		case 0: // brute left angle.
			fake_desync[player->EntIndex()] = math::normalize_yaw(min_body_yaw + max_body_yaw) + unk3;
			break;
		case 1: // brute fake angle.
			fake_desync[player->EntIndex()] = eye_angles;
			break;
		case 2: // brute right angle.
			fake_desync[player->EntIndex()] = math::normalize_yaw(max_body_yaw - min_body_yaw) + unk3;
			break;
		default:
			break;
		}
	}
}

template <class T>
constexpr const T& Max(const T& x, const T& y)
{
	return (x < y) ? y : x;
}
void inline SinCos(float radians, float* sine, float* cosine)
{
	*sine = sin(radians);
	*cosine = cos(radians);

}

void AngleVectors(const Vector& angles, Vector* forward)
{
	float	sp, sy, cp, cy;

	SinCos(DEG2RAD(angles[1]), &sy, &cy);
	SinCos(DEG2RAD(angles[0]), &sp, &cp);

	forward->x = cp * cy;
	forward->y = cp * sy;
	forward->z = -sp;
}

float GetLBYRotatedYaw(float lby, float yaw)
{
	float delta = math::normalize_yaw(yaw - lby);
	if (fabs(delta) < 25.f)
		return lby;

	if (delta > 0.f)
		return yaw + 25.f;

	return yaw;
}

//bool resolver::is_slow_walking(player_t* entity)
//{
//	float velocity_2D[64], old_velocity_2D[64];
//
//	if (entity->m_vecVelocity().Length2D() != velocity_2D[entity->EntIndex()] && entity->m_vecVelocity().Length2D() != NULL) {
//		old_velocity_2D[entity->EntIndex()] = velocity_2D[entity->EntIndex()];
//		velocity_2D[entity->EntIndex()] = entity->m_vecVelocity().Length2D();
//	}
//
//	if (velocity_2D[entity->EntIndex()] > 0.1)
//	{
//		int tick_counter[64];
//
//		if (velocity_2D[entity->EntIndex()] == old_velocity_2D[entity->EntIndex()])
//			++tick_counter[entity->EntIndex()];
//		else
//			tick_counter[entity->EntIndex()] = 0;
//
//		while (tick_counter[entity->EntIndex()] > (1 / m_globals()->m_intervalpertick) * fabsf(0.1f))
//			return true;
//
//	}
//	return false;
//}

//void resolver::create_move(Vector latency_based_eye_pos)
//{
//	//if (!g_cfg.ragebot.antiaim_correction) return;
//
//	player_t* e;
//
//	const float height = 64;
//
//	Vector direction_1, direction_2;
//	math::angle_vectors( Vector( 0.f, math::calculate_angle( g_ctx.m_local->m_vecOrigin( ), e->m_vecOrigin( ) ).y - 90.f, 0.f ), direction_1 );
//	math::angle_vectors( Vector( 0.f, math::calculate_angle( g_ctx.m_local->m_vecOrigin( ), e->m_vecOrigin( ) ).y + 90.f, 0.f ), direction_2 );
//
//	const auto left_eye_pos = e->m_vecOrigin( ) + Vector( 0, 0, height ) + ( direction_1 * 16.f );
//	const auto right_eye_pos = e->m_vecOrigin( ) + Vector( 0, 0, height ) + ( direction_2 * 16.f );
//
//	m_antifreestand.left_damage = ñ_autowall::get( ).calculate_return_info( latency_based_eye_pos, left_eye_pos, g_ctx.m_local, e, 1 ).m_damage;
//	m_antifreestand.right_damage = autowall::get( ).calculate_return_info( latency_based_eye_pos, right_eye_pos, g_ctx.m_local, e, 1 ).m_damage;
//
//	Ray_t ray;
//	trace_t trace;
//	CTraceFilterWorldOnly filter;
//
//	ray.Init( left_eye_pos, latency_based_eye_pos );
//	g_csgo.m_trace( )->TraceRay( ray, MASK_ALL, &filter, &trace );
//	m_antifreestand.left_fraction = trace.fraction;
//
//	ray.Init( right_eye_pos, latency_based_eye_pos );
//	g_csgo.m_trace( )->TraceRay( ray, MASK_ALL, &filter, &trace );
//	m_antifreestand.right_fraction = trace.fraction;
//
//}



//bool resolver::freestand_target(player_t* target, float* yaw)
//{
//	float dmg_left = 0.f;
//	float dmg_right = 0.f;
//
//	static auto get_rotated_pos = [](Vector start, float rotation, float distance)
//	{
//		float rad = DEG2RAD(rotation);
//		start.x += cos(rad) * distance;
//		start.y += sin(rad) * distance;
//
//		return start;
//	};
//
//	const auto local = g_ctx.local();
//
//	if (!local || !target || !local->is_alive())
//		return false;
//
//	Vector local_eye_pos = target->get_shoot_position();
//	Vector eye_pos = local->get_shoot_position();
//	Vector angle = math::calc_angle(local_eye_pos, eye_pos);
//
//	auto backwards = target->m_angEyeAngles().y; // angle.y;
//
//	Vector pos_left = get_rotated_pos(eye_pos, angle.y + 90.f, 40.f);
//	Vector pos_right = get_rotated_pos(eye_pos, angle.y - 90.f, -40.f);
//
//	const auto wall_left = autowall->wall_penetration(local_eye_pos, pos_left,
//		nullptr, nullptr, local);
//
//	const auto wall_right = trace_system->wall_penetration(local_eye_pos, pos_right,
//		nullptr, nullptr, local);
//
//	if (wall_left.has_value())
//		dmg_left = wall_left.value().damage;
//
//	if (wall_right.has_value())
//		dmg_right = wall_right.value().damage;
//
//	if (dmg_left == 0.f && dmg_right == 0.f)
//	{
//		*yaw = backwards;
//		return false;
//	}
//
//	// we can hit both sides, lets force backwards
//	if (fabsf(dmg_left - dmg_right) < 10.f)
//	{
//		*yaw = backwards;
//		return false;
//	}
//
//	bool direction = dmg_left > dmg_right;
//	*yaw = direction ? angle.y - 90.f : angle.y + 90.f;
//
//	return true;
//}

float resolver::resolve_pitch()
{
	return original_pitch;
}