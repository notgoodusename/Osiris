#include "WeaponNames.h"

#include <SDK/ItemSchema.h>
#include <SDK/Localize.h>

#include <Helpers.h>
#include <Interfaces.h>

namespace inventory_changer
{

[[nodiscard]] static WeaponNames createWeaponNamesStorage(const Interfaces& interfaces, ItemSchema* itemSchema)
{
    WeaponNames storage;
    ToUtf8Converter converter{ interfaces.getLocalize() };
    Helpers::ToUpperConverter toUpperConverter;

    for (const auto& node : itemSchema->getPOD()->itemsSorted) {
        const auto item = EconItemDefinition{ retSpoofGadgets.jmpEbxInClient, node.value };
        const auto nameWide = interfaces.getLocalize().findSafe(item.getItemBaseName());
        storage.add(item.getWeaponId(), converter.convertUnicodeToAnsi(nameWide), toUpperConverter.toUpper(nameWide));
    }
    return storage;
}

const WeaponNames& WeaponNames::instance(const Interfaces& interfaces, const Memory& memory)
{
    static const WeaponNames weaponNames{ createWeaponNamesStorage(interfaces, memory.itemSystem()->getItemSchema()) };
    return weaponNames;
}

}
