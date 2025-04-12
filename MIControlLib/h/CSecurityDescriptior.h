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

#include <accctrl.h>
#include <aclapi.h>
#include <array>

using PSIDHandle = CAutoHandle < PSID, nullptr, [](PSID p) -> BOOL { return !FreeSid(p); } > ;

class ObjectValidity
{
public:
    virtual bool    IsObjectValid() const = 0;
};


class SysPerson : public ObjectValidity
{
    PSIDHandle          m_psid;
    TRUSTEE_FORM        m_tf = TRUSTEE_IS_SID;
    TRUSTEE_TYPE        m_tt = TRUSTEE_IS_USER;

public:
    SysPerson(
        const SID_IDENTIFIER_AUTHORITY& sid,
        std::initializer_list<DWORD> subAuthority,
        TRUSTEE_TYPE tt = TRUSTEE_IS_USER,
        TRUSTEE_FORM tf = TRUSTEE_IS_SID
    );

    SysPerson(const SysPerson&) = delete;
    SysPerson(SysPerson&&) = delete;
    SysPerson& operator=(const SysPerson&) = delete;

    TRUSTEE_FORM        TrusteeForm() const;
    TRUSTEE_TYPE        TrusteeType() const;
    PSID                Person() const;
    bool                IsObjectValid() const override;
};


class AccessObject : public ObjectValidity
{
    EXPLICIT_ACCESS         m_ea;
    const SysPerson&        m_sid;
    const bool              m_fObjectValid;

public:
    AccessObject(const SysPerson& person, ACCESS_MODE am, DWORD dwAccessType, DWORD dwInheritance = NO_INHERITANCE);
    const EXPLICIT_ACCESS& Object() const;
    bool                    IsObjectValid() const override;
};

using PACLHandle = CAutoHandle < PACL, nullptr, [](PACL p) -> BOOL { LocalFree(p); return true; }>;

class AccessObjectArray : public ObjectValidity
{
    std::vector<EXPLICIT_ACCESS>    m_vea;
    PACLHandle                      m_pACL;

public:
    AccessObjectArray(const AccessObjectArray&) = delete;
    AccessObjectArray(AccessObjectArray&&) = delete;
    AccessObjectArray& operator=(const AccessObjectArray&) = delete;

    AccessObjectArray(std::convertible_to<AccessObject> auto&& ...ao)
    {
        for (const auto& ea : std::initializer_list<AccessObject>{ ao... })
        {
            if (!ea.IsObjectValid())
            {
                return;
            }

            m_vea.emplace_back(ea.Object());
        }

        if (!m_vea.empty())
        {
            if (ERROR_SUCCESS != SetEntriesInAcl(static_cast<DWORD>(m_vea.size()), m_vea.data(), nullptr, &m_pACL))
            {
                m_pACL.reset();
            }
        }
    }

    bool IsObjectValid() const override
    {
        return m_pACL;
    }

    PACL AccessObjectList() const
    {
        return m_pACL;
    }
};

class SecurityDescriptor : public ObjectValidity
{
    PSECURITY_DESCRIPTOR            m_pSD = nullptr;
    SECURITY_ATTRIBUTES             m_sa{ 0 };

public:
    SecurityDescriptor(const SecurityDescriptor&) = delete;
    SecurityDescriptor(SecurityDescriptor&&) = delete;
    SecurityDescriptor& operator=(const SecurityDescriptor&) = delete;

    SecurityDescriptor(const AccessObjectArray& acl, bool fInherited = false);
    ~SecurityDescriptor();
    const SECURITY_ATTRIBUTES&      Attributes() const;
    bool                            IsObjectValid() const override;

};
