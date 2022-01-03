%{?_datarootdir: %global mydatarootdir %_datarootdir}
%{!?_datarootdir: %global mydatarootdir %{buildroot}/usr/share}

%global module_dir %{_libdir}/qore-modules
%global user_module_dir %{mydatarootdir}/qore-modules/

Name:           qore-jni-module
Version:        2.0.6
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
BuildRequires:  cmake >= 3.12.4
BuildRequires:  qore-devel >= 1.0
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
/usr/share/qore/java/qore-jni-compiler.jar
/usr/share/qore/java/qore-jni.jar


%changelog
* Thu Jan 27 2022 David Nichols <david@qore.org>
- initial spec file for 2.0.6 release

