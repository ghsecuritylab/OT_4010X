

#include "config.h"
#include "JSBase.h"

JSBase::JSBase(JSTypeID type) : fTypeID(type), fRetainCount(1)
{
}

JSBase::~JSBase()
{
}

JSBase* JSBase::Retain()
{
    fRetainCount++; return this;
}

void JSBase::Release()
{
    if (--fRetainCount == 0)
    {
        JSGlueAPIEntry entry;
        delete this;
    }
}

CFIndex JSBase::RetainCount() const
{
    return fRetainCount;
}

JSTypeID JSBase::GetTypeID() const
{
    return fTypeID;
}

CFStringRef JSBase::CopyDescription()
{
    return CFStringCreateWithFormat(
                0,
                0,
                CFSTR("<JSTypeRef- ptr:0x%lx type: %d, retaincount: %ld>"),
                (long)this,
                (int)fTypeID,
                (long)fRetainCount);
}

UInt8 JSBase::Equal(JSBase* other)
{
    return this == other;
}
