//
// Generated file, do not edit! Created by opp_msgtool 6.0 from SourcedMsg.msg.
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
#include "SourcedMsg_m.h"

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

Register_Class(SourcedMsg)

SourcedMsg::SourcedMsg(const char *name, short kind) : ::omnetpp::cPacket(name, kind)
{
}

SourcedMsg::SourcedMsg(const SourcedMsg& other) : ::omnetpp::cPacket(other)
{
    copy(other);
}

SourcedMsg::~SourcedMsg()
{
}

SourcedMsg& SourcedMsg::operator=(const SourcedMsg& other)
{
    if (this == &other) return *this;
    ::omnetpp::cPacket::operator=(other);
    copy(other);
    return *this;
}

void SourcedMsg::copy(const SourcedMsg& other)
{
    this->source = other.source;
    this->ID = other.ID;
    this->time_send = other.time_send;
    this->time_in = other.time_in;
    this->time_queue = other.time_queue;
    this->time_serv = other.time_serv;
    this->time_out = other.time_out;
    this->time_recv = other.time_recv;
}

void SourcedMsg::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cPacket::parsimPack(b);
    doParsimPacking(b,this->source);
    doParsimPacking(b,this->ID);
    doParsimPacking(b,this->time_send);
    doParsimPacking(b,this->time_in);
    doParsimPacking(b,this->time_queue);
    doParsimPacking(b,this->time_serv);
    doParsimPacking(b,this->time_out);
    doParsimPacking(b,this->time_recv);
}

void SourcedMsg::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->source);
    doParsimUnpacking(b,this->ID);
    doParsimUnpacking(b,this->time_send);
    doParsimUnpacking(b,this->time_in);
    doParsimUnpacking(b,this->time_queue);
    doParsimUnpacking(b,this->time_serv);
    doParsimUnpacking(b,this->time_out);
    doParsimUnpacking(b,this->time_recv);
}

int SourcedMsg::getSource() const
{
    return this->source;
}

void SourcedMsg::setSource(int source)
{
    this->source = source;
}

int SourcedMsg::getID() const
{
    return this->ID;
}

void SourcedMsg::setID(int ID)
{
    this->ID = ID;
}

omnetpp::simtime_t SourcedMsg::getTime_send() const
{
    return this->time_send;
}

void SourcedMsg::setTime_send(omnetpp::simtime_t time_send)
{
    this->time_send = time_send;
}

omnetpp::simtime_t SourcedMsg::getTime_in() const
{
    return this->time_in;
}

void SourcedMsg::setTime_in(omnetpp::simtime_t time_in)
{
    this->time_in = time_in;
}

omnetpp::simtime_t SourcedMsg::getTime_queue() const
{
    return this->time_queue;
}

void SourcedMsg::setTime_queue(omnetpp::simtime_t time_queue)
{
    this->time_queue = time_queue;
}

omnetpp::simtime_t SourcedMsg::getTime_serv() const
{
    return this->time_serv;
}

void SourcedMsg::setTime_serv(omnetpp::simtime_t time_serv)
{
    this->time_serv = time_serv;
}

omnetpp::simtime_t SourcedMsg::getTime_out() const
{
    return this->time_out;
}

void SourcedMsg::setTime_out(omnetpp::simtime_t time_out)
{
    this->time_out = time_out;
}

omnetpp::simtime_t SourcedMsg::getTime_recv() const
{
    return this->time_recv;
}

void SourcedMsg::setTime_recv(omnetpp::simtime_t time_recv)
{
    this->time_recv = time_recv;
}

class SourcedMsgDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_source,
        FIELD_ID,
        FIELD_time_send,
        FIELD_time_in,
        FIELD_time_queue,
        FIELD_time_serv,
        FIELD_time_out,
        FIELD_time_recv,
    };
  public:
    SourcedMsgDescriptor();
    virtual ~SourcedMsgDescriptor();

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

Register_ClassDescriptor(SourcedMsgDescriptor)

SourcedMsgDescriptor::SourcedMsgDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(SourcedMsg)), "omnetpp::cPacket")
{
    propertyNames = nullptr;
}

SourcedMsgDescriptor::~SourcedMsgDescriptor()
{
    delete[] propertyNames;
}

bool SourcedMsgDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<SourcedMsg *>(obj)!=nullptr;
}

const char **SourcedMsgDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *SourcedMsgDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int SourcedMsgDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 8+base->getFieldCount() : 8;
}

unsigned int SourcedMsgDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_source
        FD_ISEDITABLE,    // FIELD_ID
        FD_ISEDITABLE,    // FIELD_time_send
        FD_ISEDITABLE,    // FIELD_time_in
        FD_ISEDITABLE,    // FIELD_time_queue
        FD_ISEDITABLE,    // FIELD_time_serv
        FD_ISEDITABLE,    // FIELD_time_out
        FD_ISEDITABLE,    // FIELD_time_recv
    };
    return (field >= 0 && field < 8) ? fieldTypeFlags[field] : 0;
}

const char *SourcedMsgDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "source",
        "ID",
        "time_send",
        "time_in",
        "time_queue",
        "time_serv",
        "time_out",
        "time_recv",
    };
    return (field >= 0 && field < 8) ? fieldNames[field] : nullptr;
}

int SourcedMsgDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "source") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "ID") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "time_send") == 0) return baseIndex + 2;
    if (strcmp(fieldName, "time_in") == 0) return baseIndex + 3;
    if (strcmp(fieldName, "time_queue") == 0) return baseIndex + 4;
    if (strcmp(fieldName, "time_serv") == 0) return baseIndex + 5;
    if (strcmp(fieldName, "time_out") == 0) return baseIndex + 6;
    if (strcmp(fieldName, "time_recv") == 0) return baseIndex + 7;
    return base ? base->findField(fieldName) : -1;
}

const char *SourcedMsgDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",    // FIELD_source
        "int",    // FIELD_ID
        "omnetpp::simtime_t",    // FIELD_time_send
        "omnetpp::simtime_t",    // FIELD_time_in
        "omnetpp::simtime_t",    // FIELD_time_queue
        "omnetpp::simtime_t",    // FIELD_time_serv
        "omnetpp::simtime_t",    // FIELD_time_out
        "omnetpp::simtime_t",    // FIELD_time_recv
    };
    return (field >= 0 && field < 8) ? fieldTypeStrings[field] : nullptr;
}

const char **SourcedMsgDescriptor::getFieldPropertyNames(int field) const
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

const char *SourcedMsgDescriptor::getFieldProperty(int field, const char *propertyName) const
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

int SourcedMsgDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    SourcedMsg *pp = omnetpp::fromAnyPtr<SourcedMsg>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void SourcedMsgDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    SourcedMsg *pp = omnetpp::fromAnyPtr<SourcedMsg>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'SourcedMsg'", field);
    }
}

const char *SourcedMsgDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    SourcedMsg *pp = omnetpp::fromAnyPtr<SourcedMsg>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string SourcedMsgDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    SourcedMsg *pp = omnetpp::fromAnyPtr<SourcedMsg>(object); (void)pp;
    switch (field) {
        case FIELD_source: return long2string(pp->getSource());
        case FIELD_ID: return long2string(pp->getID());
        case FIELD_time_send: return simtime2string(pp->getTime_send());
        case FIELD_time_in: return simtime2string(pp->getTime_in());
        case FIELD_time_queue: return simtime2string(pp->getTime_queue());
        case FIELD_time_serv: return simtime2string(pp->getTime_serv());
        case FIELD_time_out: return simtime2string(pp->getTime_out());
        case FIELD_time_recv: return simtime2string(pp->getTime_recv());
        default: return "";
    }
}

void SourcedMsgDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    SourcedMsg *pp = omnetpp::fromAnyPtr<SourcedMsg>(object); (void)pp;
    switch (field) {
        case FIELD_source: pp->setSource(string2long(value)); break;
        case FIELD_ID: pp->setID(string2long(value)); break;
        case FIELD_time_send: pp->setTime_send(string2simtime(value)); break;
        case FIELD_time_in: pp->setTime_in(string2simtime(value)); break;
        case FIELD_time_queue: pp->setTime_queue(string2simtime(value)); break;
        case FIELD_time_serv: pp->setTime_serv(string2simtime(value)); break;
        case FIELD_time_out: pp->setTime_out(string2simtime(value)); break;
        case FIELD_time_recv: pp->setTime_recv(string2simtime(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'SourcedMsg'", field);
    }
}

omnetpp::cValue SourcedMsgDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    SourcedMsg *pp = omnetpp::fromAnyPtr<SourcedMsg>(object); (void)pp;
    switch (field) {
        case FIELD_source: return pp->getSource();
        case FIELD_ID: return pp->getID();
        case FIELD_time_send: return pp->getTime_send().dbl();
        case FIELD_time_in: return pp->getTime_in().dbl();
        case FIELD_time_queue: return pp->getTime_queue().dbl();
        case FIELD_time_serv: return pp->getTime_serv().dbl();
        case FIELD_time_out: return pp->getTime_out().dbl();
        case FIELD_time_recv: return pp->getTime_recv().dbl();
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'SourcedMsg' as cValue -- field index out of range?", field);
    }
}

void SourcedMsgDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    SourcedMsg *pp = omnetpp::fromAnyPtr<SourcedMsg>(object); (void)pp;
    switch (field) {
        case FIELD_source: pp->setSource(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_ID: pp->setID(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_time_send: pp->setTime_send(value.doubleValue()); break;
        case FIELD_time_in: pp->setTime_in(value.doubleValue()); break;
        case FIELD_time_queue: pp->setTime_queue(value.doubleValue()); break;
        case FIELD_time_serv: pp->setTime_serv(value.doubleValue()); break;
        case FIELD_time_out: pp->setTime_out(value.doubleValue()); break;
        case FIELD_time_recv: pp->setTime_recv(value.doubleValue()); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'SourcedMsg'", field);
    }
}

const char *SourcedMsgDescriptor::getFieldStructName(int field) const
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

omnetpp::any_ptr SourcedMsgDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    SourcedMsg *pp = omnetpp::fromAnyPtr<SourcedMsg>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void SourcedMsgDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    SourcedMsg *pp = omnetpp::fromAnyPtr<SourcedMsg>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'SourcedMsg'", field);
    }
}

namespace omnetpp {

}  // namespace omnetpp

