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

#include "md5.h"

#define EVENT_PIPE_NAME			TEXT("\\\\.\\pipe\\MICONTROL-E09DF953-5B84-4CB7-A23B-969218E24781")

extern "C"
WINUSERAPI
BOOL
WINAPI
NtUserEnableTouchPad(BOOL);

extern tstring ImagePath();
#define EnableTouchPad      NtUserEnableTouchPad
extern bool CheckServicePresence();