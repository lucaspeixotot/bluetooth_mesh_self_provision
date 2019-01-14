#include "ic_version.h"

#include "firmware_version.h"

static const ic_version_t __version = {VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD};

const ic_version_t *ic_version_instance()
{
    return &__version;
}

u8_t ic_version_get_major()
{
    return __version.major;
}

u8_t ic_version_get_minor()
{
    return __version.minor;
}
u16_t ic_version_get_build()
{
    return __version.build;
}