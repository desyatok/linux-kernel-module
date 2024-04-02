#include <linux/slab.h>
#include "library.h"

#define UINT8_MAX 255
#define MYMAX(a, b) (a) > (b) ? (a) : (b)

Field *getField(u8 mod, const u8 *poly, u8 poly_deg)
{
    Field *newField = (Field *)kmalloc(sizeof(Field), GFP_KERNEL);
    newField->mod = mod;
    newField->poly_deg = poly_deg;
    newField->irred_poly = (u8 *)kmalloc((poly_deg + 1) * sizeof(u8), GFP_KERNEL);
    for (size_t i = 0; i <= poly_deg; ++i)
    {
        newField->irred_poly[i] = poly[i];
    }
    return newField;
}

void freeField(Field *field)
{
    if (field == NULL) return;
    kfree(field->irred_poly);
    kfree(field);
}

void freeFieldMember(FieldMember *mem, _Bool delete_field)
{
    if (mem == NULL) return;
    if (delete_field)
    {
        freeField(mem->field);
    }
    kfree(mem->poly);
    kfree(mem);
}

FieldMember *getZero(Field *field)
{
    if (field == NULL) return NULL;
    FieldMember *zero = (FieldMember *)kmalloc(sizeof(FieldMember), GFP_KERNEL);
    zero->field = field;
    zero->deg = UINT8_MAX;
    zero->poly = (u8 *)kmalloc((field->poly_deg + 1) * sizeof(u8), GFP_KERNEL);
    for (u8 i = 0; i <= field->poly_deg; ++i)
    {
        zero->poly[i] = 0;
    }
    return zero;
}

FieldMember *getIdentity(Field *field)
{
    if (field == NULL) return NULL;
    FieldMember *identity = (FieldMember *)kmalloc(sizeof(FieldMember), GFP_KERNEL);
    identity->field = field;
    identity->deg = 0;
    identity->poly = (u8 *)kmalloc((field->poly_deg + 1) * sizeof(u8), GFP_KERNEL);
    for (u8 i = 1; i <= field->poly_deg; ++i)
    {
        identity->poly[i] = 0;
    }
    identity->poly[0] = 1;
    return identity;
}

u8 ff_to_uint8(const FieldMember *elem)
{
    if (elem == NULL || elem->field->mod != 2 || elem->field->poly_deg != 8) return 0;
    u8 num = 0;
    for (u8 i = 0; i < elem->field->poly_deg; ++i)
    {
        num |= elem->poly[i] << i;
    }
    return num;
}

u16 ff_to_uint16(const FieldMember *elem)
{
    if (elem == NULL || elem->field->mod != 2 || elem->field->poly_deg != 16) return 0;
    u16 num = 0;
    for (u8 i = 0; i < elem->field->poly_deg; ++i)
    {
        num |= elem->poly[i] << i;
    }
    return num;
}

u32 ff_to_uint32(const FieldMember *elem)
{
    if (elem == NULL || elem->field->mod != 2 || elem->field->poly_deg != 32) return 0;
    u32 num = 0;
    for (u8 i = 0; i < elem->field->poly_deg; ++i)
    {
        num |= elem->poly[i] << i;
    }
    return num;
}

FieldMember *u8_to_ff(u8 elem)
{
    u8 poly[] = {1,0,1,1,1,0,0,0,1};
    Field *field = getField(2,poly,8);
    FieldMember *mem = getZero(field);
    for (u8 i = 0; i < mem->field->poly_deg; ++i)
    {
        mem->poly[i] = elem % 2;
        elem /= 2;
        if (mem->poly[i] != 0) mem->deg = i;
    }
    return mem;
}

FieldMember *u16_to_ff(u16 elem)
{
    u8 poly[] = {1,1,0,1,0,0,0,0,
                       0,0,0,0,1,0,0,0, 1};
    Field *field = getField(2,poly,16);
    FieldMember *mem = getZero(field);
    for (u8 i = 0; i < mem->field->poly_deg; ++i)
    {
        mem->poly[i] = elem % 2;
        elem /= 2;
        if (mem->poly[i] != 0) mem->deg = i;
    }
    return mem;
}

FieldMember *u32_to_ff(u32 elem)
{
    u8 poly[] = {1,1,1,0,0,0,0,0,
                      0,0,0,0,0,0,0,0,
                      0,0,0,0,0,0,1,0,
                      0,0,0,0,0,0,0,0,1};
    Field *field = getField(2,poly,32);
    FieldMember *mem = getZero(field);
    for (u8 i = 0; i < mem->field->poly_deg; ++i)
    {
        mem->poly[i] = elem % 2;
        elem /= 2;
        if (mem->poly[i] != 0) mem->deg = i;
    }
    return mem;
}

FieldMember *fieldMemberInit(Field *field, const u8 *poly, u8 poly_deg)
{
    if (field == NULL || poly == NULL || field->poly_deg <= poly_deg) return NULL;
    FieldMember *member = getZero(field);
    for (u8 i = 0; i <= field->poly_deg; ++i)
    {
        member->poly[i] = i <= poly_deg ? poly[i] % field->mod : 0;
        if (member->poly[i] != 0) member->deg = i;
    }
    return member;
}

_Bool fieldsAreEqual(const Field *left, const Field *right)
{
    if (left == NULL || right == NULL || left->mod != right->mod || left->poly_deg != right->poly_deg) return 0;
    for (u8 i = 0; i <= left->poly_deg; ++i)
    {
        if (left->irred_poly[i] != right->irred_poly[i]) return 0;
    }
    return 1;
}

