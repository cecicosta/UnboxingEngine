#pragma once

struct SMaterial {

    float materialSpe[4];
    float materialAmb[4];
    float materialDif[4];
    float materialEmis[4];
    int shininess;
    bool enable;
};
