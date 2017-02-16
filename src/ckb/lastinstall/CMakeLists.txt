if (APPLE)
    # deploy QT
    set(pfx "")
    set(sfx ".app")
    get_target_property(output_name ckb-next OUTPUT_NAME)
    if (output_name)
        set(exe "${pfx}${output_name}${sfx}")
    else ()
        set(exe "${pfx}ckb-next${sfx}")
    endif ()

    set(dirs "")
    if (CMAKE_PREFIX_PATH)
        foreach (dir ${CMAKE_PREFIX_PATH})
            list(APPEND dirs "${dir}/bin" "${dir}/lib")
        endforeach ()
    endif ()

    include(InstallRequiredSystemLibraries)
    include(DeployQt5)
    install_qt5_executable("${exe}" "" "" "${dirs}" "")

    install(CODE [[execute_process(COMMAND sudo launchctl load /Library/LaunchDaemons/ckb-next-daemon.plist)]])
elseif (LINUX)
    # ...
endif ()
