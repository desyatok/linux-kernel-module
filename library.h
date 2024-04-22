#ifndef FINITE_FIELDS_LIBRARY_H
#define FINITE_FIELDS_LIBRARY_H

typedef struct
{
    uint8_t mod;
    uint8_t poly_deg;
    uint8_t *irred_poly;
} Field;

typedef struct
{
    Field *field;
    uint8_t *poly;
} FieldMember;

Field *getField(uint8_t mod, const uint8_t *poly, uint8_t poly_deg);

FieldMember *getZero(Field *field);

FieldMember *getIdentity(Field *field);

FieldMember *fieldMemberInit(Field *field, const uint8_t *poly, uint8_t poly_deg);

FieldMember *fieldMemberCopy(FieldMember *elem);

FieldMember *ffAdd(const FieldMember *left, const FieldMember *right);

FieldMember *ffSub(const FieldMember *left, const FieldMember *right);

FieldMember *ffMul(const FieldMember *left, const FieldMember *right);

FieldMember *ffDiv(const FieldMember *left, FieldMember *right);

FieldMember *ffInv(FieldMember *elem); // обратное

FieldMember *ffNeg(const FieldMember *elem); // противоположное

FieldMember *uint8_to_ff(uint8_t elem);

FieldMember *uint16_to_ff(uint16_t elem);

FieldMember *uint32_to_ff(uint32_t elem);

uint8_t ff_to_uint8(const FieldMember *elem);

uint16_t ff_to_uint16(const FieldMember *elem);

uint32_t ff_to_uint32(const FieldMember *elem);

void freeField(Field *field);

bool fieldsAreEqual(const Field *left, const Field *right);

bool fieldMembersAreEqual(const FieldMember *left, const FieldMember *right);

void freeFieldMember(FieldMember *mem, bool delete_field);

#endif //FINITE_FIELDS_LIBRARY_H
