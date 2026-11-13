# Export selection — localization handoff

## Repository workflow

All new user-facing strings are declared with MuseScore's translation APIs:

- QML labels and accessible names use `qsTrc("project/export", ...)`;
- the menu action uses `TranslatableString("action", ...)`.

MuseScore's `tools/translations/run_lupdate.sh` updates the English source
catalogue. Localized `.ts` files are supplied by Transifex, so they should not
be edited manually in the feature pull request. This keeps generated catalogue
changes out of the code review while making every new string discoverable by
the normal translation pipeline.

The table below is a translation seed for the initial review and Transifex
handoff. Translators remain authoritative and may adjust terminology to match
the established MuseScore glossary.

## Translation seed

| English source | Spanish | Catalan | German | Italian | French |
|---|---|---|---|---|---|
| Export &selection… | Exportar &selección… | Exporta la &selecció… | &Auswahl exportieren… | Esporta &selezione… | Exporter la &sélection… |
| Export selection | Exportar selección | Exporta la selecció | Auswahl exportieren | Esporta selezione | Exporter la sélection |
| Only the selected range will be exported. | Solo se exportará el intervalo seleccionado. | Només s’exportarà l’interval seleccionat. | Nur der ausgewählte Bereich wird exportiert. | Verrà esportato solo l’intervallo selezionato. | Seule la plage sélectionnée sera exportée. |
| Tempo: | Tempo: | Tempo: | Tempo: | Tempo: | Tempo : |
| Export tempo | Tempo de exportación | Tempo de l’exportació | Exporttempo | Tempo di esportazione | Tempo d’exportation |
| Metronome | Metrónomo | Metrònom | Metronom | Metronomo | Métronome |
| Export metronome | Metrónomo de exportación | Metrònom de l’exportació | Exportmetronom | Metronomo di esportazione | Métronome d’exportation |
| Fade in | Fundido de entrada | Fosa d’entrada | Einblenden | Dissolvenza in entrata | Fondu entrant |
| Fade-in duration | Duración del fundido de entrada | Durada de la fosa d’entrada | Einblenddauer | Durata della dissolvenza in entrata | Durée du fondu entrant |
| Fade out | Fundido de salida | Fosa de sortida | Ausblenden | Dissolvenza in uscita | Fondu sortant |
| Fade-out duration | Duración del fundido de salida | Durada de la fosa de sortida | Ausblenddauer | Durata della dissolvenza in uscita | Durée du fondu sortant |
| Mute other instruments | Silenciar los demás instrumentos | Silencia la resta d’instruments | Andere Instrumente stummschalten | Disattiva gli altri strumenti | Couper les autres instruments |
| Other instruments: | Otros instrumentos: | Altres instruments: | Andere Instrumente: | Altri strumenti: | Autres instruments : |
| Other instruments volume | Volumen de los demás instrumentos | Volum de la resta d’instruments | Lautstärke der anderen Instrumente | Volume degli altri strumenti | Volume des autres instruments |
| Selected instrument levels | Niveles de los instrumentos seleccionados | Nivells dels instruments seleccionats | Pegel der ausgewählten Instrumente | Livelli degli strumenti selezionati | Niveaux des instruments sélectionnés |
| Individual other-instrument levels | Niveles individuales de los demás instrumentos | Nivells individuals de la resta d’instruments | Einzelpegel der anderen Instrumente | Livelli individuali degli altri strumenti | Niveaux individuels des autres instruments |
| %1 volume | Volumen de %1 | Volum de %1 | Lautstärke von %1 | Volume di %1 | Volume de %1 |

## Localization checks

Before submission:

1. Run the project's `lupdate` command against the completed source tree.
2. Confirm that every row above is present in the English source catalogue in
   the expected context.
3. Run the placeholder-translation mode and inspect the dialog at 100%, 125%,
   and 150% display scaling to expose hard-coded or clipped strings.
4. After Transifex synchronization, smoke-test Spanish, Catalan, German,
   Italian, and French. Check accelerator collisions for the menu action.
