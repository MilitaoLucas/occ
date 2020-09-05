#pragma once
#include <string>
#include <vector>
#define ELEMENT_MAX 103

namespace craso::chem {

struct ElementData
{
    ElementData(int n, const std::string& nm, const std::string& sym, float cov, float vdw, float m) :
        atomic_number(n), name(nm), symbol(sym), cov_radius(cov), vdw_radius(vdw), mass(m) {}
    ElementData(const ElementData& o) : atomic_number(o.atomic_number), name(o.name), symbol(o.symbol),
        cov_radius(o.cov_radius), vdw_radius(o.vdw_radius), mass(o.mass) {}
    int atomic_number;
    std::string name;
    std::string symbol;
    float cov_radius;
    float vdw_radius;
    float mass;
};

static ElementData ELEMENTDATA_TABLE[ELEMENT_MAX + 1] =
{
    {0, "Dummy", "Xx", 0.0f, 0.0f, 0.0f},
    {1, "hydrogen", "H", 0.23f, 1.09f, 1.00794f},
    {2, "helium", "He", 1.50f, 1.40f, 4.002602f},
    {3, "lithium", "Li", 1.28f, 1.82f, 6.941f},
    {4, "beryllium", "Be", 0.96f, 2.00, 9.012182f},
    {5, "boron", "B", 0.83f, 2.00f, 10.811f},
    {6, "carbon", "C", 0.68f, 1.70f, 12.0107f},
    {7, "nitrogen", "N", 0.68f, 1.55f, 14.0067f},
    {8, "oxygen", "O", 0.68f, 1.52f, 15.9994f},
    {9, "fluorine", "F", 0.64f, 1.47f, 18.998403f},
    {10, "neon", "Ne", 1.50f, 1.54f, 20.1797f},
    {11, "sodium", "Na", 1.66f, 2.27f, 22.98977f},
    {12, "magnesium", "Mg", 1.41f, 1.73f, 24.305f},
    {13, "aluminium", "Al", 1.21f, 2.00f, 26.981538f},
    {14, "silicon", "Si", 1.20f, 2.10f, 28.0855f},
    {15, "phosphorus", "P", 1.05f, 1.80f, 30.973761f},
    {16, "sulfur", "S", 1.02f, 1.80f, 32.065f},
    {17, "chlorine", "Cl", 0.99f, 1.75f, 35.453f},
    {18, "argon", "Ar", 1.51f, 1.88f, 39.948f},
    {19, "potassium", "K", 2.03f, 2.75f, 39.0983f},
    {20, "calcium", "Ca", 1.76f, 2.00f, 40.078f},
    {21, "scandium", "Sc", 1.70f, 2.00f, 44.95591f},
    {22, "titanium", "Ti", 1.60f, 2.00f, 47.867f},
    {23, "vanadium", "V", 1.53f, 2.00f, 50.9415f},
    {24, "chromium", "Cr", 1.39f, 2.00f, 51.9961f},
    {25, "manganese", "Mn", 1.61f, 2.00f, 54.938049f},
    {26, "iron", "Fe", 1.52f, 2.00f, 55.845f},
    {27, "cobalt", "Co", 1.26f, 2.00f, 58.9332f},
    {28, "nickel", "Ni", 1.24f, 1.63f, 58.6934f},
    {29, "copper", "Cu", 1.32f, 1.40f, 63.546f},
    {30, "zinc", "Zn", 1.22f, 1.39f, 65.409f},
    {31, "gallium", "Ga", 1.22f, 1.87f, 69.723f},
    {32, "germanium", "Ge", 1.17f, 2.00, 72.64f},
    {33, "arsenic", "As", 1.21f, 1.85f, 74.9216f},
    {34, "selenium", "Se", 1.22f, 1.90f, 78.96f},
    {35, "bromine", "Br", 1.21f, 1.85f, 79.904f},
    {36, "krypton", "Kr", 1.50f, 2.02f, 83.798f},
    {37, "rubidium", "Rb", 2.20f, 2.00f, 85.4678f},
    {38, "strontium", "Sr", 1.95f, 2.00f, 87.62f},
    {39, "yttrium", "Y", 1.90f, 2.00f, 88.90585f},
    {40, "zirconium", "Zr", 1.75f, 2.00f, 91.224f},
    {41, "niobium", "Nb", 1.64f, 2.00f, 92.90638f},
    {42, "molybdenum", "Mo", 1.54f, 2.00f, 95.94f},
    {43, "technetium", "Tc", 1.47f, 2.00f, 98.0f},
    {44, "ruthenium", "Ru", 1.46f, 2.00f, 101.07f},
    {45, "rhodium", "Rh", 1.45f, 2.00f, 102.9055f},
    {46, "palladium", "Pd", 1.39f, 1.63f, 106.42f},
    {47, "silver", "Ag", 1.45f, 1.72f, 107.8682f},
    {48, "cadmium", "Cd", 1.44f, 1.58f, 112.411f},
    {49, "indium", "In", 1.42f, 1.93f, 114.818f},
    {50, "tin", "Sn", 1.39f, 2.17f, 118.71f},
    {51, "antimony", "Sb", 1.39f, 2.00f, 121.76f},
    {52, "tellurium", "Te", 1.47f, 2.06f, 127.6f},
    {53, "iodine", "I", 1.40f, 1.98f, 126.90447f},
    {54, "xenon", "Xe", 1.50f, 2.16f, 131.293f},
    {55, "caesium", "Cs", 2.44f, 2.00f, 132.90545f},
    {56, "barium", "Ba", 2.15f, 2.00f, 137.327f},
    {57, "lanthanum", "La", 2.07f, 2.00f, 138.9055f},
    {58, "cerium", "Ce", 2.04f, 2.00f, 140.116f},
    {59, "praseodymium", "Pr", 2.03f, 2.00f, 140.90765f},
    {60, "neodymium", "Nd", 2.01f, 2.00f, 144.24f},
    {61, "promethium", "Pm", 1.99f, 2.00f, 145.0f},
    {62, "samarium", "Sm", 1.98f, 2.00f, 150.36f},
    {63, "europium", "Eu", 1.98f, 2.00f, 151.964f},
    {64, "gadolinium", "Gd", 1.96f, 2.00f, 157.25f},
    {65, "terbium", "Tb", 1.94f, 2.00f, 158.92534f},
    {66, "dysprosium", "Dy", 1.92f, 2.00f, 162.5f},
    {67, "holmium", "Ho", 1.92f, 2.00f, 164.93032f},
    {68, "erbium", "Er", 1.89f, 2.00f, 167.259f},
    {69, "thulium", "Tm", 1.90f, 2.00f, 168.93421f},
    {70, "Ytterbium", "Yb", 1.87f, 2.00f, 173.04f},
    {71, "lutetium", "Lu", 1.87f, 2.00f, 174.967f},
    {72, "hafnium", "Hf", 1.75f, 2.00f, 178.49f},
    {73, "tantalum", "Ta", 1.70f, 2.00f, 180.9479f},
    {74, "tungsten", "W", 1.62f, 2.00f, 183.84f},
    {75, "rhenium", "Re", 1.51f, 2.00f, 186.207f},
    {76, "osmium", "Os", 1.44f, 2.00f, 190.23f},
    {77, "iridium", "Ir", 1.41f, 2.00f, 192.217f},
    {78, "platinum", "Pt", 1.36f, 1.72f, 195.078f},
    {79, "gold", "Au", 1.50f, 1.66f, 196.96655f},
    {80, "mercury", "Hg", 1.32f, 1.55f, 200.59f},
    {81, "thallium", "Tl", 1.45f, 1.96f, 204.3833f},
    {82, "lead", "Pb", 1.46f, 2.02f, 207.2f},
    {83, "bismuth", "Bi", 1.48f, 2.00f, 208.98038f},
    {84, "polonium", "Po", 1.40f, 2.00f, 290.0f},
    {85, "astatine", "At", 1.21f, 2.00f, 210.0f},
    {86, "radon", "Rn", 1.50f, 2.00f, 222.0f},
    {87, "francium", "Fr", 2.60f, 2.00f, 223.0f},
    {88, "radium", "Ra", 2.21f, 2.00f, 226.0f},
    {89, "actinium", "Ac", 2.15f, 2.00f, 227.0f},
    {90, "thorium", "Th", 2.06f, 2.00f, 232.0381f},
    {91, "protactinium", "Pa", 2.00f, 2.00f, 231.03588f},
    {92, "uranium", "U", 1.96f, 1.86f, 238.02891f},
    {93, "neptunium", "Np", 1.90f, 2.00f, 237.0f},
    {94, "plutonium", "Pu", 1.87f, 2.00f, 244.0f},
    {95, "americium", "Am", 1.80f, 2.00f, 243.0f},
    {96, "curium", "Cm", 1.69f, 2.00f, 247.0f},
    {97, "berkelium", "Bk", 1.54f, 2.00f, 247.0f},
    {98, "californium", "Cf", 1.83f, 2.00f, 251.0f},
    {99, "einsteinium", "Es", 1.50f, 2.00f, 252.0f},
    {100, "fermium", "Fm", 1.50f, 2.00f, 257.0f},
    {101, "mendelevium", "Md", 1.50f, 2.00f, 258.0f},
    {102, "nobelium", "No", 1.50f, 2.00f, 259.0f},
    {103, "lawrencium", "Lr", 1.50f, 2.00f, 262.0f}
};

class Element
{
public:
    Element() = delete;
    Element(const std::string&);
    Element(int);
    inline const std::string& symbol() const { return m_data.symbol; }
    inline const std::string& name() const { return m_data.name; }
    inline float mass() const { return m_data.mass; }
    inline float cov() const { return m_data.cov_radius; }
    inline float covalentRadius() const { return m_data.cov_radius; }
    inline float vdw() const { return m_data.vdw_radius; }
    inline float vdwRadius() const { return m_data.vdw_radius; }
    inline int atomicNumber() const { return m_data.atomic_number; }
    inline int n() const { return m_data.atomic_number; }
    bool operator<(const Element& rhs) const { return m_data.atomic_number < rhs.m_data.atomic_number; }
    bool operator>(const Element& rhs) const { return m_data.atomic_number > rhs.m_data.atomic_number; }
    bool operator==(const Element& rhs) const { return m_data.atomic_number == rhs.m_data.atomic_number; }
    bool operator!=(const Element& rhs) const { return m_data.atomic_number != rhs.m_data.atomic_number; }

private:
    ElementData m_data;
};


std::string chemical_formula(const std::vector<Element>&);

}
