# copy resources into build directory that are linked to from other SVGs.
foreach(FILE cymbals.svg folded-page.svg mu.svg musicxml-logo.svg splash-background.jpg)
  file(COPY "${FILE}" DESTINATION .)
endforeach(FILE)