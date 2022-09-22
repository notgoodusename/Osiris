#include <algorithm>
#include <cstring>
#include <deque>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>

#include "Chams.h"
#include "../Config.h"
#include "../Helpers.h"
#include "../Hooks.h"
#include "../Interfaces.h"
#include "../Memory.h"
#include "Backtrack.h"
#include "../InputUtil.h"
#include <SDK/Constants/ClassId.h>
#include "../SDK/ClientClass.h"
#include "../SDK/Entity.h"
#include "../SDK/EntityList.h"
#include "../SDK/GlobalVars.h"
#include "../SDK/LocalPlayer.h"
#include "../SDK/Material.h"
#include "../SDK/MaterialSystem.h"
#include "../SDK/ModelRender.h"
#include "../SDK/StudioRender.h"
#include "../SDK/KeyValues.h"
#include "../SDK/Utils.h"

static Material* normal;
static Material* flat;
static Material* animated;
static Material* platinum;
static Material* glass;
static Material* crystal;
static Material* chrome;
static Material* silver;
static Material* gold;
static Material* plastic;
static Material* glow;
static Material* pearlescent;
static Material* metallic;

static constexpr auto dispatchMaterial(int id) noexcept
{
    switch (id) {
    default:
    case 0: return normal;
    case 1: return flat;
    case 2: return animated;
    case 3: return platinum;
    case 4: return glass;
    case 5: return chrome;
    case 6: return crystal;
    case 7: return silver;
    case 8: return gold;
    case 9: return plastic;
    case 10: return glow;
    case 11: return pearlescent;
    case 12: return metallic;
    }
}

static void initializeMaterials(const Memory& memory) noexcept
{
    normal = interfaces->materialSystem->createMaterial("normal", KeyValues::fromString(memory, "VertexLitGeneric", nullptr));
    flat = interfaces->materialSystem->createMaterial("flat", KeyValues::fromString(memory, "UnlitGeneric", nullptr));
    chrome = interfaces->materialSystem->createMaterial("chrome", KeyValues::fromString(memory, "VertexLitGeneric", "$envmap env_cubemap"));
    glow = interfaces->materialSystem->createMaterial("glow", KeyValues::fromString(memory, "VertexLitGeneric", "$additive 1 $envmap models/effects/cube_white $envmapfresnel 1 $alpha .8"));
    pearlescent = interfaces->materialSystem->createMaterial("pearlescent", KeyValues::fromString(memory, "VertexLitGeneric", "$ambientonly 1 $phong 1 $pearlescent 3 $basemapalphaphongmask 1"));
    metallic = interfaces->materialSystem->createMaterial("metallic", KeyValues::fromString(memory, "VertexLitGeneric", "$basetexture white $ignorez 0 $envmap env_cubemap $normalmapalphaenvmapmask 1 $envmapcontrast 1 $nofog 1 $model 1 $nocull 0 $selfillum 1 $halfambert 1 $znearer 0 $flat 1"));

    {
        const auto kv = KeyValues::fromString(memory, "VertexLitGeneric", "$envmap editor/cube_vertigo $envmapcontrast 1 $basetexture dev/zone_warning proxies { texturescroll { texturescrollvar $basetexturetransform texturescrollrate 0.6 texturescrollangle 90 } }");
        kv->setString(memory, "$envmaptint", "[.7 .7 .7]");
        animated = interfaces->materialSystem->createMaterial("animated", kv);
    }

    {
        const auto kv = KeyValues::fromString(memory, "VertexLitGeneric", "$baseTexture models/player/ct_fbi/ct_fbi_glass $envmap env_cubemap");
        kv->setString(memory, "$envmaptint", "[.4 .6 .7]");
        platinum = interfaces->materialSystem->createMaterial("platinum", kv);
    }

    {
        const auto kv = KeyValues::fromString(memory, "VertexLitGeneric", "$baseTexture detail/dt_metal1 $additive 1 $envmap editor/cube_vertigo");
        kv->setString(memory, "$color", "[.05 .05 .05]");
        glass = interfaces->materialSystem->createMaterial("glass", kv);
    }

    {
        const auto kv = KeyValues::fromString(memory, "VertexLitGeneric", "$baseTexture black $bumpmap effects/flat_normal $translucent 1 $envmap models/effects/crystal_cube_vertigo_hdr $envmapfresnel 0 $phong 1 $phongexponent 16 $phongboost 2");
        kv->setString(memory, "$phongtint", "[.2 .35 .6]");
        crystal = interfaces->materialSystem->createMaterial("crystal", kv);
    }

    {
        const auto kv = KeyValues::fromString(memory, "VertexLitGeneric", "$baseTexture white $bumpmap effects/flat_normal $envmap editor/cube_vertigo $envmapfresnel .6 $phong 1 $phongboost 2 $phongexponent 8");
        kv->setString(memory, "$color2", "[.05 .05 .05]");
        kv->setString(memory, "$envmaptint", "[.2 .2 .2]");
        kv->setString(memory, "$phongfresnelranges", "[.7 .8 1]");
        kv->setString(memory, "$phongtint", "[.8 .9 1]");
        silver = interfaces->materialSystem->createMaterial("silver", kv);
    }

    {
        const auto kv = KeyValues::fromString(memory, "VertexLitGeneric", "$baseTexture white $bumpmap effects/flat_normal $envmap editor/cube_vertigo $envmapfresnel .6 $phong 1 $phongboost 6 $phongexponent 128 $phongdisablehalflambert 1");
        kv->setString(memory, "$color2", "[.18 .15 .06]");
        kv->setString(memory, "$envmaptint", "[.6 .5 .2]");
        kv->setString(memory, "$phongfresnelranges", "[.7 .8 1]");
        kv->setString(memory, "$phongtint", "[.6 .5 .2]");
        gold = interfaces->materialSystem->createMaterial("gold", kv);
    }

    {
        const auto kv = KeyValues::fromString(memory, "VertexLitGeneric", "$baseTexture black $bumpmap models/inventory_items/trophy_majors/matte_metal_normal $additive 1 $envmap editor/cube_vertigo $envmapfresnel 1 $normalmapalphaenvmapmask 1 $phong 1 $phongboost 20 $phongexponent 3000 $phongdisablehalflambert 1");
        kv->setString(memory, "$phongfresnelranges", "[.1 .4 1]");
        kv->setString(memory, "$phongtint", "[.8 .9 1]");
        plastic = interfaces->materialSystem->createMaterial("plastic", kv);
    }
}

