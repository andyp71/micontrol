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

#pragma once

#include <WbemCli.h>

class WbemClassProp
{
	VARIANT			m_vtProp = { 0 };

public:
	WbemClassProp()
	{
		VariantInit(&m_vtProp);
	}

	WbemClassProp(const VARIANT &vt)
	{
		std::ignore = VariantCopy(&m_vtProp, &vt);
	}

	WbemClassProp(const WbemClassProp& other)
	{
		if (&other != &this->m_vtProp)
		{
			VariantInit(&m_vtProp);
			std::ignore = VariantCopy(&m_vtProp, &other.m_vtProp);
		}
	}

	WbemClassProp(WbemClassProp&& other) noexcept
	{
		VariantInit(&m_vtProp);
		std::ignore = VariantCopy(&m_vtProp, &other.m_vtProp);
		VariantClear(&other.m_vtProp);
	}

	WbemClassProp& operator=(VARIANT& vt)
	{
		std::ignore = VariantCopy(&m_vtProp, &vt);
		return *this;
	}

	~WbemClassProp()
	{
		VariantClear(&m_vtProp);
	}

	LPVARIANT operator&()
	{
		return &m_vtProp;
	}

	LPVARIANT operator->()
	{
		return operator&();
	}

	const VARIANT* operator&() const
	{
		return &m_vtProp;
	}

	const VARIANT* operator->() const
	{
		return operator&();
	}
};

class SafeArray1
{
	const bool m_fAutoDestroy = true;
	SAFEARRAY* m_pNames = nullptr;

public:
	SafeArray1() {}
	SafeArray1(SAFEARRAY * p)
		: m_fAutoDestroy(false), m_pNames(p)
	{}

	~SafeArray1()
	{
		if (m_pNames && m_fAutoDestroy)
			SafeArrayDestroy(m_pNames);
	}

	SAFEARRAY** operator&()
	{
		return &m_pNames;
	}

	LONG		begin() const
	{
		LONG l = 0;
		if (FAILED(SafeArrayGetLBound(m_pNames, 1, &l)))
			return 0;

		return l;
	}

	LONG		end() const
	{
		LONG l = 0;
		if (FAILED(SafeArrayGetUBound(m_pNames, 1, &l)))
			return 0;

		return l;
	}

	template <class T>
	T Get(LONG l)
	{
		T v;
		auto hres = SafeArrayGetElement(m_pNames, &l, &v);
		if (FAILED(hres))
			throw COMException(hres);

		return v;
	}
};


//using WbemClassItem = std::map<tstring, WbemClassProp>;
class WbemClassItem
{
	std::map<tstring, WbemClassProp>			m_mData;
	BOOL										m_fInitialized = FALSE;
	const bool									m_fShared = false;
	IWbemClassObject*							m_pSourceObject = nullptr;

	void			InitMap();

public:
	WbemClassItem(IWbemClassObject* pSource, bool fShared = false);
	WbemClassItem(const WbemClassItem&) = delete;
	WbemClassItem& operator=(WbemClassItem&&) = delete;
	WbemClassItem(WbemClassItem&&) noexcept;
	~WbemClassItem();

	WbemClassItem& operator=(const WbemClassItem&) = delete;

	IWbemClassObject** operator&();
	IWbemClassObject* operator->();

	operator bool() const;
	WbemClassProp& operator[](const tstring& key);

	decltype(m_mData.cbegin()) begin();
	decltype(m_mData.cend()) end();
	decltype(m_mData.cend()) find(const tstring&);

};

using LISTENER_CALLBACK = std::function<void(WbemClassItem&)>;

class WMISinc: public IWbemObjectSink
{
	volatile ULONG			m_ref	= 0;
	LISTENER_CALLBACK		m_callback = nullptr;

public:
	ULONG STDMETHODCALLTYPE AddRef() override;
	ULONG STDMETHODCALLTYPE Release() override;
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override;
	HRESULT STDMETHODCALLTYPE Indicate(LONG lObjectCount, IWbemClassObject** apObjArray) override;
	HRESULT STDMETHODCALLTYPE SetStatus(LONG lFlags, HRESULT hResult, BSTR strParam, IWbemClassObject* pObjParam) override;

	void	SetListener(LISTENER_CALLBACK callback);
};

class WMIUnsecApp
{
	IUnsecuredApartment*			m_pAp;

public:
	WMIUnsecApp();
	~WMIUnsecApp();
	IUnsecuredApartment* operator->();
	operator IUnsecuredApartment* ();

	IWbemObjectSink* StubSinc(WMISinc* pSinc, IUnknown** pStubUnk);
};

class WMIListener
{
	LISTENER_CALLBACK		m_callback;
	WbemLocator				m_locator;
	WbemServices			m_svc;
	WMIUnsecApp				m_app;
	IUnknown*				m_pStubUnk = nullptr;
	IWbemObjectSink*		m_pStubSink = nullptr;

	WMISinc					*m_psinc	= nullptr;

public:
	WMIListener(const tstring& strRoot, const tstring& WQL, LISTENER_CALLBACK callback = nullptr);
	~WMIListener();

	virtual void			Listener(WbemClassItem&);

};