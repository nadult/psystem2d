#include "fx_manager.h"

#include "fx_color.h"
#include "fx_particle_system.h"

namespace fx {

using SubSystemDef = ParticleSystemDef::SubSystem;

static void addTestEffect(FXManager &mgr) {
	EmitterDef edef;
	edef.strength_min = edef.strength_max = 30.0f;
	edef.direction = 0.0f;
	edef.direction_spread = fconstant::pi;
	edef.frequency = {{10.0f, 55.0f, 0.0f, 0.0}, InterpType::cosine};

	ParticleDef pdef;
	pdef.life = 1.0f;
	pdef.size = 32.0f;
	pdef.alpha = {{0.0f, 0.1, 0.8f, 1.0f}, {0.0, 1.0, 1.0, 0.0}, InterpType::linear};

	pdef.color = {{{1.0f, 1.0f, 0.0f}, {0.5f, 1.0f, 0.5f}, {0.2f, 0.5f, 1.0f}}, InterpType::linear};
	pdef.texture_name = "circular.png";

	ParticleSystemDef psdef;
	psdef.subsystems.emplace_back(mgr.addDef(pdef), mgr.addDef(edef));
	psdef.anim_length = 10.0f;
	psdef.name = "test_effect";
	mgr.addDef(psdef);
}

static void addWoodSplinters(FXManager &mgr) {
	EmitterDef edef;
	edef.strength_min = 20.0f;
	edef.strength_max = 60.0f;
	edef.direction = 0.0f;
	edef.direction_spread = fconstant::pi;
	edef.rotation_speed_min = -0.5f;
	edef.rotation_speed_max = 0.5f;
	edef.frequency = 999.0f;

	ParticleDef pdef;
	pdef.life = 5.0f;
	pdef.size = 4.0f;
	pdef.slowdown = {{0.0f, 0.1f}, {5.0f, 1000.0f}};
	pdef.alpha = {{0.0f, 0.8f, 1.0f}, {1.0, 1.0, 0.0}};

	auto animate_func = [](AnimationContext &ctx, Particle &pinst) {
		defaultAnimateParticle(ctx, pinst);
		float shadow_min = 5.0f, shadow_max = 10.0f;
		if(pinst.pos.y < shadow_min) {
			float dist = shadow_min - pinst.pos.y;
			pinst.movement += FVec2(0.0f, dist);
		}
		pinst.pos.y = min(pinst.pos.y, shadow_max);
	};

	FColor brown(IColor(120, 87, 46));
	// Kiedy cząsteczki opadną pod drzewo, robią się w zasięgu cienia
	// TODO: lepiej rysować je po prostu pod cieniem
	pdef.color = {{0.0f, 0.04f, 0.06}, {brown.rgb(), brown.rgb(), brown.rgb() * 0.6f}};
	pdef.texture_name = "flakes_4x4_borders.png";
	pdef.texture_tiles = {4, 4};

	SubSystemDef ssdef(mgr.addDef(pdef), mgr.addDef(edef));
	ssdef.max_total_particles = 4;
	ssdef.animate_func = animate_func;

	ParticleSystemDef psdef;
	psdef.subsystems = {ssdef};
	psdef.anim_length = 5.0f;
	psdef.name = "wood_splinters";
	mgr.addDef(psdef);

	// Kierunkowo to wygląda słabo, może lepiej genereować we wszystkich kierunkach
	// i najlepiej, jakby drzazgi lądowały pod drzewem!
}

static void addRockSplinters(FXManager &mgr) {
	// Opcja: spawnowanie splinterów na tym samym kaflu co minion:
	// - Problem: Te particle powinny się spawnować na tym samym tile-u co imp
	//   i spadać mu pod nogi, tak jak drewnianie drzazgi; Tylko jak to
	//   zrobić w przypadku kafli po diagonalach ?
	// - Problem: czy te particle wyświetlają się nad czy pod impem?
	//
	// Chyba prościej jest po prostu wyświetlać te particle na kaflu z rozwalanym
	// murem; Zresztą jest to bardziej spójne z particlami dla drzew
	EmitterDef edef;
	edef.strength_min = 20.0f;
	edef.strength_max = 60.0f;
	edef.direction = 0.0f;
	edef.direction_spread = fconstant::pi;
	edef.rotation_speed_min = -0.5f;
	edef.rotation_speed_max = 0.5f;
	edef.frequency = 999.0f;

	ParticleDef pdef;
	pdef.life = 5.0f;
	pdef.size = 4.0f;
	pdef.slowdown = {{0.0f, 0.1f}, {5.0f, 1000.0f}};
	pdef.alpha = {{0.0f, 0.8f, 1.0f}, {1.0, 1.0, 0.0}};

	pdef.color = FVec3(0.4, 0.4, 0.4);
	pdef.texture_name = "flakes_4x4_borders.png";
	pdef.texture_tiles = {4, 4};

	SubSystemDef ssdef(mgr.addDef(pdef), mgr.addDef(edef));
	ssdef.max_total_particles = 5;

	ParticleSystemDef psdef;
	psdef.subsystems = {ssdef};
	// Animacja może trwać dopóki są jakieś cząsteczki i emiter jeszcze żyje
	// TODO:
	// - cząsteczki mogą mieć różny czas życia
	// - emiter musi mieć parametr długości emisji (a może nawet początek i koniec)
	psdef.anim_length = 5.0f;
	psdef.name = "rock_splinters";
	mgr.addDef(psdef);
}

static void addRockCloud(FXManager &mgr) {
	// Spawnujemy kilka chmurek w ramach kafla;
	// mogą być większe lub mniejsze
	//
	// czy zostawiają po sobie jakieś ślady?
	// może niech zostają ślady po splinterach, ale po chmurach nie?
	EmitterDef edef;
	edef.source = FRect(-7.0f, -7.0f, 7.0f, 7.0f);
	edef.strength_min = 5.0f;
	edef.strength_max = 8.0f;
	edef.direction = 0.0f;
	edef.direction_spread = fconstant::pi;

	// TODO: możliwość precyzowania czasu emisji tutaj też się przyda
	// TODO: czas emisji w klatkach ? Chyba lepiej nie, bo animacja może różnie wyglądać
	// zależnie od fpsów...; chyba, że system fxów wykrywałby dropnięte klatki i
	// ew. odpalał simulate kilka razy ?
	edef.frequency = {{0.0f, 0.1f}, {60.0f, 0.0f}};

	ParticleDef pdef;
	pdef.life = 3.5f;
	pdef.size = {{0.0f, 0.1f, 1.0f}, {15.0f, 30.0f, 38.0f}};
	pdef.alpha = {{0.0f, 0.05f, 0.2f, 1.0f}, {0.0f, 0.3f, 0.4f, 0.0f}};
	pdef.slowdown = {{0.0f, 0.2f}, {0.0f, 10.0f}};

	FVec3 start_color(0.6), end_color(0.4);
	pdef.color = {{start_color, end_color}};
	pdef.texture_name = "clouds_soft_4x4.png";
	pdef.texture_tiles = {4, 4};

	SubSystemDef ssdef(mgr.addDef(pdef), mgr.addDef(edef));
	// TODO: różna liczba początkowych cząsteczek
	ssdef.max_total_particles = 5;

	ParticleSystemDef psdef;
	psdef.subsystems = {ssdef};
	psdef.anim_length = 5.0f;
	psdef.name = "rock_clouds";
	mgr.addDef(psdef);
}

static void addExplosionEffect(FXManager &mgr) {
	// TODO: tutaj trzeba zrobić tak, żeby cząsteczki które spawnują się później
	// zaczynały z innym kolorem
	EmitterDef edef;
	edef.strength_min = edef.strength_max = 15.0f;
	edef.direction = 0.0f;
	edef.direction_spread = fconstant::pi;
	edef.frequency = 60.0f;

	ParticleDef pdef;
	pdef.life = 0.5f;
	pdef.size = {{5.0f, 30.0f}};
	pdef.alpha = {{0.0f, 0.5f, 1.0f}, {0.3, 0.4, 0.0}};

	IColor start_color(255, 244, 88), end_color(225, 92, 19);
	pdef.color = {{FColor(start_color).rgb(), FColor(end_color).rgb()}};
	pdef.texture_name = "clouds_soft_borders_4x4.png";
	pdef.texture_tiles = {4, 4};

	SubSystemDef ssdef(mgr.addDef(pdef), mgr.addDef(edef));
	ssdef.max_total_particles = 20;

	ParticleSystemDef psdef;
	psdef.subsystems = {ssdef};
	psdef.anim_length = 2.0f;
	psdef.name = "explosion";
	mgr.addDef(psdef);
}

static void addRippleEffect(FXManager &mgr) {
	EmitterDef edef;

	// Ta animacja nie ma sprecyzowanej długości;
	// Zamiast tego może być włączona / wyłączona albo może się zwięszyć/zmniejszyć jej siła
	// krzywe które zależą od czasu animacji tracą sens;
	// animacja może być po prostu zapętlona
	edef.frequency = 1.5f;
	edef.initial_spawn_count = 1.0f;

	// First scalar parameter controls how fast the ripples move
	auto prep_func = [](AnimationContext &ctx, EmissionState &em) {
		float freq = defaultPrepareEmission(ctx, em);
		float mod = 1.0f + ctx.ps.params.scalar[0];
		return freq * mod;
	};

	auto animate_func = [](AnimationContext &ctx, Particle &pinst) {
		float ptime = pinst.particleTime();
		float mod = 1.0f + ctx.ps.params.scalar[0];
		float time_delta = ctx.time_delta * mod;
		pinst.pos += pinst.movement * time_delta;
		pinst.rot += pinst.rot_speed * time_delta;
		pinst.life += time_delta;
	};

	ParticleDef pdef;
	pdef.life = 1.5f;
	pdef.size = {{10.0f, 50.0f}};
	pdef.alpha = {{0.0f, 0.3f, 0.6f, 1.0f}, {0.0f, 0.3f, 0.5f, 0.0f}};

	pdef.color = FVec3(1.0f, 1.0f, 1.0f);
	pdef.texture_name = "torus.png";

	SubSystemDef ssdef(mgr.addDef(pdef), mgr.addDef(edef));
	ssdef.max_active_particles = 10;
	ssdef.prepare_func = prep_func;
	ssdef.animate_func = animate_func;

	ParticleSystemDef psdef;
	psdef.subsystems = {ssdef};
	psdef.anim_length = 1.0f;
	psdef.is_looped = true;
	psdef.name = "ripple";

	mgr.addDef(psdef);
}

static void addCircularBlast(FXManager &mgr) {
	EmitterDef edef;

	// TODO: odseparować czas emisji od czasu animacji ?
	// domyślnie czas emisji jest taki sam, ale można go zmienić
	//
	// TODO: odseparować czas subsystemu od czasu całej animacji?
	edef.frequency = {{0.0f, 0.05f}, {50.0f, 0.0f}};
	edef.initial_spawn_count = 10.0;

	auto prep_func = [](AnimationContext &ctx, EmissionState &em) {
		defaultPrepareEmission(ctx, em);
		return 0.0f;
	};

	auto emit_func = [](AnimationContext &ctx, EmissionState &em, Particle &new_inst) {
		new_inst.life = min(em.max_life, float(ctx.ss.total_particles) * 0.01f);
		new_inst.max_life = em.max_life;
	};

	auto animate_func = [](AnimationContext &ctx, Particle &pinst) {
		float ptime = pinst.particleTime();
		float mod = 1.0f + ctx.ps.params.scalar[0];
		float time_delta = ctx.time_delta * mod;
		pinst.life += pinst.movement.x;
		pinst.movement.x = 0;
		pinst.life += time_delta;
	};

	ParticleDef pdef;
	pdef.life = 0.5f;
	pdef.size = {{10.0f, 80.0f}, InterpType::cosine};
	pdef.alpha = {{0.0f, 0.03f, 0.2f, 1.0f}, {0.0f, 0.15f, 0.15f, 0.0f}, InterpType::cosine};

	pdef.color = {{0.5f, 0.8f}, {{1.0f, 1.0f, 1.0f}, {0.5f, 0.5f, 1.0f}}};
	pdef.texture_name = "torus.png";

	SubSystemDef ssdef(mgr.addDef(pdef), mgr.addDef(edef));
	ssdef.max_active_particles = 20;
	ssdef.prepare_func = prep_func;
	ssdef.animate_func = animate_func;
	ssdef.emit_func = emit_func;

	ParticleSystemDef psdef;
	psdef.subsystems = {ssdef};
	psdef.anim_length = 1.0f;
	psdef.name = "circular_blast";

	mgr.addDef(psdef);
}

void FXManager::addDefaultDefs() {
	addTestEffect(*this);
	addWoodSplinters(*this);
	addRockSplinters(*this);
	addRockCloud(*this);
	addExplosionEffect(*this);
	addRippleEffect(*this);
	addCircularBlast(*this);
};
}
