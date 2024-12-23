//
// Generated file, do not edit! Created by opp_msgtool 6.0 from IntVector.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wshadow"
#  pragma clang diagnostic ignored "-Wconversion"
#  pragma clang diagnostic ignored "-Wunused-parameter"
#  pragma clang diagnostic ignored "-Wc++98-compat"
#  pragma clang diagnostic ignored "-Wunreachable-code-break"
#  pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wsuggest-attribute=noreturn"
#  pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif

#include <iostream>
#include <sstream>
#include <memory>
#include <type_traits>
#include "IntVector_m.h"

namespace omnetpp {

// Template pack/unpack rules. They are declared *after* a1l type-specific pack functions for multiple reasons.
// They are in the omnetpp namespace, to allow them to be found by argument-dependent lookup via the cCommBuffer argument

// Packing/unpacking an std::vector
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::vector<T,A>& v)
{
    int n = v.size();
    doParsimPacking(buffer, n);
    for (int i = 0; i < n; i++)
        doParsimPacking(buffer, v[i]);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::vector<T,A>& v)
{
    int n;
    doParsimUnpacking(buffer, n);
    v.resize(n);
    for (int i = 0; i < n; i++)
        doParsimUnpacking(buffer, v[i]);
}

// Packing/unpacking an std::list
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::list<T,A>& l)
{
    doParsimPacking(buffer, (int)l.size());
    for (typename std::list<T,A>::const_iterator it = l.begin(); it != l.end(); ++it)
        doParsimPacking(buffer, (T&)*it);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::list<T,A>& l)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        l.push_back(T());
        doParsimUnpacking(buffer, l.back());
    }
}

// Packing/unpacking an std::set
template<typename T, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::set<T,Tr,A>& s)
{
    doParsimPacking(buffer, (int)s.size());
    for (typename std::set<T,Tr,A>::const_iterator it = s.begin(); it != s.end(); ++it)
        doParsimPacking(buffer, *it);
}

template<typename T, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::set<T,Tr,A>& s)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        T x;
        doParsimUnpacking(buffer, x);
        s.insert(x);
    }
}

// Packing/unpacking an std::map
template<typename K, typename V, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::map<K,V,Tr,A>& m)
{
    doParsimPacking(buffer, (int)m.size());
    for (typename std::map<K,V,Tr,A>::const_iterator it = m.begin(); it != m.end(); ++it) {
        doParsimPacking(buffer, it->first);
        doParsimPacking(buffer, it->second);
    }
}

template<typename K, typename V, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::map<K,V,Tr,A>& m)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        K k; V v;
        doParsimUnpacking(buffer, k);
        doParsimUnpacking(buffer, v);
        m[k] = v;
    }
}

// Default pack/unpack function for arrays
template<typename T>
void doParsimArrayPacking(omnetpp::cCommBuffer *b, const T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimPacking(b, t[i]);
}

template<typename T>
void doParsimArrayUnpacking(omnetpp::cCommBuffer *b, T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimUnpacking(b, t[i]);
}

// Default rule to prevent compiler from choosing base class' doParsimPacking() function
template<typename T>
void doParsimPacking(omnetpp::cCommBuffer *, const T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimPacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

template<typename T>
void doParsimUnpacking(omnetpp::cCommBuffer *, T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimUnpacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

}  // namespace omnetpp

Register_Class(IntVector)

IntVector::IntVector(const char *name) : ::omnetpp::cOwnedObject(name)
{
}

IntVector::IntVector(const IntVector& other) : ::omnetpp::cOwnedObject(other)
{
    copy(other);
}

IntVector::~IntVector()
{
    delete [] this->val;
}

IntVector& IntVector::operator=(const IntVector& other)
{
    if (this == &other) return *this;
    ::omnetpp::cOwnedObject::operator=(other);
    copy(other);
    return *this;
}

void IntVector::copy(const IntVector& other)
{
    delete [] this->val;
    this->val = (other.val_arraysize==0) ? nullptr : new int[other.val_arraysize];
    val_arraysize = other.val_arraysize;
    for (size_t i = 0; i < val_arraysize; i++) {
        this->val[i] = other.val[i];
    }
}

void IntVector::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cOwnedObject::parsimPack(b);
    b->pack(val_arraysize);
    doParsimArrayPacking(b,this->val,val_arraysize);
}

void IntVector::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cOwnedObject::parsimUnpack(b);
    delete [] this->val;
    b->unpack(val_arraysize);
    if (val_arraysize == 0) {
        this->val = nullptr;
    } else {
        this->val = new int[val_arraysize];
        doParsimArrayUnpacking(b,this->val,val_arraysize);
    }
}

size_t IntVector::getValArraySize() const
{
    return val_arraysize;
}

int IntVector::getVal(size_t k) const
{
    if (k >= val_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)val_arraysize, (unsigned long)k);
    return this->val[k];
}

