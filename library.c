#include <linux/slab.h>
#include <linux/string.h>
#include "library.h"

#define UINT8_MAX 255

Field *getField(uint8_t mod, const uint8_t *poly, uint8_t poly_deg)
{
    Field *newField = (Field *)kmalloc(sizeof(Field), GFP_KERNEL);
    if (newField == NULL) return NULL;
    newField->mod = mod;
    newField->poly_deg = poly_deg;
    newField->irred_poly = (uint8_t *)kmalloc((poly_deg + 1) * sizeof(uint8_t), GFP_KERNEL);
    if (newField->irred_poly == NULL)
    {
        kfree(newField);
        return NULL;
    }
    memcpy(newField->irred_poly, poly, poly_deg + 1);
    return newField;
}

void freeField(Field *field)
{
    if (field == NULL) return;
    kfree(field->irred_poly);
    kfree(field);
}

void freeFieldMember(FieldMember *mem, bool delete_field)
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
    if (zero == NULL) return NULL;
    zero->field = field;
    zero->poly = (uint8_t *)kmalloc(field->poly_deg * sizeof(uint8_t), GFP_KERNEL);
    if (zero->poly == NULL)
    {
        kfree(zero);
        return NULL;
    }
    memset(zero->poly, 0, field->poly_deg);
    return zero;
}

FieldMember *getIdentity(Field *field)
{
    if (field == NULL) return NULL;
    FieldMember *identity = getZero(field);
    identity->poly[0] = 1;
    return identity;
}

uint8_t ff_to_uint8(const FieldMember *elem)
{
    if (elem == NULL || elem->field->mod != 2 || elem->field->poly_deg != 8) return 0;
    uint8_t num = 0;
    for (uint8_t i = 0; i < elem->field->poly_deg; ++i)
    {
        num |= elem->poly[i] << i;
    }
    return num;
}

uint16_t ff_to_uint16(const FieldMember *elem)
{
    if (elem == NULL || elem->field->mod != 2 || elem->field->poly_deg != 16) return 0;
    uint16_t num = 0;
    for (uint8_t i = 0; i < elem->field->poly_deg; ++i)
    {
        num |= elem->poly[i] << i;
    }
    return num;
}

uint32_t ff_to_uint32(const FieldMember *elem)
{
    if (elem == NULL || elem->field->mod != 2 || elem->field->poly_deg != 32) return 0;
    uint32_t num = 0;
    for (uint8_t i = 0; i < elem->field->poly_deg; ++i)
    {
        num |= elem->poly[i] << i;
    }
    return num;
}

FieldMember *uint8_to_ff(uint8_t elem)
{
    static const uint8_t poly[] = {1,0,1,1,1,0,0,0,1};
    Field *field = getField(2,poly,8);
    FieldMember *mem = getZero(field);
    for (uint8_t i = 0; i < mem->field->poly_deg; ++i)
    {
        mem->poly[i] = elem % 2;
        elem /= 2;
    }
    return mem;
}

FieldMember *uint16_to_ff(uint16_t elem)
{
    static const uint8_t poly[] = {1,1,0,1,0,0,0,0,
                       0,0,0,0,1,0,0,0,1};
    Field *field = getField(2,poly,16);
    FieldMember *mem = getZero(field);
    for (uint8_t i = 0; i < mem->field->poly_deg; ++i)
    {
        mem->poly[i] = elem % 2;
        elem /= 2;
    }
    return mem;
}

FieldMember *uint32_to_ff(uint32_t elem)
{
    static const uint8_t poly[] = {1,1,1,0,0,0,0,0,
                      0,0,0,0,0,0,0,0,
                      0,0,0,0,0,0,1,0,
                      0,0,0,0,0,0,0,0,1};
    Field *field = getField(2,poly,32);
    FieldMember *mem = getZero(field);
    for (uint8_t i = 0; i < mem->field->poly_deg; ++i)
    {
        mem->poly[i] = elem % 2;
        elem /= 2;
    }
    return mem;
}

FieldMember *fieldMemberInit(Field *field, const uint8_t *poly, uint8_t poly_deg)
{
    if (poly_deg == 255) return getZero(field);
    if (field == NULL || poly == NULL || field->poly_deg <= poly_deg) return NULL;
    FieldMember *member = getZero(field);
    for (uint8_t i = 0; i < field->poly_deg; ++i)
    {
        member->poly[i] = i <= poly_deg ? poly[i] % field->mod : 0;
    }
    return member;
}

