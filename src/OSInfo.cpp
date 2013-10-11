/* Copyright (C) 2005-2011, Thorvald Natvig <thorvald@natvig.com>

	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions
	are met:

	- Redistributions of source code must retain the above copyright notice,
	  this list of conditions and the following disclaimer.
	- Redistributions in binary form must reproduce the above copyright notice,
	  this list of conditions and the following disclaimer in the documentation
	  and/or other materials provided with the distribution.
	- Neither the name of the Mumble Developers nor the names of its
	  contributors may be used to endorse or promote products derived from this
	  software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
	A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
	CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
	EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
	PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
	LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
	NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "murmur_pch.h"

#if defined(Q_OS_WIN)
#include <intrin.h>
#endif

#ifdef Q_OS_UNIX && !defined(Q_OS_MAC)
#include <sys/utsname.h>
#endif

#if defined(Q_OS_MAC)
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach-o/arch.h>
#endif

#include "OSInfo.h"
#include "Version.h"

QString OSInfo::getMacHash(const QList<QHostAddress> &qlBind) {
	QString first, second, third;
	foreach(const QNetworkInterface &qni, QNetworkInterface::allInterfaces()) {
		if (! qni.isValid())
			continue;
		if (qni.flags() & QNetworkInterface::IsLoopBack)
			continue;
		if (qni.hardwareAddress().isEmpty())
			continue;

		QString hash = QString::fromUtf8(QCryptographicHash::hash(qni.hardwareAddress().toUtf8(), QCryptographicHash::Sha1).toHex());

		if (third.isEmpty() || third > hash)
			third = hash;

		if (!(qni.flags() & (QNetworkInterface::IsUp | QNetworkInterface::IsRunning)))
			continue;

		if (second.isEmpty() || second > hash)
			second = hash;

		foreach(const QNetworkAddressEntry &qnae, qni.addressEntries()) {
			const QHostAddress &qha = qnae.ip();
			if (qlBind.isEmpty() || qlBind.contains(qha)) {
				if (first.isEmpty() || first > hash)
					first = hash;
			}
		}
	}
	if (! first.isEmpty())
		return first;
	if (! second.isEmpty())
		return second;
	if (! third.isEmpty())
		return third;
	return QString();
}

QString OSInfo::getOS() {
#if defined(Q_OS_WIN)
	return QLatin1String("Windows");
#elif defined(Q_OS_MAC)
	return QLatin1String("Mac OS X");
#else
	return QLatin1String("Linux");
#endif
}

QString OSInfo::getOSVersion() {
	static QString qsCached;

	if (! qsCached.isNull())
		return qsCached.isEmpty() ? QString() : qsCached;

	QString os;

#if defined(Q_OS_WIN)
	OSVERSIONINFOEXW ovi;
	_SYSTEM_INFO si;
	typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);
	PGPI pGPI;
	DWORD dwType;

	memset(&ovi, 0, sizeof(ovi));

	ovi.dwOSVersionInfoSize=sizeof(ovi);
	GetVersionEx(reinterpret_cast<OSVERSIONINFOW *>(&ovi));
	GetNativeSystemInfo(&si);

	if (ovi.dwMajorVersion == 6) {
		if(ovi.dwMinorVersion == 0) {
			if (ovi.wProductType == VER_NT_WORKSTATION)
				os = QLatin1String("Vista");
			else
				os = QLatin1String("Server 2008");
		}
		else if (ovi.dwMinorVersion == 1) {
			if (ovi.wProductType == VER_NT_WORKSTATION)
				os = QLatin1String("7");
			else
				os = QLatin1String("Server 2008 R2");
		}
		else if (ovi.dwMinorVersion == 2) {
			if (ovi.wProductType == VER_NT_WORKSTATION)
				os = QLatin1String("8");
			else
				os = QLatin1String("Server 2012");
		}

		pGPI = (PGPI) GetProcAddress(
		GetModuleHandle(TEXT("kernel32.dll")),
		"GetProductInfo");
		pGPI(ovi.dwMajorVersion, ovi.dwMinorVersion, 0, 0, &dwType);

		switch(dwType) {
		case PRODUCT_ULTIMATE:
			os.append(QLatin1String(" Ultimate Edition"));
			break;
		case PRODUCT_PROFESSIONAL:
			os.append(QLatin1String(" Professional"));
			break;
		case PRODUCT_HOME_PREMIUM:
			os.append(QLatin1String(" Home Premium Edition"));
			break;
		case PRODUCT_HOME_BASIC:
			os.append(QLatin1String(" Home Basic Edition"));
			break;
		case PRODUCT_ENTERPRISE:
			os.append(QLatin1String(" Enterprise Edition"));
			break;
		case PRODUCT_BUSINESS:
			os.append(QLatin1String(" Business Edition"));
			break;
		case PRODUCT_STARTER:
			os.append(QLatin1String(" Starter Edition"));
			break;
		case PRODUCT_CLUSTER_SERVER:
			os.append(QLatin1String(" Cluster Server Edition"));
			break;
		case PRODUCT_DATACENTER_SERVER:
			os.append(QLatin1String(" Datacenter Edition"));
			break;
		case PRODUCT_DATACENTER_SERVER_CORE:
			os.append(QLatin1String(" Datacenter Edition (core installation)"));
			break;
		case PRODUCT_ENTERPRISE_SERVER:
			os.append(QLatin1String(" Enterprise Edition"));
			break;
		case PRODUCT_ENTERPRISE_SERVER_CORE:
			os.append(QLatin1String(" Enterprise Edition (core installation)"));
			break;
		case PRODUCT_ENTERPRISE_SERVER_IA64:
			os.append(QLatin1String(" Enterprise Edition for Itanium-based Systems"));
			break;
		case PRODUCT_SMALLBUSINESS_SERVER:
			os.append(QLatin1String(" Small Business Server"));
			break;
		case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
			os.append(QLatin1String(" Small Business Server Premium Edition"));
			break;
		case PRODUCT_STANDARD_SERVER:
			os.append(QLatin1String(" Standard Edition"));
			break;
		case PRODUCT_STANDARD_SERVER_CORE:
			os.append(QLatin1String(" Standard Edition (core installation)"));
			break;
		case PRODUCT_WEB_SERVER:
			os.append(QLatin1String(" Web Server Edition"));
			break;
		}
	}

	if (ovi.dwMajorVersion == 5 && ovi.dwMinorVersion == 0) {
		os = QLatin1String("2000");
		if (ovi.wProductType == VER_NT_WORKSTATION)
			os.append(QLatin1String("Professional"));
		else
			if (ovi.wSuiteMask & VER_SUITE_DATACENTER)
				os.append(QLatin1String("Datacenter Server"));
			else if (ovi.wSuiteMask & VER_SUITE_ENTERPRISE)
				os.append(QLatin1String("Advanced Server"));
			else 
				os.append(QLatin1String("Server"));
	}

	if (ovi.dwMajorVersion == 5 && ovi.dwMinorVersion == 1) {
		os = QLatin1String("XP ");
		if (ovi.wSuiteMask & VER_SUITE_PERSONAL)
			os.append(QLatin1String("Home Edition"));
		else
			os.append(QLatin1String("Professional"));
	}

	if (ovi.dwMajorVersion == 5 && ovi.dwMinorVersion == 2) {
		if (GetSystemMetrics(SM_SERVERR2))
			os = QLatin1String("Server 2003 R2");
		else if (ovi.wSuiteMask & VER_SUITE_STORAGE_SERVER)
			os = QLatin1String("Storage Server 2003");
		else if (ovi.wSuiteMask & VER_SUITE_WH_SERVER)
			os = QLatin1String("Home Server");
		else if (ovi.wProductType == VER_NT_WORKSTATION && si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
			os = QLatin1String("XP Professional x64 Edition");
		else
			os = QLatin1String("Server 2003");

		if (ovi.wProductType != VER_NT_WORKSTATION) {
			if (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64) {
				if (ovi.wSuiteMask & VER_SUITE_DATACENTER)
					os.append(QLatin1String(" Datacenter x64 Edition"));
				else if (ovi.wSuiteMask & VER_SUITE_ENTERPRISE)
					os.append(QLatin1String(" Enterprise x64 Edition"));
				else 
					os.append(QLatin1String(" Standard x64 Edition"));
			} else {
				if (ovi.wSuiteMask & VER_SUITE_COMPUTE_SERVER)
					os.append(QLatin1String(" Compute Cluster Edition"));
				else if (ovi.wSuiteMask & VER_SUITE_DATACENTER)
					os.append(QLatin1String(" Datacenter Edition"));
				else if (ovi.wSuiteMask & VER_SUITE_ENTERPRISE)
					os.append(QLatin1String(" Enterprise Edition"));
				else if (ovi.wSuiteMask & VER_SUITE_BLADE)
					os.append(QLatin1String(" Web Edition"));
				else 
					os.append(QLatin1String(" Standard Edition"));
			}
		}
	}

	// Service Packs
	QString sp = sp.fromStdWString(ovi.szCSDVersion);
	if (sp.length() > 0) {
		sp.prepend(QLatin1String("\n"));
		os.append(sp);
		os.append(QLatin1String(","));
	}

	// 32/64?
	if (ovi.dwMajorVersion >= 6) {
		if (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
			os.append(QLatin1String(" 64-bit"));
		else if (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_INTEL)
			os.append(QLatin1String(" 32-bit"));
	}

	QString osv;
	osv.sprintf(" (%d.%d.%d)", ovi.dwMajorVersion, ovi.dwMinorVersion, ovi.dwBuildNumber);
	os.append(osv);
#elif defined(Q_OS_MAC)
	SInt32 major, minor, bugfix;
	OSErr err = Gestalt(gestaltSystemVersionMajor, &major);
	if (err == noErr)
		err = Gestalt(gestaltSystemVersionMinor, &minor);
	if (err == noErr)
		err = Gestalt(gestaltSystemVersionBugFix, &bugfix);
	if (err != noErr)
		return QString::number(QSysInfo::MacintoshVersion, 16);

	const NXArchInfo *local = NXGetLocalArchInfo();
	const NXArchInfo *ai = local ? NXGetArchInfoFromCpuType(local->cputype, CPU_SUBTYPE_MULTIPLE) : NULL;
	const char *arch = ai ? ai->name : "unknown";

	os.sprintf("%i.%i.%i (%s)", major, minor, bugfix, arch);
#else
#ifdef Q_OS_LINUX
	QProcess qp;
	QStringList args;
	args << QLatin1String("-s");
	args << QLatin1String("-d");
	qp.start(QLatin1String("lsb_release"), args);
	if (qp.waitForFinished(5000)) {
		os = QString::fromUtf8(qp.readAll()).simplified();
		if (os.startsWith(QLatin1Char('"')) && os.endsWith(QLatin1Char('"')))
			os = os.mid(1, os.length() - 2).trimmed();
	}
	if (os.isEmpty())
		qWarning("OSInfo: Failed to execute lsb_release");

	qp.terminate();
	if (! qp.waitForFinished(1000))
		qp.kill();
#endif
	if (os.isEmpty()) {
		struct utsname un;
		if (uname(&un) == 0) {
			os.sprintf("%s %s", un.sysname, un.release);
		}
	}
#endif

	if (! os.isNull())
		qsCached = os;
	else
		qsCached = QLatin1String("");

	return qsCached;
}

void OSInfo::fillXml(QDomDocument &doc, QDomElement &root, const QString &os, const QString &osver, const QList<QHostAddress> &qlBind) {
	QDomElement tag;
	QDomText t;
	bool bIs64;
	bool bSSE2 = false;
	QString cpu_id, cpu_extid;

	tag=doc.createElement(QLatin1String("machash"));
	root.appendChild(tag);
	t=doc.createTextNode(OSInfo::getMacHash(qlBind));
	tag.appendChild(t);

	tag=doc.createElement(QLatin1String("version"));
	root.appendChild(tag);
	t=doc.createTextNode(QLatin1String(MUMTEXT(MUMBLE_VERSION_STRING)));
	tag.appendChild(t);

	tag=doc.createElement(QLatin1String("release"));
	root.appendChild(tag);
	t=doc.createTextNode(QLatin1String(MUMBLE_RELEASE));
	tag.appendChild(t);

	tag=doc.createElement(QLatin1String("os"));
	root.appendChild(tag);
	t=doc.createTextNode(os);
	tag.appendChild(t);

	tag=doc.createElement(QLatin1String("osver"));
	root.appendChild(tag);
	t=doc.createTextNode(osver);
	tag.appendChild(t);

	tag=doc.createElement(QLatin1String("qt"));
	root.appendChild(tag);
	t=doc.createTextNode(QString::fromLatin1(qVersion()));
	tag.appendChild(t);

#if defined(Q_OS_WIN)
	BOOL bIsWow64 = FALSE;
	IsWow64Process(GetCurrentProcess(), &bIsWow64);
	bIs64 = bIsWow64;
#elif defined(Q_OS_MAC)
	size_t len = sizeof(bool);
	sysctlbyname("hw.cpu64bit_capable", &bIs64, &len, NULL, 0);
#else
	bIs64 = (QSysInfo::WordSize == 64);
#endif
	tag=doc.createElement(QLatin1String("is64bit"));
	root.appendChild(tag);
	t=doc.createTextNode(QString::number(bIs64 ? 1 : 0));
	tag.appendChild(t);

#if defined(Q_OS_WIN)
#define regstr(x) QString::fromLatin1(reinterpret_cast<const char *>(& x), 4)
	int chop;
	int cpuinfo[4];

	__cpuid(cpuinfo, 1);
	bSSE2 = (cpuinfo[3] & 0x04000000);

	__cpuid(cpuinfo, 0);

	cpu_id = regstr(cpuinfo[1]) + regstr(cpuinfo[3]) + regstr(cpuinfo[2]);

	for (unsigned int j=2; j<=4;++j) {
		__cpuid(cpuinfo, 0x80000000 + j);
		cpu_extid += regstr(cpuinfo[0]) + regstr(cpuinfo[1]) + regstr(cpuinfo[2]) + regstr(cpuinfo[3]);
	}

	cpu_id = cpu_id.trimmed();
	chop = cpu_id.indexOf(QLatin1Char('\0'));
	if (chop != -1)
		cpu_id.truncate(chop);

	cpu_extid = cpu_extid.trimmed();
	chop = cpu_extid.indexOf(QLatin1Char('\0'));
	if (chop != -1)
		cpu_extid.truncate(chop);
#endif

	tag=doc.createElement(QLatin1String("cpu_id"));
	root.appendChild(tag);
	t=doc.createTextNode(cpu_id);
	tag.appendChild(t);

	tag=doc.createElement(QLatin1String("cpu_extid"));
	root.appendChild(tag);
	t=doc.createTextNode(cpu_extid);
	tag.appendChild(t);

	tag=doc.createElement(QLatin1String("cpu_sse2"));
	root.appendChild(tag);
	t=doc.createTextNode(QString::number(bSSE2 ? 1 : 0));
	tag.appendChild(t);
}
