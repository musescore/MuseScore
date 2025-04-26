declare_module(iex_lyrics)
set(MODULE_SRC exportlrc.cpp exportlrc.h)
set(MODULE_LINK
    engraving
    project
    notation
)
setup_module()