void Chams::updateInput(Config& config) noexcept
{
    config.chamsToggleKey.handleToggle();
}

bool Chams::render(const Memory& memory, Config& config, void* ctx, void* state, const ModelRenderInfo& info, matrix3x4* customBoneToWorld) noexcept
{
    if (config.chamsToggleKey.isSet()) {
        if (!config.chamsToggleKey.isToggled() && !config.chamsHoldKey.isDown())
            return false;
    } else if (config.chamsHoldKey.isSet() && !config.chamsHoldKey.isDown()) {
        return false;
    }

    static bool materialsInitialized = false;
    if (!materialsInitialized) {
        initializeMaterials(memory);
        materialsInitialized = true;
    }

    appliedChams = false;
    this->ctx = ctx;
    this->state = state;
    this->info = &info;
    this->customBoneToWorld = customBoneToWorld;

    if (std::string_view{ info.model->name }.starts_with("models/weapons/v_")) {
        // info.model->name + 17 -> small optimization, skip "models/weapons/v_"
        if (std::strstr(info.model->name + 17, "sleeve"))
            renderSleeves(memory, config);
        else if (std::strstr(info.model->name + 17, "arms"))
            renderHands(memory, config);
        else if (!std::strstr(info.model->name + 17, "tablet")
            && !std::strstr(info.model->name + 17, "parachute")
            && !std::strstr(info.model->name + 17, "fists"))
            renderWeapons(memory, config);
    } else {
        const auto entity = interfaces->entityList->getEntity(info.entityIndex);
        if (entity && !entity->isDormant() && entity->isPlayer())
            renderPlayer(memory, config, entity);
    }

    return appliedChams;
}

void Chams::renderPlayer(const Memory& memory, Config& config, Entity* player) noexcept
{
    if (!localPlayer)
        return;

    const auto health = player->health();

    if (const auto activeWeapon = player->getActiveWeapon(); activeWeapon && activeWeapon->getClientClass()->classId == ClassId::C4 && activeWeapon->c4StartedArming() && std::ranges::any_of(config.chams["Planting"].materials, [](const Config::Chams::Material& mat) { return mat.enabled; })) {
        applyChams(memory, config.chams["Planting"].materials, health);
    } else if (player->isDefusing() && std::ranges::any_of(config.chams["Defusing"].materials, [](const Config::Chams::Material& mat) { return mat.enabled; })) {
        applyChams(memory, config.chams["Defusing"].materials, health);
    } else if (player == localPlayer.get()) {
        applyChams(memory, config.chams["Local player"].materials, health);
    } else if (localPlayer->isOtherEnemy(memory, player)) {
        applyChams(memory, config.chams["Enemies"].materials, health);

        const auto records = Backtrack::getRecords(player->index());
        if (records && !records->empty() && Backtrack::valid(memory, records->front().simulationTime)) {
            if (!appliedChams)
                hooks->modelRender.callOriginal<void, 21>(ctx, state, info, customBoneToWorld);
            applyChams(memory, config.chams["Backtrack"].materials, health, records->back().matrix);
            interfaces->studioRender->forcedMaterialOverride(nullptr);
        }
    } else {
        applyChams(memory, config.chams["Allies"].materials, health);
    }
}

