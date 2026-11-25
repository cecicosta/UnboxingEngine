#include "Matrix.h"
#include <cmath>

#include <iomanip>
#include <iostream>
#include <vector>


#define uniform const
#define discard \
    return {}

constexpr int SIZE = 8;

using vec4 = Matrix<double, 1, 4>;
using vec3 = Matrix<double, 1, 3>;
using vec2 = Matrix<double, 1, 2>;
using sampler2D = Matrix<vec4, SIZE>;

using namespace std;

vec4 texture(sampler2D tex, vec2 uv) {
    return tex.at(uv.at(1), uv.at(0));
}

vec2 textureSize(sampler2D text, int lod) {
    return vec2({static_cast<double>(text.cols), static_cast<double>(text.rows)});
}

vec2 operator/(const double a, const vec2 v) {
    return vec2({a / v.at(0), a / v.at(1)});
}

// 1 picometer value in meters
double pm = 1e-12;//[m]

// Pi
double PI = 3.141592;

// Elementary charge
double e = 1.602e-19;// [C]

// Nucleus Charge
double Q = e;

// Electron mass
double m = 9.10938188e-31;

// Nucleus mass
double M = 1.00783 * 1.660539e-27;

// Permittivity of free space
double E0 = 8.854e-12;// F/m [Farads per meter] [C/(V.m)] 1C charge per 1V potential difference, per 1 metter

// Atomic unit of length
double a0 = 5.29177210544e-11;//[m] Borh radius (4\pi*\empsilon_0*\hbar^2) / (e^2 * m_e);

// Calculate the constant factors of Coulomb's potential expression.
double k = 1 / (4 * PI * E0);

double g = 6.6743e-11;// [m^3/(kg.s^2)]

double C_kg = (k * Q) / (g * M);


double length(vec3 texel) {
    return texel.length();
}

vec4 positive_field_tex_render_frag(
    const sampler2D &u_texture,
    const vec4 &v_color,
    const vec2 &v_uv,
    const double u_visualization_scale,
    const double u_camera_far) {

    double maximumDistance = max(u_camera_far, 0.000001);

    vec4 accumulated = texture(u_texture, v_uv);
    // accumulated.rgb is the line-cross-section of the potential region obtained by the volumetric calculation fragment-shader pass
    double thickness = length(vec3({accumulated.at(0), accumulated.at(1), accumulated.at(2)}));
    double volumetric_opacity = (thickness / maximumDistance);
    double radius = accumulated.at(3);// Was actually using alpha for the radius accumulation

    if (volumetric_opacity < 0.00001) {
        discard;
    }

    double sqr_half_ray = (thickness * thickness) / 4;
    double sqr_radius = radius * radius;

    if (sqr_half_ray > sqr_radius) {
        vec4 o_color = v_color;
        return o_color;
    }

    // Min distance the line-cross-section reach when crossing the sphere
    double r_min = sqrt(sqr_radius - sqr_half_ray);

    // We now convert the dimensions to picometer
    double radius_pm = radius * pm;
    double r_min_pm = r_min * pm;

    double dM = (g * M) / (r_min_pm * r_min_pm);
    double dQ = -(k * Q) / (r_min_pm * r_min_pm);

    vec4 o_color = vec4({dQ, dM, 1.0 / (r_min_pm * r_min_pm), dQ / dM});

    return o_color;
}


