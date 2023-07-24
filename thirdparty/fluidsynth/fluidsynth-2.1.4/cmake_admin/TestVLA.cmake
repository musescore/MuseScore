include ( CheckCSourceCompiles )
if ( NOT SUPPORTS_VLA )
    check_c_source_compiles ( 
        "int main(int argc, char *argv[]){int arr[argc]; return 0;}"
        _have_vla )
    if ( _have_vla )
        set ( SUPPORTS_VLA 1 )
    endif ( _have_vla )
endif ( NOT SUPPORTS_VLA )