void Chams::renderWeapons(const Memory& memory, Config& config) noexcept
{
    if (!localPlayer || !localPlayer->isAlive() || localPlayer->isScoped())
        return;

    applyChams(memory, config.chams["Weapons"].materials, localPlayer->health());
}

void Chams::renderHands(const Memory& memory, Config& config) noexcept
{
    if (!localPlayer || !localPlayer->isAlive())
        return;

    applyChams(memory, config.chams["Hands"].materials, localPlayer->health());
}

void Chams::renderSleeves(const Memory& memory, Config& config) noexcept
{
    if (!localPlayer || !localPlayer->isAlive())
        return;

    applyChams(memory, config.chams["Sleeves"].materials, localPlayer->health());
}

void Chams::applyChams(const Memory& memory, const std::array<Config::Chams::Material, 7>& chams, int health, const matrix3x4* customMatrix) noexcept
{
    for (const auto& cham : chams) {
        if (!cham.enabled || !cham.ignorez)
            continue;

        const auto material = dispatchMaterial(cham.material);
        if (!material)
            continue;
        
        float r, g, b;
        if (cham.healthBased && health) {
            Helpers::healthColor(std::clamp(health / 100.0f, 0.0f, 1.0f), r, g, b);
        } else if (cham.rainbow) {
            std::tie(r, g, b) = rainbowColor(memory, cham.rainbowSpeed);
        } else {
            r = cham.color[0];
            g = cham.color[1];
            b = cham.color[2];
        }

        if (material == glow || material == chrome || material == plastic || material == glass || material == crystal)
            material->findVar("$envmaptint")->setVectorValue(r, g, b);
        else
            material->colorModulate(r, g, b);

        const auto pulse = cham.color[3] * (cham.blinking ? std::sin(memory.globalVars->currenttime * 5) * 0.5f + 0.5f : 1.0f);

        if (material == glow)
            material->findVar("$envmapfresnelminmaxexp")->setVecComponentValue(9.0f * (1.2f - pulse), 2);
        else
            material->alphaModulate(pulse);

        material->setMaterialVarFlag(MaterialVarFlag::IGNOREZ, true);
        material->setMaterialVarFlag(MaterialVarFlag::WIREFRAME, cham.wireframe);
        interfaces->studioRender->forcedMaterialOverride(material);
        hooks->modelRender.callOriginal<void, 21>(ctx, state, info, customMatrix ? customMatrix : customBoneToWorld);
        interfaces->studioRender->forcedMaterialOverride(nullptr);
    }

    for (const auto& cham : chams) {
        if (!cham.enabled || cham.ignorez)
            continue;

        const auto material = dispatchMaterial(cham.material);
        if (!material)
            continue;

        float r, g, b;
        if (cham.healthBased && health) {
            Helpers::healthColor(std::clamp(health / 100.0f, 0.0f, 1.0f), r, g, b);
        } else if (cham.rainbow) {
            std::tie(r, g, b) = rainbowColor(memory, cham.rainbowSpeed);
        } else {
            r = cham.color[0];
            g = cham.color[1];
            b = cham.color[2];
        }

        if (material == glow || material == chrome || material == plastic || material == glass || material == crystal)
            material->findVar("$envmaptint")->setVectorValue(r, g, b);
        else
            material->colorModulate(r, g, b);

        const auto pulse = cham.color[3] * (cham.blinking ? std::sin(memory.globalVars->currenttime * 5) * 0.5f + 0.5f : 1.0f);

        if (material == glow)
            material->findVar("$envmapfresnelminmaxexp")->setVecComponentValue(9.0f * (1.2f - pulse), 2);
        else
            material->alphaModulate(pulse);

        if (cham.cover && !appliedChams)
            hooks->modelRender.callOriginal<void, 21>(ctx, state, info, customMatrix ? customMatrix : customBoneToWorld);

        material->setMaterialVarFlag(MaterialVarFlag::IGNOREZ, false);
        material->setMaterialVarFlag(MaterialVarFlag::WIREFRAME, cham.wireframe);
        interfaces->studioRender->forcedMaterialOverride(material);
        hooks->modelRender.callOriginal<void, 21>(ctx, state, info, customMatrix ? customMatrix : customBoneToWorld);
        appliedChams = true;
    }
}
