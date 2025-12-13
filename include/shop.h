#ifndef GUARD_SHOP_H
#define GUARD_SHOP_H

extern struct ItemSlot gMartPurchaseHistory[3];

void CreatePokemartMenu(const u16 *itemsForSale);
void CreateDecorationShop1Menu(const u16 *itemsForSale);
void CreateDecorationShop2Menu(const u16 *itemsForSale);
void CB2_ExitSellMenu(void);

#if TESTING
bool32 InPokemartMenu(void);
const u8 *PokemartMenu_ItemText(u32 index);
#endif

#endif // GUARD_SHOP_H