vec4 fields_divergence_frag(
    const sampler2D &u_texture,
    const sampler2D &u_electron,
    const vec4 &v_color,
    const vec2 &v_uv,
    const double u_visualization_scale,
    const double tex_coord_parameter) {

    vec4 nucleusTex = texture(u_texture, v_uv);
    vec2 texel = tex_coord_parameter / vec2(textureSize(u_texture, 0));

    vec2 texelCoords[] = {
        v_uv - vec2({texel.at(0), 0.0}),
        v_uv + vec2({texel.at(0), 0.0}),
        v_uv - vec2({0.0, texel.at(1)}),
        v_uv + vec2({0.0, texel.at(1)})};

    vec4 cellsSum{0.0, 0.0, 0.0, 0.0};
    uint8_t cellsCount = 0;
    for (int i = 0; i < 4; i++) {
        vec2 c = texelCoords[i];
        if (texelCoords[i].at(0) >= SIZE) {
//            c.at(0) -= (SIZE - 2);
            continue;
        }
        if (texelCoords[i].at(0) < 0) {
//            c.at(0) += 2;
            continue;
        }
        if (texelCoords[i].at(1) >= SIZE) {
//            c.at(1) -= (SIZE - 2);
            continue;
        }
        if (texelCoords[i].at(1) < 0) {
//            c.at(1) += 2;
            continue;
        }

        cellsSum = cellsSum + texture(u_texture, c);
        cellsCount++;
    }

    vec4 o_color = nucleusTex + (cellsSum - (nucleusTex)*1.0/(cellsCount*5));
    return o_color;
    /*
    vec4 left, right, up, down;

    auto axials = {&left, &right, &up, &down};
    left = texture(u_texture, v_uv - vec2({texel.at(0), 0.0}));
    right = texture(u_texture, v_uv + vec2({texel.at(0), 0.0}));
    down = texture(u_texture, v_uv - vec2({0.0, texel.at(1)}));
    up = texture(u_texture, v_uv + vec2({0.0, texel.at(1)}));

    vec4 upLeft =
        texture(u_texture, v_uv + vec2({-texel.at(0), texel.at(1)}));

    vec4 upRight =
        texture(u_texture, v_uv + vec2({texel.at(0), texel.at(1)}));

    vec4 downLeft =
        texture(u_texture, v_uv + vec2({-texel.at(0), -texel.at(1)}));

    vec4 downRight =
        texture(u_texture, v_uv + vec2({texel.at(0), -texel.at(1)}));


    vec4 nCenter = nucleusTex;

    vec4 nAxial =
        left + right + up + down;

    vec4 nDiagonal =
        upLeft + upRight + downLeft + downRight;

    vec4 divergence1st =
        (nAxial * 4.0 +
         nDiagonal -
         nCenter * 20.0) *
        (1.0 / 6.0);


    // Charge density divergence plus gravity density divergence
    double effDivergence1st = k * divergence1st.at(0) + g * divergence1st.at(1);

    vec4 electronTex = texture(u_electron, v_uv);
    texel = tex_coord_parameter / vec2(textureSize(u_electron, 0));

    left = texture(u_electron, v_uv - vec2({texel.at(0), 0.0}));
    right = texture(u_electron, v_uv + vec2({texel.at(0), 0.0}));
    down = texture(u_electron, v_uv - vec2({0.0, texel.at(1)}));
    up = texture(u_electron, v_uv + vec2({0.0, texel.at(1)}));

    upLeft =
        texture(u_electron, v_uv + vec2({-texel.at(0), texel.at(1)}));

    upRight =
        texture(u_electron, v_uv + vec2({texel.at(0), texel.at(1)}));

    downLeft =
        texture(u_electron, v_uv + vec2({-texel.at(0), -texel.at(1)}));

    downRight =
        texture(u_electron, v_uv + vec2({texel.at(0), -texel.at(1)}));



    vec4 center = electronTex;

    vec4 axial =
        left + right + up + down;
    vec4 diagonal =
        upLeft + upRight + downLeft + downRight;

    double divergenceE =
        (
            4.0 * axial.at(1) +
            diagonal.at(1) -
            20.0 * center.at(1)
        ) / 6.0;

    double divergenceG =
    (
        4.0 * axial.at(2) +
        diagonal.at(2) -
        20.0 * center.at(2)
    ) / 6.0;
    double force = divergenceE + divergenceG/C_kg;

    if (force*C_kg > 0.0 && std::isinf(force*C_kg)) {
        return center + vec4({force, force*C_kg , force, 1});
    }
    // <distance vector> * <charge 1> * <charge 2> / <distance squared>, but distance squared is 1
    vec2 fELeft  = vec2({-texel.at(0), 0.0}).normalize() * left.at(0) * center.at(0);
    vec2 fERight = vec2({texel.at(0), 0.0}).normalize() * right.at(0) * center.at(0);
    vec2 fEDown  = vec2({0.0, -texel.at(1)}).normalize() * down.at(0) * center.at(0);
    vec2 fEUp    = vec2({0.0, texel.at(1)}).normalize() *   up.at(0) * center.at(0);

    vec2 FEa = fEDown + fEUp + fELeft + fERight;
    double divergenceE = (left.at(0) + right.at(0) + up.at(0) + down.at(0) - 4 * center.at(0));
    double divergenceG = (left.at(1) + right.at(1) + up.at(1) + down.at(1) - 4 * center.at(1));

    double force = divergenceE;//* center.at(0);// + divergenceG * center.at(1);

    vec4 o_color = center;

    double divergenceEEff = force;
    double divergenceGEff = force;

    o_color =
        center + vec4({divergenceEEff, divergenceGEff, 1, 1});

    return o_color;
*/
}
// From Joel Sjögren from Stack Overflow [https://stackoverflow.com/questions/2616906/how-do-i-output-coloured-text-to-a-linux-terminal]
#include <ostream>
namespace Color {
enum Code {
    FG_RED      = 31,
    FG_GREEN    = 32,
    FG_BLUE     = 34,
    FG_DEFAULT  = 39,
    FG_YELLOW   = 33,
    FG_WHITE    = 37,
    BG_RED      = 41,
    BG_GREEN    = 42,
    BG_BLUE     = 44,
    BG_DEFAULT  = 49
};
class Modifier {
    Code code;
public:
    Modifier(Code pCode) : code(pCode) {}
    friend std::ostream&
    operator<<(std::ostream& os, const Modifier& mod) {
        return os << "\033[" << mod.code << "m";
    }
};
}

