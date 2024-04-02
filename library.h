#ifndef FINITE_FIELDS_LIBRARY_H
#define FINITE_FIELDS_LIBRARY_H


typedef struct
{
    u8 mod;
    u8 poly_deg;
    u8 *irred_poly;
} Field;

typedef struct
{
    Field *field;
    u8 deg;
    u8 *poly;
} FieldMember;

Field *getField(u8 mod, const u8 *poly, u8 poly_deg);

FieldMember *getZero(Field *field);

FieldMember *getIdentity(Field *field);

FieldMember *fieldMemberInit(Field *field, const u8 *poly, u8 poly_deg);

FieldMember *fieldMemberCopy(FieldMember *elem);

FieldMember *ffAdd(const FieldMember *left, const FieldMember *right);

FieldMember *ffSub(const FieldMember *left, const FieldMember *right);

FieldMember *ffMul(const FieldMember *left, const FieldMember *right);

FieldMember *ffDiv(const FieldMember *left, FieldMember *right);

FieldMember *ffInv(FieldMember *elem); // обратное

FieldMember *ffNeg(const FieldMember *elem); // противоположное

FieldMember *u8_to_ff(u8 elem);

FieldMember *u16_to_ff(u16 elem);

FieldMember *u32_to_ff(u32 elem);

u8 ff_to_uint8(const FieldMember *elem);

u16 ff_to_uint16(const FieldMember *elem);

u32 ff_to_uint32(const FieldMember *elem);

void freeField(Field *field);

_Bool fieldsAreEqual(const Field *left, const Field *right);

_Bool fieldMembersAreEqual(const FieldMember *left, const FieldMember *right);

void freeFieldMember(FieldMember *mem, _Bool delete_field);

#endif //FINITE_FIELDS_LIBRARY_H
