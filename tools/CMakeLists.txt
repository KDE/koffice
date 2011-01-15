macro_optional_add_subdirectory( converter )
macro_optional_add_subdirectory( thumbnail )
macro_optional_add_subdirectory( kthesaurus )
macro_optional_add_subdirectory( quickprint )
macro_optional_add_subdirectory( slidecompare )

macro_optional_find_package(Okular)
macro_log_feature(OKULAR_FOUND "Okular" "Okular ODP Plugin" "http://okular.kde.org/" FALSE "" "Required to build the Okular OpenDocument Presenter plugin")

if(OKULAR_FOUND)
    add_subdirectory( okularodpgenerator )
endif()

# strigi ships with the odfsaxanalyzer which does already fetch the meta-data of
# odf files. Also the current implementations of the analyzers are deactivated
# anyway. So, what would be needed is to connect somehow with KoStore or hook
# in the koffice-filters another way somehow.
#add_subdirectory( strigi-analyzer )
