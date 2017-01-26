# set variables for plist
set(MACOSX_BUNDLE_BUNDLE_NAME "ckb-next")
set(MACOSX_BUNDLE_EXECUTABLE_NAME "ckb-next")
set(MACOSX_BUNDLE_INFO_STRING "CKB Next - version ${ckb-next_VERSION}")
set(MACOSX_BUNDLE_ICON_FILE "ckb-logo.icns")
set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.next.ckb")
set(MACOSX_BUNDLE_LONG_VERSION_STRING "${ckb-next_VERSION}")
set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${ckb-next_VERSION_MAJOR}.${ckb-next_VERSION_MINOR}.${ckb-next_VERSION_PATCH}")
set(MACOSX_BUNDLE_BUNDLE_VERSION "${ckb-next_VERSION}")
set(MACOSX_BUNDLE_COPYRIGHT "Copyright © 2014-2016 ccMSC\nCopyright © 2017 ckb-next dev. team")

# generate the plist
configure_file("CkbNext.plist.in" "CkbNext.plist")