void prettyPrintMatrix(const sampler2D &matrix, uint32_t channel) {
    constexpr int width = 12;

    Color::Modifier red(Color::FG_RED);
    Color::Modifier gree(Color::FG_GREEN);
    Color::Modifier blue(Color::FG_BLUE);
    Color::Modifier white(Color::FG_WHITE);
    Color::Modifier yellow(Color::FG_YELLOW);
    Color::Modifier def(Color::FG_DEFAULT);
    Color::Modifier current = def;

    for (int x = 0; x < matrix.cols; ++x) {
        for (int y = 0; y < matrix.rows; ++y) {
            auto value = matrix.at(y, x);

            if (value.at(channel) == 0) {
                current = def;
            } else if (value.at(channel) > 0) {
                current = red;
            } else if (value.at(channel) < 0) {
                current = blue;
            } else if (std::isnan(value.at(channel))) {
                current = yellow;
            } else if (std::isinf(value.at(channel))) {
                current = white;
            }

            std::cout << current << std::setw(width) << value.ToArray()[channel];

        }

        std::cout << '\n';
    }
}

#include <cmath>


void initializeSphere(sampler2D &matrix) {
    constexpr double centerX = SIZE / 2.0;
    constexpr double centerY = SIZE / 2.0;
    constexpr double radius = 5.0;

    const double radiusSquared = radius * radius;

    for (int y = 0; y < SIZE; ++y) {
        for (int x = 0; x < SIZE; ++x) {
            double dx = static_cast<double>(x) - centerX;
            double dy = static_cast<double>(y) - centerY;

            double distanceSquared = dx * dx + dy * dy;

            double z = 0.0f;

            if (distanceSquared <= radiusSquared) {
                z = 2.0f * std::sqrt(radiusSquared - distanceSquared);
            }

            matrix.at(y, x) = vec4({static_cast<double>(0),
                                    static_cast<double>(0),
                                    z,
                                    radius});
        }
    }
}


