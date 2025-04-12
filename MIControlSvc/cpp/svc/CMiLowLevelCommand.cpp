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

CMiLowLevelCommand::CMiLowLevelCommand()
{
	try
	{
		WMIQuery q(WMI_EXEC_ROOT);
		std::vector<WbemClassItem> result;
		if (q.Query(TEXT("SELECT InstanceName FROM ") WMI_EXEC_CLASS, result))
		{
			std::vector<tstring> vInstances;
			std::for_each(result.begin(), result.end(),
				[&vInstances](auto& row)
				{
					auto it = row.find(TEXT("InstanceName"));
					if (row.end() == it)
						return;

					auto& vt = it->second;
					if (VT_BSTR == vt->vt)
						vInstances.push_back(vt->bstrVal);
				}
			);

			if (!vInstances.empty())
				m_strInstanceName = vInstances[0];
			else
				throw COMException(WBEM_E_INITIALIZATION_FAILURE);
		}

	}
	catch (COMException& comerr)
	{
		m_hres = comerr.what();
	}
}

bool CMiLowLevelCommand::ExecQuery(const std::vector<uint8_t>& inData, std::vector<uint8_t>& outData)
{
	auto data = inData;
	auto& result = outData;
	// g_log->info(TEXT("Doing ExecQuery()"));

	data.resize(0x20);
	SAFEARRAYBOUND bound;
	bound.lLbound = 0;
	bound.cElements = static_cast<ULONG>(data.size());

	tstring strlog;

	// g_log->info(TEXT("Making array()"));
	auto arr = SafeArrayCreate(VT_UI1, 1, &bound);
	if (!arr)
		return false;

	LONG i = 0;
	for (auto& ch : data)
	{
		std::ignore = SafeArrayPutElement(arr, &i, &ch);

		TCHAR x[15];
		wsprintf(x, TEXT("%02X "), ch);
		strlog += x;
		i++;
	}

	VARIANT vt;
	vt.vt = VT_ARRAY | VT_UI1;
	std::ignore = SafeArrayCopy(arr, &vt.parray);
	SafeArrayDestroy(arr);

	// g_log->info(TEXT("Array created"));
	std::unique_ptr<WMIPut> pExec;

	try
	{
		// g_log->info(TEXT("Sending low command: %s"), strlog.c_str());
		pExec = std::make_unique<WMIPut>(WMI_EXEC_ROOT, WMI_EXEC_CLASS, WMI_EXEC_METHOD);
		pExec->Put(WMI_EXEC_IN_DATA, vt);
	}
	catch (COMException&)// comerr)
	{
		//g_log->info(TEXT("Worker Failed! 0x%08X"), comerr.what());
		VariantClear(&vt);
		//g_log->info(TEXT("Bail out"));
		return false;
	}

	try
	{
		//g_log->info(TEXT("Making request to \"%s\""), (tstring(WMI_EXEC_CLASS_PRED) + TEXT("'") + m_strInstanceName + TEXT("'")).c_str());
		auto outData = pExec->Exec(tstring(WMI_EXEC_CLASS_PRED) + TEXT("'") + m_strInstanceName + TEXT("'"));
		VariantClear(&vt);

		auto out = outData.find(WMI_EXEC_OUT_DATA);
		if (out == outData.end())
			return true;

		auto& vt = out->second;
		if ((VT_ARRAY | VT_UI1) != vt->vt)
			return true;

		SafeArray1 arr(vt->parray);
		decltype(arr.begin())i1 = arr.begin();
		decltype(arr.end())i2 = arr.end() + 1;
		result.reserve(i2 - i1 + 1);

		strlog.clear();

		for (decltype(i1)i = i1; i < i2; ++i)
		{
			result.push_back(arr.Get<uint8_t>(i));

			TCHAR x[15];
			wsprintf(x, TEXT("%02X "), result.back());
			strlog += x;
		}

		// g_log->info(TEXT("Low command result: %s"), strlog.c_str());
	}
	catch (COMException&)// comerr)
	{
		VariantClear(&vt);

		//g_log->error(TEXT("Failed to exec MI command: 0x%08X"), comerr.what());
		return false;
	}

	return true;
}

bool CMiLowLevelCommand::Put(MiLowLevelCommand cmd, uint8_t arg, std::vector<uint8_t>& result)
{
	return Put(cmd, arg, 0, result);
}

bool CMiLowLevelCommand::Put(MiLowLevelCommand cmd, uint8_t arg, uint8_t arg2, std::vector<uint8_t>& result)
{
	if (FAILED(m_hres))
		return false;

	std::vector<uint8_t> data =
	{
		0x00, 0xFB, 0x00, cmd, arg, 0x00, arg2, 0x00
	};

	return ExecQuery(data, result);
}

bool CMiLowLevelCommand::Get(MiLowLevelCommand cmd, uint8_t arg, uint8_t arg2, std::vector<uint8_t>& result)
{
	if (FAILED(m_hres))
		return false;

	std::vector<uint8_t> data =
	{
		0x00, 0xFA, 0x00, cmd, arg, 0x00, arg2, 0x00
	};

	return ExecQuery(data, result);
}

bool CMiLowLevelCommand::Get(MiLowLevelCommand cmd, uint8_t arg, std::vector<uint8_t>& result)
{
	return Get(cmd, arg, 0, result);
}

HRESULT CMiLowLevelCommand::Error() const
{
	return m_hres;
}