bool fieldsAreEqual(const Field *left, const Field *right)
{
    if (left == NULL || right == NULL || left->mod != right->mod || left->poly_deg != right->poly_deg) return false;
    for (uint8_t i = 0; i <= left->poly_deg; ++i)
    {
        if (left->irred_poly[i] != right->irred_poly[i]) return false;
    }
    return true;
}

bool fieldMembersAreEqual(const FieldMember *left, const FieldMember *right)
{
    if (left == NULL || right == NULL
        || !fieldsAreEqual(left->field, right->field)) return false;
    for (uint8_t i = 0; i < left->field->poly_deg; ++i)
    {
        if (left->poly[i] != right->poly[i]) return false;
    }
    return true;
}

FieldMember *ffAdd(const FieldMember *left, const FieldMember *right)
{
    if (left == NULL || right == NULL || !fieldsAreEqual(left->field, right->field)) return NULL;
    FieldMember *result = getZero(left->field);
    for (uint8_t i = 0; i < left->field->poly_deg; ++i)
    {
        result->poly[i] = (left->poly[i] + right->poly[i]) % left->field->mod;
    }
    return result;
}

FieldMember *ffNeg(const FieldMember *elem)
{
    if (elem == NULL || elem->field == NULL) return NULL;
    FieldMember *result = getZero(elem->field);
    for (uint8_t i = 0; i < elem->field->poly_deg; ++i)
    {
        result->poly[i] = (elem->field->mod - elem->poly[i]) % elem->field->mod;
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
    memcpy(copy->poly,elem->poly,copy->field->poly_deg);
    return copy;
}

static inline void calcRealDeg(const uint8_t *poly, uint8_t *poly_deg)
{
    while (*poly_deg != UINT8_MAX && poly[*poly_deg] == 0)
    {
        (*poly_deg)--;
    }
}

static FieldMember *takeMod(const uint8_t *poly, uint8_t poly_deg, Field *field)
{
    uint8_t *res_poly = (uint8_t *)kmalloc((poly_deg + 1) * sizeof(uint8_t), GFP_KERNEL);
    if (res_poly == NULL) return NULL;
    memcpy(res_poly, poly, poly_deg + 1);
    uint8_t res_deg = poly_deg;

    while (res_deg >= field->poly_deg)
    {
        uint8_t leading_coefficient = res_poly[res_deg] / field->irred_poly[field->poly_deg];
        for (uint8_t i = 0; i <= field->poly_deg; ++i)
        {
            uint8_t neg_sub = (field->mod - (leading_coefficient * field->irred_poly[i]) % field->mod)
                              % field->mod;
            res_poly[res_deg - field->poly_deg + i] = (res_poly[res_deg - field->poly_deg + i] + neg_sub)
                                                      % field->mod;
        }
        calcRealDeg(res_poly, &res_deg);
    }
    FieldMember *result = fieldMemberInit(field,res_poly,res_deg);
    kfree(res_poly);
    return result;
}

FieldMember *ffMul(const FieldMember *left, const FieldMember *right)
{
    if (left == NULL || right == NULL || !fieldsAreEqual(left->field, right->field)) return NULL;
    FieldMember *nil = getZero(left->field);
    if (fieldMembersAreEqual(left, nil) || fieldMembersAreEqual(right, nil)) return nil;
    uint8_t res_deg = left->field->poly_deg * 2 - 2;
    uint8_t *res_poly = (uint8_t *)kmalloc((res_deg + 1) * sizeof(uint8_t), GFP_KERNEL);
    memset(res_poly,0,(res_deg + 1));
    for (size_t i = 0; i < left->field->poly_deg; ++i)
    {
        for (size_t j = 0; j < right->field->poly_deg; ++j)
        {
            res_poly[i + j] = (res_poly[i + j] + (left->poly[i] * right->poly[j]) % left->field->mod) % left->field->mod;
        }
    }
    FieldMember *result = takeMod(res_poly, res_deg, left->field);
    kfree(res_poly);
    return result;
}

FieldMember *fastPow(FieldMember* elem, uint64_t power)
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

uint64_t fastPowIntegers(uint64_t base, uint8_t power)
{
    uint64_t result = 1;
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
    uint64_t power = fastPowIntegers((uint64_t)elem->field->mod,elem->field->poly_deg) - 2;
    return fastPow(elem,power);
}

FieldMember *ffDiv(const FieldMember *left, FieldMember *right)
{
    FieldMember *right_inv = ffInv(right);
    FieldMember *result = ffMul(left,right_inv);
    freeFieldMember(right_inv,0);
    return result;
}
