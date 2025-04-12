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

SysPerson::SysPerson(
    const SID_IDENTIFIER_AUTHORITY& sid,
    std::initializer_list<DWORD> subAuthority,
    TRUSTEE_TYPE tt, TRUSTEE_FORM tf)
    : m_tt(tt), m_tf(tf)
{
    std::array<DWORD, 8> nSubAuthority = {};
    auto nCount = min(nSubAuthority.size(), subAuthority.size());
    std::copy_n(subAuthority.begin(), nCount, nSubAuthority.begin());

    SID_IDENTIFIER_AUTHORITY lsid = sid;
    if (!AllocateAndInitializeSid(
        &lsid,
        static_cast<BYTE>(nCount),
        nSubAuthority[0],
        nSubAuthority[1],
        nSubAuthority[2],
        nSubAuthority[3],
        nSubAuthority[4],
        nSubAuthority[5],
        nSubAuthority[6],
        nSubAuthority[7],
        &m_psid
    ))
    {
        m_psid.reset();
    }
}

TRUSTEE_FORM SysPerson::TrusteeForm() const
{
    return m_tf;
}

TRUSTEE_TYPE SysPerson::TrusteeType() const
{
    return m_tt;
}

PSID SysPerson::Person() const
{
    return m_psid;
}

bool SysPerson::IsObjectValid() const
{
    return m_psid;
}

AccessObject::AccessObject(const SysPerson& person, ACCESS_MODE am, DWORD dwAccessType, DWORD dwInheritance)
    :
    m_sid(person),
    m_ea({ 0 }),
    m_fObjectValid(person.IsObjectValid())
{
    m_ea.grfAccessMode = am;
    m_ea.grfAccessPermissions = dwAccessType;
    m_ea.grfInheritance = dwInheritance;
    m_ea.Trustee.TrusteeForm = m_sid.TrusteeForm();
    m_ea.Trustee.TrusteeType = m_sid.TrusteeType();
    m_ea.Trustee.ptstrName = (person.IsObjectValid() ? static_cast<LPTSTR>(m_sid.Person()) : nullptr);
}

const EXPLICIT_ACCESS& AccessObject::Object() const
{
    return m_ea;
}

bool AccessObject::IsObjectValid() const
{
    return m_fObjectValid;
}

SecurityDescriptor::SecurityDescriptor(const AccessObjectArray& acl, bool fInherited)
    :
    m_pSD(LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH))
{
    if (m_pSD && acl.IsObjectValid())
    {
        if (
            InitializeSecurityDescriptor(m_pSD, SECURITY_DESCRIPTOR_REVISION) &&
            SetSecurityDescriptorDacl(m_pSD, TRUE, acl.AccessObjectList(), FALSE)
            )
        {
            m_sa.nLength = sizeof(m_sa);
            m_sa.lpSecurityDescriptor = m_pSD;
            m_sa.bInheritHandle = fInherited;
            return;
        }
        LocalFree(m_pSD);
        m_pSD = nullptr;
    }
}

SecurityDescriptor::~SecurityDescriptor()
{
    LocalFree(m_pSD);
}

const SECURITY_ATTRIBUTES& SecurityDescriptor::Attributes() const
{
    return m_sa;
}

bool SecurityDescriptor::IsObjectValid() const
{
    return (nullptr != m_pSD);
}