_Bool fieldMembersAreEqual(const FieldMember *left, const FieldMember *right)
{
    if (left == NULL || right == NULL
        || !fieldsAreEqual(left->field, right->field)
        || left->deg != right->deg) return 0;
    for (u8 i = 0; i <= left->field->poly_deg; ++i)
    {
        if (left->poly[i] != right->poly[i]) return 0;
    }
    return 1;
}

FieldMember *ffAdd(const FieldMember *left, const FieldMember *right)
{
    if (left == NULL || right == NULL || !fieldsAreEqual(left->field, right->field)) return NULL;
    FieldMember *result = getZero(left->field);
    u8 iterations = MYMAX(left->deg, right->deg);
    for (u8 i = 0; i <= iterations; ++i)
    {
        result->poly[i] = (left->poly[i] + right->poly[i]) % left->field->mod;
        if (result->poly[i] != 0) result->deg = i;
    }
    return result;
}

FieldMember *ffNeg(const FieldMember *elem)
{
    if (elem == NULL || elem->field == NULL) return NULL;
    FieldMember *result = getZero(elem->field);
    for (u8 i = 0; i <= elem->deg; ++i)
    {
        result->poly[i] = (elem->field->mod - elem->poly[i]) % elem->field->mod;
        if (result->poly[i] != 0) result->deg = i;
    }
    return result;
}

FieldMember *ffSub(const FieldMember *left, const FieldMember *right)
{
    FieldMember *neg_right = ffNeg(right);
    FieldMember *result = ffAdd(left, neg_right);
    freeFieldMember(neg_right, 0);
    return result;
}

FieldMember *fieldMemberCopy(FieldMember *elem)
{
    if (elem == NULL || elem->field == NULL) return NULL;
    FieldMember *copy = getZero(elem->field);
    copy->deg = elem->deg;
    for (u8 i = 0; i <= copy->deg; ++i)
    {
        copy->poly[i] = elem->poly[i];
    }
    return copy;
}

FieldMember *takeMod(const u8 *left, u8 left_deg, Field *field)
{
    u8 *res_poly = (u8 *)kmalloc((left_deg + 1) * sizeof(u8), GFP_KERNEL);
    for (u8 i = 0; i <= left_deg; ++i)
    {
        res_poly[i] = left[i];
    }
    u8 res_deg = left_deg;

    while (res_deg >= field->poly_deg)
    {
        u8 leading_coefficient = res_poly[res_deg] / field->irred_poly[field->poly_deg];
        for (u8 i = 0; i <= field->poly_deg; ++i)
        {
            u8 neg_sub = (field->mod - (leading_coefficient * field->irred_poly[i]) % field->mod)
                              % field->mod;
            res_poly[res_deg - field->poly_deg + i] = (res_poly[res_deg - field->poly_deg + i] + neg_sub)
                                                      % field->mod;
        }
        while (res_deg != UINT8_MAX && res_poly[res_deg] == 0)
        {
            res_deg--;
        }
    }
    FieldMember *result = fieldMemberInit(field,res_poly,res_deg);
    kfree(res_poly);
    return result;
}

FieldMember *ffMul(const FieldMember *left, const FieldMember *right)
{
    if (left == NULL || right == NULL || !fieldsAreEqual(left->field, right->field)) return NULL;
    if (right->deg == UINT8_MAX || left->deg == UINT8_MAX) return getZero(left->field);
    u8 res_deg = left->deg + right->deg;
    u8 *res_poly = (u8 *)kmalloc((res_deg + 1) * sizeof(u8), GFP_KERNEL);
    for (u8 i = 0; i <= res_deg; ++i)
    {
        res_poly[i] = 0;
    }
    u8 product, carry;

    for (size_t i = 0; i <= left->deg; ++i)
    {
        carry = 0;
        for (int j = 0; j <= right->deg; ++j)
        {
            product = (res_poly[i + j]
                       + (i <= left->deg ? (left->poly[i] * right->poly[j]) % left->field->mod : 0)
                       + carry) % left->field->mod;
            res_poly[i + j] = product % 10;
            carry = product / 10;
        }
        if (carry)
        {
            res_poly[i + right->deg] = carry;
        }
    }

    for (u8 i = 0; i <= left->deg + right->deg; ++i)
    {
        if (res_poly[i] != 0) res_deg = i;
    }
    FieldMember *result = takeMod(res_poly, res_deg,left->field);
    kfree(res_poly);
    return result;
}

FieldMember *fastPow(FieldMember* elem, u64 power)
{
    if (elem == NULL || elem->field == NULL) return NULL;
    FieldMember *result = getIdentity(elem->field);
    FieldMember *base = fieldMemberCopy(elem);
    while (power > 0)
    {
        if (power & 1)
        {
            FieldMember *tmp = result;
            result = ffMul(result,base);
            freeFieldMember(tmp,0);
        }
        FieldMember *tmp = base;
        base = ffMul(base,base);
        freeFieldMember(tmp,0);
        power >>= 1;
    }
    freeFieldMember(base,0);
    return result;
}

u64 fastPowIntegers(u64 base, u8 power)
{
    u64 result = 1;
    while (power > 0)
    {
        if (power & 1)
        {
            result *= base;
        }
        base *= base;
        power >>= 1;
    }
    return result;
}

FieldMember *ffInv(FieldMember *elem)
{
    u64 power = fastPowIntegers((u64)elem->field->mod,elem->field->poly_deg) - 2;
    return fastPow(elem,power);
}

FieldMember *ffDiv(const FieldMember *left, FieldMember *right)
{
    FieldMember *right_inv = ffInv(right);
    FieldMember *result = ffMul(left,right_inv);
    freeFieldMember(right_inv,0);
    return result;
}
