/*
	MI Control Utility — v1.2.5 [public version]
	Copyright (C) 2025 ALXR aka loginsin
	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "precomp.h"

WbemClassItem::WbemClassItem(IWbemClassObject* pSource, bool fShared)
    : m_pSourceObject(pSource), m_fShared(fShared)
{
    InitMap();
}

WbemClassItem::WbemClassItem(WbemClassItem&& other) noexcept
{
    if (other.m_pSourceObject == this->m_pSourceObject && m_fInitialized && other.m_fInitialized)
        return;

    if (m_fInitialized)
    {
        m_fInitialized = FALSE;
        m_pSourceObject->Release();
        m_pSourceObject = nullptr;
    }

    m_pSourceObject = other.m_pSourceObject;
    other.m_pSourceObject = nullptr;
    
    if (other.m_fInitialized)
    {
        m_mData = other.m_mData;
        other.m_mData.clear();
        m_fInitialized = TRUE;
    }

    other.m_fInitialized = FALSE;
}

WbemClassItem::~WbemClassItem()
{
    if (m_pSourceObject && !m_fShared)
        m_pSourceObject->Release();
}

IWbemClassObject** WbemClassItem::operator&()
{
    return &m_pSourceObject;
}

IWbemClassObject* WbemClassItem::operator->()
{
    return m_pSourceObject;
}

WbemClassItem::operator bool() const
{
    return nullptr != m_pSourceObject;
}

WbemClassProp& WbemClassItem::operator[](const tstring& key)
{
    InitMap();

    auto it = m_mData.find(key);
    if (it != m_mData.end())
        return it->second;

    throw COMException(WBEM_E_NOT_FOUND);
}

decltype(WbemClassItem::m_mData.cbegin()) WbemClassItem::begin()
{
    InitMap();
    return m_mData.cbegin();
}

decltype(WbemClassItem::m_mData.cend()) WbemClassItem::end()
{
    InitMap();
    return m_mData.cend();
}

decltype(WbemClassItem::m_mData.cend()) WbemClassItem::find(const tstring& key)
{
    InitMap();
    return m_mData.find(key);
}

void WbemClassItem::InitMap()
{
    if (!m_pSourceObject)
        throw COMException(WBEM_E_NOT_AVAILABLE);

    if (m_fInitialized)
        return;

    auto& obj = m_pSourceObject;
    obj->BeginEnumeration(0);

    HRESULT hres = S_OK;
    BSTR strName = nullptr;
    VARIANT vt = { 0 };
    while (WBEM_S_NO_MORE_DATA != (hres = obj->Next(0, &strName, &vt, nullptr, nullptr)))
    {
        if (FAILED(hres))
            break;

        m_mData[strName] = vt;
        SysFreeString(strName);
        VariantClear(&vt);
    }

    m_fInitialized = TRUE;
}

WMIUnsecApp::WMIUnsecApp()
{
    auto hres = CoCreateInstance(CLSID_UnsecuredApartment, NULL,
        CLSCTX_LOCAL_SERVER, IID_IUnsecuredApartment,
        (void**)&m_pAp);

    if (FAILED(hres))
        throw COMException(hres);
}

WMIUnsecApp::~WMIUnsecApp()
{
    m_pAp->Release();
}

IUnsecuredApartment* WMIUnsecApp::operator->()
{
    return m_pAp;
}

WMIUnsecApp::operator IUnsecuredApartment* ()
{
    return operator->();
}

IWbemObjectSink* WMIUnsecApp::StubSinc(WMISinc* pSink, IUnknown ** pStubUnk)
{
    IUnknown* pStubUnknown = NULL;
    auto hres = m_pAp->CreateObjectStub(pSink, &pStubUnknown);
    if (FAILED(hres))
        throw COMException(hres);

    IWbemObjectSink* pStubSink = NULL;
    hres = pStubUnknown->QueryInterface(IID_IWbemObjectSink,
        (void**)&pStubSink);

    if (FAILED(hres))
    {
        pStubUnknown->Release();
        throw COMException(hres);
    }

    *pStubUnk = pStubUnknown;
    return pStubSink;
}

ULONG STDMETHODCALLTYPE WMISinc::AddRef()
{
    return InterlockedIncrement(&m_ref);
}

ULONG STDMETHODCALLTYPE WMISinc::Release()
{
    auto k = InterlockedDecrement(&m_ref);
    if (!k)
        delete this;

    return k;
}

HRESULT STDMETHODCALLTYPE WMISinc::QueryInterface(REFIID riid, void** ppv)
{
    if (riid == IID_IUnknown || riid == IID_IWbemObjectSink) 
    {
        *ppv = (IWbemObjectSink*)this;
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

HRESULT STDMETHODCALLTYPE WMISinc::Indicate(LONG lObjectCount, IWbemClassObject** apObjArray)
{
    if (!m_callback)
        return E_NOT_SET;
    
    for (decltype(lObjectCount)i = 0; i < lObjectCount; ++i)
    {
        WbemClassItem item(apObjArray[i], true);
        m_callback(item);
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE WMISinc::SetStatus(LONG lFlags, HRESULT hResult, BSTR strParam, IWbemClassObject* pObjParam)
{
    return WBEM_S_NO_ERROR;
}

void WMISinc::SetListener(LISTENER_CALLBACK callback)
{
    m_callback = callback;
}

WMIListener::WMIListener(const tstring& strRoot, const tstring& WQL, LISTENER_CALLBACK callback)
    :
    m_callback(callback)
    , m_locator()
    , m_svc(m_locator, strRoot)
{
    m_svc.SetProxyBlanket();

    m_psinc = new WMISinc;
    m_psinc->AddRef();
    m_psinc->SetListener(std::bind(&WMIListener::Listener, this, std::placeholders::_1));
    m_pStubSink = m_app.StubSinc(m_psinc, &m_pStubUnk);

    auto hres = m_svc->ExecNotificationQueryAsync(BSTR(L"WQL"),
        BSTR(WQL.c_str()),
        WBEM_FLAG_SEND_STATUS, NULL, m_pStubSink);

    if (FAILED(hres))
        throw COMException(hres);
}

WMIListener::~WMIListener()
{
    m_svc->CancelAsyncCall(m_psinc);	
    if (m_pStubSink)
        m_pStubSink->Release();

    if (m_pStubUnk)
        m_pStubUnk->Release();

    delete m_psinc;
}

void WMIListener::Listener(WbemClassItem& obj)
{
    if (m_callback)
        m_callback(obj);
}