void IntVector::setValArraySize(size_t newSize)
{
    int *val2 = (newSize==0) ? nullptr : new int[newSize];
    size_t minSize = val_arraysize < newSize ? val_arraysize : newSize;
    for (size_t i = 0; i < minSize; i++)
        val2[i] = this->val[i];
    for (size_t i = minSize; i < newSize; i++)
        val2[i] = 0;
    delete [] this->val;
    this->val = val2;
    val_arraysize = newSize;
}

void IntVector::setVal(size_t k, int val)
{
    if (k >= val_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)val_arraysize, (unsigned long)k);
    this->val[k] = val;
}

void IntVector::insertVal(size_t k, int val)
{
    if (k > val_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)val_arraysize, (unsigned long)k);
    size_t newSize = val_arraysize + 1;
    int *val2 = new int[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        val2[i] = this->val[i];
    val2[k] = val;
    for (i = k + 1; i < newSize; i++)
        val2[i] = this->val[i-1];
    delete [] this->val;
    this->val = val2;
    val_arraysize = newSize;
}

void IntVector::appendVal(int val)
{
    insertVal(val_arraysize, val);
}

void IntVector::eraseVal(size_t k)
{
    if (k >= val_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)val_arraysize, (unsigned long)k);
    size_t newSize = val_arraysize - 1;
    int *val2 = (newSize == 0) ? nullptr : new int[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        val2[i] = this->val[i];
    for (i = k; i < newSize; i++)
        val2[i] = this->val[i+1];
    delete [] this->val;
    this->val = val2;
    val_arraysize = newSize;
}

class IntVectorDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_val,
    };
  public:
    IntVectorDescriptor();
    virtual ~IntVectorDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(IntVectorDescriptor)

IntVectorDescriptor::IntVectorDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(IntVector)), "omnetpp::cOwnedObject")
{
    propertyNames = nullptr;
}

IntVectorDescriptor::~IntVectorDescriptor()
{
    delete[] propertyNames;
}

bool IntVectorDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<IntVector *>(obj)!=nullptr;
}

const char **IntVectorDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *IntVectorDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int IntVectorDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 1+base->getFieldCount() : 1;
}

unsigned int IntVectorDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISARRAY | FD_ISEDITABLE | FD_ISRESIZABLE,    // FIELD_val
    };
    return (field >= 0 && field < 1) ? fieldTypeFlags[field] : 0;
}

const char *IntVectorDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "val",
    };
    return (field >= 0 && field < 1) ? fieldNames[field] : nullptr;
}

int IntVectorDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "val") == 0) return baseIndex + 0;
    return base ? base->findField(fieldName) : -1;
}

const char *IntVectorDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",    // FIELD_val
    };
    return (field >= 0 && field < 1) ? fieldTypeStrings[field] : nullptr;
}

const char **IntVectorDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *IntVectorDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int IntVectorDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    IntVector *pp = omnetpp::fromAnyPtr<IntVector>(object); (void)pp;
    switch (field) {
        case FIELD_val: return pp->getValArraySize();
        default: return 0;
    }
}

void IntVectorDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    IntVector *pp = omnetpp::fromAnyPtr<IntVector>(object); (void)pp;
    switch (field) {
        case FIELD_val: pp->setValArraySize(size); break;
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'IntVector'", field);
    }
}

const char *IntVectorDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    IntVector *pp = omnetpp::fromAnyPtr<IntVector>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string IntVectorDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    IntVector *pp = omnetpp::fromAnyPtr<IntVector>(object); (void)pp;
    switch (field) {
        case FIELD_val: return long2string(pp->getVal(i));
        default: return "";
    }
}

void IntVectorDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    IntVector *pp = omnetpp::fromAnyPtr<IntVector>(object); (void)pp;
    switch (field) {
        case FIELD_val: pp->setVal(i,string2long(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'IntVector'", field);
    }
}

omnetpp::cValue IntVectorDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    IntVector *pp = omnetpp::fromAnyPtr<IntVector>(object); (void)pp;
    switch (field) {
        case FIELD_val: return pp->getVal(i);
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'IntVector' as cValue -- field index out of range?", field);
    }
}

void IntVectorDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    IntVector *pp = omnetpp::fromAnyPtr<IntVector>(object); (void)pp;
    switch (field) {
        case FIELD_val: pp->setVal(i,omnetpp::checked_int_cast<int>(value.intValue())); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'IntVector'", field);
    }
}

const char *IntVectorDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

omnetpp::any_ptr IntVectorDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    IntVector *pp = omnetpp::fromAnyPtr<IntVector>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void IntVectorDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    IntVector *pp = omnetpp::fromAnyPtr<IntVector>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'IntVector'", field);
    }
}

namespace omnetpp {

}  // namespace omnetpp

