include ( CheckCSourceCompiles )
foreach ( _keyword "inline" "__inline__" "__inline" )
    if ( NOT INLINE_KEYWORD )
        set ( CMAKE_REQUIRED_DEFINITIONS "-DTESTKEYWORD=${_keyword}" )
        check_c_source_compiles ( 
            "typedef int foo_t;
             static TESTKEYWORD foo_t static_foo(){return 0;}
             foo_t foo(){return 0;}
             int main(int argc, char *argv[]){return 0;}"
            _have_${_keyword} )
        if ( _have_${_keyword} )
            set ( INLINE_KEYWORD ${_keyword} )
        endif ( _have_${_keyword} )
    endif ( NOT INLINE_KEYWORD )
endforeach ( _keyword )
