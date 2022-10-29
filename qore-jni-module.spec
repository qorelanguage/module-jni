%{?_datarootdir: %global mydatarootdir %_datarootdir}
%{!?_datarootdir: %global mydatarootdir %{buildroot}/usr/share}

%global module_dir %{_libdir}/qore-modules
%global user_module_dir %{mydatarootdir}/qore-modules/

Name:           qore-jni-module
Version:        2.0.10
Release:        1
Summary:        Qorus Integration Engine - Qore jni module
License:        MIT
Group:          Productivity/Networking/Other
Url:            https://qoretechnologies.com
Source:         qore-jni-module-%{version}.tar.bz2
BuildRequires:  gcc-c++
%if 0%{?el7}
BuildRequires:  devtoolset-7-gcc-c++
%endif
BuildRequires:  cmake >= 3.5
BuildRequires:  qore-devel >= 1.0
BuildRequires:  qore >= 1.0
BuildRequires:  qore-stdlib >= 1.0
BuildRequires:  java-11-openjdk-devel
BuildRequires:  unzip

%if 0%{?suse_version}
%if 0%{?sles_version} && %{?sles_version} <= 10
BuildRequires:  bzip2
%else
BuildRequires:  libbz2-devel
%endif
%else
BuildRequires:  bzip2-devel
%endif

Requires:       %{_bindir}/env
Requires:       qore >= 1.0
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
Requires:       java-11-openjdk-devel
Requires:       java-11-openjdk
%if 0%{?el8}
# disable automatic library dependencies due to broken java 11 lib handling in centos 8
AutoReqProv: no
%endif

%description
This package contains the jni module for the Qore Programming Language.

%prep
%setup -q

%build
%if 0%{?el7}
# enable devtoolset7
. /opt/rh/devtoolset-7/enable
%endif
export CXXFLAGS="%{?optflags}"
cmake -DCMAKE_INSTALL_PREFIX=%{_prefix} -DCMAKE_BUILD_TYPE=RELWITHDEBINFO -DCMAKE_SKIP_RPATH=1 -DCMAKE_SKIP_INSTALL_RPATH=1 -DCMAKE_SKIP_BUILD_RPATH=1 -DCMAKE_PREFIX_PATH=${_prefix}/lib64/cmake/Qore .
make %{?_smp_mflags}

%install
make DESTDIR=%{buildroot} install %{?_smp_mflags}

%files
%{module_dir}
%{_bindir}/qjava2jar
%{_bindir}/qjavac
%dir /usr/share/qore/java
/usr/share/qore/java/qore-jni-compiler.jar
/usr/share/qore/java/qore-jni.jar

%changelog
* Sat Oct 29 2022 David Nichols <david@qore.org>
- updated to version 2.0.10

* Sat Sep 17 2022 David Nichols <david@qore.org>
- updated to version 2.0.9

* Tue Aug 9 2022 David Nichols <david@qore.org>
- updated to version 2.0.8

* Thu Apr 21 2022 David Nichols <david@qore.org>
- updated to version 2.0.7

* Thu Jan 27 2022 David Nichols <david@qore.org>
- initial spec file for 2.0.6 release

