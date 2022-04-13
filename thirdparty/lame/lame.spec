%define name lame
%define ver 3.100
%define rel 1
%define prefix %{_usr}
%define docdir %{_defaultdocdir}

Summary : LAME Ain't an MP3 Encoder... but it's the best.
Summary(fr) : LAME n'est pas un encodeur MP3 ;->
Name: %{name}
Version: %{ver}
Release: %{rel}
License: LGPL
Vendor: The LAME Project
Packager: Yosi Markovich <yosim@bigfoot.com>
URL: http://www.mp3dev.org
Group: Applications/Multimedia
Source: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-root
Requires: ncurses >= 5.0
BuildRequires: gcc => 3.0.1, /usr/bin/find, ncurses-devel
%ifarch %{ix86} x86_64
BuildRequires: nasm
%endif
Provides: mp3encoder

%description
LAME is an educational tool to be used for learning about MP3 encoding.  The
goal of the LAME project is to use the open source model to improve the
psycho acoustics, noise shaping and speed of MP3. 

%description -l fr
LAME est un outil d'enseignement pour l'apprentissage de l'encodage MP3.
Le but du projet LAME est d'utiliser un mod�le "open source" afin
d'am�liorer la qualit� et la vitesse du MP3. 



%package devel
Summary: Shared and static libraries for LAME.
Group: Development/Libraries
Requires: %{name} = %{version}

%description devel
LAME is an educational tool to be used for learning about MP3 encoding.
This package contains both the shared and the static libraries from the
LAME project.

You will also need to install the main lame package in order to install
these libraries.

%prep
%setup

%build

# Vorbis makes the build fail for now. . .
rm -f config.cache
 
%configure \
%ifarch %{ix86} x86_64
	--enable-nasm \
%endif
	--enable-decoder \
	--without-vorbis \
	--enable-analyzer=no \
	--enable-brhist \
	--disable-debug
%{__make} %{?_smp_mflags} test CFLAGS="%{optflags}"

%install
%{__rm} -rf %{buildroot}
%makeinstall

### Some apps still expect to find <lame.h>
%{__ln_s} -f lame/lame.h %{buildroot}%{_includedir}/lame.h


find doc/html -name "Makefile*" | xargs rm -f
### make install really shouldn't install these
%{__rm} -rf %{buildroot}%{_docdir}/lame/


%post
/sbin/ldconfig 2>/dev/null

%postun
/sbin/ldconfig 2>/dev/null

%clean
%{__rm} -rf %{buildroot}

%files
%defattr (-,root,root)
%doc COPYING ChangeLog README TODO USAGE doc/html
%doc doc/html
%{_bindir}/lame
%{_libdir}/libmp3lame.so.*
%{_mandir}/man1/lame.1*

%files devel
%defattr (-, root, root)
%doc API HACKING STYLEGUIDE
%{_libdir}/libmp3lame.a
%{_libdir}/libmp3lame.la
%{_libdir}/libmp3lame.so
%{_includedir}/*

%changelog

* Sun May 14 2006 Kyle VanderBeek <kylev@kylev.com>
- Remove requirements for specific gcc versions, since modern ones "just work".
- Remove out-dated hyper-optimizations (some of which weren't valid compiler
  flags anymore).
- Update to current RPM techniques and macros (inspired by freshrpms.net spec).

* Sat May 11 2002 Yosi Markovich <yosim@bigfoot.com>
- Fixes to the spec file that include:
- Making sure the compiler is gcc version 3.0.1. Lame compiled with a version
  greater than 3.0.1 is broken.
- Optimization flags for i686 will use i686 for march and mcpu, and not 
  athlon.
- Fix the dates in this Changelog section.
- Various small fixes merged from Matthias Saou.
- Thanks Fred Maciel <fred-m@crl.hitachi.co.jp> for his useful comments.

- 
* Tue Jan 22 2002 Mark Taylor <mt@mp3dev.org>
- replaced lame.spec.in with Yosi's version.  Merged some stuff from
  the prevous lame.spec.in file, and appended changelog below.

* Tue Jan 22 2002 Yosi Markovich <yosim@bigfoot.com>
- Rewrote lame.spec.in to create a correct and nice spec file.
  imho, this spec file is not good for anyone who wants to build
  daily cvs snapshots. Closes bug #495975

* Tue Dec 11 2001 Yosi Markovich <yosim@bigfoot.com>
- Shamelessly stole Matthias Saou's excellent spec file to create daily
  CVS snapshots of Lame. Other than that, nothing is new.

* Tue Oct 23 2001 Matthias Saou <matthias.saou@est.une.marmotte.net>
- Fixed the %pre and %post that should have been %post and %postun, silly me!
- Removed -malign-double (it's evil, Yosi told me and I tested, brrr ;-)).
- Now build with gcc3, VBR encoding gets a hell of a boost, impressive!
  I recommend you now use "lame --r3mix", it's the best.
- Tried to re-enable vorbis, but it's a no-go.

* Thu Jul 26 2001 Matthias Saou <matthias.saou@est.une.marmotte.net>
- Build with kgcc to have VBR working.

* Wed Jul 25 2001 Matthias Saou <matthias.saou@est.une.marmotte.net>
- Update to 3.89beta : Must be built with a non-patched version of nasm
  to work!

* Mon May  7 2001 Matthias Saou <matthias.saou@est.une.marmotte.net>
- Rebuilt for Red Hat 7.1.
- Disabled the vorbis support since it fails to build with it.
- Added a big optimisation section, thanks to Yosi Markovich
  <senna@camelot.com> for this and other pointers.

* Sun Feb 11 2001 Matthias Saou <matthias.saou@est.une.marmotte.net>
- Split the package, there is now a -devel

* Thu Nov 26 2000 Matthias Saou <matthias.saou@est.une.marmotte.net>
- Initial RPM release for RedHat 7.0 from scratch

* Wed Nov 21 2000 Eric Lassauge <lassauge@mail.dotcom.fr>
- Updated and corrected RPM to 3.89beta.
- Added french translations

* Sat Aug 04 2000 Markus Linnala �maage@cs.tut.fi�
- Build and include docs and libs correctly
- Build extra programs

* Tue Aug 01 2000 Stuart Young �cefiar1@optushome.com.au�
- Updated RPM to 3.85beta.
- Modified spec file (merged George and Keitaro's specs)
- Added reasonable info to the specs to reflect the maintainer
- Renamed lame.spec (plain spec is bad, mmkay?).

* Fri Jun 30 2000 Keitaro Yosimura �ramsy@linux.or.jp�
- Updated RPM to 3.84alpha.
- Better attempt at an RPM, independant of 3.83 release.
- (This is all surmise as there was no changelog).

* Thu May 30 2000 Georges Seguin �crow@planete.net� 
- First RPM build around 3.83beta


