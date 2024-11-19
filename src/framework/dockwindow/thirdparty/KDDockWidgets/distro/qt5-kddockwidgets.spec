Name:           qt5-kddockwidgets
Version:        2.1.0
Release:        1
Summary:        KDAB's Dock Widget Framework for Qt5
Source0:        %{name}-%{version}.tar.gz
Source1:        %{name}-%{version}.tar.gz.asc
Source2:        %{name}-rpmlintrc
URL:            https://github.com/KDAB/KDDockWidgets
Group:          System/Libraries
License:        GPL-2.0-only OR GPL-3.0-only
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
Vendor:         Klaralvdalens Datakonsult AB (KDAB)
Packager:       Klaralvdalens Datakonsult AB (KDAB) <info@kdab.com>

BuildRequires: cmake
%if %{defined suse_version}
BuildRequires:  libqt5-qtbase-devel libqt5-qtbase-private-headers-devel libqt5-qtx11extras-devel
%endif

%if %{defined fedora}
BuildRequires:  gcc-c++ qt5-qtbase-devel qt5-qtbase-private-devel qt5-qtx11extras-devel desktop-file-utils util-linux
%endif

%if %{defined rhel}
BuildRequires:  gcc-c++ qt5-qtbase-devel qt5-qtbase-private-devel qt5-qtx11extras-devel desktop-file-utils
%endif

%description
KDDockWidgets is a Qt dock widget library written by KDAB, suitable for replacing
QDockWidget and implementing advanced functionalities missing in Qt, including:
 - Nesting dock widgets in a floating window and docking that group back to main window
  - Docking to any main window, not only to the parent main window
  - Docking to center of main window, or simply removing the concept of "central widget"
  - Main window supporting detachable tabs in center widget
  - Detaching arbitrary tabs from a tab bar into a dock area
  - Exposing inner helper widgets so the user can customize them or provide his own
    - Customize tab widgets
    - Customize title bars
    - Customize window frames
    - Custom widget separators
  ...and much more

Authors:
--------
      Klaralvdalens Datakonsult AB (KDAB) <info@kdab.com>

%define debug_package %{nil}
%global __debug_install_post %{nil}

%package devel
Summary:        Development files for %{name}
Group:          Development/Libraries/C and C++
Requires:       %{name} = %{version}

%description devel
This package contains header files and associated tools and libraries to
develop programs using kddockwidgets.

%prep
%autosetup

%build
cmake . -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release
%__make %{?_smp_mflags}

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%install
%make_install

%clean
%__rm -rf "%{buildroot}"

%files
%defattr(-,root,root)
%{_prefix}/share/doc/KDDockWidgets
%{_libdir}/libkddockwidgets.so.*

%files devel
%defattr(-,root,root,-)
%if 0%{?sle_version} >= 150200 && 0%{?is_opensuse}
%{_libdir}/qt5/mkspecs/modules/*
%endif
%if 0%{?suse_version} > 1500
%{_libdir}/qt5/mkspecs/modules/*
%endif
%if 0%{?fedora} > 28
%{_libdir}/qt5/mkspecs/modules/*
%endif
%if %{defined rhel}
%{_libdir}/qt5/mkspecs/modules/*
%endif
%dir %{_includedir}/kddockwidgets
%{_includedir}/kddockwidgets/*
%dir %{_libdir}/cmake/KDDockWidgets
%{_libdir}/cmake/KDDockWidgets/*
%{_libdir}/libkddockwidgets.so

%changelog
* Wed May 08 2024 Allen Winter <allen.winter@kdab.com> 2.1.0
  2.1.0 final
* Tue Dec 05 2023 Allen Winter <allen.winter@kdab.com> 2.0.0
  2.0.0 final
* Wed May 03 2023 Allen Winter <allen.winter@kdab.com> 1.7.0
  1.7.0 final
* Wed Sep 14 2022 Allen Winter <allen.winter@kdab.com> 1.6.0
  1.6.0 final
* Wed Nov 24 2021 Allen Winter <allen.winter@kdab.com> 1.5.0
  1.5.0 final
* Fri Jul 16 2021 Allen Winter <allen.winter@kdab.com> 1.4.0
  1.4.0 final
* Mon Jun 07 2021 Allen Winter <allen.winter@kdab.com> 1.3.1
  1.3.1 final
* Mon Feb 08 2021 Allen Winter <allen.winter@kdab.com> 1.3.0
  1.3.0 final
* Thu Dec 17 2020 Allen Winter <allen.winter@kdab.com> 1.2.0
  1.2.0 final
* Fri Dec 11 2020 Allen Winter <allen.winter@kdab.com> 1.1.1
  1.1.1 final
* Mon Oct 26 2020 Allen Winter <allen.winter@kdab.com> 1.1.0
  1.1.0 final
* Wed Sep 02 2020 Allen Winter <allen.winter@kdab.com> 1.0.0
  1.0.0 final
* Thu Aug 06 2020 Allen Winter <allen.winter@kdab.com> 0.99.9
  1.0.0 release candidate