void InitializeWith(sampler2D &matrix, const double radius, const double valuet) {
    constexpr double centerX = SIZE/2.0;
    constexpr double centerY = SIZE/2.0;

    const double radiusSquared = radius * radius;

    for (int y = 0; y < SIZE; ++y) {
        for (int x = 0; x < SIZE; ++x) {
            double dx = static_cast<double>(x) - centerX;
            double dy = static_cast<double>(y) - centerY;

            double distanceSquared = dx * dx + dy * dy;

            double z = 0.0f;

            if (distanceSquared <= radiusSquared) {
                matrix.at(y, x) = vec4({valuet, valuet, valuet, valuet});
            }
        }
    }
}


double TotalSum(sampler2D &matrix, uint32_t channel) {
    double sum = 0;
    for (int y = 0; y < SIZE; ++y) {
        for (int x = 0; x < SIZE; ++x) {
            sum += matrix.at(y, x).at(channel);
        }
    }
    return sum;
}

int main() {
    sampler2D u_texture;
    sampler2D u_electron;
    sampler2D u_next;
    double u_visualization_scale = 1;
    vec4 v_color({1, 1, 1, 1});
    vec2 v_uv;
    vec4 o_color;

    //InitializeWith(u_texture, 1, 4);

    constexpr double centerX = SIZE/2.0;
    constexpr double centerY = SIZE/2.0;
    u_texture.at(centerY, centerX) = vec4({5, 0, 0, 1});

    std::cout << "Total sum:" << TotalSum(u_texture, 0) << std::endl;
    prettyPrintMatrix(u_texture, 0);

    while (true) {
        for (double x = 0; x < u_texture.cols; ++x) {
            for (double y = 0; y < u_texture.rows; ++y) {
                u_next.at(x, y) = fields_divergence_frag(u_texture, u_texture, v_color, vec2({x, y}), u_visualization_scale, SIZE);
            }
        }
        u_texture = u_next;

        std::cout << "Total sum:" << TotalSum(u_next, 0) << std::endl;
        prettyPrintMatrix(u_next, 0);
        std::cin.get();

    }
    /*
    initializeSphere(u_electron);
    prettyPrintMatrix(u_electron, 2);

    auto frag = positive_field_tex_render_frag(u_electron, v_color, vec2({SIZE/2.0, SIZE/2.0 + 3}), u_visualization_scale, 1);


    for (double x = 0; x < u_electron.cols; ++x) {
        for (double y = 0; y < u_electron.rows; ++y) {
            u_texture.at(y,x) = positive_field_tex_render_frag(u_electron, v_color, vec2({y, x}), u_visualization_scale, 1);
        }
    }
    std::cout << "Charge per cell" << std::endl;
    prettyPrintMatrix(u_texture, 0);
    std::cout << "Mass per cell" << std::endl;
    prettyPrintMatrix(u_texture, 1);
    std::cout << "Units per cell" << std::endl;
    prettyPrintMatrix(u_texture, 2);
    std::cout << "Charge per mass per cell" << std::endl;
    prettyPrintMatrix(u_texture, 3);

    frag = fields_divergence_frag(u_electron, u_texture, v_color, vec2({SIZE/2.0, SIZE/2.0 + 3}), u_visualization_scale, SIZE);

    for (double x = 0; x < u_electron.cols; ++x) {
        for (double y = 0; y < u_electron.rows; ++y) {
            u_next.at(x, y) = fields_divergence_frag(u_electron, u_texture, v_color, vec2({x, y}), u_visualization_scale, SIZE);
        }
    }

    std::cout << "Divergence E" << std::endl;
    prettyPrintMatrix(u_next, 0);

    while (1) {
        sampler2D next;
        for (double x = 0; x < u_electron.cols; ++x) {
            for (double y = 0; y < u_electron.rows; ++y) {
                u_next.at(x, y) = fields_divergence_frag(u_next, u_next, v_color, vec2({x, y}), u_visualization_scale, SIZE);
            }
            std::cout << "Divergence E" << std::endl;
            prettyPrintMatrix(u_next, 1);
        }
        std::cin.get();
    }

    //auto frag = texture(u_electron, vec2(SIZE/2.0f, SIZE/2.0f));
    //std::cout << "(" << frag.x << "," << frag.y << "," << frag.z << ","<< frag.w << ")" << std::endl;
*/
    return 0;
}
