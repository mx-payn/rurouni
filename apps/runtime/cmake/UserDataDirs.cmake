macro(UserDataDirs)
    if(UNIX)
        set(USER_HOME_DIR "$ENV{HOME}")
        if(DEFINED ENV{XDG_CONFIG_HOME} AND DEFINED ENV{XDG_DATA_HOME})
            set(USER_CONFIG_DIR "$ENV{XDG_CONFIG_HOME}")
            set(USER_DATA_DIR "$ENV{XDG_DATA_HOME}")
        else()
            set(USER_CONFIG_DIR "$ENV{HOME}/.config")
            set(USER_DATA_DIR "$ENV{HOME}/.local/share")
        endif()
    elseif(WIN32)
        set(USER_HOME_DIR "$ENV{HOMEPATH}")
        set(USER_CONFIG_DIR "$ENV{CSIDL_LOCAL_APPDATA}")
        set(USER_DATA_DIR "$ENV{CSIDL_LOCAL_APPDATA}")
    else()
        message(FATAL_ERROR "unsupported platform")
    endif()
endmacro()

UserDataDirs()
