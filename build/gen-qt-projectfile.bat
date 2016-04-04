@echo off

set OLD_DIR=%CD%

echo TRANSLATIONS = \
echo       %1/share/locale/mscore_en.ts \
echo       %1/share/locale/mscore_af.ts \
echo       %1/share/locale/mscore_ar.ts \
echo       %1/share/locale/mscore_ast.ts \
echo       %1/share/locale/mscore_be.ts \
echo       %1/share/locale/mscore_bg.ts \
echo       %1/share/locale/mscore_ca.ts \
echo       %1/share/locale/mscore_ca@valencia.ts \
echo       %1/share/locale/mscore_cs.ts \
echo       %1/share/locale/mscore_cy.ts \
echo       %1/share/locale/mscore_da.ts \
echo       %1/share/locale/mscore_de.ts \
echo       %1/share/locale/mscore_el.ts \
echo       %1/share/locale/mscore_en_GB.ts \
echo       %1/share/locale/mscore_en_US.ts \
echo       %1/share/locale/mscore_eo.ts \
echo       %1/share/locale/mscore_es.ts \
echo       %1/share/locale/mscore_et.ts \
echo       %1/share/locale/mscore_eu.ts \
echo       %1/share/locale/mscore_fa.ts \
echo       %1/share/locale/mscore_fi.ts \
echo       %1/share/locale/mscore_fo.ts \
echo       %1/share/locale/mscore_fr.ts \
echo       %1/share/locale/mscore_ga.ts \
echo       %1/share/locale/mscore_gl.ts \
echo       %1/share/locale/mscore_he.ts \
echo       %1/share/locale/mscore_hi_IN.ts \
echo       %1/share/locale/mscore_hr.ts \
echo       %1/share/locale/mscore_hu.ts \
echo       %1/share/locale/mscore_id.ts \
echo       %1/share/locale/mscore_it.ts \
echo       %1/share/locale/mscore_ja.ts \
echo       %1/share/locale/mscore_ka.ts \
echo       %1/share/locale/mscore_ko.ts \
echo       %1/share/locale/mscore_lt.ts \
echo       %1/share/locale/mscore_lv.ts \
echo       %1/share/locale/mscore_mn_MN.ts \
echo       %1/share/locale/mscore_nb.ts \
echo       %1/share/locale/mscore_nl.ts \
echo       %1/share/locale/mscore_nn.ts \
echo       %1/share/locale/mscore_pl.ts \
echo       %1/share/locale/mscore_pt_BR.ts \
echo       %1/share/locale/mscore_pt.ts \
echo       %1/share/locale/mscore_ro.ts \
echo       %1/share/locale/mscore_ru.ts \
echo       %1/share/locale/mscore_sk.ts \
echo       %1/share/locale/mscore_sl.ts \
echo       %1/share/locale/mscore_sr.ts \
echo       %1/share/locale/mscore_sr_RS.ts \
echo       %1/share/locale/mscore_sv.ts \
echo       %1/share/locale/mscore_th.ts \
echo       %1/share/locale/mscore_tr.ts \
echo       %1/share/locale/mscore_uk.ts \
echo       %1/share/locale/mscore_uz@Latn.ts \
echo       %1/share/locale/mscore_vi.ts \
echo       %1/share/locale/mscore_zh_CN.ts \
echo       %1/share/locale/mscore_zh_TW.ts

cd /d %1

echo FORMS = \
for /r %1 %%a in (*.ui) do echo     %%a \
echo.

echo SOURCES = \
for /r %1 %%a in (*.cpp) do echo     %%a \
echo.
echo.

cd /d %OLD_DIR%
