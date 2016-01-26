#include "DOtherSide/DosQObjectWrapper.h"
#include "DOtherSide/DosIQObjectImpl.h"
#include "DOtherSide/DosQObject.h"
#include <QDebug>
#include <QtQml/qqml.h>

namespace DOS {

template<int>
class DosQObjectWrapper : public QObject
{
public:
    static const QMetaObject staticMetaObject;

    DosQObjectWrapper(QObject *parent = nullptr);
    ~DosQObjectWrapper();

    const QMetaObject* metaObject() const override;
    int qt_metacall(QMetaObject::Call, int, void **) override;

    static CreateDObject createDObject();
    static DeleteDObject deleteDObject();
    static void setCreateDObject(CreateDObject createDObject);
    static void setDeleteDObject(DeleteDObject deleteDObject);
    static void setStaticMetaObject(const QMetaObject& metaObject);

private:
    void* m_dObject;
    DosQObject* m_impl;
    static CreateDObject m_createDObject;
    static DeleteDObject m_deleteDObject;
};

template<int N>
const QMetaObject DosQObjectWrapper<N>::staticMetaObject = QObject::staticMetaObject;

template<int N>
CreateDObject DosQObjectWrapper<N>::m_createDObject;

template<int N>
DeleteDObject DosQObjectWrapper<N>::m_deleteDObject;

template<int N>
DosQObjectWrapper<N>::DosQObjectWrapper(QObject *parent)
    : QObject(parent)
    , m_dObject(nullptr)
    , m_impl(nullptr)
{
    void* impl = nullptr;
    m_createDObject(&m_dObject, &impl);
    m_impl = static_cast<DosQObject*>(impl);
    Q_ASSERT(m_dObject);
    Q_ASSERT(m_impl);
}

template<int N>
DosQObjectWrapper<N>::~DosQObjectWrapper()
{
    m_deleteDObject(m_dObject);
    m_dObject = nullptr;
    m_impl = nullptr;
}

template<int N>
const QMetaObject *DosQObjectWrapper<N>::metaObject() const
{
    Q_ASSERT(m_impl);
    return m_impl->metaObject();
}

template<int N>
int DosQObjectWrapper<N>::qt_metacall(QMetaObject::Call call, int index, void **args)
{
    Q_ASSERT(m_impl);
    return m_impl->qt_metacall(call, index, args);
}

template<int N>
CreateDObject DosQObjectWrapper<N>::createDObject() { return m_createDObject; }

template<int N>
DeleteDObject DosQObjectWrapper<N>::deleteDObject() { return m_deleteDObject; }

template<int N>
void DosQObjectWrapper<N>::setCreateDObject(CreateDObject createDObject) { m_createDObject = createDObject; }

template<int N>
void DosQObjectWrapper<N>::setDeleteDObject(DeleteDObject deleteDObject) { m_deleteDObject = deleteDObject; }

template<int N>
void DosQObjectWrapper<N>::setStaticMetaObject(const QMetaObject &metaObject)
{
    *(const_cast<QMetaObject*>(&staticMetaObject)) = metaObject;
}

template<int N>
int dosQmlRegisterType(const char *uri, int major, int minor,
                       const char *qmlName, const QMetaObject& staticMetaObject,
                       CreateDObject createDObject,
                       DeleteDObject deleteDObject)
{
    DosQObjectWrapper<N>::setCreateDObject(std::move(createDObject));
    DosQObjectWrapper<N>::setDeleteDObject(std::move(deleteDObject));
    DosQObjectWrapper<N>::setStaticMetaObject(staticMetaObject);
    return qmlRegisterType<DosQObjectWrapper<N>>(uri, major, minor, qmlName);
}

template<int N>
struct DosQmlRegisterHelper
{
    static int Register(int i, const char *uri, int major, int minor,
                 const char *qmlName, const QMetaObject& staticMetaObject,
                 CreateDObject createDObject,
                 DeleteDObject deleteDObject)
    {
        if (i == N)
            return dosQmlRegisterType<N>(uri, major, minor, qmlName, staticMetaObject, std::move(createDObject), std::move(deleteDObject));
        else
            return DosQmlRegisterHelper<N-1>::Register(i, uri, major, minor, qmlName, staticMetaObject, std::move(createDObject), std::move(deleteDObject));
    }
};

template<>
struct DosQmlRegisterHelper<0>
{
    static int Register(int i, const char *uri, int major, int minor,
                 const char *qmlName, const QMetaObject& staticMetaObject,
                 CreateDObject createDObject,
                 DeleteDObject deleteDObject)
    {
        if (i == 0)
            return dosQmlRegisterType<0>(uri, major, minor, qmlName, staticMetaObject, std::move(createDObject), std::move(deleteDObject));
        else
            return -1;
    }
};


int dosQmlRegisterType(const char *uri, int major, int minor,
                       const char *qmlName, const QMetaObject& staticMetaObject,
                       CreateDObject createDObject,
                       DeleteDObject deleteDObject)
{
    static int i = 0;
    DosQmlRegisterHelper<50>::Register(i++, uri, major, minor, qmlName, staticMetaObject, std::move(createDObject), std::move(deleteDObject));
}



}