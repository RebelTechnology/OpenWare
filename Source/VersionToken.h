#ifndef __VERSION_TOKEN_H__
#define __VERSION_TOKEN_H__

struct VersionToken {
    uint32_t magic;
    uint32_t hardware_id;
    char version[16];
};

#endif
